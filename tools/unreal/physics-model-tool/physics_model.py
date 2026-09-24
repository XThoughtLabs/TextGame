"""Configure existing mesh actors in place; create only when none match."""
from __future__ import annotations

import json
import math
import os
from pathlib import Path
import traceback
from typing import Any

VERSION = "0.1.0"


class SafetyError(RuntimeError):
    pass


def require(ok: bool, message: str) -> None:
    if not ok:
        raise SafetyError(message)


def object_path(value: Any) -> str | None:
    return value.get_path_name() if value is not None else None


def asset_path(value: str) -> str:
    require(
        isinstance(value, str) and value.startswith("/"),
        "Asset must be an Unreal package/object path."
    )
    require(
        not any(c in value for c in "\\\r\n\"'"),
        "Invalid asset path."
    )
    require(
        not value.lower().endswith((".uasset", ".umap", ".fbx", ".obj")),
        "Use /Game/Folder/Asset without a file extension."
    )
    if "." in value.rsplit("/", 1)[-1]:
        return value
    return value + "." + value.rsplit("/", 1)[-1]


def vec(value: Any) -> list[float]:
    return [float(value.x), float(value.y), float(value.z)]


def rot(value: Any) -> list[float]:
    return [float(value.roll), float(value.pitch), float(value.yaw)]


def same(a: Any, b: Any) -> bool:
    if isinstance(a, (float, int)) and not isinstance(a, bool):
        return (
            isinstance(b, (float, int))
            and math.isclose(a, b, rel_tol=1e-7, abs_tol=1e-3)
        )
    if isinstance(a, list):
        return (
            isinstance(b, list)
            and len(a) == len(b)
            and all(same(x, y) for x, y in zip(a, b))
        )
    if isinstance(a, dict):
        return (
            isinstance(b, dict)
            and a.keys() == b.keys()
            and all(same(a[k], b[k]) for k in a)
        )
    return a == b


def mesh_of(component: Any, u: Any) -> Any:
    if isinstance(component, u.SkeletalMeshComponent):
        return component.get_editor_property("skeletal_mesh_asset")
    if isinstance(component, u.StaticMeshComponent):
        return component.get_editor_property("static_mesh")
    return None


def mesh_components(actor: Any, u: Any) -> list[Any]:
    return (
        list(actor.get_components_by_class(u.StaticMeshComponent))
        + list(actor.get_components_by_class(u.SkeletalMeshComponent))
    )


def stable_actor(actor: Any, u: Any) -> dict:
    """Properties that this tool must preserve."""
    meshes = []
    for comp in mesh_components(actor, u):
        body = comp.get_editor_property("body_instance")
        meshes.append({
            "path": object_path(comp),
            "mesh": object_path(mesh_of(comp, u)),
            "relative_location": vec(comp.get_editor_property("relative_location")),
            "relative_rotation": rot(comp.get_editor_property("relative_rotation")),
            "relative_scale": vec(comp.get_editor_property("relative_scale3d")),
            "attachment": object_path(comp.get_attach_parent()),
            "materials": [
                object_path(comp.get_material(i))
                for i in range(comp.get_num_materials())
            ],
            "mass_scale": float(body.get_editor_property("mass_scale")),
            "mass_override": float(body.get_editor_property("mass_in_kg_override")),
            "linear_damping": float(body.get_editor_property("linear_damping")),
            "angular_damping": float(body.get_editor_property("angular_damping")),
            "physical_material_override": object_path(
                body.get_editor_property("phys_material_override")
            ),
            "physics_asset_override": (
                object_path(comp.get_editor_property("physics_asset_override"))
                if isinstance(comp, u.SkeletalMeshComponent) else None
            ),
        })
    return {
        "path": object_path(actor),
        "class": actor.get_class().get_path_name(),
        "label": actor.get_actor_label(),
        "location": vec(actor.get_actor_location()),
        "rotation_roll_pitch_yaw": rot(actor.get_actor_rotation()),
        "scale": vec(actor.get_actor_scale3d()),
        "folder": str(actor.get_folder_path()),
        "tags": [str(t) for t in actor.get_editor_property("tags")],
        "meshes": sorted(meshes, key=lambda entry: entry["path"]),
    }


def physics_state(actor: Any, comp: Any, u: Any) -> dict:
    body = comp.get_editor_property("body_instance")
    return {
        "actor_collision": bool(actor.get_actor_enable_collision()),
        "profile": str(comp.get_collision_profile_name()),
        "collision": str(comp.get_collision_enabled()),
        "mobility": str(comp.get_editor_property("mobility")),
        "simulate_physics": bool(body.get_editor_property("simulate_physics")),
        "gravity": bool(body.get_editor_property("enable_gravity")),
        "ccd": bool(body.get_editor_property("use_ccd")),
        "world_static": str(
            comp.get_collision_response_to_channel(u.CollisionChannel.ECC_WORLD_STATIC)
        ),
        "world_dynamic": str(
            comp.get_collision_response_to_channel(u.CollisionChannel.ECC_WORLD_DYNAMIC)
        ),
        "physics_body": str(
            comp.get_collision_response_to_channel(u.CollisionChannel.ECC_PHYSICS_BODY)
        ),
    }


def desired_physics(skeletal: bool, u: Any) -> dict:
    return {
        "actor_collision": True,
        "profile": "Ragdoll" if skeletal else "PhysicsActor",
        "collision": str(u.CollisionEnabled.QUERY_AND_PHYSICS),
        "mobility": str(u.ComponentMobility.MOVABLE),
        "simulate_physics": True,
        "gravity": True,
        "ccd": True,
        "world_static": str(u.CollisionResponseType.ECR_BLOCK),
        "world_dynamic": str(u.CollisionResponseType.ECR_BLOCK),
        "physics_body": str(u.CollisionResponseType.ECR_BLOCK),
    }


def collision_preflight(mesh: Any, comp: Any, u: Any) -> dict:
    if isinstance(mesh, u.SkeletalMesh):
        require(
            mesh.get_editor_property("skeleton") is not None,
            "SkeletalMesh has no Skeleton."
        )
        override = comp.get_editor_property("physics_asset_override") if comp else None
        pa = override or mesh.get_editor_property("physics_asset")
        require(
            pa is not None,
            "No Physics Asset. Source assets will not be generated or modified."
        )
        if comp:
            require(
                not comp.get_editor_property("enable_per_poly_collision"),
                "Per-poly skeletal collision requires separate review."
            )
        return {
            "kind": "SkeletalMesh",
            "physics_asset": object_path(pa),
            "body_and_constraint_quality": "NOT_GAMEPLAY_TESTED",
        }

    require(
        isinstance(mesh, u.StaticMesh),
        "Only StaticMesh and SkeletalMesh assets are supported."
    )
    sm = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
    simple = sm.get_simple_collision_count(mesh)
    convex = sm.get_convex_collision_count(mesh)
    require(simple >= 0 and convex >= 0, "Could not inspect collision.")
    require(
        simple > 0 or convex > 0,
        "No simple/convex collision. Source mesh will not be modified."
    )
    complexity = sm.get_collision_complexity(mesh)
    if complexity == u.CollisionTraceFlag.CTF_USE_DEFAULT:
        complexity = u.get_default_object(u.PhysicsSettings).get_editor_property(
            "default_shape_complexity"
        )
    require(
        complexity != u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE,
        "UseComplexAsSimple is unsuitable for this dynamic-body setup."
    )
    return {
        "kind": "StaticMesh",
        "simple_count": simple,
        "convex_count": convex,
        "effective_collision_complexity": str(complexity),
    }


def check_instance(actor: Any, comp: Any, mesh: Any, current_level: Any, u: Any) -> None:
    expected = (
        "/Script/Engine.SkeletalMeshActor"
        if isinstance(mesh, u.SkeletalMesh)
        else "/Script/Engine.StaticMeshActor"
    )
    require(
        actor.get_class().get_path_name() == expected,
        "Matching mesh belongs to a custom/Blueprint actor: "
        + actor.get_actor_label()
        + ". No replacement or duplicate will be created."
    )
    require(
        actor.get_level() == current_level,
        "Matching model is in a sublevel. Separate review required."
    )
    require(
        actor.get_editor_property("root_component") == comp,
        "Matching component is not the actor root."
    )
    require(
        comp.get_attach_parent() is None,
        "Model is attached. Refusing implicit detachment."
    )
    scale = vec(actor.get_actor_scale3d())
    require(
        all(math.isfinite(x) and x > 0 for x in scale),
        "Negative/zero scale requires separate review."
    )
    require(
        max(scale) - min(scale) < 1e-4 or not isinstance(mesh, u.SkeletalMesh),
        "Non-uniform skeletal scale requires separate review."
    )
    for name in (
        "modify", "set_mobility", "set_collision_profile_name",
        "set_collision_enabled", "set_enable_gravity", "set_all_use_ccd",
        "set_simulate_physics", "set_collision_response_to_channel"
    ):
        require(callable(getattr(comp, name, None)), "Unavailable API: " + name)


def check_bounds(actor: Any) -> dict:
    origin, extent = actor.get_actor_bounds(False)
    require(
        all(math.isfinite(v) for v in vec(origin) + vec(extent))
        and max(vec(extent)) > 0,
        "Invalid/zero bounds: " + actor.get_actor_label()
    )
    return {"origin": vec(origin), "extent": vec(extent)}


def configure(actor: Any, comp: Any, desired: dict, u: Any) -> bool:
    before = physics_state(actor, comp, u)
    if before == desired:
        return False

    actor.modify()
    comp.modify()
    if not before["actor_collision"]:
        actor.set_actor_enable_collision(True)
    if before["mobility"] != desired["mobility"]:
        comp.set_mobility(u.ComponentMobility.MOVABLE)
    if before["profile"] != desired["profile"]:
        comp.set_collision_profile_name(u.Name(desired["profile"]))
    if comp.get_collision_enabled() != u.CollisionEnabled.QUERY_AND_PHYSICS:
        comp.set_collision_enabled(u.CollisionEnabled.QUERY_AND_PHYSICS)

    for channel in (
        u.CollisionChannel.ECC_WORLD_STATIC,
        u.CollisionChannel.ECC_WORLD_DYNAMIC,
        u.CollisionChannel.ECC_PHYSICS_BODY,
    ):
        if comp.get_collision_response_to_channel(channel) != u.CollisionResponseType.ECR_BLOCK:
            comp.set_collision_response_to_channel(channel, u.CollisionResponseType.ECR_BLOCK)

    if not comp.get_editor_property("body_instance").get_editor_property("enable_gravity"):
        comp.set_enable_gravity(True)
    # CCD_PERSISTENCE_V1
    body = comp.get_editor_property("body_instance")
    if not body.get_editor_property("use_ccd"):
        body.set_editor_property("use_ccd", True)
        comp.set_editor_property("body_instance", body)
    if not comp.get_editor_property("body_instance").get_editor_property("simulate_physics"):
        comp.set_simulate_physics(True)
    comp.set_all_use_ccd(True)

    require(
        physics_state(actor, comp, u) == desired,
        "Physics settings mismatch:\n" + json.dumps({"actor": actor.get_actor_label(), "expected": desired, "actual": physics_state(actor, comp, u)}, indent=2, ensure_ascii=False)
    )
    return True


def execute(request: dict, u: Any) -> dict:
    mode = request["mode"]
    require(mode in ("plan", "apply", "verify"), "Unknown mode.")
    requested_asset = asset_path(request["asset"])
    level_path = request["level"]
    require(
        level_path.startswith("/Game/") and "." not in level_path,
        "Level must be a /Game/... package path without extension."
    )

    level_editor = u.get_editor_subsystem(u.LevelEditorSubsystem)
    actor_editor = u.get_editor_subsystem(u.EditorActorSubsystem)
    require(not level_editor.is_in_play_in_editor(), "Stop PIE first.")
    require(level_editor.load_level(level_path), "Could not load Level.")
    current_level = level_editor.get_current_level()
    require(
        current_level.get_path_name().split(".", 1)[0] == level_path,
        "Wrong Level loaded."
    )
    scene = list(actor_editor.get_all_level_actors())
    require(
        all(a.get_level() == current_level for a in scene),
        "This version requires a single Level without streamed sublevels."
    )

    def find_mesh(path: str) -> list:
        found = []
        for a in scene:
            for c in mesh_components(a, u):
                m = mesh_of(c, u)
                if m is not None and object_path(m).casefold() == path.casefold():
                    found.append((a, c, m))
        return found

    # Search the Level before loading/creating anything new.
    matches = find_mesh(requested_asset)
    mesh = matches[0][2] if matches else u.load_asset(requested_asset)
    require(mesh is not None, "Asset not found: " + requested_asset)
    require(
        isinstance(mesh, (u.StaticMesh, u.SkeletalMesh)),
        "Unsupported asset class."
    )
    canonical_asset = object_path(mesh)
    if not matches:
        matches = find_mesh(canonical_asset)

    skeletal = isinstance(mesh, u.SkeletalMesh)
    desired = desired_physics(skeletal, u)
    summaries = []
    for a, c, m in matches:
        check_instance(a, c, m, current_level, u)
        collision = collision_preflight(m, c, u)
        state = physics_state(a, c, u)
        summaries.append({
            "actor": object_path(a),
            "label": a.get_actor_label(),
            "component": object_path(c),
            "collision_source": collision,
            "before_physics": state,
            "desired_physics": desired,
            "action": "UNCHANGED" if state == desired else "CONFIGURE_EXISTING",
            "bounds": check_bounds(a),
        })

    if not matches:
        require(mode != "verify", "Model missing after reopening; nothing created.")
        collision_preflight(mesh, None, u)
        location = request.get("new_location")
        require(
            isinstance(location, list) and len(location) == 3
            and all(isinstance(v, (int, float)) and math.isfinite(v) for v in location),
            "A finite new location [X,Y,Z] is required."
        )

    report = {
        "tool_version": VERSION,
        "run_id": request["run_id"],
        "mode": mode,
        "engine_version": u.SystemLibrary.get_engine_version(),
        "level": level_path,
        "asset": canonical_asset,
        "asset_type": "SkeletalMesh" if skeletal else "StaticMesh",
        "matched_instances": len(matches),
        "created_instances": 0,
        "changed_instances": 0,
        "instances": summaries,
        "saved": False,
        "visual_physics_test": "NOT_RUN",
    }

    if mode == "plan":
        report.update(
            status="PLAN_OK",
            operation="REUSE" if matches else "CREATE_ONE",
            would_create=0 if matches else 1,
            would_configure=sum(
                s["action"] == "CONFIGURE_EXISTING" for s in summaries
            ),
        )
        if not matches:
            report["new_location"] = request["new_location"]
        return report

    if mode == "verify":
        prior = json.loads(
            Path(request["expected_report"]).read_text(encoding="utf-8-sig")
        )
        require(
            prior["run_id"] == request["run_id"] and prior["asset"] == canonical_asset,
            "Verification report belongs to another run."
        )
        expected_paths = {s["path"] for s in prior["expected_targets"]}
        require(
            {object_path(a) for a, _, _ in matches} == expected_paths,
            "Target identity/count changed after reopening."
        )
        for a, c, _ in matches:
            expected = next(
                s for s in prior["expected_targets"]
                if s["path"] == object_path(a)
            )
            require(
                same(expected, stable_actor(a, u)),
                "Identity/transform/material changed: " + object_path(a)
            )
            require(
                physics_state(a, c, u) == desired,
                "Physics flags did not persist: " + object_path(a)
            )
        by_path = {object_path(a): a for a in scene}
        for original in prior["protected_mesh_actors"]:
            require(
                original["path"] in by_path,
                "An unrelated model is missing after reopening."
            )
            require(
                same(original, stable_actor(by_path[original["path"]], u)),
                "An unrelated model changed after reopening."
            )
        report.update(
            status="VERIFIED_FROM_DISK",
            persistence_verified=True,
            preserved_existing_models=True,
        )
        return report

    originals = {object_path(a): stable_actor(a, u) for a in scene}
    target_paths = {object_path(a) for a, _, _ in matches}
    report["protected_mesh_actors"] = [
        originals[object_path(a)] for a in scene
        if mesh_components(a, u) and object_path(a) not in target_paths
    ]

    if not matches:
        cls = u.SkeletalMeshActor if skeletal else u.StaticMeshActor
        a = actor_editor.spawn_actor_from_class(
            cls,
            u.Vector(*request["new_location"]),
            u.Rotator(roll=0.0, pitch=0.0, yaw=0.0),
        )
        require(a is not None, "Actor creation failed.")
        c = a.get_component_by_class(
            u.SkeletalMeshComponent if skeletal else u.StaticMeshComponent
        )
        require(c is not None, "New actor has no mesh component; Level not saved.")
        c.set_mobility(u.ComponentMobility.MOVABLE)
        if skeletal:
            c.set_skeletal_mesh_asset(mesh)
        else:
            c.set_static_mesh(mesh)
        require(object_path(mesh_of(c, u)) == canonical_asset, "Mesh assignment failed.")
        a.set_actor_label("PhysicsModel_" + mesh.get_name())
        check_instance(a, c, mesh, current_level, u)
        check_bounds(a)
        matches = [(a, c, mesh)]
        report["created_instances"] = 1

    expected_targets = []
    for a, c, _ in matches:
        before = stable_actor(a, u)
        if configure(a, c, desired, u):
            report["changed_instances"] += 1
        require(
            same(before, stable_actor(a, u)),
            "Configuration changed protected properties; Level not saved."
        )
        expected_targets.append(stable_actor(a, u))

    after_scene = list(actor_editor.get_all_level_actors())
    after_by_path = {object_path(a): a for a in after_scene}
    require(
        len(after_scene) == len(scene) + report["created_instances"],
        "Unexpected actor count; Level not saved."
    )
    for path, original in originals.items():
        require(path in after_by_path, "Original actor disappeared; Level not saved.")
        require(
            same(original, stable_actor(after_by_path[path], u)),
            "Original actor properties changed; Level not saved."
        )

    report["expected_targets"] = expected_targets
    if report["created_instances"] or report["changed_instances"]:
        require(level_editor.save_current_level(), "Level save failed.")
        report["saved"] = True
        report["status"] = "APPLY_SAVED"
    else:
        report["status"] = "ALREADY_CONFIGURED"

    report["preserved_existing_models"] = True
    return report


def main() -> None:
    import unreal

    request_path = Path(os.environ["UE_PHYSICS_MODEL_REQUEST"])
    request = json.loads(request_path.read_text(encoding="utf-8-sig"))
    output = Path(request["report"])

    try:
        report = execute(request, unreal)
    except Exception as exc:
        report = {
            "status": "FAILED",
            "run_id": request.get("run_id"),
            "mode": request.get("mode"),
            "error": str(exc),
            "traceback": traceback.format_exc(),
        }
        output.write_text(
            json.dumps(report, indent=2, ensure_ascii=False),
            encoding="utf-8"
        )
        unreal.log_error("PHYSICS_MODEL_TOOL: " + str(exc))
        raise

    output.write_text(
        json.dumps(report, indent=2, ensure_ascii=False),
        encoding="utf-8"
    )
    unreal.log("PHYSICS_MODEL_TOOL: " + report["status"])


if __name__ == "__main__":
    main()
