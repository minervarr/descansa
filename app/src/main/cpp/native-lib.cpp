#include <jni.h>
#include <string>
#include <memory>
#include "DescansaCore.h"

// Global core instance
static std::unique_ptr<descansa::DescansaCore> g_core;

// Helper function to initialize core if needed
void ensure_core_initialized(const std::string& data_path = "") {
    if (!g_core) {
        g_core.reset(new descansa::DescansaCore(data_path));
    }
}

extern "C" {

// Initialize the core with data path
JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_initializeCore(
        JNIEnv* env, jobject /* this */, jstring data_path) {

    const char* path_chars = env->GetStringUTFChars(data_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(data_path, path_chars);

    g_core.reset(new descansa::DescansaCore(path));
}

// Session management
JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_startSleepSession(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    g_core->start_sleep_session();
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_endSleepSession(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    g_core->end_sleep_session();
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isSessionRunning(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    return g_core->is_session_running();
}

// Configuration
JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setTargetSleepHours(
        JNIEnv* env, jobject /* this */, jdouble hours) {
    ensure_core_initialized();
    g_core->set_target_sleep_hours(hours);
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setTargetWakeTime(
        JNIEnv* env, jobject /* this */, jint hour, jint minute) {
    ensure_core_initialized();
    g_core->set_target_wake_time(hour, minute);
}

// Main status queries
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getStatusSummary(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    std::string status = g_core->get_status_summary();
    return env->NewStringUTF(status.c_str());
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getLastSleepHours(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    auto duration = g_core->get_last_sleep_duration();
    return duration.count() / 3600.0; // Convert to hours
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getRemainingWorkHours(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    auto duration = g_core->get_remaining_work_time();
    return duration.count() / 3600.0; // Convert to hours
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getAverageSleepHours(
        JNIEnv* env, jobject /* this */, jint days) {
    ensure_core_initialized();
    auto duration = g_core->get_average_sleep_duration(days);
    return duration.count() / 3600.0; // Convert to hours
}

// Data management
JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_saveData(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    return g_core->save_data();
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_exportData(
        JNIEnv* env, jobject /* this */, jstring export_path) {
    ensure_core_initialized();

    const char* path_chars = env->GetStringUTFChars(export_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(export_path, path_chars);

    return g_core->export_data(path);
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_clearHistory(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    g_core->clear_history();
}

// Statistics
JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getSessionCount(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    return static_cast<jint>(g_core->get_session_count());
}

// Utility functions for formatted strings
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_formatDuration(
        JNIEnv* env, jobject /* this */, jdouble hours) {
    descansa::Duration d(hours * 3600.0);
    std::string formatted = descansa::utils::format_duration(d);
    return env->NewStringUTF(formatted.c_str());
}

// Formatted remaining work time
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getRemainingWorkTimeFormatted(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    auto duration = g_core->get_remaining_work_time();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

// Formatted last sleep duration
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getLastSleepDurationFormatted(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    auto duration = g_core->get_last_sleep_duration();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

// Formatted average sleep duration
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getAverageSleepDurationFormatted(
        JNIEnv* env, jobject /* this */, jint days) {
    ensure_core_initialized();
    auto duration = g_core->get_average_sleep_duration(days);
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

// Formatted current session duration - NOW PROPERLY IMPLEMENTED
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentSessionDurationFormatted(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();

    auto duration = g_core->get_current_session_duration();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

// Sleep period detection
JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isInSleepPeriod(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    return g_core->is_in_sleep_period();
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isBeforeTargetWakeTime(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    return g_core->is_before_target_wake_time();
}

// Formatted time until wake
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getTimeUntilWakeFormatted(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    auto duration = g_core->get_time_until_target_wake();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

// Get current target sleep hours
JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentTargetSleepHours(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    const auto& config = g_core->get_config();
    return config.target_sleep_hours.count() / 3600.0; // Convert to hours
}

// Get current wake hour
JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentWakeHour(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    const auto& config = g_core->get_config();
    return static_cast<jint>(config.target_wake_hour.count());
}

// Get current wake minute
JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentWakeMinute(
        JNIEnv* env, jobject /* this */) {
    ensure_core_initialized();
    const auto& config = g_core->get_config();
    return static_cast<jint>(config.target_wake_minute.count());
}

} // extern "C"