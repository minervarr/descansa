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
        SleepSession session(current_session_start, wake_time);
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

        // Save sessions
        for (const auto& session : sleep_history) {
            if (session.is_complete) {
                auto start_time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto end_time_t = std::chrono::system_clock::to_time_t(session.wake_up);

                file << "SESSION:" << start_time_t << "," << end_time_t << ","
                     << session.sleep_duration.count() << "\n";
            }
        }

        // Save current session if active
        if (session_active) {
            auto start_time_t = std::chrono::system_clock::to_time_t(current_session_start);
            file << "ACTIVE:" << start_time_t << "\n";
        }

        return file.good();
    }

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

                std::time_t start_t = 0, end_t = 0;
                if (std::getline(ss, token, ',')) {
                    start_t = static_cast<std::time_t>(std::stoll(token));
                }
                if (std::getline(ss, token, ',')) {
                    end_t = static_cast<std::time_t>(std::stoll(token));
                }

                if (start_t != 0 && end_t != 0) {
                    TimePoint start = std::chrono::system_clock::from_time_t(start_t);
                    TimePoint end = std::chrono::system_clock::from_time_t(end_t);
                    sleep_history.emplace_back(start, end);
                }
            }
            else if (type == "ACTIVE") {
                std::time_t start_t = static_cast<std::time_t>(std::stoll(data));
                current_session_start = std::chrono::system_clock::from_time_t(start_t);
                session_active = true;
            }
        }

        return true;
    }

    bool DescansaCore::export_data(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        file << "Descansa Sleep Data Export\n";
        file << "Generated: " << utils::format_time(utils::now()) << "\n\n";

        file << "Configuration:\n";
        file << "Target Sleep Hours: " << utils::format_duration(config.target_sleep_hours) << "\n";
        file << "Target Wake Time: " << config.target_wake_hour.count()
             << ":" << std::setfill('0') << std::setw(2) << config.target_wake_minute.count() << "\n\n";

        file << "Sleep History:\n";
        file << "Date,Sleep Start,Wake Up,Duration (hours)\n";

        for (const auto& session : sleep_history) {
            if (session.is_complete) {
                file << utils::format_time(session.sleep_start) << ","
                     << utils::format_time(session.sleep_start) << ","
                     << utils::format_time(session.wake_up) << ","
                     << (session.sleep_duration.count() / 3600.0) << "\n";
            }
        }

        return file.good();
    }

    void DescansaCore::clear_history() {
        sleep_history.clear();
        save_data();
    }

    std::vector<SleepSession> DescansaCore::get_recent_sessions(int count) const {
        if (static_cast<int>(sleep_history.size()) <= count) {
            return sleep_history;
        }

        return {sleep_history.end() - count, sleep_history.end()};
    }

    std::string DescansaCore::get_status_summary() const {
        std::ostringstream ss;

        if (session_active) {
            Duration elapsed = std::chrono::duration_cast<Duration>(
                    utils::now() - current_session_start);
            ss << "Sleeping for: " << utils::format_duration(elapsed) << "\n";
        } else {
            Duration last_sleep = get_last_sleep_duration();
            Duration remaining_work = get_remaining_work_time();

            ss << "Last sleep: " << utils::format_duration(last_sleep) << "\n";
            ss << "Work time remaining: " << utils::format_duration(remaining_work) << "\n";
            ss << "Next bedtime: " << utils::format_time(get_next_recommended_bedtime()) << "\n";
        }

        return ss.str();
    }

    bool DescansaCore::has_slept_today() const {
        if (sleep_history.empty()) return false;

        TimePoint today_start = utils::start_of_day(utils::now());
        const auto& last_session = sleep_history.back();

        return last_session.wake_up >= today_start;
    }

    Duration DescansaCore::get_time_since_last_wake() const {
        if (sleep_history.empty() || session_active) {
            return Duration(0);
        }

        return std::chrono::duration_cast<Duration>(
                utils::now() - sleep_history.back().wake_up);
    }

// Utility functions implementation
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

        bool is_same_day(const TimePoint& t1, const TimePoint& t2) {
            auto time1 = std::chrono::system_clock::to_time_t(t1);
            auto time2 = std::chrono::system_clock::to_time_t(t2);

            auto tm1 = *std::localtime(&time1);
            auto tm2 = *std::localtime(&time2);

            return (tm1.tm_year == tm2.tm_year &&
                    tm1.tm_yday == tm2.tm_yday);
        }

        TimePoint start_of_day(const TimePoint& tp) {
            auto time_t = std::chrono::system_clock::to_time_t(tp);
            auto tm = *std::localtime(&time_t);

            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;

            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }

        TimePoint end_of_day(const TimePoint& tp) {
            return start_of_day(tp) + std::chrono::hours(24) - std::chrono::seconds(1);
        }

    } // namespace utils

} // namespace descansa