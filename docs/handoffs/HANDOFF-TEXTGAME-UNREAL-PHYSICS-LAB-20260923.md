# HANDOFF — TextGame / Unreal Engine 5.8 / Physics Lab

**Date:** 2026-09-23  
**Repository:** `XThoughtLabs/TextGame`  
**Canonical local checkout:** `C:\Users\vlbra\GitHub\TextGame`  
**Current active branch:** `lab/physics-cube`  
**Current user intent:** continue code-first Unreal Engine work in a fresh chat without redoing setup or losing context.

## 1. Project direction

TextGame is the implementation repository for the future Blood of Chaos 3D game.

Current working direction:
- Unreal Engine 5.8 is the primary game/world runtime.
- Code-first development. Core gameplay logic is C++.
- Blueprint is not the default workflow.
- Unreal Editor is used for runtime inspection, scene verification, asset work, and visual validation.
- GitHub is the canonical remote.
- Research-before-implementation remains the rule for major architecture subsystems.
- The first Unreal exercise is a Physics Lab, not production gameplay.

Existing architecture baseline:
`docs/architecture/INITIAL-HYPOTHETICAL-ARCHITECTURE-PLAN.20260922.md`

Existing environment workmap:
`docs/workmaps/UNREAL-DEVELOPMENT-ENVIRONMENT-WORKMAP.20260923.md`

## 2. Completed environment setup

Stages 1–9 of the Unreal environment workmap are completed and verified.

### Stage 1 — Environment verification
Verified:
- Git
- GitHub remote
- Git LFS 3.7.1
- Unreal Engine 5.8
- UnrealEditor.exe
- UnrealBuildTool
- Build.bat
- Visual Studio Community 2026
- C++ toolchain
- Windows SDK 10.0.26100.0

UE path:
`C:\Program Files\Epic Games\UE_5.8`

VS path:
`C:\Program Files\Microsoft Visual Studio\18\Community`

### Stage 2 — setup branch
Created and used:
`setup/unreal-environment`

### Stage 3 — Git/LFS
Repository has:
- `.gitignore`
- `.gitattributes`

Git LFS tracks:
- `*.uasset`
- `*.umap`
- `*.fbx`
- `*.glb`
- `*.gltf`
- `*.blend`

Generated Unreal/VS artifacts are ignored, including:
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- `.vs/`
- `*.sln`
- `*.slnx`

### Stage 4 — repo structure
Current top-level structure:
```text
TextGame/
├── game/
├── services/
├── database/
├── tools/
├── tests/
├── docs/
└── WorkStuff/
```

### Stage 5 — C++ Unreal project
Real C++ project created under:
`game/`

Important files:
```text
game/
├── TextGame.uproject
├── Config/
├── Source/
│   ├── TextGame.Target.cs
│   ├── TextGameEditor.Target.cs
│   └── TextGame/
│       ├── TextGame.Build.cs
│       ├── Public/TextGame.h
│       └── Private/TextGame.cpp
```

### Stage 6 — VS project generation
UnrealBuildTool successfully generated:
- `game/TextGame.sln`
- `game/TextGame.slnx`

### Stage 7 — first real build
Build command pattern:
```powershell
$Build = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat'
$Project = 'C:\Users\vlbra\GitHub\TextGame\game\TextGame.uproject'

& $Build TextGameEditor Win64 Development "-Project=$Project" -WaitMutex -Progress
```

Verified:
`EXIT CODE = 0`

Built module:
`UnrealEditor-TextGame.dll`

### Stage 8 — editor launch/relaunch
Project opens correctly with:
```powershell
$Editor  = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$Project = 'C:\Users\vlbra\GitHub\TextGame\game\TextGame.uproject'

Start-Process $Editor -ArgumentList "`"$Project`""
```

No missing module/build-version errors.

### Stage 9 — C++ smoke test
`TextGame.cpp` was changed so module StartupModule writes:
`TextGame C++ smoke test: module loaded successfully.`

Verified in:
`game/Saved/Logs/TextGame.log`

Result confirmed full pipeline:
C++ source -> UBT -> MSVC -> DLL -> Unreal Editor -> runtime execution.

## 3. Git history / merge state

Setup work was merged through PR #2.

PR:
`Set up Unreal Engine C++ development environment`

Merge commit:
`8a5156736c47be346909b65d94c37efe1359f9dd`

Then branch created for Stage 10:
`lab/physics-cube`

At branch creation:
`lab/physics-cube` and `main` both pointed to `8a51567`.

## 4. Stage 10 — Physics Lab

Goal from workmap:
- small 3D environment
- physical cube
- gravity
- collision
- mass
- mouse grab
- movement while grabbed
- release
- throw
- collision with environment

Current implementation is partway through Stage 10.

### 4.1 Physics cube C++ class

Created:
- `game/Source/TextGame/Public/PhysicsLabCube.h`
- `game/Source/TextGame/Private/PhysicsLabCube.cpp`

Class:
`APhysicsLabCube : public AActor`

Current behavior:
- root is `UStaticMeshComponent`
- engine cube mesh `/Engine/BasicShapes/Cube.Cube`
- collision profile `PhysicsActor`
- simulate physics = true
- mass override = 20 kg
- linear damping = 0.15
- angular damping = 0.10

This compiled successfully through UnrealHeaderTool and UBT.

### 4.2 Physics Lab GameMode

Created:
- `game/Source/TextGame/Public/PhysicsLabGameMode.h`
- `game/Source/TextGame/Private/PhysicsLabGameMode.cpp`

Originally GameMode spawned:
- floor
- cube
- directional light
- camera

at `BeginPlay()`.

This was tested and worked:
- cube fell
- collision worked
- floor worked

A z-fighting issue appeared because the first test was over an existing Landscape level. Temporary workaround:
- laboratory moved to Z=2000
- old directional lights removed

This workaround worked, but user correctly decided the lab should have its own clean level instead.

## 5. Clean PhysicsLab level

A dedicated blank level was created programmatically:

`game/Content/Maps/PhysicsLab.umap`

Creation script:
`tools/unreal/create_physics_lab_level.py`

The map was created through `UnrealEditor-Cmd.exe` and editor Python automation.

Verified:
- automation exit code 0
- `PhysicsLab.umap` exists
- `*.umap` is tracked through Git LFS

`DefaultEngine.ini` was updated so:
```ini
[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=/Game/Maps/PhysicsLab
GameDefaultMap=/Game/Maps/PhysicsLab
GlobalDefaultGameMode=/Script/TextGame.PhysicsLabGameMode
```

The temporary Z=2000 workaround was removed.

## 6. Lighting fix

When running on the clean level, Unreal showed:
`LIGHTING NEEDS TO BE REBUILT (1 unbuilt object)`

Cause:
runtime DirectionalLight was not marked Movable.

Fixed in C++ with:
```cpp
Light->GetLightComponent()->SetMobility(EComponentMobility::Movable);
```

Rebuilt:
`EXIT CODE = 0`

Runtime retest:
- warning disappeared
- cube still fell correctly

## 7. Important design decision made by user

User explicitly decided:

**The laboratory must be visually inspectable before Play.**

Desired rule:
```text
Editor, before Play
→ see everything already built visually:
   floor
   cube
   lighting
   sky/background
   camera/scene layout

Play
→ actions/behavior begin:
   gravity
   collision
   grab
   movement
   release
   throw
```

So:
- persistent visual world belongs in `PhysicsLab.umap`
- runtime gameplay behavior stays in C++
- no Blueprint requirement
- editor automation is allowed to build/update the map

This is now the intended Physics Lab architecture.

## 8. Persistent visual level work

A new editor automation script was introduced:
`tools/unreal/populate_physics_lab_level.py`

Intended persistent actors in `PhysicsLab.umap`:
- `PhysicsLab_Floor`
- `PhysicsLab_Cube`
- `PhysicsLab_Sun`
- `PhysicsLab_Sky`
- `PhysicsLab_Camera`

The script:
- loads `/Game/Maps/PhysicsLab`
- removes previously generated actors tagged `PhysicsLabPersistent`
- creates floor
- creates C++ `APhysicsLabCube`
- creates DirectionalLight
- sets light Movable
- enables atmosphere sun light
- creates SkyAtmosphere
- creates CameraActor
- tags camera with `PhysicsLabCamera`
- attempts to place editor viewport camera at useful lab view
- saves the level

The current `PhysicsLabGameMode::BeginPlay()` was simplified so it no longer spawns visual actors.

Current runtime responsibility:
- find actor tagged `PhysicsLabCamera`
- set PlayerController view target to that actor
- log `Physics Lab runtime started.`

This separates persistent scene from runtime behavior.

## 9. Latest visual verification

Before Play, Unreal now visibly shows:
- sky
- floor
- cube hanging in the air
- lighting
- persistent camera
- persistent actors in Outliner

So the main “visual before Play” goal is working.

Latest screenshot showed:
- `PhysicsLab_Camera` editor gizmo visible as a large blue camera object on the left
- another white circular editor sprite/icon visible near center
- user asked what these are

Explanation already given:
- blue camera is the **CameraActor editor visualization/gizmo**, not an in-game object
- white circle is another **editor-only actor icon/sprite** (likely light/atmosphere-related)
- these are editor clutter, not gameplay geometry

User wants the viewport clean while still keeping persistent actors in the level.

## 10. Exact current next task

Do NOT jump ahead to launchers or grab/throw mechanics yet.

The next task is:

**Clean the editor viewport so the persistent lab stays visible, but technical actor gizmos/sprites do not visually clutter it.**

User wants:
- keep camera/light/sky actors saved in the level
- see the actual lab visually before Play
- avoid large blue camera gizmo and white technical sprite/icon if possible
- keep behavior after Play

Likely approach:
- use editor show flags / editor-only visualization settings to hide actor icons/sprites/gizmos in viewport
- do not remove the camera unless necessary
- do not change gameplay behavior just to hide editor visualization
- preserve code-first/persistent-level architecture

Need to inspect current Unreal editor APIs/settings before changing anything. Prefer a reversible editor-only solution.

## 11. Current branch and checkpoint state

Current branch:
`lab/physics-cube`

A working Physics Lab baseline checkpoint was committed earlier with message:
`feat: establish working Physics Lab baseline`

After that, additional persistent-level changes were made:
- persistent map population
- simplified GameMode
- editor automation script
- sky/sun/camera actors in map

These latest changes were being visually verified and may not yet be committed.

Before any new mutation in the fresh chat:
1. run `git status --short --untracked-files=all`
2. inspect exact current diff
3. do not assume latest map/script changes are already committed
4. do not discard anything

## 12. Current expected Git-relevant files around Physics Lab

Likely modified/untracked files include some subset of:
- `game/Config/DefaultEngine.ini`
- `game/TextGame.uproject`
- `game/Content/Maps/PhysicsLab.umap`
- `game/Source/TextGame/Private/PhysicsLabCube.cpp`
- `game/Source/TextGame/Public/PhysicsLabCube.h`
- `game/Source/TextGame/Private/PhysicsLabGameMode.cpp`
- `game/Source/TextGame/Public/PhysicsLabGameMode.h`
- `tools/unreal/create_physics_lab_level.py`
- `tools/unreal/populate_physics_lab_level.py`

Again: inspect actual Git state first.

## 13. User workflow preferences

Important:
- user wants one step at a time
- do not race ahead
- after every code change:
  1. build
  2. verify exit code
  3. launch
  4. visually test
  5. only then commit
- do not skip verification
- user explicitly stopped assistant once for jumping ahead to desktop launchers before testing the latest code
- if user says stop, stop immediately

User prefers PowerShell commands and exact paths.

## 14. Launch/build commands

Build:
```powershell
$Repo = 'C:\Users\vlbra\GitHub\TextGame'
$Build = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat'
$Project = "$Repo\game\TextGame.uproject"

& $Build TextGameEditor Win64 Development "-Project=$Project" -WaitMutex -Progress
$LASTEXITCODE
```

Launch:
```powershell
$Repo = 'C:\Users\vlbra\GitHub\TextGame'
$Editor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$Project = "$Repo\game\TextGame.uproject"

Start-Process $Editor -ArgumentList "`"$Project`""
```

Editor automation:
```powershell
$EditorCmd = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
```

## 15. Desktop launcher idea — NOT YET IMPLEMENTED

User asked whether they need to launch through terminal every time.

Agreed future plan:
- create quick launcher
- create Build & Launch launcher
- optionally desktop shortcuts

Possible future files:
- `tools/unreal/Start-TextGame.ps1`
- `tools/unreal/Build-And-Start-TextGame.ps1`

BUT:
**do not implement this next.**
User explicitly redirected back to verification and current Physics Lab work.
Finish the current level/editor-visibility task first.

## 16. Broader architecture guardrails

Do not treat Physics Lab as production architecture.

The architecture baseline says:
- Unreal client/runtime is current direction
- future server architecture is hypothetical
- Login / Character / World / Web / SQL services are not finalized
- server-authoritative direction is a hypothesis
- major subsystem implementation requires research first

Do not start:
- networking
- database
- login server
- world server
- large-world system
- creature architecture

unless explicitly authorized later.

## 17. Immediate instruction for the new chat

Start by saying that the handoff was read and that the next task is to clean editor-only gizmos/sprites while preserving the persistent PhysicsLab scene.

Then:
1. inspect Git status
2. inspect current `PhysicsLabGameMode.cpp`
3. inspect `populate_physics_lab_level.py`
4. decide on the cleanest editor-only way to hide camera/light/actor icons
5. apply one change only
6. build
7. launch
8. verify visually
9. then commit if successful

Do not re-explain or redo Stages 1–9.
