$ErrorActionPreference = 'Stop'

$Repo = 'C:\Users\vlbra\GitHub\TextGame'
$Project = "$Repo\game\TextGame.uproject"
$Editor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$Map = '/Game/Maps/PhysicsLab.umap'

if (-not (Test-Path $Editor))
{
    throw "Unreal Editor not found: $Editor"
}

if (-not (Test-Path $Project))
{
    throw "TextGame project not found: $Project"
}

Start-Process `
    -FilePath $Editor `
    -ArgumentList "`"$Project`" $Map"
