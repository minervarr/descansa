// ThemeManager.cpp - Implementation
#include "ThemeManager.h"
#include <algorithm>
#include <sstream>

namespace descansa {

    ThemeManager::ThemeManager(const std::string& prefs_path)
            : current_theme_id("white"),
              preferences_file_path(prefs_path.empty() ? "theme_prefs.txt" : prefs_path) {
        initialize_available_themes();
        load_preferences();
    }

    ThemeManager::~ThemeManager() {
        save_preferences();
    }

    void ThemeManager::initialize_available_themes() {
        available_themes.clear();

        // Light themes
        available_themes.push_back(ThemeConfig("white", "White", true, false));
        available_themes.push_back(ThemeConfig("solarized", "Solarized", true, true));
        available_themes.push_back(ThemeConfig("everforest", "Everforest", true, true));

        // Dark themes
        available_themes.push_back(ThemeConfig("amoled", "AMOLED", false, true));
        available_themes.push_back(ThemeConfig("dracula", "Dracula", false, true));
        available_themes.push_back(ThemeConfig("nordic", "Nordic", false, true));
    }

    bool ThemeManager::set_current_theme(const std::string& theme_id) {
        if (!is_valid_theme(theme_id)) {
            return false;
        }

        current_theme_id = theme_id;
        return save_preferences();
    }

    bool ThemeManager::is_current_theme_light() const {
        for (const auto& theme : available_themes) {
            if (theme.id == current_theme_id) {
                return theme.is_light;
            }
        }
        return true; // Default fallback
    }

    bool ThemeManager::does_current_theme_require_recreate() const {
        for (const auto& theme : available_themes) {
            if (theme.id == current_theme_id) {
                return theme.requires_recreate;
            }
        }
        return false; // Default fallback
    }

    ThemeConfig ThemeManager::get_current_theme_config() const {
        for (const auto& theme : available_themes) {
            if (theme.id == current_theme_id) {
                return theme;
            }
        }
        return ThemeConfig(); // Default fallback
    }

    std::vector<ThemeConfig> ThemeManager::get_light_themes() const {
        std::vector<ThemeConfig> light_themes;
        for (const auto& theme : available_themes) {
            if (theme.is_light) {
                light_themes.push_back(theme);
            }
        }
        return light_themes;
    }

    std::vector<ThemeConfig> ThemeManager::get_dark_themes() const {
        std::vector<ThemeConfig> dark_themes;
        for (const auto& theme : available_themes) {
            if (!theme.is_light) {
                dark_themes.push_back(theme);
            }
        }
        return dark_themes;
    }

    bool ThemeManager::is_valid_theme(const std::string& theme_id) const {
        for (const auto& theme : available_themes) {
            if (theme.id == theme_id) {
                return true;
            }
        }
        return false;
    }

    bool ThemeManager::save_preferences() const {
        std::ofstream file(preferences_file_path);
        if (!file.is_open()) return false;

        file << "CURRENT_THEME:" << current_theme_id << "\n";
        return file.good();
    }

    bool ThemeManager::load_preferences() {
        std::ifstream file(preferences_file_path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.find("CURRENT_THEME:") == 0) {
                std::string theme_id = line.substr(14); // Length of "CURRENT_THEME:"
                if (is_valid_theme(theme_id)) {
                    current_theme_id = theme_id;
                }
            }
        }
        return true;
    }

    bool ThemeManager::export_theme_history(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        file << "# Descansa Theme Configuration Export\n";
        file << "current_theme," << current_theme_id << "\n";
        file << "theme_count," << available_themes.size() << "\n";

        file << "\n# Available Themes\n";
        file << "theme_id,display_name,is_light,requires_recreate\n";
        for (const auto& theme : available_themes) {
            file << theme.id << "," << theme.display_name << ","
                 << (theme.is_light ? "true" : "false") << ","
                 << (theme.requires_recreate ? "true" : "false") << "\n";
        }

        return file.good();
    }

} // namespace descansa