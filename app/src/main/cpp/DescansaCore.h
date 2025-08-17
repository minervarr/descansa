#ifndef DESCANSA_CORE_H
#define DESCANSA_CORE_H

#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <cstdint>

namespace descansa {

// Time point using system clock
    using TimePoint = std::chrono::system_clock::time_point;
    using Duration = std::chrono::duration<double>;

// Forward declaration
    struct ScheduleConfig;

// Daily schedule configuration - MOVED BEFORE SleepSession
    struct ScheduleConfig {
        Duration target_sleep_hours;
        std::chrono::hours target_wake_hour;  // Hour of day (0-23)
        std::chrono::minutes target_wake_minute; // Minute of hour (0-59)

        ScheduleConfig()
                : target_sleep_hours(Duration(8.0 * 3600.0)),  // Fix: explicit Duration constructor
                  target_wake_hour(8),
                  target_wake_minute(0) {}
    };

// Modified SleepSession struct for complete data retention
    struct SleepSession {
        // Core sleep data
        TimePoint sleep_start;
        TimePoint wake_up;
        Duration sleep_duration;
        bool is_complete;

        // Configuration context (what settings were active during this session)
        Duration target_sleep_hours_at_session;
        std::chrono::hours target_wake_hour_at_session;
        std::chrono::minutes target_wake_minute_at_session;

        // Session metadata
        TimePoint session_recorded;

        // Default constructor - Fix initialization
        SleepSession() : sleep_duration(0), is_complete(false),
                         target_sleep_hours_at_session(Duration(8.0 * 3600)),
                         target_wake_hour_at_session(std::chrono::hours(8)),
                         target_wake_minute_at_session(std::chrono::minutes(0)) {
            session_recorded = std::chrono::system_clock::now();
        }

        // Constructor with configuration context
        SleepSession(TimePoint start, TimePoint end, const ScheduleConfig& active_config)
                : sleep_start(start), wake_up(end), is_complete(true),
                  target_sleep_hours_at_session(active_config.target_sleep_hours),
                  target_wake_hour_at_session(active_config.target_wake_hour),
                  target_wake_minute_at_session(active_config.target_wake_minute) {
            sleep_duration = std::chrono::duration_cast<Duration>(end - start);
            session_recorded = std::chrono::system_clock::now();
        }
    };

// Core sleep tracking and calculation engine
    class DescansaCore {
    private:
        std::vector<SleepSession> sleep_history;
        ScheduleConfig config;
        TimePoint current_session_start;
        bool session_active;
        std::string data_file_path;

        // Helper methods
        TimePoint get_today_target_wake_time() const;
        TimePoint get_tomorrow_target_wake_time() const;
        Duration calculate_remaining_work_time(TimePoint current_time) const;

    public:
        explicit DescansaCore(const std::string& data_path = "");
        ~DescansaCore();

        // Session management
        void start_sleep_session();
        void end_sleep_session();
        bool is_session_running() const { return session_active; }

        // Configuration
        void set_target_sleep_hours(double hours);
        void set_target_wake_time(int hour, int minute);
        const ScheduleConfig& get_config() const { return config; }

        // Calculations
        Duration get_last_sleep_duration() const;
        Duration get_remaining_work_time() const;
        Duration get_average_sleep_duration(int days = 7) const;
        TimePoint get_next_recommended_bedtime() const;

        // Current session tracking
        Duration get_current_session_duration() const;

        // Data management
        bool save_data() const;
        bool load_data();
        bool export_data(const std::string& export_path) const;
        bool export_analysis_csv(const std::string& export_path) const;
        void clear_history();

        // Statistics
        size_t get_session_count() const { return sleep_history.size(); }
        std::vector<SleepSession> get_recent_sessions(int count = 10) const;

        // Current status
        std::string get_status_summary() const;
        bool has_slept_today() const;
        Duration get_time_since_last_wake() const;

        bool is_in_sleep_period() const;
        bool is_before_target_wake_time() const;
        Duration get_time_until_target_wake() const;
    };

// Utility functions
    namespace utils {
        std::string format_duration(const Duration& d);
        std::string format_time(const TimePoint& tp);
        TimePoint now();
        bool is_same_day(const TimePoint& t1, const TimePoint& t2);
        TimePoint start_of_day(const TimePoint& tp);
        TimePoint end_of_day(const TimePoint& tp);
    }

} // namespace descansa

#endif // DESCANSA_CORE_H