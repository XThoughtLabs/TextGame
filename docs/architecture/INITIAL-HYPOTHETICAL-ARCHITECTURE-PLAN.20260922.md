# TextGame — Initial Hypothetical Architecture Plan

Status: HYPOTHETICAL_INITIAL_PLAN
Date: 2026-09-22
Repository: XThoughtLabs/TextGame

## Purpose

This document records the current working hypothesis for the future Blood of Chaos game architecture. It is not a final technical decision and does not authorize implementation. Every major subsystem remains subject to later deep research in Work mode using books, official documentation, current engineering practice, and project-specific requirements.

## Project direction

The game is planned as a 3D interactive world rather than a 2D web atlas.

Current working direction:

- Unreal Engine as the primary 3D game client and world runtime.
- The world map itself is 3D.
- Creatures and important world objects are 3D.
- Unreal Engine will be used for world construction, physical interaction, collision, animation, creature behavior, and gameplay experimentation.
- Blood of Chaos concept and canon remain governed by the canonical project materials in XThoughtLabs/WorkZone, especially the Conceptologist Office package for BLOOD_OF_CHAOS.
- TextGame is the implementation repository and must not silently become a competing source of lore truth.

## Hypothetical distributed online architecture

The current first hypothesis is a multi-service online-game architecture.

Potential components:

1. Unreal Client
   - 3D world rendering
   - input and user interface
   - client-side presentation
   - local prediction where appropriate
   - communication with game services

2. Login Server
   - account authentication
   - session creation
   - access checks
   - ban / permission checks
   - handoff into the game session

3. Character / Player Server
   - character ownership
   - persistent player state
   - progression
   - inventory
   - character metadata
   - transitions between world services

4. Map / World Servers
   - authoritative regional simulation
   - positions
   - creatures and NPCs
   - combat
   - physical interactions
   - region state
   - multiple map servers may cooperate as one world

5. Web Server
   - project website
   - account-related web functions
   - administration interfaces
   - auxiliary APIs
   - possible future knowledge-base or World Bible access

6. SQL Server
   - separate persistent database server
   - MariaDB is a current example, not a final selection
   - schema, replication, backup, recovery, indexing, and security require dedicated research

## Initial topology hypothesis

```text
Unreal Client
    |
    v
Login Server
    |
    v
Character / Player Server
    |
    +-------------------------+
    |                         |
    v                         v
Map / World Server A    Map / World Server B
    |                         |
    +------------+------------+
                 |
                 v
             SQL Server

Web Server communicates with the relevant backend services and/or database according to the architecture selected after research.
```

This topology is illustrative only. The exact network paths, service boundaries, protocols, direct-vs-mediated database access, and deployment model are intentionally unresolved.

## Server-authoritative principle

The current architectural hypothesis is that the online game should follow a server-authoritative model.

The Unreal client expresses player intent. Authoritative game services validate and determine persistent or competitive game state.

Examples include:

- movement validation
- combat
- item acquisition
- world interaction
- physical actions relevant to shared state
- character progression
- persistent inventory changes

Exact prediction, reconciliation, physics-authority, anti-cheat, and latency strategies remain research topics.

## 3D and world-data separation

The 3D world should not be the sole knowledge store.

The intended separation is:

- WorkZone / BLOOD_OF_CHAOS: concept, canon, provenance, decisions.
- TextGame: implementation, game code, runtime schemas, technical assets.
- Unreal Engine: 3D world, creatures, physical interaction, animation, gameplay.
- SQL infrastructure: persistent runtime data.
- Stable identifiers: links between world entities, database records, and 3D actors.

This separation is a design hypothesis and must be validated during the architecture research phase.

## Unreal Engine learning path

The first Unreal exercise is intentionally small and is not the game itself.

Initial laboratory target:

- small 3D environment
- physical cube
- collision
- gravity
- mass
- mouse grab
- movement while grabbed
- throwing
- collision with the environment

The purpose is to learn the Unreal workflow and basic physical interaction before attempting creatures, complex rigs, large landscapes, networking, or the full Blood of Chaos world.

## 3D asset pipeline hypothesis

A likely future pipeline is:

```text
Blood of Chaos concept
    ->
3D creation / DCC tool such as Blender
    ->
mesh / rig / textures / animations
    ->
Unreal Engine
    ->
materials / physics / animation / gameplay
    ->
3D world
```

The exact modeling tools, interchange formats, rigging workflow, animation pipeline, and source-control treatment of large assets remain open research questions.

## Relationship to the earlier atlas plan

The earlier Blood of Chaos planning in WorkZone described a React / Leaflet / Python / FastAPI / SQLite interactive atlas.

The present game idea differs materially:

- the map is intended to become a 3D world;
- creatures are 3D;
- Unreal Engine is the primary runtime candidate;
- the project is moving toward an online-game architecture.

The earlier atlas work remains useful as conceptual background for structured world data, stable object IDs, map/database relationships, and separation between canon and presentation. It should not be treated as the final runtime architecture for this game without explicit later review.

## Research-before-implementation rule

Before implementing each major subsystem, conduct a dedicated research phase.

Planned research areas include:

1. overall online-game architecture
2. Unreal Engine multiplayer architecture
3. login and authentication
4. character / player service
5. map / world servers
6. SQL server and MariaDB-class database options
7. web server
8. inter-service communication
9. security and anti-cheat
10. deployment and operations
11. 3D asset pipeline
12. large 3D world architecture
13. data synchronization between canon, game data, and runtime state
14. source control and large binary asset strategy

Each research phase should use project sources plus external materials and should produce a separate implementation plan before coding begins.

## Current status

- This document is the first hypothetical architecture baseline only.
- No subsystem described here is considered final.
- No server technology has been selected beyond Unreal Engine as the current 3D/game-runtime direction.
- MariaDB is an example of the intended separate SQL-server class, not a final database decision.
- No production networking or database implementation is authorized by this plan.
- Deep research in Work mode is the required next step before architecture is frozen.
