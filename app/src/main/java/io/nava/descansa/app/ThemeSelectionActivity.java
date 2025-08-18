package io.nava.descansa.app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;
import androidx.annotation.NonNull;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

public class ThemeSelectionActivity extends AppCompatActivity {

    private RecyclerView themeRecyclerView;
    private ThemeAdapter themeAdapter;
    private String currentTheme;
    private Button shuffleButton;

    // ENHANCED: Theme data structure with smart options
    public static class Theme {
        public final String id;
        public final String displayName;
        public final boolean isLight;
        public final int styleResId;
        public final boolean requiresRecreate;
        public final int previewColorResId;
        public final boolean isSmartOption; // NEW: For system/random options
        public final String description;     // NEW: Theme description

        public Theme(String id, String displayName, boolean isLight, int styleResId,
                     boolean requiresRecreate, int previewColorResId, boolean isSmartOption, String description) {
            this.id = id;
            this.displayName = displayName;
            this.isLight = isLight;
            this.styleResId = styleResId;
            this.requiresRecreate = requiresRecreate;
            this.previewColorResId = previewColorResId;
            this.isSmartOption = isSmartOption;
            this.description = description;
        }

        // Convenience constructor for regular themes
        public Theme(String id, String displayName, boolean isLight, int styleResId,
                     boolean requiresRecreate, int previewColorResId) {
            this(id, displayName, isLight, styleResId, requiresRecreate, previewColorResId, false, "");
        }
    }

    // ENHANCED: Complete theme catalog with smart options
    private List<Theme> getAllThemes() {
        List<Theme> themes = new ArrayList<>();

        // SMART OPTIONS (displayed at top)
        themes.add(new Theme("follow_system", "Follow System", true, R.style.Theme_Descansa,
                false, R.color.purple_500, true, "Adapts to system dark/light mode"));

        themes.add(new Theme("random", "Random", true, R.style.Theme_Descansa,
                true, R.color.purple_500, true, "Surprise me with any theme"));

        themes.add(new Theme("random_light", "Random Light", true, R.style.Theme_Descansa,
                true, R.color.purple_500, true, "Random from light themes"));

        themes.add(new Theme("random_dark", "Random Dark", false, R.style.Theme_Descansa,
                true, R.color.purple_500, true, "Random from dark themes"));

        // REGULAR THEMES - Light
        themes.add(new Theme("white", "White", true, R.style.Theme_Descansa,
                false, R.color.purple_500, false, "Clean and minimal"));

        themes.add(new Theme("solarized", "Solarized", true, R.style.Theme_Descansa_Solarized,
                true, R.color.solarized_primary, false, "Warm precision colors"));

        themes.add(new Theme("rose_pine", "Rose Pine", true, R.style.Theme_Descansa_RosePine,
                true, R.color.rose_primary, false, "Warm pink and purple tones"));

        // REGULAR THEMES - Dark
        themes.add(new Theme("amoled", "AMOLED", false, R.style.Theme_Descansa_AMOLED,
                true, R.color.amoled_primary, false, "Pure black for OLED"));

        themes.add(new Theme("dracula", "Dracula", false, R.style.Theme_Descansa_Dracula,
                true, R.color.dracula_primary, false, "Dark vampire colors"));

        themes.add(new Theme("nordic", "Nordic", false, R.style.Theme_Descansa_Nordic,
                true, R.color.nordic_primary, false, "Cool frost palette"));

        return organizeThemesForDisplay(themes);
    }

    // ENHANCED: Organize themes for optimal display
    private List<Theme> organizeThemesForDisplay(List<Theme> allThemes) {
        List<Theme> smartOptions = new ArrayList<>();
        List<Theme> lightThemes = new ArrayList<>();
        List<Theme> darkThemes = new ArrayList<>();

        // Separate themes by type
        for (Theme theme : allThemes) {
            if (theme.isSmartOption) {
                smartOptions.add(theme);
            } else if (theme.isLight) {
                lightThemes.add(theme);
            } else {
                darkThemes.add(theme);
            }
        }

        // Sort regular themes alphabetically
        Collections.sort(lightThemes, (a, b) -> a.displayName.compareToIgnoreCase(b.displayName));
        Collections.sort(darkThemes, (a, b) -> a.displayName.compareToIgnoreCase(b.displayName));

        // Organize for 2-column grid: Smart options first, then light|dark alternating
        List<Theme> organizedThemes = new ArrayList<>();

        // Add smart options (full width pairs)
        for (int i = 0; i < smartOptions.size(); i += 2) {
            organizedThemes.add(smartOptions.get(i));
            if (i + 1 < smartOptions.size()) {
                organizedThemes.add(smartOptions.get(i + 1));
            } else {
                organizedThemes.add(null); // Empty slot
            }
        }

        // Add light|dark alternating pairs
        int maxRegularThemes = Math.max(lightThemes.size(), darkThemes.size());
        for (int i = 0; i < maxRegularThemes; i++) {
            if (i < lightThemes.size()) {
                organizedThemes.add(lightThemes.get(i));
            } else {
                organizedThemes.add(null);
            }

            if (i < darkThemes.size()) {
                organizedThemes.add(darkThemes.get(i));
            } else {
                organizedThemes.add(null);
            }
        }

        return organizedThemes;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_theme_selection);

        // Get current theme (might be a resolved random theme)
        currentTheme = getActualCurrentTheme();

        setupToolbar();
        setupShuffleButton();
        setupRecyclerView();
    }

    private void setupToolbar() {
        if (getSupportActionBar() != null) {
            getSupportActionBar().setTitle("Choose Theme");
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        }
    }

    // NEW: Shuffle button for random themes
    private void setupShuffleButton() {
        // Add shuffle button to layout (you'd need to add this to your XML)
        // For now, we'll create it programmatically
        shuffleButton = new Button(this);
        shuffleButton.setText("🎲 Shuffle Random");
        shuffleButton.setOnClickListener(v -> shuffleRandomTheme());

        // You would add this to your layout XML instead:
        // shuffleButton = findViewById(R.id.shuffle_button);
    }

    private void setupRecyclerView() {
        themeRecyclerView = findViewById(R.id.theme_recycler_view);

        GridLayoutManager layoutManager = new GridLayoutManager(this, 2);
        themeRecyclerView.setLayoutManager(layoutManager);

        themeAdapter = new ThemeAdapter(getAllThemes());
        themeRecyclerView.setAdapter(themeAdapter);
    }

    // NEW: Get the actual applied theme (resolve random selections)
    private String getActualCurrentTheme() {
        SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
        String storedTheme = prefs.getString("theme", "white");

        // If it's a random theme, get the resolved theme
        if (storedTheme.startsWith("random")) {
            return prefs.getString("resolved_theme", "white");
        }

        return storedTheme;
    }

    // NEW: Shuffle random theme manually
    private void shuffleRandomTheme() {
        SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
        String currentSelection = prefs.getString("theme", "white");

        if (currentSelection.startsWith("random")) {
            // Re-apply the same random type to get a new selection
            applyTheme(findThemeById(currentSelection));

            // Update the adapter to show new current theme
            currentTheme = getActualCurrentTheme();
            themeAdapter.notifyDataSetChanged();

            Toast.makeText(this, "🎲 New random theme applied!", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, "Select a random theme option first", Toast.LENGTH_SHORT).show();
        }
    }

    // Helper to find theme by ID
    private Theme findThemeById(String themeId) {
        for (Theme theme : getAllThemes()) {
            if (theme != null && theme.id.equals(themeId)) {
                return theme;
            }
        }
        return null;
    }

    private class ThemeAdapter extends RecyclerView.Adapter<ThemeAdapter.ThemeViewHolder> {
        private final List<Theme> themes;

        public ThemeAdapter(List<Theme> themes) {
            this.themes = themes;
        }

        @NonNull
        @Override
        public ThemeViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.item_theme, parent, false);
            return new ThemeViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull ThemeViewHolder holder, int position) {
            Theme theme = themes.get(position);

            if (theme == null) {
                holder.itemView.setVisibility(View.INVISIBLE);
                return;
            }

            holder.itemView.setVisibility(View.VISIBLE);
            holder.themeName.setText(theme.displayName);

            // Set theme preview color
            View colorPreview = holder.itemView.findViewById(R.id.theme_color_preview);
            if (colorPreview != null) {
                int color = ContextCompat.getColor(ThemeSelectionActivity.this, theme.previewColorResId);
                colorPreview.setBackgroundColor(color);

                // ENHANCED: Special preview for smart options
                if (theme.isSmartOption) {
                    // Add a gradient or pattern to indicate it's a smart option
                    if (theme.id.equals("follow_system")) {
                        // Use a gradient representing system adaptation
                        colorPreview.setBackgroundResource(R.drawable.gradient_system_theme);
                    } else if (theme.id.startsWith("random")) {
                        // Use a rainbow gradient for random options
                        colorPreview.setBackgroundResource(R.drawable.gradient_random_theme);
                    }
                }
            }

            // ENHANCED: Show current theme indicator
            boolean isCurrentTheme = isCurrentlyAppliedTheme(theme);

            // Set background and text color
            if (theme.isSmartOption) {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_smart_bg);
                holder.themeName.setTextColor(ContextCompat.getColor(ThemeSelectionActivity.this, R.color.purple_700));
            } else if (theme.isLight) {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_light_bg);
                holder.themeName.setTextColor(ContextCompat.getColor(ThemeSelectionActivity.this, android.R.color.black));
            } else {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_dark_bg);
                holder.themeName.setTextColor(ContextCompat.getColor(ThemeSelectionActivity.this, android.R.color.white));
            }

            // Show current indicator
            TextView currentIndicator = holder.itemView.findViewById(R.id.current_indicator);
            TextView descriptionText = holder.itemView.findViewById(R.id.theme_description);

            if (currentIndicator != null) {
                currentIndicator.setVisibility(isCurrentTheme ? View.VISIBLE : View.GONE);
            }

            // ENHANCED: Show theme description
            if (descriptionText != null && !theme.description.isEmpty()) {
                descriptionText.setText(theme.description);
                descriptionText.setVisibility(View.VISIBLE);
            } else if (descriptionText != null) {
                descriptionText.setVisibility(View.GONE);
            }

            // Selection indicator
            if (isCurrentTheme) {
                holder.itemView.setForeground(ContextCompat.getDrawable(ThemeSelectionActivity.this, R.drawable.theme_item_selector));
            } else {
                holder.itemView.setForeground(null);
            }

            holder.itemView.setOnClickListener(v -> applyTheme(theme));
        }

        // ENHANCED: Better current theme detection for smart options
        private boolean isCurrentlyAppliedTheme(Theme theme) {
            SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
            String selectedTheme = prefs.getString("theme", "white");
            String resolvedTheme = prefs.getString("resolved_theme", "white");

            if (theme.isSmartOption) {
                // For smart options, check if this smart option is selected
                return theme.id.equals(selectedTheme);
            } else {
                // For regular themes, check if this theme is currently applied
                return theme.id.equals(resolvedTheme) || theme.id.equals(currentTheme);
            }
        }

        @Override
        public int getItemCount() {
            return themes.size();
        }

        class ThemeViewHolder extends RecyclerView.ViewHolder {
            TextView themeName;

            public ThemeViewHolder(@NonNull View itemView) {
                super(itemView);
                themeName = itemView.findViewById(R.id.theme_name);
            }
        }
    }

    // ENHANCED: Smart theme application logic
    private void applyTheme(Theme theme) {
        try {
            SharedPreferences.Editor editor = getSharedPreferences("DescansaPrefs", MODE_PRIVATE).edit();

            // Always save the selected theme preference
            editor.putString("theme", theme.id);

            String actualThemeToApply;
            String message;

            // ENHANCED: Handle smart theme options
            switch (theme.id) {
                case "follow_system":
                    actualThemeToApply = "follow_system";
                    message = "Following system preferences";
                    editor.putString("resolved_theme", "follow_system");
                    break;

                case "random":
                    actualThemeToApply = selectRandomTheme(false, false);
                    message = "Random theme: " + getThemeDisplayName(actualThemeToApply);
                    editor.putString("resolved_theme", actualThemeToApply);
                    break;

                case "random_light":
                    actualThemeToApply = selectRandomTheme(true, false);
                    message = "Random light theme: " + getThemeDisplayName(actualThemeToApply);
                    editor.putString("resolved_theme", actualThemeToApply);
                    break;

                case "random_dark":
                    actualThemeToApply = selectRandomTheme(false, true);
                    message = "Random dark theme: " + getThemeDisplayName(actualThemeToApply);
                    editor.putString("resolved_theme", actualThemeToApply);
                    break;

                default:
                    // Regular theme
                    actualThemeToApply = theme.id;
                    message = theme.displayName + " theme applied";
                    editor.putString("resolved_theme", actualThemeToApply);
                    break;
            }

            editor.apply();

            // Show feedback
            Toast.makeText(this, message, Toast.LENGTH_SHORT).show();

            // Apply the resolved theme
            applyResolvedTheme(actualThemeToApply);

        } catch (Exception e) {
            Toast.makeText(this, "Theme change failed", Toast.LENGTH_SHORT).show();
        }
    }

    // NEW: Select random theme from available options
    private String selectRandomTheme(boolean lightOnly, boolean darkOnly) {
        List<String> availableThemes = new ArrayList<>();

        // Get all regular (non-smart) themes
        for (Theme theme : getAllThemes()) {
            if (theme != null && !theme.isSmartOption) {
                if (lightOnly && theme.isLight) {
                    availableThemes.add(theme.id);
                } else if (darkOnly && !theme.isLight) {
                    availableThemes.add(theme.id);
                } else if (!lightOnly && !darkOnly) {
                    availableThemes.add(theme.id);
                }
            }
        }

        if (availableThemes.isEmpty()) {
            return "white"; // Fallback
        }

        Random random = new Random();
        return availableThemes.get(random.nextInt(availableThemes.size()));
    }

    // NEW: Get display name for a theme ID
    private String getThemeDisplayName(String themeId) {
        for (Theme theme : getAllThemes()) {
            if (theme != null && theme.id.equals(themeId)) {
                return theme.displayName;
            }
        }
        return themeId;
    }

    // ENHANCED: Apply resolved theme with proper handling
    private void applyResolvedTheme(String themeId) {
        switch (themeId) {
            case "follow_system":
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
                break;
            case "white":
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO);
                break;
            case "solarized":
                setTheme(R.style.Theme_Descansa_Solarized);
                break;
            case "everforest":
                setTheme(R.style.Theme_Descansa_EverforestLight);
                break;
            case "amoled":
                setTheme(R.style.Theme_Descansa_AMOLED);
                break;
            case "dracula":
                setTheme(R.style.Theme_Descansa_Dracula);
                break;
            case "nordic":
                setTheme(R.style.Theme_Descansa_Nordic);
                break;
            default:
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
        }

        recreateMainActivity();
    }

    private void recreateMainActivity() {
        Intent intent = new Intent(this, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        finish();
    }

    @Override
    public boolean onSupportNavigateUp() {
        finish();
        return true;
    }
}