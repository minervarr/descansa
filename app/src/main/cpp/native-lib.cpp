#include <jni.h>
#include <string>
#include <memory>
#include <vector>
#include "DescansaCore.h"
#include "DescansaCoreManager.h"

// Enhanced global instances - both basic and advanced
static std::unique_ptr<descansa::DescansaCore> g_basic_core;
static std::unique_ptr<descansa::DescansaCoreManager> g_enhanced_core;

// Helper function to ensure cores are initialized
void ensure_cores_initialized(const std::string& data_path = "") {
    if (!g_basic_core) {
        g_basic_core.reset(new descansa::DescansaCore(data_path));
    }
    if (!g_enhanced_core) {
        std::string enhanced_path = data_path.empty() ? "enhanced_data" : data_path + "_enhanced";
        g_enhanced_core.reset(new descansa::DescansaCoreManager(enhanced_path));
    }
}

extern "C" {

// ========== INITIALIZATION ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_initializeCore(
        JNIEnv* env, jobject, jstring data_path) {

    const char* path_chars = env->GetStringUTFChars(data_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(data_path, path_chars);

    // Initialize both cores
    g_basic_core.reset(new descansa::DescansaCore(path));
    std::string enhanced_path = path + "_enhanced";
    g_enhanced_core.reset(new descansa::DescansaCoreManager(enhanced_path));
}

// ========== ENHANCED SESSION MANAGEMENT ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_startSleepSession(JNIEnv*, jobject) {
    ensure_cores_initialized();
    g_basic_core->start_sleep_session();
    g_enhanced_core->start_enhanced_sleep_session();
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_endSleepSession(JNIEnv*, jobject) {
    ensure_cores_initialized();
    g_basic_core->end_sleep_session();
    g_enhanced_core->end_enhanced_sleep_session();
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isSessionRunning(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_enhanced_core->is_enhanced_session_active();
}

// ========== SLEEP QUALITY TRACKING ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setSleepQuality(JNIEnv*, jobject, jint quality) {
    ensure_cores_initialized();
    auto sleep_quality = static_cast<descansa::SleepQuality>(quality);
    g_enhanced_core->set_sleep_quality(sleep_quality);
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_addSessionNote(JNIEnv* env, jobject, jstring note) {
    ensure_cores_initialized();
    const char* note_chars = env->GetStringUTFChars(note, nullptr);
    std::string note_str(note_chars);
    env->ReleaseStringUTFChars(note, note_chars);

    g_enhanced_core->add_session_note(note_str);
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_markAsNap(JNIEnv*, jobject, jboolean is_nap) {
    ensure_cores_initialized();
    g_enhanced_core->mark_as_nap(is_nap);
}

// ========== ENVIRONMENTAL TRACKING ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_updateEnvironment(
        JNIEnv*, jobject, jdouble temperature, jint noise_level, jint light_level) {
    ensure_cores_initialized();

    descansa::SleepEnvironment env;
    env.temperature = temperature;
    env.noise_level = noise_level;
    env.light_level = light_level;
    env.measurement_time = descansa::utils::now();

    g_enhanced_core->update_environment_data(env);
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_recordCaffeineIntake(JNIEnv*, jobject) {
    ensure_cores_initialized();
    g_enhanced_core->record_caffeine_intake(descansa::utils::now());
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_recordScreenTimeEnd(JNIEnv*, jobject) {
    ensure_cores_initialized();
    g_enhanced_core->record_screen_time_end(descansa::utils::now());
}

// ========== ADVANCED ANALYTICS ==========

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getSleepScore(JNIEnv*, jobject) {
    ensure_cores_initialized();
    auto recent_summaries = g_enhanced_core->get_recent_summaries(1);
    if (recent_summaries.empty()) return 0.0;
    return recent_summaries.back().get_sleep_score();
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getSleepScoreExplanation(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    std::string explanation = g_enhanced_core->get_sleep_score_explanation();
    return env->NewStringUTF(explanation.c_str());
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getSleepEfficiency(JNIEnv*, jobject) {
    ensure_cores_initialized();
    auto sessions = g_enhanced_core->get_sessions(1);
    if (sessions.empty()) return 0.0;
    return sessions.back().sleep_efficiency;
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentRecommendations(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto recommendations = g_enhanced_core->get_current_recommendations();

    std::string result;
    for (size_t i = 0; i < recommendations.size(); ++i) {
        if (i > 0) result += "\n• ";
        else result += "• ";
        result += recommendations[i];
    }

    return env->NewStringUTF(result.c_str());
}

// ========== SLEEP DEBT MANAGEMENT ==========

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getSleepDebtFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto debt = g_enhanced_core->calculate_current_sleep_debt();

    if (debt.count() <= 0) {
        return env->NewStringUTF("No debt");
    }

    std::string formatted = descansa::utils::format_duration(debt);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isInSleepDebt(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_enhanced_core->is_in_sleep_debt();
}

// ========== ENHANCED GOAL MANAGEMENT ==========

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setEnhancedSleepGoals(
        JNIEnv*, jobject, jdouble target_hours, jint bedtime_hour, jint wake_hour,
        jdouble target_efficiency) {
    ensure_cores_initialized();

    descansa::SleepGoals goals;
    goals.target_sleep_duration = descansa::Duration(target_hours * 3600.0);
    goals.preferred_bedtime = std::chrono::hours(bedtime_hour);
    goals.preferred_wake_time = std::chrono::hours(wake_hour);
    goals.target_sleep_efficiency = target_efficiency;

    g_enhanced_core->set_sleep_goals(goals);

    // Sync with basic core
    g_basic_core->set_target_sleep_hours(target_hours);
    g_basic_core->set_target_wake_time(wake_hour, 0);
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getGoalAdherence(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_enhanced_core->get_goal_adherence_percentage();
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isMeetingGoals(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_enhanced_core->is_meeting_goals();
}

// ========== PATTERN ANALYSIS ==========

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getSleepPatterns(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto patterns = g_enhanced_core->identify_sleep_patterns();

    std::string result;
    for (size_t i = 0; i < patterns.size(); ++i) {
        if (i > 0) result += "\n";
        result += patterns[i];
    }

    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getImprovementSuggestions(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto suggestions = g_enhanced_core->get_improvement_suggestions();

    std::string result;
    for (size_t i = 0; i < suggestions.size(); ++i) {
        if (i > 0) result += "\n• ";
        else result += "• ";
        result += suggestions[i];
    }

    return env->NewStringUTF(result.c_str());
}

// ========== STATISTICS ==========

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getWeeklyStats(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto stats = g_enhanced_core->calculate_recent_statistics(7);
    return env->NewStringUTF(stats.generate_summary_report().c_str());
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getAwakeningsCount(JNIEnv*, jobject) {
    ensure_cores_initialized();
    auto sessions = g_enhanced_core->get_sessions(1);
    if (sessions.empty()) return 0;
    return sessions.back().awakenings_count;
}

// ========== ENHANCED DATA EXPORT ==========

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_exportDetailedData(JNIEnv* env, jobject, jstring export_path) {
    ensure_cores_initialized();

    const char* path_chars = env->GetStringUTFChars(export_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(export_path, path_chars);

    return g_enhanced_core->export_detailed_data(path);
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_exportSummaryCsv(JNIEnv* env, jobject, jstring export_path) {
    ensure_cores_initialized();

    const char* path_chars = env->GetStringUTFChars(export_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(export_path, path_chars);

    return g_enhanced_core->export_summary_csv(path);
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_backupAllData(JNIEnv* env, jobject, jstring backup_path) {
    ensure_cores_initialized();

    const char* path_chars = env->GetStringUTFChars(backup_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(backup_path, path_chars);

    return g_enhanced_core->backup_all_data(path);
}

// ========== SYSTEM STATUS ==========

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getSystemStatus(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    std::string status = g_enhanced_core->get_system_status();
    return env->NewStringUTF(status.c_str());
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_runDiagnostics(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_enhanced_core->run_diagnostics();
}

// ========== LEGACY COMPATIBILITY ==========
// Keep existing methods for backward compatibility

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setTargetSleepHours(JNIEnv*, jobject, jdouble hours) {
    ensure_cores_initialized();
    g_basic_core->set_target_sleep_hours(hours);
    g_enhanced_core->update_target_sleep_duration(descansa::Duration(hours * 3600.0));
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_setTargetWakeTime(JNIEnv*, jobject, jint hour, jint minute) {
    ensure_cores_initialized();
    g_basic_core->set_target_wake_time(hour, minute);
    g_enhanced_core->update_preferred_schedule(std::chrono::hours(hour), std::chrono::hours(hour));
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_saveData(JNIEnv*, jobject) {
    ensure_cores_initialized();
    bool basic_saved = g_basic_core->save_data();
    bool enhanced_saved = g_enhanced_core->save_all_data();
    return basic_saved && enhanced_saved;
}

JNIEXPORT void JNICALL
Java_io_nava_descansa_app_MainActivity_clearHistory(JNIEnv*, jobject) {
    ensure_cores_initialized();
    g_basic_core->clear_history();
    g_enhanced_core->clear_all_data();
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getSessionCount(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return static_cast<jint>(g_enhanced_core->get_sessions().size());
}

// Keep all existing formatted methods...
JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getRemainingWorkTimeFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto duration = g_enhanced_core->get_enhanced_remaining_work_time();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getLastSleepDurationFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto sessions = g_enhanced_core->get_sessions(1);
    if (sessions.empty()) {
        return env->NewStringUTF("0h 0m");
    }
    std::string formatted = descansa::utils::format_duration(sessions.back().total_sleep_duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getAverageSleepDurationFormatted(JNIEnv* env, jobject, jint days) {
    ensure_cores_initialized();
    auto summaries = g_enhanced_core->get_recent_summaries(days);
    if (summaries.empty()) {
        return env->NewStringUTF("0h 0m");
    }

    descansa::Duration total(0);
    for (const auto& summary : summaries) {
        total += summary.total_sleep_time;
    }
    descansa::Duration average(total.count() / summaries.size());

    std::string formatted = descansa::utils::format_duration(average);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentSessionDurationFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto preview = g_enhanced_core->get_current_session_preview();
    std::string formatted = descansa::utils::format_duration(preview.total_sleep_duration);
    return env->NewStringUTF(formatted.c_str());
}

// Keep sleep period detection methods...
JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isInSleepPeriod(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_basic_core->is_in_sleep_period();
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_isBeforeTargetWakeTime(JNIEnv*, jobject) {
    ensure_cores_initialized();
    return g_basic_core->is_before_target_wake_time();
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getTimeUntilWakeFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto duration = g_basic_core->get_time_until_target_wake();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getTimeUntilNextWakeFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    auto duration = g_basic_core->get_time_until_next_wake();
    std::string formatted = descansa::utils::format_duration(duration);
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jstring JNICALL
Java_io_nava_descansa_app_MainActivity_getNextWakeTimeFormatted(JNIEnv* env, jobject) {
    ensure_cores_initialized();
    std::string formatted = g_basic_core->get_next_wake_time_formatted();
    return env->NewStringUTF(formatted.c_str());
}

JNIEXPORT jdouble JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentTargetSleepHours(JNIEnv*, jobject) {
    ensure_cores_initialized();
    const auto& config = g_basic_core->get_config();
    return config.target_sleep_hours.count() / 3600.0;
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentWakeHour(JNIEnv*, jobject) {
    ensure_cores_initialized();
    const auto& config = g_basic_core->get_config();
    return static_cast<jint>(config.target_wake_hour.count());
}

JNIEXPORT jint JNICALL
Java_io_nava_descansa_app_MainActivity_getCurrentWakeMinute(JNIEnv*, jobject) {
    ensure_cores_initialized();
    const auto& config = g_basic_core->get_config();
    return static_cast<jint>(config.target_wake_minute.count());
}

JNIEXPORT jboolean JNICALL
Java_io_nava_descansa_app_MainActivity_exportAnalysisCsv(JNIEnv* env, jobject, jstring export_path) {
    ensure_cores_initialized();

    const char* path_chars = env->GetStringUTFChars(export_path, nullptr);
    std::string path(path_chars);
    env->ReleaseStringUTFChars(export_path, path_chars);

    return g_basic_core->export_analysis_csv(path);
}

} // extern "C"