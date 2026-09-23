$ErrorActionPreference = 'Stop'

$Repo = 'C:\Users\vlbra\GitHub\TextGame'
$Project = "$Repo\game\TextGame.uproject"

$Build = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat'
$Editor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$Map = '/Game/Maps/PhysicsLab.umap'

if (-not (Test-Path -LiteralPath $Build))
{
    throw "Build.bat not found: $Build"
}

if (-not (Test-Path -LiteralPath $Editor))
{
    throw "Unreal Editor not found: $Editor"
}

if (-not (Test-Path -LiteralPath $Project))
{
    throw "TextGame project not found: $Project"
}

Set-Location $Repo

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TextGame - Physics Lab Build" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

& $Build TextGameEditor Win64 Development "-Project=$Project" -WaitMutex -Progress

$BuildExit = $LASTEXITCODE

Write-Host ""
Write-Host "BUILD EXIT CODE: $BuildExit" -ForegroundColor Cyan
Write-Host ""

if ($BuildExit -ne 0)
{
    Write-Host "BUILD FAILED. Unreal Editor will not be started." -ForegroundColor Red
    Write-Host ""
    Read-Host "Press Enter to close"
    exit $BuildExit
}

Write-Host "BUILD SUCCEEDED. Launching Physics Lab..." -ForegroundColor Green

Start-Process `
    -FilePath $Editor `
    -ArgumentList @(
        "`"$Project`""
        $Map
    )

exit 0
