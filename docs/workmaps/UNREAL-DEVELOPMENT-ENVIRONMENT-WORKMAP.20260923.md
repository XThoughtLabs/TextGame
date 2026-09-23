# TextGame — Unreal Development Environment Workmap

**Status:** Initial planning baseline  
**Date:** 2026-09-23  
**Repository:** `XThoughtLabs/TextGame`  
**Implementation status:** NOT STARTED

## 1. Purpose

Establish a clean, reproducible, code-first Unreal Engine 5.8 development environment for TextGame before any gameplay implementation begins.

The immediate goal is not to build the game itself. The goal is to create the correct development foundation so that Unreal Engine, Visual Studio, Git, Git LFS, project structure, build tooling, and later automation can be used consistently and safely.

## 2. Canonical Working Checkout

Use one canonical working checkout for TextGame:

```text
C:\Users\vlbra\GitHub\TextGame
```

This checkout is the primary local development root for the project.

Avoid creating a second independent runtime/development copy unless a later explicit architectural decision requires one.

## 3. Working Principles

1. Code-first Unreal development.
2. Unreal Editor is used primarily for runtime inspection, scene verification, asset work, and visual validation.
3. Core project structure and gameplay logic should be created and maintained through source code and version control.
4. Blueprint may be used later where justified, but the initial development workflow is not Blueprint-first.
5. GitHub is the canonical remote source-control location for the implementation repository.
6. Major environment changes should be verified before gameplay work begins.
7. Each stage should be completed and checked before moving to the next stage.

## 4. Workmap

### Stage 1 — Verify the Development Environment

Verify, without changing the repository:

- TextGame Git status and current branch.
- Git remote configuration.
- Git LFS installation.
- Unreal Engine 5.8 installation.
- Unreal Editor executable availability.
- UnrealBuildTool availability.
- Unreal `Build.bat` availability.
- Visual Studio 2026 installation.
- C++ toolchain availability.
- Windows SDK availability.

**Completion condition:** all required development tools are present and their paths/versions are known.

---

### Stage 2 — Create the Environment Setup Branch

Create a dedicated working branch:

```text
setup/unreal-environment
```

All initial Unreal environment work should be isolated there until the setup is verified.

**Completion condition:** branch exists locally and on GitHub and is the active setup branch.

---

### Stage 3 — Configure Git for Unreal

Add the project-level source-control rules required for Unreal development.

Prepare:

- `.gitignore` appropriate for Unreal-generated files.
- Git LFS tracking rules for large/binary Unreal assets where needed.
- clear separation between source-controlled project files and generated build/cache files.

Generated directories such as intermediate build output and local caches must not become accidental repository content.

**Completion condition:** Git rules are committed and a clean checkout can reproduce the intended tracked structure.

---

### Stage 4 — Establish Repository Structure

Create the initial long-term repository layout.

The exact final structure remains open to refinement, but it should provide clear boundaries for at least:

- Unreal game/client project.
- project documentation.
- future server-side components.
- future tooling and automation.
- future data/database-related components.
- test or laboratory areas where needed.

The existing `WorkStuff/` and `docs/` content must be preserved.

**Completion condition:** repository structure is explicit, documented, and ready for the Unreal project.

---

### Stage 5 — Create the C++ Unreal Project

Create the first Unreal Engine 5.8 project as a C++ project through project files and source structure rather than a Blueprint-first manual workflow.

Initial project responsibilities:

- valid `.uproject`;
- C++ module;
- module `Build.cs`;
- target files;
- minimum source entry point;
- configuration files required for the project to load.

**Completion condition:** Unreal recognizes the project as a valid C++ project.

---

### Stage 6 — Generate the Visual Studio Solution

Generate Visual Studio project/solution files for the Unreal project.

The solution must correctly reference the Unreal Engine 5.8 toolchain and the TextGame Unreal module.

**Completion condition:** the generated solution opens correctly in Visual Studio 2026 without project-generation errors.

---

### Stage 7 — Build Through UnrealBuildTool

Build the initial C++ project through the standard Unreal build pipeline.

Primary requirement:

- use UnrealBuildTool / Unreal build scripts rather than treating the project as an ordinary standalone Visual Studio C++ application.

**Completion condition:** development editor target builds successfully with no blocking errors.

---

### Stage 8 — Launch Unreal Editor From the Project

Launch Unreal Editor using the project's `.uproject`.

Verify that:

- the project opens;
- the C++ module loads;
- no missing-module or build-version errors appear;
- the project can be reopened after closing.

**Completion condition:** the Unreal Editor reliably opens the code-based project.

---

### Stage 9 — C++ Smoke Test

Create one minimal C++ runtime behavior to prove the complete code-to-engine workflow.

The smoke test should demonstrate:

```text
edit C++ source
    ->
build
    ->
launch Unreal
    ->
observe expected runtime behavior
```

The test should be intentionally small and should not yet introduce gameplay architecture.

**Completion condition:** a deliberate C++ change is successfully compiled and visibly executed in Unreal.

---

### Stage 10 — Physics Lab: First Interactive Cube

Only after the development environment is proven, begin the first gameplay laboratory.

Initial Physics Lab target:

- small 3D test environment;
- physical cube;
- gravity;
- collision;
- mass;
- mouse-based grab;
- controlled movement while held;
- release;
- throw;
- collision with the environment.

The purpose is to learn and verify the Unreal C++ physics workflow before moving to creatures, skeletal systems, large-world construction, networking, or production gameplay.

**Completion condition:** the cube can be grabbed, moved, released, thrown, and behaves correctly under physics.

---

### Stage 11 — Development Workflow Automation

After the core environment and first physics test are stable, evaluate and configure the higher-level development workflow.

Planned areas:

- Git worktrees for isolated parallel tasks;
- Codex/ChatGPT Environments for reproducible project setup;
- Hooks for automated checks and repeatable workflow actions;
- build/test actions;
- safe branch and review practices.

Automation should be added only after the repeated workflow is understood well enough to automate deliberately.

**Completion condition:** automation decisions are documented and only useful, justified automation is enabled.

## 5. Immediate Next Step

The next authorized action after recording this workmap is:

**Stage 1 — Verify the Development Environment.**

No Unreal project files, source files, build files, physics code, worktrees, hooks, or environments are created by this document.

## 6. Change Control

This workmap is an initial baseline, not a frozen architecture.

It may be amended as practical constraints are discovered, but changes should be explicit and recorded in Git.

Major deviations should be decided before implementation rather than silently introduced during setup.
