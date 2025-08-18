// ThemeManager.h - C++11 Theme Preference Management
#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <string>
#include <vector>
#include <fstream>

namespace descansa {

// Theme configuration in C++11
    struct ThemeConfig {
        std::string id;
        std::string display_name;
        bool is_light;
        bool requires_recreate;

        ThemeConfig() : is_light(true), requires_recreate(false) {}

        ThemeConfig(const std::string& theme_id, const std::string& name,
                    bool light, bool recreate)
                : id(theme_id), display_name(name), is_light(light), requires_recreate(recreate) {}
    };

// C++11 Theme Management Class
    class ThemeManager {
    private:
        std::string current_theme_id;
        std::string preferences_file_path;
        std::vector<ThemeConfig> available_themes;

        void initialize_available_themes();
        bool save_preferences() const;
        bool load_preferences();

    public:
        explicit ThemeManager(const std::string& prefs_path = "");
        ~ThemeManager();

        // Theme management
        bool set_current_theme(const std::string& theme_id);
        std::string get_current_theme() const { return current_theme_id; }

        // Theme queries
        bool is_current_theme_light() const;
        bool does_current_theme_require_recreate() const;
        ThemeConfig get_current_theme_config() const;

        // Available themes
        std::vector<ThemeConfig> get_available_themes() const { return available_themes; }
        std::vector<ThemeConfig> get_light_themes() const;
        std::vector<ThemeConfig> get_dark_themes() const;

        // Validation
        bool is_valid_theme(const std::string& theme_id) const;
        std::string get_fallback_theme() const { return "white"; }

        // Statistics
        int get_theme_count() const { return static_cast<int>(available_themes.size()); }

        // Export theme preferences
        bool export_theme_history(const std::string& export_path) const;
    };

} // namespace descansa

#endif // THEME_MANAGER_H