package io.nava.descansa.app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
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

    // Theme data structure
    public static class Theme {
        public final String id;
        public final String displayName;
        public final boolean isLight;
        public final int styleResId;
        public final boolean requiresRecreate;

        public Theme(String id, String displayName, boolean isLight, int styleResId, boolean requiresRecreate) {
            this.id = id;
            this.displayName = displayName;
            this.isLight = isLight;
            this.styleResId = styleResId;
            this.requiresRecreate = requiresRecreate;
        }
    }

    // All available themes - easy to expand
    private List<Theme> getAllThemes() {
        List<Theme> themes = new ArrayList<>();

        // Light Themes
        themes.add(new Theme("white", "White", true, R.style.Theme_Descansa, false));
        themes.add(new Theme("solarized", "Solarized", true, R.style.Theme_Descansa_Solarized, true));
        themes.add(new Theme("everforest", "Everforest", true, R.style.Theme_Descansa_EverforestLight, true));

        // Dark Themes
        themes.add(new Theme("amoled", "AMOLED", false, R.style.Theme_Descansa_AMOLED, true));
        themes.add(new Theme("dracula", "Dracula", false, R.style.Theme_Descansa_Dracula, true));
        themes.add(new Theme("nordic", "Nordic", false, R.style.Theme_Descansa_Nordic, true));

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
            getSupportActionBar().setTitle("Choose Theme");
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

            // Visual indication of current theme
            boolean isCurrentTheme = theme.id.equals(currentTheme);
            holder.itemView.setSelected(isCurrentTheme);

            // Set background based on theme type with preview colors
            // Set background based on theme type with preview colors
            if (theme.isLight) {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_light_bg);
                holder.themeName.setTextColor(getResources().getColor(android.R.color.black, null));
            } else {
                holder.itemView.setBackgroundResource(R.drawable.theme_item_dark_bg);
                holder.themeName.setTextColor(getResources().getColor(android.R.color.white, null));
            }

            // Add selection indicator without changing background
            if (isCurrentTheme) {
                // Option A: Add a border drawable on top
                holder.itemView.setForeground(getResources().getDrawable(R.drawable.theme_item_selector, null));

                // Option B: Show a checkmark icon (if you have one in your layout)
                // holder.checkmarkIcon.setVisibility(View.VISIBLE);
            } else {
                holder.itemView.setForeground(null);
                // holder.checkmarkIcon.setVisibility(View.GONE);
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

    private void applyTheme(Theme theme) {
        SharedPreferences.Editor editor = getSharedPreferences("DescansaPrefs", MODE_PRIVATE).edit();
        editor.putString("theme", theme.id);
        editor.apply();  // Use commit() if immediate persistence is needed

        // Clear day/night mode override first
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);

        if (theme.requiresRecreate) {
            // For custom themes (Solarized, Dracula, AMOLED)
            setTheme(theme.styleResId);
            recreate();  // Force activity recreation
        } else {
            // For light/dark system themes
            if (theme.isLight) {
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO);
            } else {
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES);
            }
        }

        // Apply to all activities
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