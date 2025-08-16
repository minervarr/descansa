#ifndef DESCANSA_CORE_MANAGER_H
#define DESCANSA_CORE_MANAGER_H

#include "DescansaCore.h"
#include "SleepDataStructures.h"
#include <memory>
#include <functional>
#include <vector>
#include <string>

namespace descansa {

// Enhanced core manager with comprehensive sleep tracking
    class DescansaCoreManager {
    private:
        std::unique_ptr<DescansaCore> basic_core;

        // Enhanced data storage
        std::vector<DetailedSleepSession> detailed_sessions;
        std::vector<DailySleepSummary> daily_summaries;
        std::vector<WeeklySleepPattern> weekly_patterns;
        SleepGoals user_goals;
        SleepEnvironment current_environment;

        // Current session tracking
        DetailedSleepSession current_session;
        bool enhanced_session_active;
        TimePoint session_start_time;

        // Data persistence
        std::string data_directory;
        std::string sessions_file;
        std::string summaries_file;
        std::string goals_file;
        std::string environment_file;

        // Analytics and callbacks
        std::function<void(const DetailedSleepSession&)> session_completed_callback;
        std::function<void(const DailySleepSummary&)> daily_summary_callback;

        // Helper methods
        void update_daily_summary(const DetailedSleepSession& session);
        void update_weekly_patterns();
        void analyze_sleep_trends();
        void generate_recommendations();
        TimePoint get_day_start(const TimePoint& tp) const;
        bool is_same_calendar_day(const TimePoint& t1, const TimePoint& t2) const;

    public:
        explicit DescansaCoreManager(const std::string& data_dir = "");
        ~DescansaCoreManager();

        // Basic session management (enhanced)
        void start_enhanced_sleep_session();
        void end_enhanced_sleep_session();
        void pause_session(); // For interruptions
        void resume_session();
        bool is_enhanced_session_active() const { return enhanced_session_active; }

        // Session data input during tracking
        void update_environment_data(const SleepEnvironment& env);
        void add_awakening(const TimePoint& time, const Duration& duration);
        void set_sleep_quality(SleepQuality quality);
        void add_session_note(const std::string& note);
        void mark_as_nap(bool is_nap = true);

        // Pre-sleep factor tracking
        void record_caffeine_intake(const TimePoint& time);
        void record_meal_time(const TimePoint& time);
        void record_exercise(const TimePoint& time);
        void record_screen_time_end(const TimePoint& time);
        void record_medication(const std::string& medication);

        // Goals and preferences management
        void set_sleep_goals(const SleepGoals& goals);
        const SleepGoals& get_sleep_goals() const { return user_goals; }
        void update_target_sleep_duration(const Duration& duration);
        void update_preferred_schedule(std::chrono::hours bedtime, std::chrono::hours wake_time);
        void set_weekend_flexibility(bool allow_flexibility, const Duration& extension = Duration(0));

        // Data retrieval and analysis
        std::vector<DetailedSleepSession> get_sessions(int count = -1) const;
        std::vector<DetailedSleepSession> get_sessions_in_range(const TimePoint& start, const TimePoint& end) const;
        DailySleepSummary get_daily_summary(const TimePoint& date) const;
        std::vector<DailySleepSummary> get_recent_summaries(int days = 30) const;
        WeeklySleepPattern get_weekly_pattern(const TimePoint& week_start) const;
        std::vector<WeeklySleepPattern> get_recent_weekly_patterns(int weeks = 4) const;

        // Current status and recommendations
        DetailedSleepSession get_current_session_preview() const;
        Duration get_enhanced_remaining_work_time() const;
        std::vector<std::string> get_current_recommendations() const;
        std::string get_sleep_score_explanation() const;
        bool is_meeting_goals() const;

        // Statistics and analytics
        SleepStatistics calculate_statistics(const TimePoint& start, const TimePoint& end) const;
        SleepStatistics calculate_recent_statistics(int days = 30) const;
        double get_goal_adherence_percentage() const;
        std::vector<std::string> identify_sleep_patterns() const;
        std::vector<std::string> get_improvement_suggestions() const;

        // Sleep debt and recovery
        Duration calculate_current_sleep_debt() const;
        Duration calculate_cumulative_sleep_debt(int days = 7) const;
        std::vector<TimePoint> suggest_recovery_sleep_times() const;
        bool is_in_sleep_debt() const;

        // Data export and backup
        bool export_detailed_data(const std::string& export_path) const;
        bool export_summary_csv(const std::string& export_path) const;
        bool export_weekly_patterns_json(const std::string& export_path) const;
        bool backup_all_data(const std::string& backup_path) const;
        bool restore_from_backup(const std::string& backup_path);

        // Data management
        bool save_all_data() const;
        bool load_all_data();
        void clear_all_data();
        void clear_old_data(int days_to_keep = 365);
        bool validate_data_integrity() const;

        // Event callbacks
        void set_session_completed_callback(std::function<void(const DetailedSleepSession&)> callback);
        void set_daily_summary_callback(std::function<void(const DailySleepSummary&)> callback);

        // Compatibility with basic core
        DescansaCore* get_basic_core() const { return basic_core.get(); }
        void sync_with_basic_core();

        // Advanced features
        std::vector<SleepPhase> detect_sleep_phases() const; // Placeholder for future sensor integration
        void calibrate_sleep_detection(); // Auto-tune detection algorithms
        std::vector<std::string> get_environmental_recommendations() const;
        void optimize_schedule_for_goals();

        // Debugging and diagnostics
        std::string get_system_status() const;
        bool run_diagnostics() const;
        std::vector<std::string> get_data_warnings() const;
    };

// Utility classes for specific analysis
    class SleepTrendAnalyzer {
    private:
        const std::vector<DailySleepSummary>& daily_data;

    public:
        explicit SleepTrendAnalyzer(const std::vector<DailySleepSummary>& data);

        enum class TrendType {
            DURATION,
            QUALITY,
            EFFICIENCY,
            CONSISTENCY,
            BEDTIME,
            WAKE_TIME
        };

        SleepStatistics::Trend analyze_trend(TrendType type, int days = 14) const;
        double calculate_trend_strength(TrendType type, int days = 14) const;
        std::vector<std::string> generate_trend_insights() const;
        bool detect_pattern_changes() const;
    };

    class SleepScheduleOptimizer {
    private:
        const SleepGoals& goals;
        const std::vector<DetailedSleepSession>& historical_data;

    public:
        SleepScheduleOptimizer(const SleepGoals& user_goals,
                               const std::vector<DetailedSleepSession>& history);

        struct OptimalSchedule {
            std::chrono::hours recommended_bedtime;
            std::chrono::hours recommended_wake_time;
            Duration recommended_sleep_duration;
            double confidence_score;
            std::vector<std::string> reasoning;
        };

        OptimalSchedule calculate_optimal_schedule() const;
        std::vector<std::string> get_schedule_adjustment_tips() const;
        bool should_adjust_current_schedule() const;
        Duration calculate_adjustment_period() const; // How long to phase in changes
    };

    class SleepEnvironmentAnalyzer {
    private:
        const std::vector<DetailedSleepSession>& sessions;

        // Helper method for correlation calculation
        double calculate_correlation(const std::vector<double>& x, const std::vector<double>& y) const;

    public:
        explicit SleepEnvironmentAnalyzer(const std::vector<DetailedSleepSession>& session_data);

        struct EnvironmentCorrelation {
            std::string factor;
            double correlation_strength; // -1 to 1
            std::string impact_description;
            std::vector<std::string> recommendations;
        };

        std::vector<EnvironmentCorrelation> analyze_environment_impact() const;
        SleepEnvironment get_optimal_environment() const;
        std::vector<std::string> get_environment_improvements() const;
        bool detect_environmental_sleep_disruptors() const;
    };

// Data validation and integrity
    namespace data_validation {
        bool validate_sleep_session(const DetailedSleepSession& session);
        bool validate_daily_summary(const DailySleepSummary& summary);
        std::vector<std::string> check_data_consistency(
                const std::vector<DetailedSleepSession>& sessions,
                const std::vector<DailySleepSummary>& summaries);
        bool repair_data_inconsistencies(
                std::vector<DetailedSleepSession>& sessions,
                std::vector<DailySleepSummary>& summaries);
    }

// Advanced sleep metrics calculations
    namespace sleep_metrics {
        double calculate_sleep_efficiency_percentile(
                double efficiency,
                const std::vector<DetailedSleepSession>& reference_data);
        double calculate_chronotype_score(const std::vector<DetailedSleepSession>& sessions);
        double calculate_social_jetlag(const std::vector<DailySleepSummary>& summaries);
        Duration calculate_optimal_nap_timing(const std::vector<DetailedSleepSession>& sessions);
        std::vector<std::string> generate_personalized_insights(
                const std::vector<DetailedSleepSession>& sessions,
                const SleepGoals& goals);
    }

} // namespace descansa

#endif // DESCANSA_CORE_MANAGER_H