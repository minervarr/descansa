#ifndef SLEEP_DATA_STRUCTURES_H
#define SLEEP_DATA_STRUCTURES_H

#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <cstdint>

namespace descansa {

// Time utilities
    using TimePoint = std::chrono::system_clock::time_point;
    using Duration = std::chrono::duration<double>;

// Sleep quality indicators
    enum class SleepQuality {
        UNKNOWN = 0,
        POOR = 1,
        FAIR = 2,
        GOOD = 3,
        EXCELLENT = 4
    };

// Sleep phase data
    struct SleepPhase {
        TimePoint start_time;
        Duration duration;
        std::string phase_type; // "light", "deep", "rem", "awake"

        SleepPhase() : duration(0) {}
        SleepPhase(TimePoint start, Duration dur, const std::string& type)
                : start_time(start), duration(dur), phase_type(type) {}
    };

// Detailed sleep session with comprehensive data
    struct DetailedSleepSession {
        // Basic timing
        TimePoint sleep_start;
        TimePoint wake_up;
        Duration total_sleep_duration;
        Duration time_in_bed;

        // Sleep efficiency and quality
        double sleep_efficiency;        // percentage of time in bed actually sleeping
        SleepQuality perceived_quality;
        int awakenings_count;
        Duration total_awake_time;

        // Environmental factors
        double room_temperature;        // Celsius
        int noise_level;               // 0-100 scale
        int light_level;               // 0-100 scale

        // Pre-sleep factors
        TimePoint last_caffeine_time;
        TimePoint last_meal_time;
        TimePoint last_exercise_time;
        TimePoint screen_time_end;     // when stopped using devices

        // Sleep phases (if available)
        std::vector<SleepPhase> sleep_phases;
        Duration light_sleep_duration;
        Duration deep_sleep_duration;
        Duration rem_sleep_duration;

        // Metadata
        std::string notes;             // user notes about the sleep
        bool is_nap;                  // true if this was a nap, not main sleep
        bool is_complete;             // true if session ended naturally
        bool data_validated;          // true if data has been reviewed/validated
        TimePoint created_timestamp;
        TimePoint modified_timestamp;

        // Constructors
        DetailedSleepSession();
        explicit DetailedSleepSession(TimePoint start, TimePoint end);

        // Analysis methods
        double calculate_sleep_efficiency() const;
        Duration get_sleep_latency() const;      // time to fall asleep
        Duration get_wake_after_sleep_onset() const; // time awake during sleep period
        bool is_sleep_debt() const;              // based on target sleep duration
        std::string get_quality_description() const;
    };

// Daily sleep summary
    struct DailySleepSummary {
        std::chrono::system_clock::time_point date;

        // Main sleep session
        DetailedSleepSession main_sleep;
        std::vector<DetailedSleepSession> naps;

        // Daily totals
        Duration total_sleep_time;
        Duration total_time_in_bed;
        int total_awakenings;
        double average_sleep_efficiency;

        // Daily factors
        int daily_steps;
        int daily_screen_time_minutes;
        int stress_level;              // 1-10 scale
        std::vector<TimePoint> caffeine_times;
        std::vector<TimePoint> meal_times;

        // Sleep debt calculation
        Duration sleep_debt;           // negative = surplus, positive = debt
        Duration cumulative_sleep_debt;

        // Goals and targets
        Duration target_sleep_duration;
        TimePoint target_bedtime;
        TimePoint target_wake_time;
        bool met_sleep_goal;

        DailySleepSummary();
        explicit DailySleepSummary(const std::chrono::system_clock::time_point& day);

        void calculate_daily_totals();
        bool has_main_sleep() const { return main_sleep.is_complete; }
        double get_sleep_score() const;  // 0-100 sleep quality score
    };

// Weekly sleep pattern analysis
    struct WeeklySleepPattern {
        std::chrono::system_clock::time_point week_start;
        std::vector<DailySleepSummary> daily_summaries;

        // Weekly averages
        Duration average_sleep_duration;
        Duration average_bedtime_variance;
        Duration average_wake_time_variance;
        double average_sleep_efficiency;
        double average_sleep_score;

        // Pattern analysis
        bool has_consistent_schedule;
        int weekend_schedule_shift_minutes;   // how much schedule shifts on weekends
        std::vector<int> problem_days;        // indices of days with poor sleep

        // Recommendations
        std::vector<std::string> recommendations;

        WeeklySleepPattern();
        explicit WeeklySleepPattern(const std::chrono::system_clock::time_point& week_start);

        void analyze_patterns();
        void generate_recommendations();
        double calculate_schedule_consistency() const;
    };

// Sleep environment tracking
    struct SleepEnvironment {
        TimePoint measurement_time;

        // Physical environment
        double temperature;           // Celsius
        double humidity;             // percentage
        int noise_level;            // decibels or 0-100 scale
        int light_level;            // lux or 0-100 scale

        // Device/app usage before sleep
        int screen_time_minutes;     // minutes of screen time before sleep
        TimePoint last_phone_use;
        std::vector<std::string> apps_used; // apps used close to bedtime

        // Sleep aids/medications
        std::vector<std::string> medications;
        std::vector<std::string> supplements;
        bool used_sleep_aid;

        SleepEnvironment();

        bool is_environment_optimal() const;
        std::vector<std::string> get_environment_recommendations() const;
    };

// Sleep goals and preferences
    struct SleepGoals {
        Duration target_sleep_duration;
        std::chrono::hours preferred_bedtime;
        std::chrono::hours preferred_wake_time;

        // Flexibility tolerances
        Duration bedtime_tolerance;      // how much bedtime can vary
        Duration wake_time_tolerance;    // how much wake time can vary

        // Quality goals
        double target_sleep_efficiency;  // percentage
        int max_acceptable_awakenings;
        Duration max_acceptable_sleep_latency;

        // Lifestyle preferences
        bool weekend_schedule_differs;
        Duration weekend_sleep_extension; // extra sleep on weekends
        bool allow_naps;
        Duration max_nap_duration;
        std::chrono::hours latest_nap_time;

        SleepGoals();

        bool is_within_tolerance(const DetailedSleepSession& session) const;
        double calculate_goal_adherence(const DailySleepSummary& summary) const;
    };

// Sleep statistics and analytics
    struct SleepStatistics {
        TimePoint analysis_period_start;
        TimePoint analysis_period_end;
        int total_sessions;

        // Duration statistics
        Duration average_sleep_duration;
        Duration median_sleep_duration;
        Duration shortest_sleep;
        Duration longest_sleep;
        Duration sleep_duration_std_dev;

        // Timing statistics
        std::chrono::hours average_bedtime;
        std::chrono::hours average_wake_time;
        Duration bedtime_variance;
        Duration wake_time_variance;

        // Quality statistics
        double average_sleep_efficiency;
        double average_sleep_score;
        SleepQuality most_common_quality;
        int total_awakenings;

        // Trends
        enum class Trend { IMPROVING, STABLE, DECLINING };
        Trend sleep_duration_trend;
        Trend sleep_quality_trend;
        Trend schedule_consistency_trend;

        // Sleep debt
        Duration total_sleep_debt;
        Duration average_daily_sleep_debt;
        int days_with_sleep_debt;

        SleepStatistics();

        void calculate_from_sessions(const std::vector<DetailedSleepSession>& sessions);
        void calculate_trends(const std::vector<DailySleepSummary>& daily_data);
        std::string generate_summary_report() const;
    };

} // namespace descansa

#endif // SLEEP_DATA_STRUCTURES_H