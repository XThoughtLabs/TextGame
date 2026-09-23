import unreal

LEVEL_PATH = "/Game/Maps/PhysicsLab"
LAB_TAG = unreal.Name("PhysicsLabPersistent")
CAMERA_TAG = unreal.Name("PhysicsLabCamera")

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

if not level_editor.load_level(LEVEL_PATH):
    raise RuntimeError("Failed to load PhysicsLab level.")

# Remove only previously generated persistent lab actors.
old_lab_actors = []

for actor in actor_editor.get_all_level_actors():
    if LAB_TAG in actor.get_editor_property("tags"):
        old_lab_actors.append(actor)

if old_lab_actors:
    actor_editor.destroy_actors(old_lab_actors)


def mark_actor(actor, label, extra_tags=None):
    tags = [LAB_TAG]

    if extra_tags:
        tags.extend(extra_tags)

    actor.set_actor_label(label)
    actor.set_editor_property("tags", tags)
    return actor


# ------------------------------------------------------------
# Floor
# ------------------------------------------------------------

cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")

if not cube_mesh:
    raise RuntimeError("Engine cube mesh could not be loaded.")

floor = actor_editor.spawn_actor_from_class(
    unreal.StaticMeshActor,
    unreal.Vector(0.0, 0.0, -25.0),
    unreal.Rotator()
)

mark_actor(floor, "PhysicsLab_Floor")

floor_component = floor.get_component_by_class(
    unreal.StaticMeshComponent
)

floor_component.set_static_mesh(cube_mesh)
floor_component.set_collision_profile_name("BlockAll")
floor_component.set_simulate_physics(False)

floor.set_actor_scale3d(
    unreal.Vector(12.0, 12.0, 0.5)
)


# ------------------------------------------------------------
# Physics cube
# ------------------------------------------------------------

physics_cube_class = unreal.load_class(
    None,
    "/Script/TextGame.PhysicsLabCube"
)

if not physics_cube_class:
    raise RuntimeError("PhysicsLabCube C++ class could not be loaded.")

physics_cube = actor_editor.spawn_actor_from_class(
    physics_cube_class,
    unreal.Vector(0.0, 0.0, 350.0),
    unreal.Rotator(
        roll=10.0,
        pitch=20.0,
        yaw=25.0
    )
)

mark_actor(
    physics_cube,
    "PhysicsLab_Cube"
)


# ------------------------------------------------------------
# Sun
# ------------------------------------------------------------

sun = actor_editor.spawn_actor_from_class(
    unreal.DirectionalLight,
    unreal.Vector(),
    unreal.Rotator(
        roll=0.0,
        pitch=-45.0,
        yaw=-35.0
    )
)

mark_actor(sun, "PhysicsLab_Sun")

sun_component = sun.get_editor_property(
    "directional_light_component"
)

sun_component.set_mobility(
    unreal.ComponentMobility.MOVABLE
)

sun_component.set_intensity(8.0)
sun_component.set_atmosphere_sun_light(True)


# ------------------------------------------------------------
# Sky
# ------------------------------------------------------------

sky = actor_editor.spawn_actor_from_class(
    unreal.SkyAtmosphere,
    unreal.Vector(),
    unreal.Rotator()
)

mark_actor(sky, "PhysicsLab_Sky")


# ------------------------------------------------------------
# Persistent camera
# ------------------------------------------------------------

camera_location = unreal.Vector(
    -750.0,
    -550.0,
    400.0
)

camera_rotation = unreal.Rotator(
    roll=0.0,
    pitch=-17.88,
    yaw=36.25
)

camera = actor_editor.spawn_actor_from_class(
    unreal.CameraActor,
    camera_location,
    camera_rotation
)

mark_actor(
    camera,
    "PhysicsLab_Camera",
    [CAMERA_TAG]
)


# ------------------------------------------------------------
# Put the editor viewport at the same useful laboratory view.
# ------------------------------------------------------------

try:
    unreal_editor = unreal.get_editor_subsystem(
        unreal.UnrealEditorSubsystem
    )

    unreal_editor.set_level_viewport_camera_info(
        camera_location,
        camera_rotation
    )
except Exception as exc:
    unreal.log_warning(
        "Could not set editor viewport camera: {}".format(exc)
    )


# ------------------------------------------------------------
# Save map
# ------------------------------------------------------------

if not level_editor.save_current_level():
    raise RuntimeError("Failed to save PhysicsLab level.")

unreal.log(
    "TextGame: persistent Physics Lab scene saved."
)
