#requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [Parameter(Mandatory = $true)][string]$EngineRoot,
    [Parameter(Mandatory = $true)][string]$Level,
    [Parameter(Mandatory = $true)][string]$Asset,
    [ValidateSet('Plan', 'Apply')][string]$Mode = 'Plan',
    [double]$NewX = 0,
    [double]$NewY = 0,
    [double]$NewZ = 300
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Assert-EditorsClosed {
    $Editors = @(Get-Process -Name 'UnrealEditor', 'UnrealEditor-Cmd' -ErrorAction SilentlyContinue)
    if ($Editors.Count -gt 0) {
        throw 'Close Unreal Editor normally before running this tool. No process will be killed.'
    }
}

try {
    $Project = (Resolve-Path -LiteralPath $Project).ProviderPath
    $EngineRoot = (Resolve-Path -LiteralPath $EngineRoot).ProviderPath
    $ProjectDirectory = Split-Path -Parent $Project
    $Worker = Join-Path $PSScriptRoot 'physics_model.py'
    $Editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    $BuildVersion = Join-Path $EngineRoot 'Engine\Build\Build.version'

    foreach ($Path in @($Project, $Editor, $Worker, $BuildVersion)) {
        if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
            throw "File not found: $Path"
        }
        if ($Path.Contains('"')) {
            throw 'Double quotes are not permitted in process paths.'
        }
    }

    if ([IO.Path]::GetExtension($Project) -ne '.uproject') {
        throw 'Project must be a .uproject file.'
    }

    if ($Level -notmatch '^/Game/[A-Za-z0-9_/]+$' -or $Level.Contains('//')) {
        throw 'Use a /Game/... Level path without .umap.'
    }

    $Descriptor = Get-Content -LiteralPath $Project -Raw | ConvertFrom-Json
    $EngineVersion = Get-Content -LiteralPath $BuildVersion -Raw | ConvertFrom-Json

    if ($EngineVersion.MajorVersion -ne 5 -or $EngineVersion.MinorVersion -ne 8) {
        throw 'This initial version targets UE 5.8.'
    }

    if ($Descriptor.PSObject.Properties.Name -contains 'EngineAssociation') {
        $Association = [string]$Descriptor.EngineAssociation
        if ($Association -match '^\d+\.\d+' -and $Association -notmatch '^5\.8(?:\.|$)') {
            throw "Project engine does not match UE 5.8: $Association"
        }
    }

    Assert-EditorsClosed

    $MapRelative = $Level.Substring(6).Replace('/', [IO.Path]::DirectorySeparatorChar)
    $MapFile = Join-Path $ProjectDirectory ('Content\' + $MapRelative + '.umap')

    if (-not (Test-Path -LiteralPath $MapFile -PathType Leaf)) {
        throw "Saved Level not found: $MapFile"
    }

    foreach ($Kind in @('__ExternalActors__', '__ExternalObjects__')) {
        $External = Join-Path $ProjectDirectory ('Content\' + $Kind + '\' + $MapRelative)
        if (Test-Path -LiteralPath $External) {
            throw 'This version requires a plain Level without external actor packages.'
        }
    }

    $RunId = [Guid]::NewGuid().ToString('N')
    $RunDirectory = Join-Path $ProjectDirectory ('Saved\PhysicsModelTool\' + $RunId)
    New-Item -ItemType Directory -Path $RunDirectory | Out-Null
    $Utf8 = [System.Text.UTF8Encoding]::new($false)
    $OldRequest = [Environment]::GetEnvironmentVariable('UE_PHYSICS_MODEL_REQUEST', 'Process')

    function Invoke-Worker([string]$Stage, [string]$ExpectedReport = '') {
        Assert-EditorsClosed

        $RequestFile = Join-Path $RunDirectory ($Stage + '.request.json')
        $ReportFile = Join-Path $RunDirectory ($Stage + '.report.json')
        $LogFile = Join-Path $RunDirectory ($Stage + '.unreal.log')

        $Request = [ordered]@{
            run_id = $RunId
            mode = $Stage
            level = $Level
            asset = $Asset
            new_location = @($NewX, $NewY, $NewZ)
            report = $ReportFile
            expected_report = $ExpectedReport
        }

        [IO.File]::WriteAllText(
            $RequestFile,
            ($Request | ConvertTo-Json -Depth 12),
            $Utf8
        )
        [Environment]::SetEnvironmentVariable('UE_PHYSICS_MODEL_REQUEST', $RequestFile, 'Process')

        $EditorArguments = @(
            ('"{0}"' -f $Project),
            ('-ExecutePythonScript="{0}"' -f $Worker),
            ('-abslog="{0}"' -f $LogFile),
            '-unattended',
            '-nop4',
            '-nosplash',
            '-nosound',
            '-stdout',
            '-FullStdOutLogOutput'
        )

        Write-Host "`n=== $($Stage.ToUpperInvariant()) ==="
        Write-Host "Log: $LogFile"
        Write-Host 'Wait for Unreal to finish.'

        $StartOptions = @{
            FilePath = $Editor
            ArgumentList = $EditorArguments
            NoNewWindow = $true
            Wait = $true
            PassThru = $true
            RedirectStandardOutput = (Join-Path $RunDirectory ($Stage + '.stdout.txt'))
            RedirectStandardError = (Join-Path $RunDirectory ($Stage + '.stderr.txt'))
        }

        $Process = Start-Process @StartOptions

        if (-not (Test-Path -LiteralPath $ReportFile)) {
            throw "No report was produced. Exit code: $($Process.ExitCode). Inspect: $LogFile"
        }

        $Report = Get-Content -LiteralPath $ReportFile -Raw -Encoding UTF8 | ConvertFrom-Json

        if ($Report.run_id -ne $RunId) {
            throw 'Report belongs to another run.'
        }
        if ($Report.status -eq 'FAILED') {
            throw "$($Report.error) -- Report: $ReportFile"
        }
        if ($Process.ExitCode -ne 0) {
            throw "Unreal exited with $($Process.ExitCode). Inspect: $LogFile"
        }

        Write-Host "Result: $($Report.status)"
        Write-Host "Report: $ReportFile"
        return $Report
    }

    try {
        Write-Host '=== PHYSICS MODEL TOOL ==='
        Write-Host "Mode: $Mode"
        Write-Host "Asset: $Asset"

        $BeforeHash = (Get-FileHash -LiteralPath $MapFile -Algorithm SHA256).Hash
        $Plan = Invoke-Worker 'plan'
        $AfterHash = (Get-FileHash -LiteralPath $MapFile -Algorithm SHA256).Hash

        if ($BeforeHash -ne $AfterHash) {
            throw 'Level file changed during Plan. Stop and inspect.'
        }

        Write-Host "Existing matching instances: $($Plan.matched_instances)"
        Write-Host "Would create: $($Plan.would_create)"
        Write-Host "Would configure existing: $($Plan.would_configure)"

        foreach ($Instance in @($Plan.instances)) {
            Write-Host ("  {0}: {1}" -f $Instance.label, $Instance.action)
            if ($Instance.collision_source.kind -eq 'SkeletalMesh') {
                Write-Host ("    Physics Asset: {0}" -f $Instance.collision_source.physics_asset)
            }
        }

        if ($Mode -eq 'Plan') {
            Write-Host "`nPLAN COMPLETE. No model changes or Level saves."
        }
        else {
            $BackupDirectory = Join-Path $RunDirectory 'backup'
            New-Item -ItemType Directory -Path $BackupDirectory | Out-Null

            foreach ($Extension in @('.umap', '.uexp', '.ubulk')) {
                $Source = [IO.Path]::ChangeExtension($MapFile, $Extension)
                if (Test-Path -LiteralPath $Source) {
                    Copy-Item -LiteralPath $Source -Destination $BackupDirectory
                }
            }

            Write-Host "Level backup: $BackupDirectory"

            $Applied = Invoke-Worker 'apply'
            Write-Host "Created: $($Applied.created_instances)"
            Write-Host "Configured: $($Applied.changed_instances)"

            $Verified = Invoke-Worker 'verify' (Join-Path $RunDirectory 'apply.report.json')

            Write-Host "`nPERSISTENCE VERIFIED IN A FRESH UNREAL PROCESS."
            Write-Host 'Gameplay verification is still required. No commit or push.'
        }
    }
    finally {
        [Environment]::SetEnvironmentVariable('UE_PHYSICS_MODEL_REQUEST', $OldRequest, 'Process')
    }

    exit 0
}
catch {
    Write-Host "`nSTOP: $($_.Exception.Message)"
    Write-Host 'No automatic deletion or restoration is attempted.'
    exit 1
}
