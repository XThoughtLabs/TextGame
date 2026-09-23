---
name: unreal-engine-development
description: >
  Use for Unreal Engine project development involving C++, editor automation,
  Level Actors, build/debug workflows, Codex CLI delegation, or Unreal MCP.
  Discover the real project and engine version before acting. Use small manual
  steps for simple work; delegate complex, multi-file, repetitive, or
  editor-intensive work to local Codex with explicit scope and verification.
  Never assume asset names, active Level, plugin state, or MCP tool schemas.
---

# Unreal Engine Development

Universal working skill for Unreal Engine projects.

Project-specific instructions, architecture, repository rules, and explicit
user decisions override generic guidance in this skill.

## Core operating rules

1. Discovery before implementation.
2. Research before substantial new stages.
3. Prefer small, independently verifiable changes.
4. Read existing project state before writing.
5. Verification is part of implementation.
6. Do not automate a workflow before understanding it.
7. Do not guess Unreal assets, classes, Levels, plugins, or tool schemas.

## Research

Before a substantial new stage, check as appropriate:

- Epic / Unreal Engine documentation for the exact engine version;
- recent GitHub repositories and real implementations;
- GitHub Issues, Discussions, and relevant commits;
- Unreal Developer Community / forums;
- other high-quality practical sources.

Do not blindly copy code written for another Unreal version.

## Pre-flight discovery

Before significant changes determine:

- `.uproject` location;
- Unreal Engine version;
- enabled relevant plugins;
- C++ modules;
- project instructions / architecture;
- Git status and current branch;
- relevant existing source code and assets;
- current Unreal project and Level for editor-side tasks.

Never overwrite unknown local changes.

## Execution lanes

### Lane A — manual / chat-guided

Use for:
- one checkbox or editor setting;
- one small known code/config change;
- one diagnostic command;
- simple visual verification;
- narrow tasks with easy rollback.

Work one coherent step at a time.

### Lane B — local Codex

Use when the task is:
- multi-file;
- repository-wide;
- repetitive;
- log-heavy;
- discovery-heavy;
- a larger refactor;
- difficult to perform reliably through copied commands;
- better executed with direct repository access.

Codex is an execution worker, not the authority.

Give it:
- exact project/repository;
- exact task;
- allowed changes;
- forbidden changes;
- verification requirements;
- Git constraints.

Do not accept "done" without evidence.

### Lane C — Codex + Unreal MCP

Prefer Unreal MCP for direct editor-side inspection/actions when available:
- inspect active Level;
- list/find Actors;
- inspect Actor properties;
- spawn/move/change Actors;
- scene/lighting/material operations;
- supported PIE/editor automation;
- read-back verification.

Always start an unfamiliar editor task read-only.

## Unreal MCP workflow

Unreal MCP controls the currently running Unreal Editor.

Before writes:

1. confirm the intended project is open;
2. confirm the intended Level;
3. confirm the MCP server is running;
4. discover the live toolsets/tools;
5. inspect current state read-only.

Do not guess tool names or argument schemas. The live schema is authoritative.

Recommended loop:

```text
discover live tools
↓
inspect current state
↓
identify exact Actor / asset
↓
perform one bounded change
↓
read back the changed state
↓
save if persistence is intended
↓
verify again
↓
visually verify when relevant
```

Prefer sequential MCP operations unless the current Unreal toolset explicitly
documents safe concurrency.

Actor labels are human-facing and may be non-unique. Prefer stable tags,
identifiers, or returned Actor handles when possible.

Unreal MCP is version-sensitive and may be experimental. Re-discover its
current tool surface instead of relying on memory.

## Codex delegation template

```text
Work in the specified Unreal project.

Discovery first:
- read project instructions;
- inspect .uproject and engine version;
- inspect git status, branch, and recent commits;
- inspect relevant code/assets;
- if using Unreal MCP, confirm project/Level and discover live tools.

Task:
<bounded task>

Allowed changes:
<scope>

Forbidden:
- unrelated files/assets/Actors
- destructive changes outside scope
- commit/push unless explicitly authorized

Verification:
- build if C++ changed;
- inspect complete errors/logs;
- verify the result in Unreal;
- read back editor state through MCP where possible;
- visually verify scene-facing work.

Report:
- changes made;
- files/assets/Actors touched;
- verification evidence;
- anything still unverified.
```

## Standard development cycle

```text
discover
↓
research
↓
choose the smallest appropriate solution
↓
make one coherent change
↓
Build if required
↓
check result / logs
↓
Launch or inspect Unreal
↓
functional + structural + visual verification
↓
commit
↓
push when authorized
```

Do not commit a failed stage as completed work.

## Error handling

```text
observe exact failure
↓
collect full error/log/state
↓
localize cause
↓
research the exact API/version/problem
↓
apply the smallest fix
↓
repeat verification
```

Do not change several unrelated things hoping one fixes the problem.

## C++ defaults

Use Unreal-native lifecycle and ownership rules.

Generally:
- default subobjects belong in constructors;
- world-dependent runtime logic belongs in appropriate runtime lifecycle;
- long-lived UObject references must remain GC-safe;
- avoid Tick without a reason;
- prefer events/timers when appropriate;
- maintain correct Build.cs dependencies;
- validate UCLASS/header changes with an appropriate build.

Do not impose an architecture such as Blueprint or Enhanced Input if the
existing project intentionally uses another appropriate approach.

## Level and Actor work

For modern UE5 editor scripting prefer:
- `EditorActorSubsystem` for Actor operations;
- `LevelEditorSubsystem` for Level operations;
- `UnrealEditorSubsystem` for editor-world / viewport operations.

Treat `EditorLevelLibrary` as legacy/deprecated for new work when modern
subsystems provide the required operation.

Load the exact intended Level explicitly before Actor writes.

Do not load a map through `load_asset()` merely to inspect it. Use Level APIs.

### StaticMeshActor pattern

Prefer:

```python
actor = actor_subsystem.spawn_actor_from_class(
    unreal.StaticMeshActor,
    location,
    unreal.Rotator(roll=0.0, pitch=0.0, yaw=0.0),
)

component = actor.get_component_by_class(
    unreal.StaticMeshComponent
)

component.set_static_mesh(mesh)
```

Then deliberately configure:
- transform;
- collision;
- mobility;
- materials;
- project-specific tags / identifiers.

Use keyword arguments for Python `Rotator`.

After assigning geometry verify real Actor bounds where appropriate.
Actor existence alone is not proof that the mesh is usable/visible.

## Persistent Level edits

For important persistent scene changes:

```text
create/update
↓
validate in memory
↓
save Level
↓
reload Level
↓
find object again
↓
validate mesh / transform / collision / bounds
```

A process exit code of `0` alone is not sufficient proof that a requested
Level change persisted.

## Persistent world vs runtime behavior

Default architectural guideline:

```text
persistent world content
→ Level / assets

runtime behavior
→ C++ / runtime systems
```

Do not spawn permanent environmental geometry only in BeginPlay merely to make
it visible in Play unless that is intentionally the project's architecture.

## Build verification

If C++ changed:
- build the appropriate target;
- require a successful result;
- inspect compiler/linker failures completely;
- verify behavior after the build.

If only persistent editor state changed:
- verify the Actor/asset exists;
- verify expected class/mesh;
- verify transform;
- verify collision/mobility when relevant;
- verify save/persistence.

Visual work should receive visual verification in addition to structural checks.

## Git discipline

Before significant work:
- inspect working tree;
- inspect current branch;
- inspect recent commits/remotes.

After successful verification:
- stage only intended files;
- inspect staged diff/stat;
- commit a narrow logical change;
- push only when authorized.

Never casually force-push or destructively reset unknown work.

## Stop instead of guessing when

- the wrong project may be open;
- the target Level is ambiguous;
- unknown local changes conflict with the task;
- a required asset/class cannot be identified;
- the live MCP schema does not expose the required action;
- a destructive operation has unclear scope;
- build/verification fails and the cause is not understood.

## Sources and provenance

Research snapshot: 2026-09-23.

This skill is an original synthesis; no upstream skill is copied wholesale.

### VibeUE
Repository: https://github.com/kevinpbuckley/VibeUE  
Reviewed: `Content/Skills/level-actors/SKILL.md`  
Snapshot: `0ea59b3ef5dc0a5d54bbb06c691cb45b828399a5`

Used for UE 5.8 EditorActorSubsystem / LevelEditorSubsystem practice,
StaticMeshActor + set_static_mesh, bounds verification, Rotator pitfalls,
deprecated EditorLevelLibrary migration, and UE 5.8 map-residency findings.

### rsc-harness
Repository: https://github.com/ericrisco/rsc-harness  
Reviewed: `skills/unreal/SKILL.md`  
Snapshot: `c5c7bad09501169f0f57f1473fa243826f7816df`

Used for modern UE5 C++ lifecycle/ownership guidance and verification-oriented
workflow.

### DSTN2000 Unreal Engine skill
Repository: https://github.com/DSTN2000/claude-unreal-engine-skill  
Reviewed: `SKILL.md`  
Snapshot: `d58c833a1e4a2b6cea6da8430a81ea32772cd336`

Used for zero-assumptions and project-discovery-first principles.

### VaneWorker / Unreal MCP skill
Repository: https://github.com/huawolf/VaneWorker  
Reviewed: `agent_core/optional-skills/creative/unreal-mcp/SKILL.md`  
Snapshot: `a02c1231a890cef5b7f0a3f06db54ab90a0048d5`

Used for Unreal MCP discovery-first workflow, live tool schema use,
read-back verification, explicit saves, and label-vs-Actor identity guidance.
Hermes-specific installation conventions are intentionally excluded.

### CarRenderFactory Codex Unreal skill
Repository: https://github.com/AnshunAnson/CarRenderFactory  
Reviewed: `.codex/skills/unreal-engine/SKILL.md`  
Snapshot: `6ffa14d9d5f552c3df4dd63fa954798d0ad3f8c1`

Used for Codex-oriented discovery/delegation and verification gates.
Project-specific subagent infrastructure is intentionally excluded.

### marketing-studio
Repository: https://github.com/ucsandman/marketing-studio  
Reviewed: `feeders/unreal/scenes/cube_flythrough.py`  
Snapshot: `66cd1c30919f1144c60aed3692a915bb0b2c6238`

Used for UE 5.8.2-tested StaticMeshActor scene automation.

### unreal-agent-harness
Repository: https://github.com/per-simmons/unreal-agent-harness  
Reviewed: `docs/ue58-python-api.md`  
Snapshot: `bf8dc7a5b1380d1757cd10bbc2771ff66a647b53`

Used for consolidated modern UE editor-subsystem and Python API notes.

## Maintenance rule

When updating this skill:
- prefer evidence from the actual target UE version;
- record new source repositories;
- distinguish observed behavior from general advice;
- remove or qualify stale patterns;
- treat the current live Unreal MCP schema as authoritative.
