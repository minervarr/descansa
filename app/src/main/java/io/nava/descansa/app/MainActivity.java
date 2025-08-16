package io.nava.descansa.app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AlertDialog;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.os.Handler;
import android.os.Looper;
import java.io.File;
import java.util.Locale;

import io.nava.descansa.app.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;
    private Handler updateHandler;
    private Runnable updateRunnable;
    private boolean isUpdating = false;

    // UI Elements
    private TextView statusText;
    private TextView workTimeText;
    private TextView lastSleepText;
    private TextView averageSleepText;
    private TextView sessionCountText;
    private TextView currentSessionText;
    private Button sleepButton;
    private Button exportButton;
    private Button settingsButton;
    private Button clearDataButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Initialize C++ core with app's data directory
        String dataPath = getFilesDir().getAbsolutePath() + "/descansa_data.txt";
        initializeCore(dataPath);

        // Initialize UI elements
        initializeUI();

        // Set up button listeners
        setupButtonListeners();

        // Start periodic UI updates
        startPeriodicUpdates();

        // Initial UI update
        updateUI();
    }

    private void initializeUI() {
        // Get references to UI elements from the binding
        statusText = binding.statusText;
        workTimeText = binding.workTimeText;
        lastSleepText = binding.lastSleepText;
        averageSleepText = binding.averageSleepText;
        sessionCountText = binding.sessionCountText;
        currentSessionText = binding.currentSessionText;
        sleepButton = binding.sleepButton;
        exportButton = binding.exportButton;
        settingsButton = binding.settingsButton;
        clearDataButton = binding.clearDataButton;
    }

    private void setupButtonListeners() {
        // Sleep/Wake button
        sleepButton.setOnClickListener(v -> {
            if (isSessionRunning()) {
                endSleepSession();
                showToast(getString(R.string.msg_sleep_ended));
            } else {
                startSleepSession();
                showToast(getString(R.string.msg_sleep_started));
            }
            updateUI();
        });

        // Export data button
        exportButton.setOnClickListener(v -> exportData());

        // Settings button
        settingsButton.setOnClickListener(v -> showSettingsDialog());

        // Clear data button (for debugging)
        clearDataButton.setOnClickListener(v -> {
            new AlertDialog.Builder(this)
                    .setTitle("Clear All Data")
                    .setMessage("This will delete all sleep session data. Continue?")
                    .setPositiveButton("Clear", (dialog, which) -> {
                        clearHistory();
                        showToast(getString(R.string.msg_data_cleared));
                        updateUI();
                    })
                    .setNegativeButton("Cancel", null)
                    .show();
        });
    }

    private void updateUI() {
        // Update status
        if (isSessionRunning()) {
            statusText.setText(getString(R.string.status_sleeping));
            sleepButton.setText(getString(R.string.btn_end_sleep));

            // Show current session duration
            double currentHours = getCurrentSessionHours();
            currentSessionText.setText(String.format(Locale.getDefault(),
                    "Current session: %.1f hours", currentHours));
            currentSessionText.setVisibility(TextView.VISIBLE);

            // Hide work time when sleeping
            workTimeText.setText("Work time: Sleeping...");
        } else {
            statusText.setText(getString(R.string.status_awake));
            sleepButton.setText(getString(R.string.btn_start_sleep));
            currentSessionText.setVisibility(TextView.GONE);

            // Show remaining work time
            double workHours = getRemainingWorkHours();
            if (workHours > 0) {
                workTimeText.setText(String.format(Locale.getDefault(),
                        "%s %.1f hours", getString(R.string.label_work_time_remaining), workHours));
            } else {
                workTimeText.setText("Work time: Past bedtime!");
            }
        }

        // Update sleep statistics
        double lastSleep = getLastSleepHours();
        if (lastSleep > 0) {
            lastSleepText.setText(String.format(Locale.getDefault(),
                    "%s %.1f hours", getString(R.string.label_last_sleep_duration), lastSleep));
        } else {
            lastSleepText.setText(getString(R.string.label_last_sleep_duration) + " " +
                    getString(R.string.default_no_data));
        }

        double avgSleep = getAverageSleepHours(7);
        if (avgSleep > 0) {
            averageSleepText.setText(String.format(Locale.getDefault(),
                    "%s %.1f hours", getString(R.string.label_average_sleep), avgSleep));
        } else {
            averageSleepText.setText(getString(R.string.label_average_sleep) + " " +
                    getString(R.string.default_no_data));
        }

        int sessionCount = getSessionCount();
        sessionCountText.setText(String.format(Locale.getDefault(),
                "%s %d", getString(R.string.label_session_count), sessionCount));
    }

    private void exportData() {
        try {
            // Export to Documents directory
            File documentsDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOCUMENTS);
            documentsDir.mkdirs(); // Create if doesn't exist

            String timestamp = String.valueOf(System.currentTimeMillis());
            String filename = "descansa_sleep_data_" + timestamp + ".csv";
            File exportFile = new File(documentsDir, filename);

            boolean success = exportData(exportFile.getAbsolutePath());

            if (success) {
                showToast(getString(R.string.msg_data_exported) + "\n" + filename);
            } else {
                showToast(getString(R.string.error_export_failed));
            }
        } catch (Exception e) {
            showToast(getString(R.string.error_export_failed) + ": " + e.getMessage());
        }
    }

    private void showSettingsDialog() {
        // Create a simple settings dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Sleep Settings");

        // Create input layout
        android.widget.LinearLayout layout = new android.widget.LinearLayout(this);
        layout.setOrientation(android.widget.LinearLayout.VERTICAL);
        layout.setPadding(50, 50, 50, 50);

        // Target sleep hours input
        TextView sleepLabel = new TextView(this);
        sleepLabel.setText(getString(R.string.label_target_sleep_hours));
        layout.addView(sleepLabel);

        EditText sleepInput = new EditText(this);
        sleepInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER |
                android.text.InputType.TYPE_NUMBER_FLAG_DECIMAL);
        sleepInput.setHint("8.0");
        layout.addView(sleepInput);

        // Wake hour input
        TextView wakeHourLabel = new TextView(this);
        wakeHourLabel.setText(getString(R.string.label_wake_hour));
        layout.addView(wakeHourLabel);

        EditText wakeHourInput = new EditText(this);
        wakeHourInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        wakeHourInput.setHint("7");
        layout.addView(wakeHourInput);

        // Wake minute input
        TextView wakeMinuteLabel = new TextView(this);
        wakeMinuteLabel.setText(getString(R.string.label_wake_minute));
        layout.addView(wakeMinuteLabel);

        EditText wakeMinuteInput = new EditText(this);
        wakeMinuteInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
        wakeMinuteInput.setHint("0");
        layout.addView(wakeMinuteInput);

        builder.setView(layout);

        builder.setPositiveButton("Save", (dialog, which) -> {
            try {
                // Parse and set target sleep hours
                String sleepText = sleepInput.getText().toString().trim();
                if (!sleepText.isEmpty()) {
                    double sleepHours = Double.parseDouble(sleepText);
                    if (sleepHours > 0 && sleepHours <= 12) {
                        setTargetSleepHours(sleepHours);
                    }
                }

                // Parse and set wake time
                String hourText = wakeHourInput.getText().toString().trim();
                String minuteText = wakeMinuteInput.getText().toString().trim();

                if (!hourText.isEmpty() && !minuteText.isEmpty()) {
                    int hour = Integer.parseInt(hourText);
                    int minute = Integer.parseInt(minuteText);

                    if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
                        setTargetWakeTime(hour, minute);
                    }
                }

                saveData(); // Save settings
                showToast(getString(R.string.msg_settings_saved));
                updateUI();

            } catch (NumberFormatException e) {
                showToast(getString(R.string.error_invalid_time));
            }
        });

        builder.setNegativeButton("Cancel", null);
        builder.show();
    }

    private void startPeriodicUpdates() {
        updateHandler = new Handler(Looper.getMainLooper());
        updateRunnable = new Runnable() {
            @Override
            public void run() {
                if (isUpdating) {
                    updateUI();
                    updateHandler.postDelayed(this, 30000); // Update every 30 seconds
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

    private double getCurrentSessionHours() {
        // This would need a new JNI method to get current session duration
        // For now, return 0 - this is a placeholder for future implementation
        return 0.0;
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
        saveData(); // Save data when app goes to background
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopPeriodicUpdates();
        saveData(); // Final save
    }

    // Native method declarations - all implemented in native-lib.cpp
    public native void initializeCore(String dataPath);
    public native void startSleepSession();
    public native void endSleepSession();
    public native boolean isSessionRunning();
    public native double getLastSleepHours();
    public native double getRemainingWorkHours();
    public native double getAverageSleepHours(int days);
    public native int getSessionCount();
    public native void setTargetSleepHours(double hours);
    public native void setTargetWakeTime(int hour, int minute);
    public native boolean saveData();
    public native boolean exportData(String exportPath);
    public native void clearHistory();
    public native String getStatusSummary();
    public native String formatDuration(double hours);

    // Load the native library
    static {
        System.loadLibrary("descansa");
    }
}