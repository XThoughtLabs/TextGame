import unreal

LEVEL_PATH = "/Game/Maps/PhysicsLab"

asset_library = unreal.EditorAssetLibrary
level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

asset_library.make_directory("/Game/Maps")

if asset_library.does_asset_exist(LEVEL_PATH):
    unreal.log("PhysicsLab level already exists. Loading it.")

    if not level_editor.load_level(LEVEL_PATH):
        raise RuntimeError("Failed to load existing PhysicsLab level.")
else:
    unreal.log("Creating blank PhysicsLab level.")

    if not level_editor.new_level(LEVEL_PATH, False):
        raise RuntimeError("Failed to create PhysicsLab level.")

unreal.log("TextGame: PhysicsLab level is ready.")
