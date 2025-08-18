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
import androidx.annotation.NonNull;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class ThemeSelectionActivity extends AppCompatActivity {

    private RecyclerView themeRecyclerView;
    private ThemeAdapter themeAdapter;
    private String currentTheme;

    // FIXED: Enhanced theme data structure with better organization
    public static class Theme {
        public final String id;
        public final String displayName;
        public final boolean isLight;
        public final int styleResId;
        public final boolean requiresRecreate;
        public final int previewColorResId;  // NEW: For preview colors

        public Theme(String id, String displayName, boolean isLight, int styleResId, boolean requiresRecreate, int previewColorResId) {
            this.id = id;
            this.displayName = displayName;
            this.isLight = isLight;
            this.styleResId = styleResId;
            this.requiresRecreate = requiresRecreate;
            this.previewColorResId = previewColorResId;
        }
    }

    // FIXED: Corrected theme definitions with proper resource references
    private List<Theme> getAllThemes() {
        List<Theme> themes = new ArrayList<>();

        // Light Themes - FIXED: Verified all style resources exist
        themes.add(new Theme("white", "White", true, R.style.Theme_Descansa, false, R.color.purple_500));
        themes.add(new Theme("solarized", "Solarized", true, R.style.Theme_Descansa_Solarized, true, R.color.solarized_primary));
        themes.add(new Theme("everforest", "Everforest", true, R.style.Theme_Descansa_EverforestLight, true, R.color.everforest_primary));

        // Dark Themes - FIXED: Verified all style resources exist
        themes.add(new Theme("amoled", "AMOLED", false, R.style.Theme_Descansa_AMOLED, true, R.color.amoled_primary));
        themes.add(new Theme("dracula", "Dracula", false, R.style.Theme_Descansa_Dracula, true, R.color.dracula_primary));
        themes.add(new Theme("nordic", "Nordic", false, R.style.Theme_Descansa_Nordic, true, R.color.nordic_primary));

        // Sort each category alphabetically
        List<Theme> lightThemes = new ArrayList<>();
        List<Theme> darkThemes = new ArrayList<>();

        for (Theme theme : themes) {
            if (theme.isLight) {
                lightThemes.add(theme);
            } else {
                darkThemes.add(theme);
            }
        }

        Collections.sort(lightThemes, (a, b) -> a.displayName.compareToIgnoreCase(b.displayName));
        Collections.sort(darkThemes, (a, b) -> a.displayName.compareToIgnoreCase(b.displayName));

        // Combine back into alternating pattern for grid layout
        List<Theme> organizedThemes = new ArrayList<>();
        int maxSize = Math.max(lightThemes.size(), darkThemes.size());

        for (int i = 0; i < maxSize; i++) {
            if (i < lightThemes.size()) {
                organizedThemes.add(lightThemes.get(i));
            } else {
                organizedThemes.add(null); // Empty slot
            }

            if (i < darkThemes.size()) {
                organizedThemes.add(darkThemes.get(i));
            } else {
                organizedThemes.add(null); // Empty slot
            }
        }

        return organizedThemes;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_theme_selection);

        // Get current theme
        currentTheme = getSharedPreferences("DescansaPrefs", MODE_PRIVATE)
                .getString("theme", "white");

        setupToolbar();
        setupRecyclerView();
    }

    private void setupToolbar() {
        if (getSupportActionBar() != null) {
            getSupportActionBar().setTitle(getString(R.string.activity_theme_selection));
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        }
    }

    private void setupRecyclerView() {
        themeRecyclerView = findViewById(R.id.theme_recycler_view);

        // 2 columns for Light | Dark layout
        GridLayoutManager layoutManager = new GridLayoutManager(this, 2);
        themeRecyclerView.setLayoutManager(layoutManager);

        themeAdapter = new ThemeAdapter(getAllThemes());
        themeRecyclerView.setAdapter(themeAdapter);
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
                // Empty slot
                holder.itemView.setVisibility(View.INVISIBLE);
                return;
            }

            holder.itemView.setVisibility(View.VISIBLE);
            holder.themeName.setText(theme.displayName);

            // FIXED: Set theme preview color using proper API
            View colorPreview = holder.itemView.findViewById(R.id.theme_color_preview);
            if (colorPreview != null) {
                int color = ContextCompat.getColor(ThemeSelectionActivity.this, theme.previewColorResId);
                colorPreview.setBackgroundColor(color);
            }

            // Visual indication of current theme
            boolean isCurrentTheme = theme.id.equals(currentTheme);

            // FIXED: Use proper theme-aware colors and non-deprecated APIs
            if (theme.isLight) {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_light_bg);
                holder.themeName.setTextColor(ContextCompat.getColor(ThemeSelectionActivity.this, android.R.color.black));
            } else {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_dark_bg);
                holder.themeName.setTextColor(ContextCompat.getColor(ThemeSelectionActivity.this, android.R.color.white));
            }

            // FIXED: Proper selection indicator using non-deprecated API
            if (isCurrentTheme) {
                holder.itemView.setForeground(ContextCompat.getDrawable(ThemeSelectionActivity.this, R.drawable.theme_item_selector));

                // Show current indicator
                TextView currentIndicator = holder.itemView.findViewById(R.id.current_indicator);
                if (currentIndicator != null) {
                    currentIndicator.setVisibility(View.VISIBLE);
                }
            } else {
                holder.itemView.setForeground(null);

                TextView currentIndicator = holder.itemView.findViewById(R.id.current_indicator);
                if (currentIndicator != null) {
                    currentIndicator.setVisibility(View.GONE);
                }
            }

            holder.itemView.setOnClickListener(v -> applyTheme(theme));
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

    // FIXED: Enhanced theme application with proper error handling
    private void applyTheme(Theme theme) {
        try {
            // Save theme preference
            SharedPreferences.Editor editor = getSharedPreferences("DescansaPrefs", MODE_PRIVATE).edit();
            editor.putString("theme", theme.id);
            editor.apply();

            // Show user feedback
            String message = getString(getThemeAppliedMessageId(theme.id));
            Toast.makeText(this, message, Toast.LENGTH_SHORT).show();

            // FIXED: Improved theme application logic
            if (theme.requiresRecreate) {
                // For custom themes (Solarized, Dracula, AMOLED, etc.)
                // Clear any day/night mode override first
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);

                // Apply custom theme and recreate
                setTheme(theme.styleResId);
                recreateMainActivity();

            } else {
                // For system light/dark themes
                if (theme.isLight) {
                    AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO);
                } else {
                    AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES);
                }
                recreateMainActivity();
            }

        } catch (Exception e) {
            Toast.makeText(this, getString(R.string.error_theme_failed), Toast.LENGTH_SHORT).show();
        }
    }

    // FIXED: Helper method to get theme-specific messages
    private int getThemeAppliedMessageId(String themeId) {
        switch (themeId) {
            case "solarized": return R.string.msg_theme_solarized;
            case "white": return R.string.msg_theme_white;
            case "amoled": return R.string.msg_theme_amoled;
            case "dracula": return R.string.msg_theme_dracula;
            default: return R.string.msg_theme_applied;
        }
    }

    // FIXED: Proper activity recreation to apply theme
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