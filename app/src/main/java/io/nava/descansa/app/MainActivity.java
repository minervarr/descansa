package io.nava.descansa.app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;

import android.content.Intent;

import io.nava.descansa.app.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;
    private Handler updateHandler;
    private Runnable updateRunnable;
    private boolean isUpdating = false;

    // UI Elements - Enhanced with new features
    private TextView statusText;
    private TextView currentSessionText;
    private Button sleepButton;

    // Enhanced Information Grid
    private TextView workTimeText;
    private TextView lastSleepText;
    private TextView averageSleepText;
    private TextView sessionCountText;

    // NEW: Enhanced tracking displays
    private TextView sleepScoreText;
    private TextView sleepEfficiencyText;
    private TextView sleepDebtText;
    private TextView goalAdherenceText;

    // Action Buttons
    private Button exportButton;
    private Button settingsButton;
    private Button clearDataButton;
    private Button themeButton;

    // NEW: Enhanced action buttons
    private Button qualityButton;
    private Button recommendationsButton;
    private Button analysisButton;

    // Debug
    private TextView debugText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Apply theme before UI setup
        applyStoredTheme();
        setupSystemUI();

        initializeCore();

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        initializeUI();
        setupButtonListeners();
        startPeriodicUpdates();
        updateUI();
    }

    // ========== ENHANCED INITIALIZATION ==========

    private void initializeCore() {
        String dataPath = getFilesDir().getAbsolutePath() + "/descansa_data.txt";
        initializeCore(dataPath);

        // Initialize enhanced sleep goals
        setEnhancedSleepGoals(8.0, 22, 6, 85.0);
    }

    private void initializeUI() {
        // Basic UI elements
        statusText = binding.statusText;
        currentSessionText = binding.currentSessionText;
        sleepButton = binding.sleepButton;

        // Information grid
        workTimeText = binding.workTimeText;
        lastSleepText = binding.lastSleepText;
        averageSleepText = binding.averageSleepText;
        sessionCountText = binding.sessionCountText;

        // Action buttons
        exportButton = binding.exportButton;
        settingsButton = binding.settingsButton;
        clearDataButton = binding.clearDataButton;
        themeButton = binding.themeButton;

        // Debug
        debugText = binding.debugText;

        // TODO: Add enhanced UI elements when layout is updated
        // These would be added to an enhanced layout
        // sleepScoreText = binding.sleepScoreText;
        // sleepEfficiencyText = binding.sleepEfficiencyText;
        // etc.
    }

    private void setupButtonListeners() {
        sleepButton.setOnClickListener(v -> handleSleepButtonClick());
        exportButton.setOnClickListener(v -> showExportOptionsDialog());
        settingsButton.setOnClickListener(v -> showEnhancedSettingsDialog());
        clearDataButton.setOnClickListener(v -> handleClearData());
        themeButton.setOnClickListener(v -> showThemeSelectionDialog());

        // NEW: Long press for quality rating
        sleepButton.setOnLongClickListener(v -> {
            if (!isSessionRunning()) {
                showSleepQualityDialog();
                return true;
            }
            return false;
        });
    }

    // ========== ENHANCED UI UPDATES ==========

    private void updateUI() {
        updateStatusSection();
        updateInformationGrid();
        updateEnhancedMetrics();
        updateDebugInfo();
    }

    private void updateEnhancedMetrics() {
        // Update sleep score
        double sleepScore = getSleepScore();
        // TODO: Update UI when layout includes these elements
        // sleepScoreText.setText(String.format("%.0f/100", sleepScore));

        // Update sleep efficiency
        double efficiency = getSleepEfficiency();
        // sleepEfficiencyText.setText(String.format("%.1f%%", efficiency));

        // Update sleep debt
        String debtText = getSleepDebtFormatted();
        // sleepDebtText.setText(debtText);

        // Update goal adherence
        double adherence = getGoalAdherence();
        // goalAdherenceText.setText(String.format("%.0f%%", adherence));
    }

    private void updateInformationGrid() {
        // Basic metrics
        String lastSleep = getLastSleepDurationFormatted();
        if (lastSleep.equals(getString(R.string.default_zero_time))) {
            lastSleepText.setText(getString(R.string.default_no_data));
        } else {
            lastSleepText.setText(lastSleep);
        }

        String avgSleep = getAverageSleepDurationFormatted(7);
        if (avgSleep.equals(getString(R.string.default_zero_time))) {
            averageSleepText.setText(getString(R.string.default_no_data));
        } else {
            averageSleepText.setText(avgSleep);
        }

        sessionCountText.setText(String.valueOf(getSessionCount()));

        // Enhanced: Show sleep score in debug text for now
        double sleepScore = getSleepScore();
        if (sleepScore > 0) {
            String enhancedDebug = String.format("Score: %.0f/100 | Efficiency: %.1f%% | %s",
                    sleepScore, getSleepEfficiency(),
                    isInSleepDebt() ? "In Debt" : "On Track");
            debugText.setText(enhancedDebug);
        }
    }

    // ========== ENHANCED DIALOGS ==========

    private void showEnhancedSettingsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Enhanced Sleep Settings");

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(40, 40, 40, 40);

        // Current settings
        double currentSleepHours = getCurrentTargetSleepHours();
        int currentWakeHour = getCurrentWakeHour();
        int currentWakeMinute = getCurrentWakeMinute();

        // Target sleep hours
        TextView sleepLabel = new TextView(this);
        sleepLabel.setText("Target Sleep Hours:");
        sleepLabel.setTextSize(14);
        layout.addView(sleepLabel);

        EditText sleepInput = new EditText(this);
        sleepInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER |
                android.text.InputType.TYPE_NUMBER_FLAG_DECIMAL);
        sleepInput.setText(String.valueOf(currentSleepHours));
        layout.addView(sleepInput);

        // Wake time
        TextView wakeLabel = new TextView(this);
        wakeLabel.setText("Wake Time:");
        wakeLabel.setTextSize(14);
        wakeLabel.setPadding(0, 20, 0, 8);
        layout.addView(wakeLabel);

        LinearLayout timeLayout = new LinearLayout(this);
        timeLayout.setOrientation(LinearLayout.HORIZONTAL);

        EditText hourInput = new EditText(this);
        hourInput.setLayoutParams(new LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        hourInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        hourInput.setText(String.valueOf(currentWakeHour));
        timeLayout.addView(hourInput);

        TextView colonLabel = new TextView(this);
        colonLabel.setText(" : ");
        colonLabel.setTextSize(18);
        colonLabel.setPadding(16, 0, 16, 0);
        timeLayout.addView(colonLabel);

        EditText minuteInput = new EditText(this);
        minuteInput.setLayoutParams(new LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        minuteInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        minuteInput.setText(String.valueOf(currentWakeMinute));
        timeLayout.addView(minuteInput);

        layout.addView(timeLayout);

        // NEW: Target efficiency
        TextView efficiencyLabel = new TextView(this);
        efficiencyLabel.setText("Target Sleep Efficiency (%):");
        efficiencyLabel.setTextSize(14);
        efficiencyLabel.setPadding(0, 20, 0, 8);
        layout.addView(efficiencyLabel);

        SeekBar efficiencySeekBar = new SeekBar(this);
        efficiencySeekBar.setMax(30); // 70-100%
        efficiencySeekBar.setProgress(15); // Default 85%
        layout.addView(efficiencySeekBar);

        TextView efficiencyValue = new TextView(this);
        efficiencyValue.setText("85%");
        efficiencyValue.setTextSize(12);
        layout.addView(efficiencyValue);

        efficiencySeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int efficiency = 70 + progress;
                efficiencyValue.setText(efficiency + "%");
            }
            @Override public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        builder.setView(layout);

        builder.setPositiveButton("Save", (dialog, which) -> {
            try {
                double sleepHours = Double.parseDouble(sleepInput.getText().toString().trim());
                int hour = Integer.parseInt(hourInput.getText().toString().trim());
                int minute = Integer.parseInt(minuteInput.getText().toString().trim());
                double efficiency = 70 + efficiencySeekBar.getProgress();

                if (sleepHours > 0 && sleepHours <= 12 &&
                        hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {

                    setEnhancedSleepGoals(sleepHours, hour, hour, efficiency);
                    saveData();
                    showToast("Enhanced settings saved");
                    updateUI();
                }
            } catch (NumberFormatException e) {
                showToast("Invalid time format");
            }
        });

        builder.setNegativeButton("Cancel", null);
        builder.show();
    }

    private void showSleepQualityDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Rate Last Sleep Quality");

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(40, 40, 40, 40);

        RadioGroup qualityGroup = new RadioGroup(this);

        String[] qualities = {"Poor", "Fair", "Good", "Excellent"};
        for (int i = 0; i < qualities.length; i++) {
            RadioButton rb = new RadioButton(this);
            rb.setText(qualities[i]);
            rb.setId(i + 1);
            qualityGroup.addView(rb);
        }

        qualityGroup.check(3); // Default to "Good"
        layout.addView(qualityGroup);

        // Add note section
        TextView noteLabel = new TextView(this);
        noteLabel.setText("Optional Note:");
        noteLabel.setTextSize(14);
        noteLabel.setPadding(0, 20, 0, 8);
        layout.addView(noteLabel);

        EditText noteInput = new EditText(this);
        noteInput.setHint("How did you sleep?");
        layout.addView(noteInput);

        builder.setView(layout);

        builder.setPositiveButton("Save", (dialog, which) -> {
            int selectedId = qualityGroup.getCheckedRadioButtonId();
            if (selectedId > 0) {
                setSleepQuality(selectedId);

                String note = noteInput.getText().toString().trim();
                if (!note.isEmpty()) {
                    addSessionNote(note);
                }

                showToast("Sleep quality recorded");
                updateUI();
            }
        });

        builder.setNegativeButton("Cancel", null);
        builder.show();
    }

    private void showExportOptionsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Export Options");

        String[] options = {
                "Basic CSV (Analysis)",
                "Detailed Report",
                "Summary CSV",
                "Complete Backup"
        };

        builder.setItems(options, (dialog, which) -> {
            switch (which) {
                case 0: handleExportAnalysisCsv(); break;
                case 1: handleExportDetailedData(); break;
                case 2: handleExportSummaryCsv(); break;
                case 3: handleBackupAllData(); break;
            }
        });

        builder.show();
    }

    private void showRecommendationsDialog() {
        String recommendations = getCurrentRecommendations();
        if (recommendations.isEmpty()) {
            recommendations = "No specific recommendations at this time.\nKeep up the good work!";
        }

        new AlertDialog.Builder(this)
                .setTitle("Sleep Recommendations")
                .setMessage(recommendations)
                .setPositiveButton("OK", null)
                .show();
    }

    private void showAnalysisDialog() {
        String patterns = getSleepPatterns();
        String suggestions = getImprovementSuggestions();

        String analysis = "Sleep Patterns:\n" + patterns +
                "\n\nImprovement Suggestions:\n" + suggestions;

        new AlertDialog.Builder(this)
                .setTitle("Sleep Analysis")
                .setMessage(analysis)
                .setPositiveButton("OK", null)
                .show();
    }

    // ========== ENHANCED EXPORT METHODS ==========

    private void handleExportAnalysisCsv() {
        try {
            File documentsDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOCUMENTS);

            if (!documentsDir.exists() && !documentsDir.mkdirs()) {
                showToast("Cannot create directory");
                return;
            }

            String timestamp = String.valueOf(System.currentTimeMillis());
            String filename = "descansa_analysis_" + timestamp + ".csv";
            File exportFile = new File(documentsDir, filename);

            boolean success = exportAnalysisCsv(exportFile.getAbsolutePath());

            if (success) {
                showToast("Exported: " + filename);
            } else {
                showToast("Export failed");
            }
        } catch (Exception e) {
            showToast("Export error: " + e.getMessage());
        }
    }

    private void handleExportDetailedData() {
        try {
            File documentsDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOCUMENTS);

            if (!documentsDir.exists() && !documentsDir.mkdirs()) {
                showToast("Cannot create directory");
                return;
            }

            String timestamp = String.valueOf(System.currentTimeMillis());
            String filename = "descansa_detailed_" + timestamp + ".txt";
            File exportFile = new File(documentsDir, filename);

            boolean success = exportDetailedData(exportFile.getAbsolutePath());

            if (success) {
                showToast("Detailed report exported: " + filename);
            } else {
                showToast("Export failed");
            }
        } catch (Exception e) {
            showToast("Export error: " + e.getMessage());
        }
    }

    private void handleExportSummaryCsv() {
        try {
            File documentsDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOCUMENTS);

            if (!documentsDir.exists() && !documentsDir.mkdirs()) {
                showToast("Cannot create directory");
                return;
            }

            String timestamp = String.valueOf(System.currentTimeMillis());
            String filename = "descansa_summary_" + timestamp + ".csv";
            File exportFile = new File(documentsDir, filename);

            boolean success = exportSummaryCsv(exportFile.getAbsolutePath());

            if (success) {
                showToast("Summary exported: " + filename);
            } else {
                showToast("Export failed");
            }
        } catch (Exception e) {
            showToast("Export error: " + e.getMessage());
        }
    }

    private void handleBackupAllData() {
        try {
            File documentsDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOCUMENTS);

            if (!documentsDir.exists() && !documentsDir.mkdirs()) {
                showToast("Cannot create directory");
                return;
            }

            String timestamp = String.valueOf(System.currentTimeMillis());
            String filename = "descansa_backup_" + timestamp + ".bak";
            File exportFile = new File(documentsDir, filename);

            boolean success = backupAllData(exportFile.getAbsolutePath());

            if (success) {
                showToast("Complete backup: " + filename);
            } else {
                showToast("Backup failed");
            }
        } catch (Exception e) {
            showToast("Backup error: " + e.getMessage());
        }
    }

    // ========== ENHANCED SESSION MANAGEMENT ==========

    private void handleSleepButtonClick() {
        if (isSessionRunning()) {
            endSleepSession();
            showToast("Sleep session ended");

            // Automatically show quality dialog after ending session
            new Handler(Looper.getMainLooper()).postDelayed(() -> {
                showSleepQualityDialog();
            }, 500);

        } else {
            startSleepSession();
            showToast("Sleep session started");

            // Record environmental data
            recordCurrentEnvironment();
        }
        updateUI();
    }

    private void recordCurrentEnvironment() {
        // Record current screen time end
        recordScreenTimeEnd();

        // Could add more environmental tracking here
        // updateEnvironment(22.0, 25, 5); // temp, noise, light
    }

    // ========== STATUS UPDATES (Enhanced) ==========

    private void updateStatusSection() {
        if (isSessionRunning()) {
            updateSleepingStatus();
        } else if (isInSleepPeriod() && isBeforeTargetWakeTime()) {
            updateSleepOpportunityStatus();
        } else {
            updateAwakeStatus();
        }

        // Enhanced: Show additional context
        updateContextualInfo();
    }

    private void updateContextualInfo() {
        // Show recommendations or sleep debt warnings
        if (isInSleepDebt()) {
            String debt = getSleepDebtFormatted();
            // Could show a subtle warning in UI
        }

        if (!isMeetingGoals()) {
            // Could show goal adherence warning
        }
    }

    private void updateSleepingStatus() {
        statusText.setText(getString(R.string.status_sleeping));
        sleepButton.setText(getString(R.string.btn_end_sleep));

        String duration = getCurrentSessionDurationFormatted();
        String sessionText = getString(R.string.format_session_duration, duration);
        currentSessionText.setText(sessionText);
        currentSessionText.setVisibility(TextView.VISIBLE);

        workTimeText.setText(getString(R.string.status_sleeping_msg));
    }

    private void updateSleepOpportunityStatus() {
        statusText.setText(getString(R.string.status_sleep_window));
        sleepButton.setText(getString(R.string.btn_start_sleep));

        String availableTime = getTimeUntilWakeFormatted();
        String availableText = getString(R.string.format_sleep_available, availableTime);
        currentSessionText.setText(availableText);
        currentSessionText.setVisibility(TextView.VISIBLE);

        workTimeText.setText(availableTime);
    }

    private void updateAwakeStatus() {
        TextView workTimeLabel = findViewById(R.id.work_time_label);

        if (isInSleepPeriod()) {
            String nextWakeTime = getNextWakeTimeFormatted();
            String timeUntilWake = getTimeUntilNextWakeFormatted();

            String leftText = "Time until " + nextWakeTime + ":";
            workTimeLabel.setText(leftText);
            workTimeLabel.setVisibility(View.VISIBLE);

            workTimeText.setText(timeUntilWake);

            if (isBeforeTargetWakeTime()) {
                statusText.setText(getString(R.string.status_sleeping));
            } else {
                statusText.setText(getString(R.string.status_good_morning));
            }

        } else {
            workTimeLabel.setText(getString(R.string.label_work_time_short));
            workTimeLabel.setVisibility(View.VISIBLE);

            statusText.setText(getString(R.string.status_awake));
            String workTime = getRemainingWorkTimeFormatted();
            if (workTime.equals(getString(R.string.default_zero_time))) {
                workTimeText.setText(getString(R.string.status_past_bedtime));
            } else {
                workTimeText.setText(workTime);
            }
        }

        sleepButton.setText(getString(R.string.btn_start_sleep));
        currentSessionText.setVisibility(View.GONE);
    }

    // ENHANCED: MainActivity theme management methods with smart options support
// Replace the existing theme-related methods in MainActivity.java with these:

// ========== ENHANCED THEME MANAGEMENT WITH SMART OPTIONS ==========

    private void showThemeSelectionDialog() {
        Intent intent = new Intent(this, ThemeSelectionActivity.class);
        startActivity(intent);
    }

    // ENHANCED: Smart theme application with random and system support
    private void applyStoredTheme() {
        try {
            SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
            String selectedTheme = prefs.getString("theme", "white");
            String resolvedTheme = prefs.getString("resolved_theme", "white");

            // Determine which theme to actually apply
            String themeToApply = resolvedTheme;

            // Handle smart theme options
            if (selectedTheme.equals("follow_system")) {
                themeToApply = "follow_system";
            } else if (selectedTheme.startsWith("random")) {
                // For random themes, check if we need to select a new one
                if (resolvedTheme.isEmpty() || shouldReselectRandomTheme(selectedTheme)) {
                    themeToApply = selectNewRandomTheme(selectedTheme, prefs);
                }
                // Otherwise use the existing resolved theme
            }

            // Apply the determined theme
            applySpecificTheme(themeToApply);

            // Update debug info
            updateThemeDebugInfo(selectedTheme, themeToApply);

        } catch (Exception e) {
            // Fallback on any theme application error
            AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
            showToast("Theme error - using system default");
        }
    }

    // NEW: Determine if random theme should be reselected
    private boolean shouldReselectRandomTheme(String selectedTheme) {
        SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);

        // Check if this is the first time or if user wants daily randomization
        long lastRandomSelection = prefs.getLong("last_random_selection", 0);
        long oneDayAgo = System.currentTimeMillis() - (24 * 60 * 60 * 1000);

        // Optional: Re-randomize daily for random themes
        boolean dailyRandomization = prefs.getBoolean("daily_random", false);

        return dailyRandomization && (lastRandomSelection < oneDayAgo);
    }

    // NEW: Select new random theme based on type
    private String selectNewRandomTheme(String randomType, SharedPreferences prefs) {
        List<String> availableThemes = new ArrayList<>();

        // Define available themes for each category
        switch (randomType) {
            case "random":
                availableThemes.addAll(Arrays.asList("white", "solarized", "everforest", "amoled", "dracula", "nordic"));
                break;
            case "random_light":
                availableThemes.addAll(Arrays.asList("white", "solarized", "rose_pine"));
                break;
            case "random_dark":
                availableThemes.addAll(Arrays.asList("amoled", "dracula", "nordic"));
                break;
            default:
                return "white";
        }

        // Select random theme
        Random random = new Random();
        String selectedTheme = availableThemes.get(random.nextInt(availableThemes.size()));

        // Save the selection
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString("resolved_theme", selectedTheme);
        editor.putLong("last_random_selection", System.currentTimeMillis());
        editor.apply();

        return selectedTheme;
    }

    // ENHANCED: Apply specific theme with all options
    private void applySpecificTheme(String themeId) {
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
            case "rose_pine":
                setTheme(R.style.Theme_Descansa_RosePine);
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
                // Fallback to system
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
                getSharedPreferences("DescansaPrefs", MODE_PRIVATE)
                        .edit()
                        .putString("theme", "follow_system")
                        .putString("resolved_theme", "follow_system")
                        .apply();
        }

        // Ensure theme is applied properly
        getDelegate().applyDayNight();
    }

    // NEW: Update debug info with theme information
    private void updateThemeDebugInfo(String selectedTheme, String appliedTheme) {
        if (selectedTheme.startsWith("random") && !selectedTheme.equals(appliedTheme)) {
            // For random themes, show what was selected
            String debugInfo = String.format("Theme: %s (%s)", getThemeDisplayName(selectedTheme), getThemeDisplayName(appliedTheme));
            // You could display this in a debug text view or log it
        }
    }

    // NEW: Get display name for theme
    private String getThemeDisplayName(String themeId) {
        switch (themeId) {
            case "follow_system": return "System";
            case "random": return "Random";
            case "random_light": return "Random Light";
            case "random_dark": return "Random Dark";
            case "white": return "White";
            case "solarized": return "Solarized";
            case "rose_pine": return "Rose Pine";
            case "amoled": return "AMOLED";
            case "dracula": return "Dracula";
            case "nordic": return "Nordic";
            default: return themeId;
        }
    }

    // ENHANCED: System UI setup with smart theme awareness
    private void setupSystemUI() {
        try {
            WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

            WindowInsetsControllerCompat windowInsetsController =
                    WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());

            // Determine if current theme is light
            boolean isLightTheme = determineIfCurrentThemeIsLight();

            windowInsetsController.setAppearanceLightStatusBars(isLightTheme);
            windowInsetsController.setAppearanceLightNavigationBars(isLightTheme);

            if (getSupportActionBar() != null) {
                getSupportActionBar().hide();
            }

        } catch (Exception e) {
            if (getSupportActionBar() != null) {
                getSupportActionBar().hide();
            }
        }
    }

    // ENHANCED: Smart theme light/dark detection
    private boolean determineIfCurrentThemeIsLight() {
        SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
        String selectedTheme = prefs.getString("theme", "white");
        String resolvedTheme = prefs.getString("resolved_theme", "white");

        // For smart themes, use resolved theme
        String themeToCheck = selectedTheme.startsWith("random") || selectedTheme.equals("follow_system")
                ? resolvedTheme : selectedTheme;

        // Handle follow_system specially
        if (themeToCheck.equals("follow_system")) {
            return (getResources().getConfiguration().uiMode &
                    android.content.res.Configuration.UI_MODE_NIGHT_MASK) ==
                    android.content.res.Configuration.UI_MODE_NIGHT_NO;
        }

        // Regular theme detection
        return isLightTheme(themeToCheck);
    }

    // ENHANCED: Check if specific theme is light
    private boolean isLightTheme(String themeId) {
        switch (themeId) {
            case "white":
            case "solarized":
            case "rose_pine":
                return true;
            case "amoled":
            case "dracula":
            case "nordic":
                return false;
            case "follow_system":
                return (getResources().getConfiguration().uiMode &
                        android.content.res.Configuration.UI_MODE_NIGHT_MASK) ==
                        android.content.res.Configuration.UI_MODE_NIGHT_NO;
            default:
                return true; // Default to light
        }
    }

    // NEW: Add random theme shuffle button to settings or main screen
    private void addThemeShuffleOption() {
        // This could be added to the settings dialog or as a separate button
        // For now, you could add a long-press on the theme button to shuffle
        themeButton.setOnLongClickListener(v -> {
            SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
            String currentTheme = prefs.getString("theme", "white");

            if (currentTheme.startsWith("random")) {
                // Trigger new random selection
                selectNewRandomTheme(currentTheme, prefs);

                // Restart activity to apply new theme
                recreate();

                showToast("🎲 New random theme selected!");
                return true;
            } else {
                showToast("Select a random theme first");
                return false;
            }
        });
    }



    // NEW: Check for theme changes and apply if needed
    private void checkForThemeChanges() {
        SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
        String currentSelection = prefs.getString("theme", "white");
        String lastAppliedTheme = prefs.getString("last_applied_theme", "");

        // If theme selection changed, the activity will be recreated automatically
        // This is just for tracking
        if (!currentSelection.equals(lastAppliedTheme)) {
            SharedPreferences.Editor editor = prefs.edit();
            editor.putString("last_applied_theme", currentSelection);
            editor.apply();
        }
    }

    // NEW: Enhanced debug info in updateDebugInfo method
    private void updateDebugInfo() {
        if (isSessionRunning()) {
            debugText.setText(getString(R.string.debug_session_active));
        } else {
            int sessionCount = getSessionCount();
            double score = getSleepScore();
            boolean meetingGoals = isMeetingGoals();

            // Add theme info to debug
            SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
            String selectedTheme = prefs.getString("theme", "white");
            String resolvedTheme = prefs.getString("resolved_theme", "white");

            String themeInfo = selectedTheme.equals(resolvedTheme) ?
                    getThemeDisplayName(selectedTheme) :
                    getThemeDisplayName(selectedTheme) + "→" + getThemeDisplayName(resolvedTheme);

            String debugInfo = String.format("Sessions: %d | Score: %.0f | Goals: %s | Theme: %s",
                    sessionCount, score, meetingGoals ? "✓" : "✗", themeInfo);
            debugText.setText(debugInfo);
        }
    }

    // NEW: Add theme preferences to enhanced settings dialog
    private void addThemePreferencesToSettings(LinearLayout layout) {
        // Add theme randomization option
        TextView randomLabel = new TextView(this);
        randomLabel.setText("Daily Theme Randomization:");
        randomLabel.setTextSize(14);
        randomLabel.setPadding(0, 20, 0, 8);
        layout.addView(randomLabel);

        CheckBox dailyRandomBox = new CheckBox(this);
        dailyRandomBox.setText("Change random theme daily");
        boolean currentSetting = getSharedPreferences("DescansaPrefs", MODE_PRIVATE)
                .getBoolean("daily_random", false);
        dailyRandomBox.setChecked(currentSetting);
        layout.addView(dailyRandomBox);

        // Save the setting when dialog is saved
        // You'd need to handle this in your settings save method:
    /*
    SharedPreferences.Editor editor = getSharedPreferences("DescansaPrefs", MODE_PRIVATE).edit();
    editor.putBoolean("daily_random", dailyRandomBox.isChecked());
    editor.apply();
    */
    }

    // ========== DATA MANAGEMENT ==========

    private void handleClearData() {
        new AlertDialog.Builder(this)
                .setTitle(getString(R.string.dialog_clear_title))
                .setMessage("This will delete ALL sleep data including detailed tracking.")
                .setPositiveButton(getString(R.string.btn_clear), (dialog, which) -> {
                    clearHistory();
                    showToast("All data cleared");
                    updateUI();
                })
                .setNegativeButton(getString(R.string.btn_cancel), null)
                .show();
    }

    // ========== LIFECYCLE ==========

    private void startPeriodicUpdates() {
        updateHandler = new Handler(Looper.getMainLooper());
        updateRunnable = new Runnable() {
            @Override
            public void run() {
                if (isUpdating) {
                    updateUI();
                    updateHandler.postDelayed(this, 30000); // 30 seconds
                }
            }
        };
        isUpdating = true;
        updateHandler.post(updateRunnable);
    }

    private void stopPeriodicUpdates() {
        isUpdating = false;
        if (updateHandler != null && updateRunnable != null) {
            updateHandler.removeCallbacks(updateRunnable);
        }
    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    // ENHANCED: Handle theme changes when returning from theme selection
    @Override
    protected void onResume() {
        super.onResume();

        // Check if theme preference changed
        checkForThemeChanges();

        updateUI();

        if (!isUpdating) {
            startPeriodicUpdates();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        saveData();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopPeriodicUpdates();
        saveData();
    }

    // ========== NATIVE METHODS - ENHANCED ==========

    // Core functionality (existing)
    public native void initializeCore(String dataPath);
    public native void startSleepSession();
    public native void endSleepSession();
    public native boolean isSessionRunning();

    // Settings (existing)
    public native void setTargetSleepHours(double hours);
    public native void setTargetWakeTime(int hour, int minute);
    public native double getCurrentTargetSleepHours();
    public native int getCurrentWakeHour();
    public native int getCurrentWakeMinute();

    // NEW: Enhanced settings
    public native void setEnhancedSleepGoals(double targetHours, int bedtimeHour, int wakeHour, double targetEfficiency);

    // Data queries (existing)
    public native String getRemainingWorkTimeFormatted();
    public native String getLastSleepDurationFormatted();
    public native String getAverageSleepDurationFormatted(int days);
    public native String getCurrentSessionDurationFormatted();
    public native int getSessionCount();

    // Enhanced wake time methods (existing)
    public native String getTimeUntilNextWakeFormatted();
    public native String getNextWakeTimeFormatted();

    // Sleep period detection (existing)
    public native boolean isInSleepPeriod();
    public native boolean isBeforeTargetWakeTime();
    public native String getTimeUntilWakeFormatted();

    // NEW: Sleep quality and tracking
    public native void setSleepQuality(int quality);
    public native void addSessionNote(String note);
    public native void markAsNap(boolean isNap);

    // NEW: Environmental tracking
    public native void updateEnvironment(double temperature, int noiseLevel, int lightLevel);
    public native void recordCaffeineIntake();
    public native void recordScreenTimeEnd();

    // NEW: Analytics and metrics
    public native double getSleepScore();
    public native String getSleepScoreExplanation();
    public native double getSleepEfficiency();
    public native String getCurrentRecommendations();

    // NEW: Sleep debt and goals
    public native String getSleepDebtFormatted();
    public native boolean isInSleepDebt();
    public native double getGoalAdherence();
    public native boolean isMeetingGoals();

    // NEW: Pattern analysis
    public native String getSleepPatterns();
    public native String getImprovementSuggestions();

    // NEW: Enhanced exports
    public native boolean exportDetailedData(String exportPath);
    public native boolean exportSummaryCsv(String exportPath);
    public native boolean backupAllData(String backupPath);

    // NEW: System status
    public native String getSystemStatus();
    public native boolean runDiagnostics();

    // Data management (existing)
    public native boolean saveData();
    public native boolean exportAnalysisCsv(String exportPath);
    public native void clearHistory();

    static {
        System.loadLibrary("descansa");
    }
}