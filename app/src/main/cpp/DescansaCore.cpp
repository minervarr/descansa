#include "DescansaCore.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

namespace descansa {

    DescansaCore::DescansaCore(const std::string& data_path)
            : session_active(false), data_file_path(data_path.empty() ? "descansa_data.txt" : data_path) {
        load_data();
    }

    DescansaCore::~DescansaCore() {
        save_data();
    }

    void DescansaCore::start_sleep_session() {
        if (session_active) {
            // End previous session first
            end_sleep_session();
        }

        current_session_start = utils::now();
        session_active = true;
    }

    void DescansaCore::end_sleep_session() {
        if (!session_active) return;

        TimePoint wake_time = utils::now();
        // NEW: Create session with current configuration context
        SleepSession session(current_session_start, wake_time, config);
        sleep_history.push_back(session);

        session_active = false;
        save_data();
    }

    void DescansaCore::set_target_sleep_hours(double hours) {
        config.target_sleep_hours = Duration(hours * 3600.0);
    }

    void DescansaCore::set_target_wake_time(int hour, int minute) {
        config.target_wake_hour = std::chrono::hours(hour);
        config.target_wake_minute = std::chrono::minutes(minute);
    }

    Duration DescansaCore::get_last_sleep_duration() const {
        if (sleep_history.empty()) {
            return Duration(0);
        }
        return sleep_history.back().sleep_duration;
    }

    Duration DescansaCore::get_remaining_work_time() const {
        return calculate_remaining_work_time(utils::now());
    }

    Duration DescansaCore::get_average_sleep_duration(int days) const {
        if (sleep_history.empty()) return Duration(0);

        TimePoint cutoff = utils::now() - std::chrono::hours(24 * days);
        Duration total(0);
        int count = 0;

        for (const auto& session : sleep_history) {
            if (session.wake_up >= cutoff) {
                total += session.sleep_duration;
                count++;
            }
        }

        return count > 0 ? Duration(total.count() / count) : Duration(0);
    }

    TimePoint DescansaCore::get_next_recommended_bedtime() const {
        TimePoint tomorrow_wake = get_tomorrow_target_wake_time();
        auto bedtime_duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(config.target_sleep_hours);
        return tomorrow_wake - bedtime_duration;
    }

    TimePoint DescansaCore::get_today_target_wake_time() const {
        TimePoint now = utils::now();
        TimePoint start_today = utils::start_of_day(now);

        return start_today + config.target_wake_hour + config.target_wake_minute;
    }

    TimePoint DescansaCore::get_tomorrow_target_wake_time() const {
        TimePoint today_wake = get_today_target_wake_time();
        return today_wake + std::chrono::hours(24);
    }

    Duration DescansaCore::calculate_remaining_work_time(TimePoint current_time) const {
        TimePoint next_bedtime = get_next_recommended_bedtime();

        if (current_time >= next_bedtime) {
            return Duration(0); // Past bedtime
        }

        return std::chrono::duration_cast<Duration>(next_bedtime - current_time);
    }

    bool DescansaCore::save_data() const {
        std::ofstream file(data_file_path);
        if (!file.is_open()) return false;

        // Save config
        file << "CONFIG:" << config.target_sleep_hours.count() << ","
             << config.target_wake_hour.count() << ","
             << config.target_wake_minute.count() << "\n";

        // Save sessions with full configuration context
        for (const auto& session : sleep_history) {
            if (session.is_complete) {
                auto start_time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto end_time_t = std::chrono::system_clock::to_time_t(session.wake_up);
                auto recorded_time_t = std::chrono::system_clock::to_time_t(session.session_recorded);

                file << "SESSION:" << start_time_t << "," << end_time_t << ","
                     << session.sleep_duration.count() << ","
                     << session.target_sleep_hours_at_session.count() << ","
                     << session.target_wake_hour_at_session.count() << ","
                     << session.target_wake_minute_at_session.count() << ","
                     << recorded_time_t << "\n";
            }
        }

        // Save current session if active
        if (session_active) {
            auto start_time_t = std::chrono::system_clock::to_time_t(current_session_start);
            file << "ACTIVE:" << start_time_t << "\n";
        }

        return file.good();
    }

    // Fix for line 177-178, 191, 199 - Use auto for cast operations
    bool DescansaCore::load_data() {
        std::ifstream file(data_file_path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos) continue;

            std::string type = line.substr(0, colon_pos);
            std::string data = line.substr(colon_pos + 1);

            if (type == "CONFIG") {
                std::istringstream ss(data);
                std::string token;

                if (std::getline(ss, token, ',')) {
                    config.target_sleep_hours = Duration(std::stod(token));
                }
                if (std::getline(ss, token, ',')) {
                    config.target_wake_hour = std::chrono::hours(std::stoi(token));
                }
                if (std::getline(ss, token, ',')) {
                    config.target_wake_minute = std::chrono::minutes(std::stoi(token));
                }
            }
            else if (type == "SESSION") {
                std::istringstream ss(data);
                std::string token;
                std::vector<std::string> tokens;

                while (std::getline(ss, token, ',')) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 3) {  // Backward compatibility
                    // Fix: Use auto for cast operations
                    auto start_t = static_cast<std::time_t>(std::stoll(tokens[0]));
                    auto end_t = static_cast<std::time_t>(std::stoll(tokens[1]));

                    TimePoint start = std::chrono::system_clock::from_time_t(start_t);
                    TimePoint end = std::chrono::system_clock::from_time_t(end_t);

                    SleepSession session(start, end, config);  // Use current config as fallback

                    // If enhanced data available, use it
                    if (tokens.size() >= 7) {
                        session.target_sleep_hours_at_session = Duration(std::stod(tokens[3]));
                        session.target_wake_hour_at_session = std::chrono::hours(std::stoi(tokens[4]));
                        session.target_wake_minute_at_session = std::chrono::minutes(std::stoi(tokens[5]));

                        auto recorded_t = static_cast<std::time_t>(std::stoll(tokens[6]));
                        session.session_recorded = std::chrono::system_clock::from_time_t(recorded_t);
                    }

                    sleep_history.push_back(session);
                }
            }
            else if (type == "ACTIVE") {
                auto start_t = static_cast<std::time_t>(std::stoll(data));
                current_session_start = std::chrono::system_clock::from_time_t(start_t);
                session_active = true;
            }
        }

        return true;
    }

    void DescansaCore::clear_history() {
        sleep_history.clear();
        save_data();
    }

    // REMOVED UNUSED FUNCTIONS:
    // - get_recent_sessions() - Never called anywhere
    // - has_slept_today() - Never called anywhere
    // - get_time_since_last_wake() - Never called anywhere
    // - get_status_summary() - Not used in MainActivity

    bool DescansaCore::is_in_sleep_period() const {
        TimePoint now = utils::now();
        TimePoint today_wake = get_today_target_wake_time();
        TimePoint tonight_bedtime = get_next_recommended_bedtime();

        // If current time is between bedtime and wake time
        return (now >= tonight_bedtime) || (now < today_wake);
    }

    bool DescansaCore::is_before_target_wake_time() const {
        TimePoint now = utils::now();
        TimePoint today_wake = get_today_target_wake_time();
        return now < today_wake;
    }

    Duration DescansaCore::get_time_until_target_wake() const {
        TimePoint now = utils::now();
        TimePoint today_wake = get_today_target_wake_time();

        if (now >= today_wake) {
            return Duration(0); // Return zero if wake time passed
        }

        return std::chrono::duration_cast<Duration>(today_wake - now);
    }

    Duration DescansaCore::get_current_session_duration() const {
        if (!session_active) {
            return Duration(0);
        }
        return std::chrono::duration_cast<Duration>(utils::now() - current_session_start);
    }

    bool DescansaCore::export_analysis_csv(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        // Pure CSV header - no decorative elements, all data columns
        file << "session_id,sleep_start_timestamp,wake_up_timestamp,"
             << "sleep_duration_seconds,target_sleep_seconds_at_session,"
             << "target_wake_hour_at_session,target_wake_minute_at_session,"
             << "session_recorded_timestamp,export_timestamp,"
             << "sleep_start_iso,wake_up_iso,session_recorded_iso,export_iso\n";

        // Current export timestamp
        auto export_time = std::chrono::system_clock::now();
        auto export_timestamp = std::chrono::system_clock::to_time_t(export_time);
        std::string export_iso = utils::format_time(export_time);

        // Export all sessions with complete raw data
        for (size_t i = 0; i < sleep_history.size(); ++i) {
            const auto& session = sleep_history[i];
            if (session.is_complete) {
                // Convert to timestamps for analysis
                auto start_timestamp = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto wake_timestamp = std::chrono::system_clock::to_time_t(session.wake_up);
                auto recorded_timestamp = std::chrono::system_clock::to_time_t(session.session_recorded);

                // ISO format strings for human reference in analysis tools
                std::string start_iso = utils::format_time(session.sleep_start);
                std::string wake_iso = utils::format_time(session.wake_up);
                std::string recorded_iso = utils::format_time(session.session_recorded);

                file << i << ","
                     << start_timestamp << ","
                     << wake_timestamp << ","
                     << static_cast<int64_t>(session.sleep_duration.count()) << ","
                     << static_cast<int64_t>(session.target_sleep_hours_at_session.count()) << ","
                     << session.target_wake_hour_at_session.count() << ","
                     << session.target_wake_minute_at_session.count() << ","
                     << recorded_timestamp << ","
                     << export_timestamp << ","
                     << "\"" << start_iso << "\","
                     << "\"" << wake_iso << "\","
                     << "\"" << recorded_iso << "\","
                     << "\"" << export_iso << "\"\n";
            }
        }

        return file.good();
    }

// Utility functions implementation - ONLY keeping functions that are actually used
    namespace utils {

        std::string format_duration(const Duration& d) {
            int hours = static_cast<int>(d.count() / 3600);
            int minutes = static_cast<int>((d.count() - hours * 3600) / 60);

            std::ostringstream ss;
            ss << hours << "h " << minutes << "m";
            return ss.str();
        }

        std::string format_time(const TimePoint& tp) {
            auto time_t = std::chrono::system_clock::to_time_t(tp);
            auto tm = *std::localtime(&time_t);

            std::ostringstream ss;
            ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }

        TimePoint now() {
            return std::chrono::system_clock::now();
        }

        TimePoint start_of_day(const TimePoint& tp) {
            auto time_t = std::chrono::system_clock::to_time_t(tp);
            auto tm = *std::localtime(&time_t);

            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;

            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }

        // REMOVED UNUSED UTILITY FUNCTIONS:
        // - is_same_day() - Not used in current implementation
        // - end_of_day() - Not used in current implementation

    } // namespace utils

} // namespace descansa