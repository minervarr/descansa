package io.nava.descansa.app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AlertDialog;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.os.Handler;
import android.os.Looper;
import java.io.File;

import io.nava.descansa.app.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;
    private Handler updateHandler;
    private Runnable updateRunnable;
    private boolean isUpdating = false;

    // UI Elements - Organized by section
    private TextView statusText;
    private TextView currentSessionText;
    private Button sleepButton;

    // Sleep Information Grid
    private TextView workTimeText;
    private TextView lastSleepText;
    private TextView averageSleepText;
    private TextView sessionCountText;

    // Action Buttons
    private Button exportButton;
    private Button settingsButton;
    private Button clearDataButton;

    // Debug
    private TextView debugText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // FIXED: Configure system UI properly BEFORE setContentView
        setupSystemUI();

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        initializeCore();
        initializeUI();
        setupButtonListeners();
        startPeriodicUpdates();
        updateUI();
    }

    // ========== SYSTEM UI CONFIGURATION ==========

    private void setupSystemUI() {
        // FIXED: Enable edge-to-edge display
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

        // FIXED: Configure system bars
        WindowInsetsControllerCompat windowInsetsController =
                WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());

        if (windowInsetsController != null) {
            // Make status bar and navigation bar content dark/light based on theme
            windowInsetsController.setAppearanceLightStatusBars(true);
            windowInsetsController.setAppearanceLightNavigationBars(true);
        }

        // FIXED: Ensure no ActionBar is created
        if (getSupportActionBar() != null) {
            getSupportActionBar().hide();
        }
    }

    // ========== INITIALIZATION ==========

    private void initializeCore() {
        String dataPath = getFilesDir().getAbsolutePath() + "/descansa_data.txt";
        initializeCore(dataPath);
    }

    private void initializeUI() {
        // Status section
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

        // Debug
        debugText = binding.debugText;
    }

    private void setupButtonListeners() {
        sleepButton.setOnClickListener(v -> handleSleepButtonClick());
        exportButton.setOnClickListener(v -> handleExportData());
        settingsButton.setOnClickListener(v -> showSettingsDialog());
        clearDataButton.setOnClickListener(v -> handleClearData());
    }

    // ========== UI UPDATE LOGIC ==========

    private void updateUI() {
        updateStatusSection();
        updateInformationGrid();
        updateDebugInfo();
    }

    private void updateStatusSection() {
        if (isSessionRunning()) {
            updateSleepingStatus();
        } else if (isInSleepPeriod() && isBeforeTargetWakeTime()) {
            updateSleepOpportunityStatus();
        } else {
            updateAwakeStatus();
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
        if (isInSleepPeriod()) {
            statusText.setText(getString(R.string.status_good_morning));
            workTimeText.setText(getString(R.string.status_wake_passed));
        } else {
            statusText.setText(getString(R.string.status_awake));
            String workTime = getRemainingWorkTimeFormatted();
            if (workTime.equals(getString(R.string.default_zero_time))) {
                workTimeText.setText(getString(R.string.status_past_bedtime));
            } else {
                workTimeText.setText(workTime);
            }
        }

        sleepButton.setText(getString(R.string.btn_start_sleep));
        currentSessionText.setVisibility(TextView.GONE);
    }

    private void updateInformationGrid() {
        // Last Sleep
        String lastSleep = getLastSleepDurationFormatted();
        if (lastSleep.equals(getString(R.string.default_zero_time))) {
            lastSleepText.setText(getString(R.string.default_no_data));
        } else {
            lastSleepText.setText(lastSleep);
        }

        // Average Sleep
        String avgSleep = getAverageSleepDurationFormatted(7);
        if (avgSleep.equals(getString(R.string.default_zero_time))) {
            averageSleepText.setText(getString(R.string.default_no_data));
        } else {
            averageSleepText.setText(avgSleep);
        }

        // Session Count
        sessionCountText.setText(String.valueOf(getSessionCount()));
    }

    private void updateDebugInfo() {
        if (isSessionRunning()) {
            debugText.setText(getString(R.string.debug_session_active));
        } else {
            int sessionCount = getSessionCount();
            String debugText = getString(R.string.debug_core_status, sessionCount);
            this.debugText.setText(debugText);
        }
    }

    // ========== BUTTON HANDLERS ==========

    private void handleSleepButtonClick() {
        if (isSessionRunning()) {
            endSleepSession();
            showToast(getString(R.string.msg_sleep_ended));
        } else {
            startSleepSession();
            showToast(getString(R.string.msg_sleep_started));
        }
        updateUI();
    }

    private void handleExportData() {
        try {
            File documentsDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOCUMENTS);
            if (documentsDir != null) {
                documentsDir.mkdirs();

                String timestamp = String.valueOf(System.currentTimeMillis());
                String filename = getString(R.string.export_filename_template, timestamp);
                File exportFile = new File(documentsDir, filename);

                boolean success = exportData(exportFile.getAbsolutePath());
                if (success) {
                    showToast(getString(R.string.msg_data_exported, filename));
                } else {
                    showToast(getString(R.string.error_export_failed));
                }
            } else {
                showToast(getString(R.string.error_export_failed));
            }
        } catch (Exception e) {
            String errorMsg = getString(R.string.error_export_error, e.getMessage());
            showToast(errorMsg);
        }
    }

    private void handleClearData() {
        new AlertDialog.Builder(this)
                .setTitle(getString(R.string.dialog_clear_title))
                .setMessage(getString(R.string.dialog_clear_message))
                .setPositiveButton(getString(R.string.btn_clear), (dialog, which) -> {
                    clearHistory();
                    showToast(getString(R.string.msg_data_cleared));
                    updateUI();
                })
                .setNegativeButton(getString(R.string.btn_cancel), null)
                .show();
    }

    // ========== SETTINGS DIALOG ==========

    private void showSettingsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(getString(R.string.dialog_settings_title));

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(40, 40, 40, 40);

        // Get current settings
        double currentSleepHours = getCurrentTargetSleepHours();
        int currentWakeHour = getCurrentWakeHour();
        int currentWakeMinute = getCurrentWakeMinute();

        // Target sleep hours
        TextView sleepLabel = new TextView(this);
        sleepLabel.setText(getString(R.string.label_target_sleep_hours));
        sleepLabel.setTextSize(14);
        layout.addView(sleepLabel);

        EditText sleepInput = new EditText(this);
        sleepInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER |
                android.text.InputType.TYPE_NUMBER_FLAG_DECIMAL);
        sleepInput.setText(String.valueOf(currentSleepHours));
        sleepInput.setHint(getString(R.string.hint_sleep_hours));
        layout.addView(sleepInput);

        // Wake time
        TextView wakeLabel = new TextView(this);
        wakeLabel.setText(getString(R.string.label_wake_time));
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
        hourInput.setHint(getString(R.string.hint_hour));
        timeLayout.addView(hourInput);

        TextView colonLabel = new TextView(this);
        colonLabel.setText(getString(R.string.label_time_separator));
        colonLabel.setTextSize(18);
        colonLabel.setPadding(16, 0, 16, 0);
        timeLayout.addView(colonLabel);

        EditText minuteInput = new EditText(this);
        minuteInput.setLayoutParams(new LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        minuteInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        minuteInput.setText(String.valueOf(currentWakeMinute));
        minuteInput.setHint(getString(R.string.hint_minute));
        timeLayout.addView(minuteInput);

        layout.addView(timeLayout);
        builder.setView(layout);

        builder.setPositiveButton(getString(R.string.btn_save), (dialog, which) -> {
            saveSettings(sleepInput, hourInput, minuteInput);
        });

        builder.setNegativeButton(getString(R.string.btn_cancel), null);
        builder.show();
    }

    private void saveSettings(EditText sleepInput, EditText hourInput, EditText minuteInput) {
        try {
            // Parse and validate sleep hours
            String sleepText = sleepInput.getText().toString().trim();
            if (!sleepText.isEmpty()) {
                double sleepHours = Double.parseDouble(sleepText);
                if (sleepHours > 0 && sleepHours <= 12) {
                    setTargetSleepHours(sleepHours);
                }
            }

            // Parse and validate wake time
            String hourText = hourInput.getText().toString().trim();
            String minuteText = minuteInput.getText().toString().trim();

            if (!hourText.isEmpty() && !minuteText.isEmpty()) {
                int hour = Integer.parseInt(hourText);
                int minute = Integer.parseInt(minuteText);

                if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
                    setTargetWakeTime(hour, minute);
                }
            }

            saveData(); // Ensure persistence
            showToast(getString(R.string.msg_settings_saved));
            updateUI();

        } catch (NumberFormatException e) {
            showToast(getString(R.string.error_invalid_time));
        }
    }

    // ========== LIFECYCLE & UPDATES ==========

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

    @Override
    protected void onResume() {
        super.onResume();
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

    // ========== NATIVE METHODS ==========

    // Core functionality
    public native void initializeCore(String dataPath);
    public native void startSleepSession();
    public native void endSleepSession();
    public native boolean isSessionRunning();

    // Settings
    public native void setTargetSleepHours(double hours);
    public native void setTargetWakeTime(int hour, int minute);
    public native double getCurrentTargetSleepHours();
    public native int getCurrentWakeHour();
    public native int getCurrentWakeMinute();

    // Data queries
    public native String getRemainingWorkTimeFormatted();
    public native String getLastSleepDurationFormatted();
    public native String getAverageSleepDurationFormatted(int days);
    public native String getCurrentSessionDurationFormatted();
    public native int getSessionCount();

    // Sleep period detection
    public native boolean isInSleepPeriod();
    public native boolean isBeforeTargetWakeTime();
    public native String getTimeUntilWakeFormatted();

    // Data management
    public native boolean saveData();
    public native boolean exportData(String exportPath);
    public native void clearHistory();

    static {
        System.loadLibrary("descansa");
    }
}