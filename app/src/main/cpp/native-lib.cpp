#include <jni.h>
#include <string>
#include <memory>
#include <android/log.h>
#include "DescansaCore.h"

#define LOG_TAG "DescansaNative"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// SIMPLIFIED: Only one core instance
static std::unique_ptr<descansa::DescansaCore> g_core;

// Helper function to ensure core is initialized
void ensure_core_initialized(const std::string& data_path = "") {
    if (!g_core) {
        LOGD("Initializing core with path: %s", data_path.c_str());
        g_core.reset(new descansa::DescansaCore(data_path));
        LOGD("Core initialized successfully");
    }
}

extern "C" {

// ========== CORE INITIALIZATION ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_initializeCore(
        JNIEnv* env, jobject, jstring data_path) {

    const char* path_chars = env->GetStringUTFChars(data_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(data_path, path_chars);

    LOGD("=== INITIALIZING CORE ===");
    LOGD("Data path: %s", path.c_str());

    g_core.reset(new descansa::DescansaCore(path));

    // Log the loaded configuration to verify settings are preserved
    const auto& config = g_core->get_config();
    double hours = config.target_sleep_hours.count() / 3600.0;
    int wake_hour = static_cast<int>(config.target_wake_hour.count());
    int wake_minute = static_cast<int>(config.target_wake_minute.count());

    LOGD("Core initialized with settings:");
    LOGD("  - Sleep hours: %.2f", hours);
    LOGD("  - Wake time: %d:%02d", wake_hour, wake_minute);
    LOGD("  - Session count: %zu", g_core->get_session_count());
    LOGD("=== CORE INITIALIZATION COMPLETE ===");
}

// ========== SESSION MANAGEMENT ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_startSleepSession(JNIEnv*, jobject) {
    ensure_core_initialized();
    LOGD("Starting sleep session");
    g_core->start_sleep_session();

    // FORCE SAVE immediately after starting
    bool saved = g_core->save_data();
    LOGD("Session started, data saved: %s", saved ? "true" : "false");
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_endSleepSession(JNIEnv*, jobject) {
    ensure_core_initialized();
    LOGD("Ending sleep session");
    g_core->end_sleep_session();

    // FORCE SAVE immediately after ending
    bool saved = g_core->save_data();
    LOGD("Session ended, data saved: %s", saved ? "true" : "false");
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isSessionRunning(JNIEnv*, jobject) {
    ensure_core_initialized();
    bool running = g_core->is_session_running();
    LOGD("Session running check: %s", running ? "true" : "false");
    return running;
}

// ========== SETTINGS ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setTargetSleepHours(JNIEnv*, jobject, jdouble hours) {
    ensure_core_initialized();
    LOGD("=== SETTING TARGET SLEEP HOURS: %.2f ===", hours);
    g_core->set_target_sleep_hours(hours);

    // Save settings immediately and verify
    bool saved = g_core->save_data();
    LOGD("Sleep hours set and saved: %s", saved ? "SUCCESS" : "FAILED");

    // Verify the setting was actually stored
    const auto& config = g_core->get_config();
    double stored_hours = config.target_sleep_hours.count() / 3600.0;
    LOGD("Verification - stored sleep hours: %.2f", stored_hours);
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setTargetWakeTime(JNIEnv*, jobject, jint hour, jint minute) {
    ensure_core_initialized();
    LOGD("=== SETTING TARGET WAKE TIME: %d:%02d ===", hour, minute);
    g_core->set_target_wake_time(hour, minute);

    // Save settings immediately and verify
    bool saved = g_core->save_data();
    LOGD("Wake time set and saved: %s", saved ? "SUCCESS" : "FAILED");

    // Verify the setting was actually stored
    const auto& config = g_core->get_config();
    int stored_hour = static_cast<int>(config.target_wake_hour.count());
    int stored_minute = static_cast<int>(config.target_wake_minute.count());
    LOGD("Verification - stored wake time: %d:%02d", stored_hour, stored_minute);
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentTargetSleepHours(JNIEnv*, jobject) {
    ensure_core_initialized();
    const auto& config = g_core->get_config();
    double hours = config.target_sleep_hours.count() / 3600.0;
    LOGD("Current target sleep hours: %.1f", hours);
    return hours;
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentWakeHour(JNIEnv*, jobject) {
    ensure_core_initialized();
    const auto& config = g_core->get_config();
    int hour = static_cast<int>(config.target_wake_hour.count());
    LOGD("Current wake hour: %d", hour);
    return hour;
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentWakeMinute(JNIEnv*, jobject) {
    ensure_core_initialized();
    const auto& config = g_core->get_config();
    int minute = static_cast<int>(config.target_wake_minute.count());
    LOGD("Current wake minute: %d", minute);
    return minute;
}

// ========== DATA QUERIES ==========

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getRemainingWorkTimeFormatted(JNIEnv* env, jobject) {
    ensure_core_initialized();
    auto duration = g_core->get_remaining_work_time();
    std::string formatted = descansa::utils::format_duration(duration);
    LOGD("Remaining work time: %s", formatted.c_str());
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getLastSleepDurationFormatted(JNIEnv* env, jobject) {
    ensure_core_initialized();
    auto duration = g_core->get_last_sleep_duration();
    std::string formatted = descansa::utils::format_duration(duration);
    LOGD("Last sleep duration: %s", formatted.c_str());
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getAverageSleepDurationFormatted(JNIEnv* env, jobject, jint days) {
    ensure_core_initialized();
    auto duration = g_core->get_average_sleep_duration(days);
    std::string formatted = descansa::utils::format_duration(duration);
    LOGD("Average sleep duration (%d days): %s", days, formatted.c_str());
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentSessionDurationFormatted(JNIEnv* env, jobject) {
    ensure_core_initialized();
    auto duration = g_core->get_current_session_duration();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getSessionCount(JNIEnv*, jobject) {
    ensure_core_initialized();
    int count = static_cast<int>(g_core->get_session_count());
    LOGD("Session count: %d", count);
    return count;
}

// ========== SLEEP PERIOD DETECTION ==========

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isInSleepPeriod(JNIEnv*, jobject) {
    ensure_core_initialized();
    bool inSleep = g_core->is_in_sleep_period();
    LOGD("In sleep period: %s", inSleep ? "true" : "false");
    return inSleep;
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isBeforeTargetWakeTime(JNIEnv*, jobject) {
    ensure_core_initialized();
    bool beforeWake = g_core->is_before_target_wake_time();
    LOGD("Before target wake: %s", beforeWake ? "true" : "false");
    return beforeWake;
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getTimeUntilWakeFormatted(JNIEnv* env, jobject) {
    ensure_core_initialized();
    auto duration = g_core->get_time_until_target_wake();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getTimeUntilNextWakeFormatted(JNIEnv* env, jobject) {
    ensure_core_initialized();
    auto duration = g_core->get_time_until_next_wake();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getNextWakeTimeFormatted(JNIEnv* env, jobject) {
    ensure_core_initialized();
    std::string formatted = g_core->get_next_wake_time_formatted();
    return env->NewStringUTF(formatted.c_str());
}

// ========== DATA MANAGEMENT ==========

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_saveData(JNIEnv*, jobject) {
    if (!g_core) {
        LOGE("Cannot save data - core not initialized");
        return false;
    }

    bool saved = g_core->save_data();
    LOGD("=== SAVE DATA CALLED - Result: %s ===", saved ? "SUCCESS" : "FAILED");
    return saved;
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_exportAnalysisCsv(JNIEnv* env, jobject, jstring export_path) {
    ensure_core_initialized();

    const char* path_chars = env->GetStringUTFChars(export_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(export_path, path_chars);

    LOGD("Exporting CSV to: %s", path.c_str());
    bool success = g_core->export_analysis_csv(path);
    LOGD("Export result: %s", success ? "SUCCESS" : "FAILED");

    return success;
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_clearHistory(JNIEnv*, jobject) {
    ensure_core_initialized();
    LOGD("=== CLEARING ALL DATA ===");
    g_core->clear_history();
    LOGD("Data cleared");
}

} // extern "C"