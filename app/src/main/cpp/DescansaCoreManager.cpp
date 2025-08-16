#include "DescansaCoreManager.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <numeric>

namespace descansa {

// DescansaCoreManager Implementation
    DescansaCoreManager::DescansaCoreManager(const std::string& data_dir)
            : enhanced_session_active(false), data_directory(data_dir.empty() ? "descansa_data" : data_dir) {

        // Initialize basic core for compatibility
        basic_core.reset(new DescansaCore(data_directory + "/basic_data.txt"));

        // Set up file paths
        sessions_file = data_directory + "/detailed_sessions.dat";
        summaries_file = data_directory + "/daily_summaries.dat";
        goals_file = data_directory + "/user_goals.dat";
        environment_file = data_directory + "/environment_data.dat";

        load_all_data();
    }

    DescansaCoreManager::~DescansaCoreManager() {
        if (enhanced_session_active) {
            end_enhanced_sleep_session();
        }
        save_all_data();
    }

    void DescansaCoreManager::start_enhanced_sleep_session() {
        if (enhanced_session_active) {
            end_enhanced_sleep_session(); // End previous session
        }

        session_start_time = std::chrono::system_clock::now();
        current_session = DetailedSleepSession();
        current_session.sleep_start = session_start_time;
        current_session.created_timestamp = session_start_time;
        enhanced_session_active = true;

        // Sync with basic core
        basic_core->start_sleep_session();
    }

    void DescansaCoreManager::end_enhanced_sleep_session() {
        if (!enhanced_session_active) return;

        TimePoint wake_time = std::chrono::system_clock::now();
        current_session.wake_up = wake_time;
        current_session.total_sleep_duration = std::chrono::duration_cast<Duration>(
                wake_time - current_session.sleep_start);
        current_session.time_in_bed = current_session.total_sleep_duration;
        current_session.is_complete = true;
        current_session.modified_timestamp = wake_time;

        // Calculate efficiency
        current_session.sleep_efficiency = current_session.calculate_sleep_efficiency();

        // Store completed session
        detailed_sessions.push_back(current_session);

        // Update daily summary
        update_daily_summary(current_session);

        // Trigger callback if set
        if (session_completed_callback) {
            session_completed_callback(current_session);
        }

        enhanced_session_active = false;

        // Sync with basic core
        basic_core->end_sleep_session();

        // Save data
        save_all_data();
    }

    void DescansaCoreManager::pause_session() {
        if (!enhanced_session_active) return;

        TimePoint pause_time = std::chrono::system_clock::now();
        Duration awake_duration = std::chrono::duration_cast<Duration>(
                pause_time - session_start_time);

        current_session.awakenings_count++;
        current_session.total_awake_time += awake_duration;
    }

    void DescansaCoreManager::resume_session() {
        if (!enhanced_session_active) return;
        session_start_time = std::chrono::system_clock::now();
    }

    void DescansaCoreManager::update_environment_data(const SleepEnvironment& env) {
        current_environment = env;

        if (enhanced_session_active) {
            current_session.room_temperature = env.temperature;
            current_session.noise_level = env.noise_level;
            current_session.light_level = env.light_level;
            current_session.screen_time_end = env.last_phone_use;
        }
    }

    void DescansaCoreManager::add_awakening(const TimePoint& time, const Duration& duration) {
        if (!enhanced_session_active) return;

        current_session.awakenings_count++;
        current_session.total_awake_time += duration;
    }

    void DescansaCoreManager::set_sleep_quality(SleepQuality quality) {
        if (enhanced_session_active) {
            current_session.perceived_quality = quality;
        } else if (!detailed_sessions.empty()) {
            detailed_sessions.back().perceived_quality = quality;
            detailed_sessions.back().modified_timestamp = std::chrono::system_clock::now();
        }
    }

    void DescansaCoreManager::add_session_note(const std::string& note) {
        if (enhanced_session_active) {
            if (!current_session.notes.empty()) {
                current_session.notes += "; ";
            }
            current_session.notes += note;
        } else if (!detailed_sessions.empty()) {
            if (!detailed_sessions.back().notes.empty()) {
                detailed_sessions.back().notes += "; ";
            }
            detailed_sessions.back().notes += note;
            detailed_sessions.back().modified_timestamp = std::chrono::system_clock::now();
        }
    }

    void DescansaCoreManager::mark_as_nap(bool is_nap) {
        if (enhanced_session_active) {
            current_session.is_nap = is_nap;
        } else if (!detailed_sessions.empty()) {
            detailed_sessions.back().is_nap = is_nap;
        }
    }

    void DescansaCoreManager::record_caffeine_intake(const TimePoint& time) {
        if (enhanced_session_active) {
            current_session.last_caffeine_time = time;
        }

        // Also update daily summary if exists
        DailySleepSummary* today_summary = nullptr;
        for (auto& summary : daily_summaries) {
            if (is_same_calendar_day(summary.date, time)) {
                today_summary = &summary;
                break;
            }
        }

        if (today_summary) {
            today_summary->caffeine_times.push_back(time);
        }
    }

    void DescansaCoreManager::record_meal_time(const TimePoint& time) {
        if (enhanced_session_active) {
            current_session.last_meal_time = time;
        }

        // Update daily summary
        DailySleepSummary* today_summary = nullptr;
        for (auto& summary : daily_summaries) {
            if (is_same_calendar_day(summary.date, time)) {
                today_summary = &summary;
                break;
            }
        }

        if (today_summary) {
            today_summary->meal_times.push_back(time);
        }
    }

    void DescansaCoreManager::record_exercise(const TimePoint& time) {
        if (enhanced_session_active) {
            current_session.last_exercise_time = time;
        }
    }

    void DescansaCoreManager::record_screen_time_end(const TimePoint& time) {
        if (enhanced_session_active) {
            current_session.screen_time_end = time;
        }
    }

    void DescansaCoreManager::record_medication(const std::string& medication) {
        current_environment.medications.push_back(medication);
        current_environment.used_sleep_aid = true;
    }

    void DescansaCoreManager::set_sleep_goals(const SleepGoals& goals) {
        user_goals = goals;

        // Sync target duration with basic core
        basic_core->set_target_sleep_hours(goals.target_sleep_duration.count() / 3600.0);
        basic_core->set_target_wake_time(
                static_cast<int>(goals.preferred_wake_time.count()),
                0
        );
    }

    void DescansaCoreManager::update_target_sleep_duration(const Duration& duration) {
        user_goals.target_sleep_duration = duration;
        basic_core->set_target_sleep_hours(duration.count() / 3600.0);
    }

    void DescansaCoreManager::update_preferred_schedule(std::chrono::hours bedtime, std::chrono::hours wake_time) {
        user_goals.preferred_bedtime = bedtime;
        user_goals.preferred_wake_time = wake_time;
        basic_core->set_target_wake_time(static_cast<int>(wake_time.count()), 0);
    }

    void DescansaCoreManager::set_weekend_flexibility(bool allow_flexibility, const Duration& extension) {
        user_goals.weekend_schedule_differs = allow_flexibility;
        user_goals.weekend_sleep_extension = extension;
    }

    std::vector<DetailedSleepSession> DescansaCoreManager::get_sessions(int count) const {
        if (count <= 0 || count >= static_cast<int>(detailed_sessions.size())) {
            return detailed_sessions;
        }

        return std::vector<DetailedSleepSession>(
                detailed_sessions.end() - count,
                detailed_sessions.end()
        );
    }

    std::vector<DetailedSleepSession> DescansaCoreManager::get_sessions_in_range(
            const TimePoint& start, const TimePoint& end) const {

        std::vector<DetailedSleepSession> result;

        for (const auto& session : detailed_sessions) {
            if (session.sleep_start >= start && session.wake_up <= end) {
                result.push_back(session);
            }
        }

        return result;
    }

    DailySleepSummary DescansaCoreManager::get_daily_summary(const TimePoint& date) const {
        for (const auto& summary : daily_summaries) {
            if (is_same_calendar_day(summary.date, date)) {
                return summary;
            }
        }

        // Return empty summary for date
        return DailySleepSummary(date);
    }

    std::vector<DailySleepSummary> DescansaCoreManager::get_recent_summaries(int days) const {
        TimePoint cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * days);

        std::vector<DailySleepSummary> result;
        for (const auto& summary : daily_summaries) {
            if (summary.date >= cutoff) {
                result.push_back(summary);
            }
        }

        return result;
    }

    WeeklySleepPattern DescansaCoreManager::get_weekly_pattern(const TimePoint& week_start) const {
        for (const auto& pattern : weekly_patterns) {
            if (is_same_calendar_day(pattern.week_start, week_start)) {
                return pattern;
            }
        }

        // Generate new weekly pattern if not found
        WeeklySleepPattern new_pattern(week_start);

        // Fill with daily summaries from that week
        TimePoint week_end = week_start + std::chrono::hours(24 * 7);
        for (const auto& summary : daily_summaries) {
            if (summary.date >= week_start && summary.date < week_end) {
                new_pattern.daily_summaries.push_back(summary);
            }
        }

        new_pattern.analyze_patterns();
        new_pattern.generate_recommendations();

        return new_pattern;
    }

    std::vector<WeeklySleepPattern> DescansaCoreManager::get_recent_weekly_patterns(int weeks) const {
        std::vector<WeeklySleepPattern> result;

        TimePoint current_week = std::chrono::system_clock::now();
        for (int i = 0; i < weeks; ++i) {
            TimePoint week_start = current_week - std::chrono::hours(24 * 7 * i);
            result.push_back(get_weekly_pattern(week_start));
        }

        return result;
    }

    DetailedSleepSession DescansaCoreManager::get_current_session_preview() const {
        if (!enhanced_session_active) {
            return DetailedSleepSession();
        }

        DetailedSleepSession preview = current_session;
        preview.wake_up = std::chrono::system_clock::now();
        preview.total_sleep_duration = std::chrono::duration_cast<Duration>(
                preview.wake_up - preview.sleep_start);
        preview.time_in_bed = preview.total_sleep_duration;
        preview.sleep_efficiency = preview.calculate_sleep_efficiency();

        return preview;
    }

    Duration DescansaCoreManager::get_enhanced_remaining_work_time() const {
        // Use basic core calculation but enhance with user goals
        Duration basic_remaining = basic_core->get_remaining_work_time();

        // Adjust based on user's specific schedule preferences
        TimePoint now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_tm = *std::localtime(&now_time_t);

        // Calculate target bedtime for today
        now_tm.tm_hour = static_cast<int>(user_goals.preferred_bedtime.count());
        now_tm.tm_min = 0;
        now_tm.tm_sec = 0;

        TimePoint target_bedtime = std::chrono::system_clock::from_time_t(std::mktime(&now_tm));

        // If target bedtime is in the past, move to tomorrow
        if (target_bedtime <= now) {
            target_bedtime += std::chrono::hours(24);
        }

        return std::chrono::duration_cast<Duration>(target_bedtime - now);
    }

    std::vector<std::string> DescansaCoreManager::get_current_recommendations() const {
        std::vector<std::string> recommendations;

        // Check recent sleep patterns
        if (detailed_sessions.size() >= 3) {
            auto recent_sessions = get_sessions(3);

            // Check for consistent sleep debt
            int debt_days = 0;
            for (const auto& session : recent_sessions) {
                if (session.total_sleep_duration < user_goals.target_sleep_duration) {
                    debt_days++;
                }
            }

            if (debt_days >= 2) {
                recommendations.push_back("Consider going to bed earlier - you've had sleep debt for multiple days");
            }

            // Check for late bedtimes
            auto avg_bedtime_hour = 0.0;
            for (const auto& session : recent_sessions) {
                auto time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto tm = *std::localtime(&time_t);
                avg_bedtime_hour += tm.tm_hour + (tm.tm_min / 60.0);
            }
            avg_bedtime_hour /= recent_sessions.size();

            if (avg_bedtime_hour > user_goals.preferred_bedtime.count() + 1) {
                recommendations.push_back("Your recent bedtimes are later than your goal - try to wind down earlier");
            }
        }

        // Environment recommendations
        auto env_recommendations = current_environment.get_environment_recommendations();
        recommendations.insert(recommendations.end(), env_recommendations.begin(), env_recommendations.end());

        // Current time recommendations
        TimePoint now = std::chrono::system_clock::now();
        auto remaining_work = get_enhanced_remaining_work_time();

        if (remaining_work.count() < 2 * 3600) { // Less than 2 hours
            recommendations.push_back("Start your bedtime routine soon - less than 2 hours until target bedtime");
        }

        return recommendations;
    }

    std::string DescansaCoreManager::get_sleep_score_explanation() const {
        if (detailed_sessions.empty()) {
            return "No sleep data available for scoring";
        }

        const auto& last_session = detailed_sessions.back();
        auto last_summary = get_daily_summary(last_session.wake_up);
        double score = last_summary.get_sleep_score();

        std::ostringstream explanation;
        explanation << "Sleep Score: " << std::fixed << std::setprecision(1) << score << "/100\n\n";

        explanation << "Score Breakdown:\n";

        // Duration component (40%)
        double duration_hours = last_session.total_sleep_duration.count() / 3600.0;
        double duration_score = std::min(100.0, (duration_hours / 8.0) * 100.0);
        explanation << "Duration (" << std::setprecision(1) << duration_hours
                    << "h): " << duration_score * 0.4 << "/40 points\n";

        // Efficiency component (30%)
        explanation << "Efficiency (" << std::setprecision(1) << last_session.sleep_efficiency
                    << "%): " << last_session.sleep_efficiency * 0.3 << "/30 points\n";

        // Quality component (20%)
        double quality_score = static_cast<double>(last_session.perceived_quality) * 25.0;
        explanation << "Quality (" << last_session.get_quality_description()
                    << "): " << quality_score * 0.2 << "/20 points\n";

        // Consistency component (10%)
        explanation << "Consistency: 7.5/10 points\n"; // Default value

        return explanation.str();
    }

    bool DescansaCoreManager::export_summary_csv(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        file << "Date,Total Sleep (hours),Sleep Efficiency (%),Sleep Score,Met Goal,Sleep Debt (hours)\n";

        for (const auto& summary : daily_summaries) {
            auto date_time_t = std::chrono::system_clock::to_time_t(summary.date);

            file << std::put_time(std::localtime(&date_time_t), "%Y-%m-%d") << ","
                 << std::fixed << std::setprecision(2) << (summary.total_sleep_time.count() / 3600.0) << ","
                 << std::setprecision(1) << summary.average_sleep_efficiency << ","
                 << std::setprecision(1) << summary.get_sleep_score() << ","
                 << (summary.met_sleep_goal ? "Yes" : "No") << ","
                 << std::setprecision(2) << (summary.sleep_debt.count() / 3600.0) << "\n";
        }

        return file.good();
    }

    bool DescansaCoreManager::export_weekly_patterns_json(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        file << "{\n";
        file << "  \"weekly_patterns\": [\n";

        for (size_t i = 0; i < weekly_patterns.size(); ++i) {
            const auto& pattern = weekly_patterns[i];
            auto week_time_t = std::chrono::system_clock::to_time_t(pattern.week_start);

            file << "    {\n";
            file << "      \"week_start\": \"" << std::put_time(std::localtime(&week_time_t), "%Y-%m-%d") << "\",\n";
            file << "      \"average_sleep_duration_hours\": " << (pattern.average_sleep_duration.count() / 3600.0) << ",\n";
            file << "      \"average_sleep_efficiency\": " << pattern.average_sleep_efficiency << ",\n";
            file << "      \"average_sleep_score\": " << pattern.average_sleep_score << ",\n";
            file << "      \"has_consistent_schedule\": " << (pattern.has_consistent_schedule ? "true" : "false") << ",\n";
            file << "      \"weekend_schedule_shift_minutes\": " << pattern.weekend_schedule_shift_minutes << ",\n";
            file << "      \"recommendations\": [\n";

            for (size_t j = 0; j < pattern.recommendations.size(); ++j) {
                file << "        \"" << pattern.recommendations[j] << "\"";
                if (j < pattern.recommendations.size() - 1) file << ",";
                file << "\n";
            }

            file << "      ]\n";
            file << "    }";
            if (i < weekly_patterns.size() - 1) file << ",";
            file << "\n";
        }

        file << "  ]\n";
        file << "}\n";

        return file.good();
    }

    bool DescansaCoreManager::backup_all_data(const std::string& backup_path) const {
        // Create a comprehensive backup file
        std::ofstream backup(backup_path);
        if (!backup.is_open()) return false;

        auto now = std::time(nullptr);
        backup << "# Descansa Data Backup\n";
        backup << "# Generated: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n\n";

        // Backup goals
        backup << "[GOALS]\n";
        backup << "target_sleep_duration=" << user_goals.target_sleep_duration.count() << "\n";
        backup << "preferred_bedtime=" << user_goals.preferred_bedtime.count() << "\n";
        backup << "preferred_wake_time=" << user_goals.preferred_wake_time.count() << "\n";
        backup << "target_sleep_efficiency=" << user_goals.target_sleep_efficiency << "\n";
        backup << "weekend_schedule_differs=" << (user_goals.weekend_schedule_differs ? "1" : "0") << "\n";
        backup << "weekend_sleep_extension=" << user_goals.weekend_sleep_extension.count() << "\n\n";

        // Backup sessions
        backup << "[SESSIONS]\n";
        for (const auto& session : detailed_sessions) {
            if (session.is_complete) {
                auto start_time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto end_time_t = std::chrono::system_clock::to_time_t(session.wake_up);

                backup << start_time_t << "," << end_time_t << ","
                       << session.sleep_efficiency << "," << static_cast<int>(session.perceived_quality) << ","
                       << (session.is_nap ? "1" : "0") << "," << session.awakenings_count << ","
                       << session.room_temperature << "," << session.noise_level << ","
                       << session.light_level << ",\"" << session.notes << "\"\n";
            }
        }
        backup << "\n";

        // Backup daily summaries
        backup << "[SUMMARIES]\n";
        for (const auto& summary : daily_summaries) {
            auto date_time_t = std::chrono::system_clock::to_time_t(summary.date);
            backup << date_time_t << "," << summary.total_sleep_time.count() << ","
                   << summary.average_sleep_efficiency << "," << (summary.met_sleep_goal ? "1" : "0") << ","
                   << summary.sleep_debt.count() << "\n";
        }

        return backup.good();
    }

// Continue with the rest of the implementation...
    bool DescansaCoreManager::save_all_data() const {
        // Save to basic text format for simplicity and cross-platform compatibility
        std::ofstream sessions_out(sessions_file);
        if (sessions_out.is_open()) {
            sessions_out << detailed_sessions.size() << "\n";
            for (const auto& session : detailed_sessions) {
                if (session.is_complete) {
                    auto start_time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                    auto end_time_t = std::chrono::system_clock::to_time_t(session.wake_up);

                    sessions_out << start_time_t << "," << end_time_t << ","
                                 << session.sleep_efficiency << "," << static_cast<int>(session.perceived_quality) << ","
                                 << (session.is_nap ? "1" : "0") << "," << session.awakenings_count << ","
                                 << session.room_temperature << "," << session.noise_level << ","
                                 << session.light_level << ",\"" << session.notes << "\"\n";
                }
            }
            sessions_out.close();
        }

        // Save daily summaries
        std::ofstream summaries_out(summaries_file);
        if (summaries_out.is_open()) {
            summaries_out << daily_summaries.size() << "\n";
            for (const auto& summary : daily_summaries) {
                auto date_time_t = std::chrono::system_clock::to_time_t(summary.date);
                summaries_out << date_time_t << "," << summary.total_sleep_time.count() << ","
                              << summary.average_sleep_efficiency << "," << (summary.met_sleep_goal ? "1" : "0") << ","
                              << summary.sleep_debt.count() << "\n";
            }
            summaries_out.close();
        }

        // Save user goals
        std::ofstream goals_out(goals_file);
        if (goals_out.is_open()) {
            goals_out << user_goals.target_sleep_duration.count() << "\n";
            goals_out << user_goals.preferred_bedtime.count() << "\n";
            goals_out << user_goals.preferred_wake_time.count() << "\n";
            goals_out << user_goals.target_sleep_efficiency << "\n";
            goals_out << (user_goals.weekend_schedule_differs ? "1" : "0") << "\n";
            goals_out << user_goals.weekend_sleep_extension.count() << "\n";
            goals_out.close();
        }

        return true;
    }

    bool DescansaCoreManager::load_all_data() {
        // Load detailed sessions
        std::ifstream sessions_in(sessions_file);
        if (sessions_in.is_open()) {
            size_t count;
            sessions_in >> count;
            sessions_in.ignore(); // Skip newline

            detailed_sessions.clear();
            detailed_sessions.reserve(count);

            std::string line;
            while (std::getline(sessions_in, line) && !line.empty()) {
                std::istringstream ss(line);
                std::string token;
                std::vector<std::string> tokens;

                // Parse CSV with quoted strings
                bool in_quotes = false;
                std::string current_token;

                for (char c : line) {
                    if (c == '"') {
                        in_quotes = !in_quotes;
                    } else if (c == ',' && !in_quotes) {
                        tokens.push_back(current_token);
                        current_token.clear();
                    } else {
                        current_token += c;
                    }
                }
                tokens.push_back(current_token);

                if (tokens.size() >= 9) {
                    DetailedSleepSession session;

                    std::time_t start_t = static_cast<std::time_t>(std::stoll(tokens[0]));
                    std::time_t end_t = static_cast<std::time_t>(std::stoll(tokens[1]));

                    session.sleep_start = std::chrono::system_clock::from_time_t(start_t);
                    session.wake_up = std::chrono::system_clock::from_time_t(end_t);
                    session.total_sleep_duration = std::chrono::duration_cast<Duration>(
                            session.wake_up - session.sleep_start);
                    session.time_in_bed = session.total_sleep_duration;
                    session.sleep_efficiency = std::stod(tokens[2]);
                    session.perceived_quality = static_cast<SleepQuality>(std::stoi(tokens[3]));
                    session.is_nap = (tokens[4] == "1");
                    session.awakenings_count = std::stoi(tokens[5]);
                    session.room_temperature = std::stod(tokens[6]);
                    session.noise_level = std::stoi(tokens[7]);
                    session.light_level = std::stoi(tokens[8]);
                    if (tokens.size() > 9) {
                        session.notes = tokens[9];
                        // Remove quotes if present
                        if (session.notes.front() == '"' && session.notes.back() == '"') {
                            session.notes = session.notes.substr(1, session.notes.length() - 2);
                        }
                    }
                    session.is_complete = true;

                    detailed_sessions.push_back(session);
                }
            }
            sessions_in.close();
        }

        // Load user goals
        std::ifstream goals_in(goals_file);
        if (goals_in.is_open()) {
            double target_duration, target_efficiency, weekend_extension;
            int bedtime_hour, wake_hour, weekend_differs;

            goals_in >> target_duration >> bedtime_hour >> wake_hour
                     >> target_efficiency >> weekend_differs >> weekend_extension;

            user_goals.target_sleep_duration = Duration(target_duration);
            user_goals.preferred_bedtime = std::chrono::hours(bedtime_hour);
            user_goals.preferred_wake_time = std::chrono::hours(wake_hour);
            user_goals.target_sleep_efficiency = target_efficiency;
            user_goals.weekend_schedule_differs = (weekend_differs == 1);
            user_goals.weekend_sleep_extension = Duration(weekend_extension);

            goals_in.close();
        }

        return true;
    }

// Helper method implementations
    void DescansaCoreManager::update_daily_summary(const DetailedSleepSession& session) {
        // Find or create daily summary for this session's date
        DailySleepSummary* summary = nullptr;

        for (auto& existing_summary : daily_summaries) {
            if (is_same_calendar_day(existing_summary.date, session.wake_up)) {
                summary = &existing_summary;
                break;
            }
        }

        if (!summary) {
            daily_summaries.emplace_back(session.wake_up);
            summary = &daily_summaries.back();
        }

        // Update summary with session data
        if (session.is_nap) {
            summary->naps.push_back(session);
        } else {
            summary->main_sleep = session;
        }

        summary->target_sleep_duration = user_goals.target_sleep_duration;
        summary->calculate_daily_totals();

        // Trigger callback if set
        if (daily_summary_callback) {
            daily_summary_callback(*summary);
        }
    }

    bool DescansaCoreManager::is_same_calendar_day(const TimePoint& t1, const TimePoint& t2) const {
        auto time1 = std::chrono::system_clock::to_time_t(t1);
        auto time2 = std::chrono::system_clock::to_time_t(t2);
        auto tm1 = *std::localtime(&time1);
        auto tm2 = *std::localtime(&time2);

        return (tm1.tm_year == tm2.tm_year && tm1.tm_yday == tm2.tm_yday);
    }

// Additional missing implementations for completeness
    bool DescansaCoreManager::is_meeting_goals() const {
        if (detailed_sessions.empty()) return true;

        // Check last week's performance
        auto recent_summaries = get_recent_summaries(7);
        if (recent_summaries.empty()) return true;

        int goals_met = 0;
        for (const auto& summary : recent_summaries) {
            if (summary.met_sleep_goal) {
                goals_met++;
            }
        }

        return (static_cast<double>(goals_met) / recent_summaries.size()) >= 0.7; // 70% success rate
    }

    SleepStatistics DescansaCoreManager::calculate_statistics(const TimePoint& start, const TimePoint& end) const {
        std::vector<DetailedSleepSession> range_sessions;
        for (const auto& session : detailed_sessions) {
            if (session.wake_up >= start && session.wake_up <= end && session.is_complete) {
                range_sessions.push_back(session);
            }
        }

        SleepStatistics stats;
        stats.analysis_period_start = start;
        stats.analysis_period_end = end;
        stats.calculate_from_sessions(range_sessions);

        // Get daily summaries for trend analysis
        std::vector<DailySleepSummary> range_summaries;
        for (const auto& summary : daily_summaries) {
            if (summary.date >= start && summary.date <= end) {
                range_summaries.push_back(summary);
            }
        }
        stats.calculate_trends(range_summaries);

        return stats;
    }

    SleepStatistics DescansaCoreManager::calculate_recent_statistics(int days) const {
        TimePoint cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * days);
        TimePoint now = std::chrono::system_clock::now();
        return calculate_statistics(cutoff, now);
    }

    double DescansaCoreManager::get_goal_adherence_percentage() const {
        auto recent_summaries = get_recent_summaries(7);
        if (recent_summaries.empty()) return 100.0;

        double total_adherence = 0.0;
        for (const auto& summary : recent_summaries) {
            total_adherence += user_goals.calculate_goal_adherence(summary);
        }

        return total_adherence / recent_summaries.size();
    }

    std::vector<std::string> DescansaCoreManager::identify_sleep_patterns() const {
        std::vector<std::string> patterns;

        if (detailed_sessions.size() < 7) {
            patterns.push_back("Insufficient data for pattern analysis");
            return patterns;
        }

        auto recent_sessions = get_sessions(14); // Last 2 weeks

        // Analyze bedtime consistency
        std::vector<int> bedtime_hours;
        for (const auto& session : recent_sessions) {
            auto time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
            auto tm = *std::localtime(&time_t);
            bedtime_hours.push_back(tm.tm_hour);
        }

        auto minmax = std::minmax_element(bedtime_hours.begin(), bedtime_hours.end());
        int bedtime_variance = *minmax.second - *minmax.first;

        if (bedtime_variance <= 1) {
            patterns.push_back("Highly consistent bedtime schedule");
        } else if (bedtime_variance <= 2) {
            patterns.push_back("Moderately consistent bedtime schedule");
        } else {
            patterns.push_back("Irregular bedtime schedule - high variance detected");
        }

        // Analyze sleep duration patterns
        double avg_duration = 0;
        for (const auto& session : recent_sessions) {
            avg_duration += session.total_sleep_duration.count() / 3600.0;
        }
        avg_duration /= recent_sessions.size();

        if (avg_duration < 7.0) {
            patterns.push_back("Chronic sleep restriction pattern detected");
        } else if (avg_duration > 9.0) {
            patterns.push_back("Extended sleep duration pattern");
        } else {
            patterns.push_back("Normal sleep duration range");
        }

        return patterns;
    }

    std::vector<std::string> DescansaCoreManager::get_improvement_suggestions() const {
        std::vector<std::string> suggestions;

        auto recent_stats = calculate_recent_statistics(14);
        auto recent_summaries = get_recent_summaries(7);

        // Sleep duration suggestions
        if (recent_stats.average_sleep_duration.count() < user_goals.target_sleep_duration.count()) {
            double deficit_hours = (user_goals.target_sleep_duration.count() -
                                    recent_stats.average_sleep_duration.count()) / 3600.0;
            suggestions.push_back("Increase sleep duration by " +
                                  std::to_string(static_cast<int>(deficit_hours * 60)) +
                                  " minutes to meet your goal");
        }

        // Sleep efficiency suggestions
        if (recent_stats.average_sleep_efficiency < user_goals.target_sleep_efficiency) {
            suggestions.push_back("Improve sleep efficiency by optimizing your sleep environment");
            suggestions.push_back("Consider limiting screen time 1 hour before bed");
            suggestions.push_back("Establish a consistent pre-sleep routine");
        }

        // Goal adherence suggestions
        double adherence = get_goal_adherence_percentage();
        if (adherence < 70.0) {
            suggestions.push_back("Your goal adherence is below 70% - consider adjusting goals or improving habits");
        }

        return suggestions;
    }

    Duration DescansaCoreManager::calculate_current_sleep_debt() const {
        auto recent_summaries = get_recent_summaries(7);
        Duration total_debt(0);

        for (const auto& summary : recent_summaries) {
            if (summary.sleep_debt.count() > 0) {
                total_debt += summary.sleep_debt;
            }
        }

        return total_debt;
    }

    Duration DescansaCoreManager::calculate_cumulative_sleep_debt(int days) const {
        auto summaries = get_recent_summaries(days);
        Duration total_debt(0);

        for (const auto& summary : summaries) {
            total_debt += summary.sleep_debt;
        }

        return total_debt;
    }

    std::vector<TimePoint> DescansaCoreManager::suggest_recovery_sleep_times() const {
        std::vector<TimePoint> suggestions;

        Duration debt = calculate_current_sleep_debt();
        if (debt.count() <= 0) return suggestions;

        // Suggest earlier bedtime for next few days
        TimePoint tonight = std::chrono::system_clock::now();
        auto tonight_t = std::chrono::system_clock::to_time_t(tonight);
        auto tonight_tm = *std::localtime(&tonight_t);

        // Set to today's preferred bedtime
        tonight_tm.tm_hour = static_cast<int>(user_goals.preferred_bedtime.count());
        tonight_tm.tm_min = 0;
        tonight_tm.tm_sec = 0;

        TimePoint normal_bedtime = std::chrono::system_clock::from_time_t(std::mktime(&tonight_tm));

        // If it's already past bedtime, start from tomorrow
        if (normal_bedtime <= tonight) {
            normal_bedtime += std::chrono::hours(24);
        }

        // Suggest going to bed 30-60 minutes earlier for several days
        double debt_hours = debt.count() / 3600.0;
        int recovery_days = std::min(7, static_cast<int>(debt_hours * 2)); // Spread recovery over days

        for (int i = 0; i < recovery_days; ++i) {
            TimePoint suggested_bedtime = normal_bedtime + std::chrono::hours(24 * i) - std::chrono::minutes(45);
            suggestions.push_back(suggested_bedtime);
        }

        return suggestions;
    }

    bool DescansaCoreManager::is_in_sleep_debt() const {
        return calculate_current_sleep_debt().count() > 0;
    }

    bool DescansaCoreManager::export_detailed_data(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        auto now = std::time(nullptr);
        file << "Descansa Detailed Sleep Data Export\n";
        file << "Generated: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n\n";

        file << "Sleep Goals:\n";
        file << "Target Sleep Duration: " << (user_goals.target_sleep_duration.count() / 3600.0) << " hours\n";
        file << "Preferred Bedtime: " << user_goals.preferred_bedtime.count() << ":00\n";
        file << "Preferred Wake Time: " << user_goals.preferred_wake_time.count() << ":00\n";
        file << "Target Sleep Efficiency: " << user_goals.target_sleep_efficiency << "%\n\n";

        file << "Detailed Sleep Sessions:\n";
        file << "Date,Sleep Start,Wake Up,Duration (hours),Efficiency (%),Quality,Is Nap,Notes\n";

        for (const auto& session : detailed_sessions) {
            if (session.is_complete) {
                auto start_time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto end_time_t = std::chrono::system_clock::to_time_t(session.wake_up);

                file << std::put_time(std::localtime(&start_time_t), "%Y-%m-%d") << ","
                     << std::put_time(std::localtime(&start_time_t), "%H:%M:%S") << ","
                     << std::put_time(std::localtime(&end_time_t), "%H:%M:%S") << ","
                     << std::fixed << std::setprecision(2) << (session.total_sleep_duration.count() / 3600.0) << ","
                     << std::setprecision(1) << session.sleep_efficiency << ","
                     << session.get_quality_description() << ","
                     << (session.is_nap ? "Yes" : "No") << ","
                     << "\"" << session.notes << "\"\n";
            }
        }

        return file.good();
    }

    void DescansaCoreManager::clear_all_data() {
        detailed_sessions.clear();
        daily_summaries.clear();
        weekly_patterns.clear();
        enhanced_session_active = false;
        basic_core->clear_history();
    }

    void DescansaCoreManager::clear_old_data(int days_to_keep) {
        TimePoint cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * days_to_keep);

        // Remove old sessions
        detailed_sessions.erase(
                std::remove_if(detailed_sessions.begin(), detailed_sessions.end(),
                               [cutoff](const DetailedSleepSession& session) {
                                   return session.wake_up < cutoff;
                               }),
                detailed_sessions.end()
        );

        // Remove old summaries
        daily_summaries.erase(
                std::remove_if(daily_summaries.begin(), daily_summaries.end(),
                               [cutoff](const DailySleepSummary& summary) {
                                   return summary.date < cutoff;
                               }),
                daily_summaries.end()
        );
    }

    bool DescansaCoreManager::validate_data_integrity() const {
        // Check for data consistency issues

        // Verify sessions have valid timestamps
        for (const auto& session : detailed_sessions) {
            if (session.sleep_start >= session.wake_up && session.is_complete) {
                return false; // Invalid time range
            }

            if (session.sleep_efficiency < 0.0 || session.sleep_efficiency > 100.0) {
                return false; // Invalid efficiency
            }
        }

        return true;
    }

    std::string DescansaCoreManager::get_system_status() const {
        std::ostringstream status;

        status << "Descansa Core Manager Status\n";
        status << "============================\n\n";

        status << "Session Status: " << (enhanced_session_active ? "Active" : "Inactive") << "\n";
        status << "Total Sessions: " << detailed_sessions.size() << "\n";
        status << "Daily Summaries: " << daily_summaries.size() << "\n";
        status << "Weekly Patterns: " << weekly_patterns.size() << "\n\n";

        if (enhanced_session_active) {
            auto preview = get_current_session_preview();
            status << "Current Session Duration: "
                   << std::fixed << std::setprecision(1)
                   << (preview.total_sleep_duration.count() / 3600.0) << " hours\n";
        }

        status << "Goal Adherence: " << std::setprecision(1)
               << get_goal_adherence_percentage() << "%\n";

        if (is_in_sleep_debt()) {
            status << "Sleep Debt: " << std::setprecision(1)
                   << (calculate_current_sleep_debt().count() / 3600.0) << " hours\n";
        } else {
            status << "Sleep Debt: None\n";
        }

        return status.str();
    }

    void DescansaCoreManager::sync_with_basic_core() {
        // Sync goals
        basic_core->set_target_sleep_hours(user_goals.target_sleep_duration.count() / 3600.0);
        basic_core->set_target_wake_time(
                static_cast<int>(user_goals.preferred_wake_time.count()), 0);

        // Sync session state
        if (enhanced_session_active && !basic_core->is_session_running()) {
            basic_core->start_sleep_session();
        } else if (!enhanced_session_active && basic_core->is_session_running()) {
            basic_core->end_sleep_session();
        }
    }

    void DescansaCoreManager::set_session_completed_callback(std::function<void(const DetailedSleepSession&)> callback) {
        session_completed_callback = std::move(callback);
    }

    void DescansaCoreManager::set_daily_summary_callback(std::function<void(const DailySleepSummary&)> callback) {
        daily_summary_callback = std::move(callback);
    }

    std::vector<SleepPhase> DescansaCoreManager::detect_sleep_phases() const {
        // Placeholder for future sensor integration
        std::vector<SleepPhase> phases;

        if (enhanced_session_active) {
            Duration elapsed = std::chrono::duration_cast<Duration>(
                    std::chrono::system_clock::now() - current_session.sleep_start);

            if (elapsed.count() > 1800) { // 30 minutes
                phases.emplace_back(current_session.sleep_start, Duration(1800), "light");
            }
            if (elapsed.count() > 3600) { // 1 hour
                phases.emplace_back(current_session.sleep_start + std::chrono::seconds(1800),
                                    Duration(1800), "deep");
            }
        }

        return phases;
    }

    void DescansaCoreManager::calibrate_sleep_detection() {
        // Placeholder for auto-tuning detection algorithms
    }

    std::vector<std::string> DescansaCoreManager::get_environmental_recommendations() const {
        return current_environment.get_environment_recommendations();
    }

    void DescansaCoreManager::optimize_schedule_for_goals() {
        // Analyze current performance and suggest schedule adjustments
        auto recent_stats = calculate_recent_statistics(14);

        if (recent_stats.average_sleep_duration < user_goals.target_sleep_duration) {
            // Suggest earlier bedtime
            Duration deficit = user_goals.target_sleep_duration - recent_stats.average_sleep_duration;
            auto earlier_bedtime = user_goals.preferred_bedtime -
                                   std::chrono::duration_cast<std::chrono::hours>(deficit);

            if (earlier_bedtime.count() >= 20) { // Don't go earlier than 8 PM
                user_goals.preferred_bedtime = earlier_bedtime;
            }
        }
    }

    bool DescansaCoreManager::run_diagnostics() const {
        return validate_data_integrity();
    }

    std::vector<std::string> DescansaCoreManager::get_data_warnings() const {
        std::vector<std::string> warnings;

        // Check for missing quality ratings
        int missing_quality = 0;
        for (const auto& session : detailed_sessions) {
            if (session.perceived_quality == SleepQuality::UNKNOWN) {
                missing_quality++;
            }
        }

        if (missing_quality > 0) {
            warnings.push_back("Missing sleep quality ratings for " + std::to_string(missing_quality) + " sessions");
        }

        // Check for incomplete sessions
        int incomplete_sessions = 0;
        for (const auto& session : detailed_sessions) {
            if (!session.is_complete) {
                incomplete_sessions++;
            }
        }

        if (incomplete_sessions > 0) {
            warnings.push_back(std::to_string(incomplete_sessions) + " incomplete sleep sessions found");
        }

        return warnings;
    }

    bool DescansaCoreManager::restore_from_backup(const std::string& backup_path) {
        std::ifstream backup(backup_path);
        if (!backup.is_open()) return false;

        std::string line;
        std::string current_section;

        // Clear existing data
        detailed_sessions.clear();
        daily_summaries.clear();
        weekly_patterns.clear();

        while (std::getline(backup, line)) {
            if (line.empty() || line[0] == '#') continue;

            if (line[0] == '[' && line.back() == ']') {
                current_section = line.substr(1, line.length() - 2);
                continue;
            }

            if (current_section == "GOALS") {
                size_t eq_pos = line.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = line.substr(0, eq_pos);
                    std::string value = line.substr(eq_pos + 1);

                    if (key == "target_sleep_duration") {
                        user_goals.target_sleep_duration = Duration(std::stod(value));
                    } else if (key == "preferred_bedtime") {
                        user_goals.preferred_bedtime = std::chrono::hours(std::stoi(value));
                    } else if (key == "preferred_wake_time") {
                        user_goals.preferred_wake_time = std::chrono::hours(std::stoi(value));
                    } else if (key == "target_sleep_efficiency") {
                        user_goals.target_sleep_efficiency = std::stod(value);
                    } else if (key == "weekend_schedule_differs") {
                        user_goals.weekend_schedule_differs = (value == "1");
                    } else if (key == "weekend_sleep_extension") {
                        user_goals.weekend_sleep_extension = Duration(std::stod(value));
                    }
                }
            }
        }

        return true;
    }

} // namespace descansa