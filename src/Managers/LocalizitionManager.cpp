#include "../Managers/LocalizitionManager.h"

Language Localization::currentLanguage = Language::English;
bool Localization::initialized = false;

std::unordered_map<std::string, std::string> Localization::en;
std::unordered_map<std::string, std::string> Localization::sk;

void Localization::Initialize() {
    if (initialized) return;
    initialized = true;

    // -------------------------
    // ENGLISH
    // -------------------------
    en["lang.english"] = "English";
    en["lang.slovak"] = "Slovak";
    en["control_panel"] = "Control Panel";
    en["language"] = "Language";
    en["asset_management"] = "Asset Management:";
    en["import_asset"] = "Import Asset (F8)";
    en["import_asset_tooltip"] = "Open Asset Importer to load textures dynamically";
    en["debug_console"] = "Debug Console";
    en["clear_console"] = "Clear Console";
    en["messages_count"] = "Messages: %zu";
    en["asset_console"] = "Asset Console";
    en["import_new_asset"] = "+ Import New Asset";
    en["add_textures_without_hardcoding"] = "Add textures without hardcoding";
    en["search"] = "Search";
    en["category"] = "Category";
    en["type"] = "Type";
    en["thumb_size"] = "Thumb Size";
    en["assets_count"] = "Assets: %d";
    en["console_size"] = "Console Size";
    en["console_size_tooltip"] = "Adjust Asset Console Height";
    en["categories"] = "Categories";
    en["all"] = "All";
    en["no_assets_found"] = "No assets found";
    en["found_assets"] = "Found %zu assets";
    en["not_loaded"] = "Not loaded";
    en["selected_asset_details"] = "Selected Asset Details";
    en["selected_frame"] = "Selected Frame";
    en["name"] = "Name: %s";
    en["id"] = "ID: %s";
    en["type_value"] = "Type: %s";
    en["category_value"] = "Category: %s";
    en["path"] = "Path: %s";
    en["texture_size"] = "Texture Size: %dx%d";
    en["selected_frame_label"] = "Selected Frame:";
    en["size"] = "Size: %.0fx%.0f";
    en["position"] = "Position: (%.0f, %.0f)";
    en["frame_col_row"] = "Frame: Column %d, Row %d";
    en["tileset_tiles"] = "Tileset Tiles: %zu";
    en["grid_tiles"] = "Grid: %dx%d tiles";
    en["not_loaded_caps"] = "NOT LOADED";
    en["select_tile"] = "Select Tile";
    en["use_asset"] = "Use Asset";
    en["remove_asset"] = "Remove Asset";
    en["edit_animations"] = "Edit Animations";
    en["show_tileset"] = "Show Tileset";
    en["placed_asset_settings"] = "Placed Asset Settings:";
    en["no_instances_placed"] = "No instances placed yet";
    en["placed_instances"] = "Placed instances: %d";
    en["select_in_scene"] = "Select in Scene";
    en["selected_instance"] = "Selected Instance:";
    en["layer"] = "Layer: %d";
    en["current_scale"] = "Current Scale: %.2fx";
    en["adjust_scale"] = "Adjust Scale:";
    en["quick_scale"] = "Quick Scale:";
    en["apply_all_instances"] = "Apply to All Instances:";
    en["scale_all_to_match"] = "Scale All to Match";
    en["deselect"] = "Deselect";
    en["select_instance_help"] = "Click 'Select in Scene' button,\nthen click on an asset instance\nto adjust its scale";
    en["all_instances"] = "All Instances:";
    en["render_settings"] = "Render Settings:";
    en["show_grid"] = "Show Grid";
    en["show_ui"] = "Show UI";
    en["tile_tools"] = "Tile Tools:";
    en["current_layer"] = "Current Layer: %d";
    en["total_layers"] = "Total Layers: %d";
    en["add_layer"] = "Add Layer";
    en["remove_layer"] = "Remove Layer";
    en["placement_tools"] = "Placement Tools:";
    en["place_player"] = "Place Player";
    en["remove_player"] = "Remove Player";
    en["place_enemy"] = "Place Enemy";
    en["remove_enemy"] = "Remove Enemy";
    en["place_asset"] = "Place Asset";
    en["remove_placed_asset"] = "Remove Asset";
    en["play_mode"] = "Play Mode";
    en["edit_mode"] = "Edit Mode";
    en["current_tool"] = "Current Tool:";
    en["tool_none"] = "None";
    en["tool_placing_tile"] = "Placing Tile";
    en["tool_removing_tile"] = "Removing Tile";
    en["tool_placing_player"] = "Placing Player";
    en["tool_removing_player"] = "Removing Player";
    en["tool_placing_enemy"] = "Placing Enemy";
    en["tool_removing_enemy"] = "Removing Enemy";
    en["tool_placing_asset"] = "Placing Asset";
    en["tool_removing_asset"] = "Removing Asset";
    en["enemies_count"] = "Enemies: %d/10";
    en["clear_tool"] = "Clear Tool";
    en["project"] = "Project:";
    en["path_input"] = "Path:";
    en["save_project"] = "Save Project";
    en["load_project"] = "Load Project";
    en["player_camera_settings"] = "Player Camera Settings:";
    en["place_player_for_camera_preview"] = "Place a player to see camera preview";
    en["camera_preview_visible"] = "Camera preview visible around player";
    en["resolution"] = "Resolution:";
    en["current_resolution"] = "Current: %dx%d (%.1f:1)";
    en["custom"] = "Custom";
    en["aspect_ratio"] = "Aspect Ratio: %s";
    en["scenes"] = "Scenes";
    en["scene_options"] = "Scene options";
    en["apply_rename"] = "Apply rename";
    en["starting_scene"] = "Starting scene";
    en["welcome"] = "Welcome To Click-Craft Creator";


    en["layers"] = "Layers:";
    en["solid"] = "Solid";
    en["no_tile_at_position"] = "No tile at this position";
    en["tileset_preview_header"] = "Tileset: %s (%zu sub-sprites)";

    en["importer.window_title"] = "Asset Importer";
    en["importer.subtitle"] = "Import textures into your game without hardcoding!";
    en["importer.step1"] = "1. Select PNG File";
    en["importer.search_directory"] = "Search Directory:";
    en["importer.scan"] = "Scan";
    en["importer.found_png_files"] = "Found %d PNG files";
    en["importer.texture_preview"] = "Texture Preview";
    en["importer.step2"] = "2. Frame Settings";
    en["importer.step3"] = "3. Asset Details";
    en["importer.asset_name"] = "Asset Name:";
    en["importer.asset_id"] = "Asset ID:";
    en["importer.auto"] = "Auto";
    en["importer.asset_type"] = "Asset Type:";
    en["importer.tileset_settings"] = "Tileset Settings:";
    en["importer.error_asset_name_required"] = "Asset name is required";
    en["importer.error_asset_id_required"] = "Asset ID is required";
    en["importer.error_no_texture_loaded"] = "No texture loaded";
    en["importer.create_asset"] = "Create Asset";
    en["importer.asset_type.individual_texture"] = "Individual Texture";
    en["importer.asset_type.static_spritesheet"] = "Static Spritesheet";
    en["importer.asset_type.animated_spritesheet"] = "Animated Spritesheet";
    en["importer.asset_type.tileset"] = "Tileset";
    en["importer.asset_type.player"] = "Player";
    en["importer.asset_type.enemy"] = "Enemy";

    // Asset types
    en["asset_type.all"] = "All";
    en["asset_type.texture"] = "Texture";
    en["asset_type.animated_sprite"] = "Animated Sprite";
    en["asset_type.static_sprite"] = "Static Sprite";
    en["asset_type.tileset"] = "Tileset";
    en["asset_type.player"] = "Player";
    en["asset_type.enemy"] = "Enemy";
    en["asset_type.other"] = "Other";

    // -------------------------
    // SLOVAK
    // -------------------------
    sk["lang.english"] = "Angličtina";
    sk["lang.slovak"] = "Slovenčina";
    sk["control_panel"] = "Ovládací panel";
    sk["language"] = "Jazyk";
    sk["asset_management"] = "Správa assetov:";
    sk["import_asset"] = "Importovať asset (F8)";
    sk["import_asset_tooltip"] = "Otvoriť importér assetov na dynamické načítanie textúr";
    sk["debug_console"] = "Info konzola";
    sk["clear_console"] = "Vymazať konzolu";
    sk["messages_count"] = "Správy: %zu";
    sk["asset_console"] = "Konzola assetov";
    sk["import_new_asset"] = "+ Importovať nový asset";
    sk["add_textures_without_hardcoding"] = "Pridávajte textúry bez hardcodovania";
    sk["search"] = "Hľadať";
    sk["category"] = "Kategória";
    sk["type"] = "Typ";
    sk["thumb_size"] = "Veľkosť náhľadu";
    sk["assets_count"] = "Assety: %d";
    sk["console_size"] = "Veľkosť konzoly";
    sk["console_size_tooltip"] = "Upraviť výšku konzoly assetov";
    sk["categories"] = "Kategórie";
    sk["all"] = "Všetko";
    sk["no_assets_found"] = "Nenašli sa žiadne assety";
    sk["found_assets"] = "Nájdené assety: %zu";
    sk["not_loaded"] = "Nenačítané";
    sk["selected_asset_details"] = "Detaily vybraného assetu";
    sk["selected_frame"] = "Vybraný frame";
    sk["name"] = "Názov: %s";
    sk["id"] = "ID: %s";
    sk["type_value"] = "Typ: %s";
    sk["category_value"] = "Kategória: %s";
    sk["path"] = "Cesta: %s";
    sk["texture_size"] = "Veľkosť textúry: %dx%d";
    sk["selected_frame_label"] = "Vybraný frame:";
    sk["size"] = "Veľkosť: %.0fx%.0f";
    sk["position"] = "Pozícia: (%.0f, %.0f)";
    sk["frame_col_row"] = "Frame: stĺpec %d, riadok %d";
    sk["tileset_tiles"] = "Dlaždice tilesetu: %zu";
    sk["grid_tiles"] = "Mriežka: %dx%d dlaždíc";
    sk["not_loaded_caps"] = "NENAČÍTANÉ";
    sk["select_tile"] = "Vybrať dlaždicu";
    sk["use_asset"] = "Použiť asset";
    sk["remove_asset"] = "Odstrániť asset";
    sk["edit_animations"] = "Upraviť animácie";
    sk["show_tileset"] = "Zobraziť tileset";
    sk["placed_asset_settings"] = "Nastavenia umiestneného assetu:";
    sk["no_instances_placed"] = "Zatiaľ nie sú umiestnené žiadne inštancie";
    sk["placed_instances"] = "Umiestnené inštancie: %d";
    sk["select_in_scene"] = "Vybrať v scéne";
    sk["selected_instance"] = "Vybraná inštancia:";
    sk["layer"] = "Vrstva: %d";
    sk["current_scale"] = "Aktuálna mierka: %.2fx";
    sk["adjust_scale"] = "Upraviť mierku:";
    sk["quick_scale"] = "Rýchla mierka:";
    sk["apply_all_instances"] = "Použiť na všetky inštancie:";
    sk["scale_all_to_match"] = "Nastaviť všetkým rovnakú mierku";
    sk["deselect"] = "Zrušiť výber";
    sk["select_instance_help"] = "Klikni na tlačidlo 'Vybrať v scéne',\npotom klikni na inštanciu assetu,\nak chceš upraviť jej mierku";
    sk["all_instances"] = "Všetky inštancie:";
    sk["render_settings"] = "Nastavenia vykresľovania:";
    sk["show_grid"] = "Zobraziť mriežku";
    sk["show_ui"] = "Zobraziť UI";
    sk["tile_tools"] = "Nástroje dlaždíc:";
    sk["current_layer"] = "Aktuálna vrstva: %d";
    sk["total_layers"] = "Celkový počet vrstiev: %d";
    sk["add_layer"] = "Pridať vrstvu";
    sk["remove_layer"] = "Odstrániť vrstvu";
    sk["placement_tools"] = "Nástroje umiestňovania:";
    sk["place_player"] = "Umiestniť hráča";
    sk["remove_player"] = "Odstrániť hráča";
    sk["place_enemy"] = "Umiestniť nepriateľa";
    sk["remove_enemy"] = "Odstrániť nepriateľa";
    sk["place_asset"] = "Umiestniť asset";
    sk["remove_placed_asset"] = "Odstrániť asset";
    sk["play_mode"] = "Režim hry";
    sk["edit_mode"] = "Editačný režim";
    sk["current_tool"] = "Aktuálny nástroj:";
    sk["tool_none"] = "Žiadny";
    sk["tool_placing_tile"] = "Umiestňovanie dlaždíc";
    sk["tool_removing_tile"] = "Odstraňovanie dlaždíc";
    sk["tool_placing_player"] = "Umiestňovanie hráča";
    sk["tool_removing_player"] = "Odstraňovanie hráča";
    sk["tool_placing_enemy"] = "Umiestňovanie nepriateľa";
    sk["tool_removing_enemy"] = "Odstraňovanie nepriateľa";
    sk["tool_placing_asset"] = "Umiestňovanie assetu";
    sk["tool_removing_asset"] = "Odstraňovanie assetu";
    sk["enemies_count"] = "Nepriatelia: %d/10";
    sk["clear_tool"] = "Vymazať nástroj";
    sk["project"] = "Projekt:";
    sk["path_input"] = "Cesta:";
    sk["save_project"] = "Uložiť projekt";
    sk["load_project"] = "Načítať projekt";
    sk["player_camera_settings"] = "Nastavenia kamery hráča:";
    sk["place_player_for_camera_preview"] = "Umiestni hráča pre náhľad kamery";
    sk["camera_preview_visible"] = "Náhľad kamery je viditeľný okolo hráča";
    sk["resolution"] = "Rozlíšenie:";
    sk["current_resolution"] = "Aktuálne: %dx%d (%.1f:1)";
    sk["custom"] = "Vlastné";
    sk["aspect_ratio"] = "Pomer strán: %s";
    sk["scenes"] = "Scény";
    sk["scene_options"] = "Možnosti scény";
    sk["apply_rename"] = "Použiť premenovanie";
    sk["starting_scene"] = "Počiatočná scéna";
    sk["welcome"] = "Vitaj v Click-Craft Creator";


    sk["layers"] = "Vrstvy:";
    sk["solid"] = "Pevná kolízia";
    sk["no_tile_at_position"] = "Na tejto pozícii nie je žiadna dlaždica";
    sk["tileset_preview_header"] = "Tileset: %s (%zu pod-spritov)";

    sk["importer.window_title"] = "Importér assetov";
    sk["importer.subtitle"] = "Importujte textúry do hry bez hardcodovania!";
    sk["importer.step1"] = "1. Vyber PNG súbor";
    sk["importer.search_directory"] = "Hľadaný priečinok:";
    sk["importer.scan"] = "Skenovať";
    sk["importer.found_png_files"] = "Nájdených PNG súborov: %d";
    sk["importer.texture_preview"] = "Náhľad textúry";
    sk["importer.step2"] = "2. Nastavenia framu";
    sk["importer.step3"] = "3. Detaily assetu";
    sk["importer.asset_name"] = "Názov assetu:";
    sk["importer.asset_id"] = "ID assetu:";
    sk["importer.auto"] = "Automaticky";
    sk["importer.asset_type"] = "Typ assetu:";
    sk["importer.tileset_settings"] = "Nastavenia tilesetu:";
    sk["importer.error_asset_name_required"] = "Názov assetu je povinný";
    sk["importer.error_asset_id_required"] = "ID assetu je povinné";
    sk["importer.error_no_texture_loaded"] = "Nie je načítaná žiadna textúra";
    sk["importer.create_asset"] = "Vytvoriť asset";
    sk["importer.asset_type.individual_texture"] = "Samostatná textúra";
    sk["importer.asset_type.static_spritesheet"] = "Statický spritesheet";
    sk["importer.asset_type.animated_spritesheet"] = "Animovaný spritesheet";
    sk["importer.asset_type.tileset"] = "Tileset";
    sk["importer.asset_type.player"] = "Hráč";
    sk["importer.asset_type.enemy"] = "Nepriateľ";

    sk["asset_type.all"] = "Všetko";
    sk["asset_type.texture"] = "Textúra";
    sk["asset_type.animated_sprite"] = "Animovaný sprite";
    sk["asset_type.static_sprite"] = "Statický sprite";
    sk["asset_type.tileset"] = "Tileset";
    sk["asset_type.player"] = "Hráč";
    sk["asset_type.enemy"] = "Nepriateľ";
    sk["asset_type.other"] = "Iné";
}

void Localization::SetLanguage(Language lang) {
    Initialize();
    currentLanguage = lang;
}

Language Localization::GetLanguage() {
    Initialize();
    return currentLanguage;
}

const char* Localization::Get(const std::string& key) {
    return GetString(key).c_str();
}

const std::string& Localization::GetString(const std::string& key) {
    Initialize();

    auto& map = (currentLanguage == Language::Slovak) ? sk : en;
    auto it = map.find(key);
    if (it != map.end()) return it->second;

    auto fallback = en.find(key);
    if (fallback != en.end()) return fallback->second;

    static std::string missing = "MISSING:" + key;
    missing = "MISSING:" + key;
    return missing;
}

const char* Localization::GetLanguageDisplayName(Language lang) {
    switch (lang) {
        case Language::English: return Get("lang.english");
        case Language::Slovak:  return Get("lang.slovak");
        default: return "Unknown";
    }
}