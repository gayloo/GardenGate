<<<<<<< HEAD
#pragma once
#include <string>
#include <vector>

#include "json.hpp"

struct GameConfig {
    std::string game_path;
    std::string custom_args;
    bool moddata_enabled = true;
    std::vector<std::string> moddata_items;
    std::string moddata_selected = "Default";

    void from_json(const nlohmann::json& j);
    nlohmann::json to_json() const;
};

struct Config {
    std::string username;
    std::string server_ip;
    std::string password;

    std::string game_selected = "GW1";

    GameConfig gw1;
    GameConfig gw2;

    void set_username(const char* val) {
        username = val;
        if (username.length() > 15) username.resize(15);
    }

    GameConfig& get_current_game() {
        return (game_selected == "GW2") ? gw2 : gw1;
    }

    const GameConfig& get_current_game() const {
        return (game_selected == "GW2") ? gw2 : gw1;
    }

    int get_game_selected_int() const {
        return (game_selected == "GW2") ? 1 : 0;
    }

    void set_game_selected_from_int(int val) {
        game_selected = (val == 1) ? "GW2" : "GW1";
    }
};

Config load_config(const std::string& file);
void save_config(const Config& cfg, const std::string& file);
=======
#pragma once
#include <string>
#include <vector>
#include <imgui.h>

#include "json.hpp"

struct GameConfig {
    std::string game_path;
    std::string custom_args;
    bool moddata_enabled = true;
    std::vector<std::string> moddata_items;
    std::string moddata_selected = "Default";

    void from_json(const nlohmann::json& j);
    nlohmann::json to_json() const;
};

struct Config {
    std::string username;
    std::string server_ip;
    std::string password;

    std::string game_selected = "gw1";
    bool launch_directly = false;

    ImVec4 baseColor = ImVec4(0.25f, 0.47f, 0.32f, 1.0f);
    ImVec4 baseHover = ImVec4(0.30f, 0.55f, 0.38f, 1.0f);
    ImVec4 baseActive = ImVec4(0.20f, 0.40f, 0.28f, 1.0f);

    GameConfig gw1;
    GameConfig gw2;
    GameConfig bfn;

    void set_username(const char* val) {
        username = val;
        if (username.length() > 15) username.resize(15);
    }

    GameConfig& get_current_game() {
        if (game_selected == "gw2") return gw2;
        if (game_selected == "bfn") return bfn;
        return gw1;
    }

    const GameConfig& get_current_game() const {
        if (game_selected == "gw2") return gw2;
        if (game_selected == "bfn") return bfn;
        return gw1;
    }

    int get_game_selected_int() const {
        if (game_selected == "gw2") return 1;
        if (game_selected == "bfn") return 2;
        return 0;
    }

    void set_game_selected_from_int(int val) {
        if (val == 1) game_selected = "gw2";
        else if (val == 2) game_selected = "bfn";
        else game_selected = "gw1";
    }
};

Config load_config(const std::string& file);
void save_config(const Config& cfg, const std::string& file);
>>>>>>> 661e310 (- Add more useful stuff to the launcher)
