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

        // FIXED: Default constructor - Initialize ALL fields including sleep_duration
        SleepSession()
                : sleep_duration(0.0),  // FIXED: Explicit initialization
                  is_complete(false),
                  target_sleep_hours_at_session(Duration(8.0 * 3600)),
                  target_wake_hour_at_session(std::chrono::hours(8)),
                  target_wake_minute_at_session(std::chrono::minutes(0)) {
            session_recorded = std::chrono::system_clock::now();
        }

        // Constructor with configuration context
        SleepSession(TimePoint start, TimePoint end, const ScheduleConfig& active_config)
                : sleep_start(start),
                  wake_up(end),
                  sleep_duration(std::chrono::duration_cast<Duration>(end - start)),  // FIXED: Initialize here too
                  is_complete(true),
                  target_sleep_hours_at_session(active_config.target_sleep_hours),
                  target_wake_hour_at_session(active_config.target_wake_hour),
                  target_wake_minute_at_session(active_config.target_wake_minute) {
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
        bool export_analysis_csv(const std::string& export_path) const;  // USED by MainActivity
        void clear_history();

        // Statistics
        size_t get_session_count() const { return sleep_history.size(); }

        // Current status - USED by MainActivity
        bool is_in_sleep_period() const;
        bool is_before_target_wake_time() const;
        Duration get_time_until_target_wake() const;

        // REMOVED UNUSED FUNCTIONS - These are never called in the codebase:
        // - get_recent_sessions() - Not used anywhere
        // - has_slept_today() - Not used anywhere
        // - get_time_since_last_wake() - Not used anywhere
        // - get_status_summary() - Not used in MainActivity
    };

// Utility functions - ONLY keep functions that are actually used
    namespace utils {
        std::string format_duration(const Duration& d);      // USED - by formatted functions
        std::string format_time(const TimePoint& tp);        // USED - by export functions
        TimePoint now();                                      // USED - throughout core
        TimePoint start_of_day(const TimePoint& tp);         // USED - by core calculations

        // REMOVED UNUSED UTILITY FUNCTIONS:
        // - is_same_day() - Not used anywhere in current implementation
        // - end_of_day() - Not used anywhere in current implementation
    }

} // namespace descansa

#endif // DESCANSA_CORE_H