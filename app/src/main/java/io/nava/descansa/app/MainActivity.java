package io.nava.descansa.app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.os.Handler;
import android.os.Looper;

import java.io.File;

import android.content.Intent;
import io.nava.descansa.app.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;
    private Handler updateHandler;
    private Runnable updateRunnable;
    private boolean isUpdating = false;

    // UI Elements
    private TextView statusText;
    private TextView currentSessionText;
    private Button sleepButton;
    private TextView workTimeText;
    private TextView lastSleepText;
    private TextView averageSleepText;
    private TextView sessionCountText;
    private Button exportButton;
    private Button settingsButton;
    private Button clearDataButton;
    private Button themeButton;
    private TextView debugText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("Descansa", "=== APP STARTED ===");
        super.onCreate(savedInstanceState);

        // Apply theme before UI setup
        applyStoredTheme();
        setupSystemUI();

        // Initialize C++ core FIRST
        initializeCore();

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        initializeUI();
        setupButtonListeners();
        startPeriodicUpdates();
        updateUI();
    }

    // ========== FIXED CORE INITIALIZATION ==========
    private void initializeCore() {
        String dataPath = getFilesDir().getAbsolutePath() + "/descansa_data.txt";
        Log.d("Descansa", "Initializing core with path: " + dataPath);

        initializeCore(dataPath);

        // FIXED: Don't overwrite user settings! Let the core load saved settings.
        // Only set defaults if this is the very first run and no data exists.
        if (getSessionCount() == 0 && getCurrentTargetSleepHours() == 0.0) {
            Log.d("Descansa", "First run - setting default values");
            setTargetSleepHours(8.0);
            setTargetWakeTime(7, 0);
        } else {
            Log.d("Descansa", "Existing data found - preserving user settings");
            // Log current settings to verify they're loaded
            double hours = getCurrentTargetSleepHours();
            int wakeHour = getCurrentWakeHour();
            int wakeMinute = getCurrentWakeMinute();
            Log.d("Descansa", String.format("Loaded settings: %.1f hours, wake %d:%02d",
                    hours, wakeHour, wakeMinute));
        }

        Log.d("Descansa", "Core initialized successfully");
    }

    private void initializeUI() {
        statusText = binding.statusText;
        currentSessionText = binding.currentSessionText;
        sleepButton = binding.sleepButton;
        workTimeText = binding.workTimeText;
        lastSleepText = binding.lastSleepText;
        averageSleepText = binding.averageSleepText;
        sessionCountText = binding.sessionCountText;
        exportButton = binding.exportButton;
        settingsButton = binding.settingsButton;
        clearDataButton = binding.clearDataButton;
        themeButton = binding.themeButton;
        debugText = binding.debugText;
    }

    private void setupButtonListeners() {
        // SIMPLE sleep button - no surveys, no complications
        sleepButton.setOnClickListener(v -> {
            if (isSessionRunning()) {
                endSleepSession();
                showToast("Sleep session ended");
            } else {
                startSleepSession();
                showToast("Sleep session started");
            }
            updateUI();
        });

        // DIRECT CSV export - no dialog, just export
        exportButton.setOnClickListener(v -> handleDirectExport());

        settingsButton.setOnClickListener(v -> showSettingsDialog());
        clearDataButton.setOnClickListener(v -> handleClearData());
        themeButton.setOnClickListener(v -> showThemeSelection());
    }

    // ========== DIRECT EXPORT - NO DIALOG ==========
    private void handleDirectExport() {
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
                Log.d("Descansa", "Export successful: " + exportFile.getAbsolutePath());
            } else {
                showToast("Export failed");
                Log.e("Descansa", "Export failed");
            }
        } catch (Exception e) {
            showToast("Export error: " + e.getMessage());
            Log.e("Descansa", "Export exception", e);
        }
    }

    // ========== FIXED SETTINGS DIALOG ==========
    private void showSettingsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Sleep Settings");

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(40, 40, 40, 40);

        // Current settings
        double currentSleepHours = getCurrentTargetSleepHours();
        int currentWakeHour = getCurrentWakeHour();
        int currentWakeMinute = getCurrentWakeMinute();

        // FIXED: Convert decimal hours to hours and minutes
        int sleepHours = (int) currentSleepHours;
        int sleepMinutes = (int) Math.round((currentSleepHours - sleepHours) * 60);

        // FIXED: Sleep duration input - now in time format
        TextView sleepLabel = new TextView(this);
        sleepLabel.setText("Target sleep duration:");
        sleepLabel.setTextSize(16);
        // FIXED: Use theme-aware text color
        android.util.TypedValue typedValue = new android.util.TypedValue();
        getTheme().resolveAttribute(android.R.attr.textColorPrimary, typedValue, true);
        sleepLabel.setTextColor(typedValue.data);
        layout.addView(sleepLabel);

        LinearLayout sleepTimeLayout = new LinearLayout(this);
        sleepTimeLayout.setOrientation(LinearLayout.HORIZONTAL);
        sleepTimeLayout.setPadding(0, 8, 0, 0);

        EditText sleepHourInput = new EditText(this);
        sleepHourInput.setLayoutParams(new LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        sleepHourInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        sleepHourInput.setText(String.valueOf(sleepHours));
        sleepHourInput.setHint("8");
        sleepTimeLayout.addView(sleepHourInput);

        TextView sleepColonLabel = new TextView(this);
        sleepColonLabel.setText(" h ");
        sleepColonLabel.setTextSize(16);
        sleepColonLabel.setPadding(8, 0, 8, 0);
        sleepTimeLayout.addView(sleepColonLabel);

        EditText sleepMinuteInput = new EditText(this);
        sleepMinuteInput.setLayoutParams(new LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        sleepMinuteInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        sleepMinuteInput.setText(String.valueOf(sleepMinutes));
        sleepMinuteInput.setHint("30");
        sleepTimeLayout.addView(sleepMinuteInput);

        TextView sleepMinLabel = new TextView(this);
        sleepMinLabel.setText(" m");
        sleepMinLabel.setTextSize(16);
        sleepMinLabel.setPadding(8, 0, 0, 0);
        sleepTimeLayout.addView(sleepMinLabel);

        layout.addView(sleepTimeLayout);

        // FIXED: Wake time input with visible label
        TextView wakeLabel = new TextView(this);
        wakeLabel.setText("Wake time (24h format):");
        wakeLabel.setTextSize(16);
        wakeLabel.setPadding(0, 20, 0, 8);
        // FIXED: Use theme-aware text color
        android.util.TypedValue typedValue2 = new android.util.TypedValue();
        getTheme().resolveAttribute(android.R.attr.textColorPrimary, typedValue2, true);
        wakeLabel.setTextColor(typedValue2.data);
        layout.addView(wakeLabel);

        LinearLayout timeLayout = new LinearLayout(this);
        timeLayout.setOrientation(LinearLayout.HORIZONTAL);

        EditText hourInput = new EditText(this);
        hourInput.setLayoutParams(new LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        hourInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        hourInput.setText(String.valueOf(currentWakeHour));
        hourInput.setHint("7");
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
        minuteInput.setHint("00");
        timeLayout.addView(minuteInput);

        layout.addView(timeLayout);
        builder.setView(layout);

        builder.setPositiveButton("Save", (dialog, which) -> {
            try {
                // FIXED: Parse hours and minutes for sleep duration
                int targetSleepHours = Integer.parseInt(sleepHourInput.getText().toString().trim());
                int targetSleepMinutes = Integer.parseInt(sleepMinuteInput.getText().toString().trim());
                int wakeHour = Integer.parseInt(hourInput.getText().toString().trim());
                int wakeMinute = Integer.parseInt(minuteInput.getText().toString().trim());

                // FIXED: Convert back to decimal hours for the core
                double totalSleepHours = targetSleepHours + (targetSleepMinutes / 60.0);

                if (totalSleepHours > 0 && totalSleepHours <= 24 &&
                        wakeHour >= 0 && wakeHour <= 23 &&
                        wakeMinute >= 0 && wakeMinute <= 59 &&
                        targetSleepMinutes >= 0 && targetSleepMinutes <= 59) {

                    setTargetSleepHours(totalSleepHours);
                    setTargetWakeTime(wakeHour, wakeMinute);

                    // FORCE SAVE immediately
                    boolean saved = saveData();
                    Log.d("Descansa", "Settings saved: " + saved);

                    showToast("Settings saved");
                    updateUI();
                } else {
                    showToast("Please enter valid times");
                }
            } catch (NumberFormatException e) {
                showToast("Please enter valid numbers");
            }
        });

        builder.setNegativeButton("Cancel", null);
        builder.show();
    }

    // ========== THEME SELECTION ==========
    private void showThemeSelection() {
        Intent intent = new Intent(this, ThemeSelectionActivity.class);
        startActivityForResult(intent, 1001);
    }

    // FIXED: Handle theme change result properly
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == 1001 && resultCode == RESULT_OK) {
            // Theme was changed - recreate activity to apply it immediately
            Log.d("Descansa", "Theme changed, recreating activity");
            recreate();
        }
    }

    // ========== UI UPDATES ==========
    private void updateUI() {
        Log.d("Descansa", "=== updateUI() called ===");
        updateStatusSection();
        updateInformationGrid();
        updateDebugInfo();
    }

    private void updateStatusSection() {
        boolean sessionRunning = isSessionRunning();
        boolean inSleepPeriod = isInSleepPeriod();
        boolean beforeWake = isBeforeTargetWakeTime();

        Log.d("Descansa", String.format("Status - Running: %b, InSleep: %b, BeforeWake: %b",
                sessionRunning, inSleepPeriod, beforeWake));

        if (sessionRunning) {
            updateSleepingStatus();
        } else if (inSleepPeriod && beforeWake) {
            updateSleepOpportunityStatus();
        } else {
            updateAwakeStatus();
        }
    }

    private void updateSleepingStatus() {
        statusText.setText("SLEEPING");
        sleepButton.setText("End Sleep");

        String duration = getCurrentSessionDurationFormatted();
        currentSessionText.setText("Session: " + duration);
        currentSessionText.setVisibility(TextView.VISIBLE);

        workTimeText.setText("Sleeping...");
    }

    private void updateSleepOpportunityStatus() {
        statusText.setText("Sleep Window Active");
        sleepButton.setText("Start Sleep");

        // RESTORED: Better dynamic labeling like before
        TextView workTimeLabel = findViewById(R.id.work_time_label);
        String nextWakeTime = getNextWakeTimeFormatted();
        String timeUntilWake = getTimeUntilWakeFormatted();

        // Set the dynamic label
        String labelText = getString(R.string.label_time_until_wake_dynamic, nextWakeTime);
        workTimeLabel.setText(labelText);
        workTimeLabel.setVisibility(View.VISIBLE);

        // Set the time value
        workTimeText.setText(timeUntilWake);

        currentSessionText.setVisibility(TextView.GONE);
    }

    private void updateAwakeStatus() {
        statusText.setText("AWAKE");
        sleepButton.setText("Start Sleep");

        // RESTORED: Better work time display logic
        TextView workTimeLabel = findViewById(R.id.work_time_label);

        boolean inSleepPeriod = isInSleepPeriod();
        String workTime = getRemainingWorkTimeFormatted();
        String nextWake = getNextWakeTimeFormatted();
        String timeUntilWake = getTimeUntilNextWakeFormatted();

        if (inSleepPeriod) {
            // We're past bedtime - show time until wake
            String labelText = getString(R.string.format_time_until_wake, nextWake, "");
            workTimeLabel.setText(labelText);
            workTimeLabel.setVisibility(View.VISIBLE);
            workTimeText.setText(timeUntilWake);
        } else {
            // Normal work time
            workTimeLabel.setText(getString(R.string.label_work_time_short));
            workTimeLabel.setVisibility(View.VISIBLE);

            if (workTime.equals("0h 0m")) {
                workTimeText.setText("Past bedtime");
            } else {
                workTimeText.setText(workTime);
            }
        }

        currentSessionText.setVisibility(TextView.GONE);
    }

    private void updateInformationGrid() {
        // Last sleep
        String lastSleep = getLastSleepDurationFormatted();
        lastSleepText.setText(lastSleep.equals("0h 0m") ? "--" : lastSleep);

        // Average sleep
        String avgSleep = getAverageSleepDurationFormatted(7);
        averageSleepText.setText(avgSleep.equals("0h 0m") ? "--" : avgSleep);

        // Session count
        sessionCountText.setText(String.valueOf(getSessionCount()));
    }

    private void updateDebugInfo() {
        int sessionCount = getSessionCount();
        if (isSessionRunning()) {
            debugText.setText("Session Active - " + sessionCount + " total");
        } else {
            debugText.setText("Ready - " + sessionCount + " sessions recorded");
        }
    }

    // ========== DATA MANAGEMENT ==========
    private void handleClearData() {
        new AlertDialog.Builder(this)
                .setTitle("Clear All Data")
                .setMessage("Delete all sleep sessions?")
                .setPositiveButton("Clear", (dialog, which) -> {
                    clearHistory();
                    showToast("All data cleared");
                    updateUI();
                })
                .setNegativeButton("Cancel", null)
                .show();
    }

    // ========== THEME MANAGEMENT ==========
    private void applyStoredTheme() {
        try {
            SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
            String selectedTheme = prefs.getString("theme", "white");
            String resolvedTheme = prefs.getString("resolved_theme", "white");

            String themeToApply = selectedTheme.startsWith("random") || selectedTheme.equals("follow_system")
                    ? resolvedTheme : selectedTheme;

            applySpecificTheme(themeToApply);
        } catch (Exception e) {
            AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
        }
    }

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
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
        }
        getDelegate().applyDayNight();
    }

    private void setupSystemUI() {
        try {
            WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
            WindowInsetsControllerCompat windowInsetsController =
                    WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());

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

    private boolean determineIfCurrentThemeIsLight() {
        SharedPreferences prefs = getSharedPreferences("DescansaPrefs", MODE_PRIVATE);
        String selectedTheme = prefs.getString("theme", "white");
        String resolvedTheme = prefs.getString("resolved_theme", "white");

        String themeToCheck = selectedTheme.startsWith("random") || selectedTheme.equals("follow_system")
                ? resolvedTheme : selectedTheme;

        switch (themeToCheck) {
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
                return true;
        }
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

    @Override
    protected void onResume() {
        super.onResume();
        Log.d("Descansa", "=== onResume() ===");

        // FIXED: Verify settings are preserved after theme changes
        double hours = getCurrentTargetSleepHours();
        int wakeHour = getCurrentWakeHour();
        int wakeMinute = getCurrentWakeMinute();
        Log.d("Descansa", String.format("Settings check - Sleep: %.1f hours, Wake: %d:%02d",
                hours, wakeHour, wakeMinute));

        // Check if we need to load data
        if (getSessionCount() == 0) {
            Log.d("Descansa", "No sessions found, checking if core needs reinitialization");
            // Force data reload
            initializeCore();
        }

        updateUI();
        if (!isUpdating) {
            startPeriodicUpdates();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d("Descansa", "=== onPause() ===");

        // FORCE SAVE on every pause
        boolean saved = saveData();
        Log.d("Descansa", "Data saved on pause: " + saved);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d("Descansa", "=== onDestroy() ===");

        stopPeriodicUpdates();

        // FINAL SAVE
        boolean saved = saveData();
        Log.d("Descansa", "Final data save on destroy: " + saved);
    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    // ========== NATIVE METHODS - BASIC ONLY ==========
    public native void initializeCore(String dataPath);
    public native void startSleepSession();
    public native void endSleepSession();
    public native boolean isSessionRunning();

    public native void setTargetSleepHours(double hours);
    public native void setTargetWakeTime(int hour, int minute);
    public native double getCurrentTargetSleepHours();
    public native int getCurrentWakeHour();
    public native int getCurrentWakeMinute();

    public native String getRemainingWorkTimeFormatted();
    public native String getLastSleepDurationFormatted();
    public native String getAverageSleepDurationFormatted(int days);
    public native String getCurrentSessionDurationFormatted();
    public native int getSessionCount();

    public native boolean isInSleepPeriod();
    public native boolean isBeforeTargetWakeTime();
    public native String getTimeUntilWakeFormatted();
    public native String getTimeUntilNextWakeFormatted();
    public native String getNextWakeTimeFormatted();

    public native boolean saveData();
    public native boolean exportAnalysisCsv(String exportPath);
    public native void clearHistory();

    static {
        System.loadLibrary("descansa");
    }
}