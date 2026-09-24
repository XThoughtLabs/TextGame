# TextGame - stopped work checkpoint

Status: STOPPED / WIP / KNOWN REGRESSION
Branch: lab/physics-cube
HEAD before this checkpoint: 8d7d2dd421ffc12092f4fdbecda0742b0a94857c

The user requested stopping development and preserving all current work.
This checkpoint is NOT a completed or fully verified feature release.

Confirmed during this session:
- SKM_Quinn_Simple and its dependencies were migrated into TextGame.
- Physics Model Tool configured the existing Quinn: 0 created, 1 configured.
- Saving and fresh-process persistence verification passed.
- The user confirmed that ragdoll physics and mouse grabbing work.
- The latest firm-grip patch improved holding strength.

KNOWN UNRESOLVED PROBLEM:
- A grabbed ragdoll can still pass through the wall during abrupt movement.
- The exact failure mechanism has not been established.
- Do not mark ragdoll collision protection as PASS.

Preservation rules:
- Keep current code, assets, map and tools as they are.
- Do not delete or recreate existing models.
- Do not automatically reset, restore, clean or resume development.
- Further changes require a new explicit user instruction.

Local logs and backups under game/Saved remain outside Git.
