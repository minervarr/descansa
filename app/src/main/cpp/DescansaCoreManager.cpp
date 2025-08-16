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
        else if (current_section == "SESSIONS") {
            std::istringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;

            // Parse CSV line with quoted strings
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
                }
                session.is_complete = true;

                detailed_sessions.push_back(session);
            }
        }
        else if (current_section == "SUMMARIES") {
            std::istringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 5) {
                DailySleepSummary summary;

                std::time_t date_t = static_cast<std::time_t>(std::stoll(tokens[0]));
                summary.date = std::chrono::system_clock::from_time_t(date_t);
                summary.total_sleep_time = Duration(std::stod(tokens[1]));
                summary.average_sleep_efficiency = std::stod(tokens[2]);
                summary.met_sleep_goal = (tokens[3] == "1");
                summary.sleep_debt = Duration(std::stod(tokens[4]));

                daily_summaries.push_back(summary);
            }
        }
    }

    return true;
}

bool DescansaCoreManager::save_all_data() const {
    // Save detailed sessions
    std::ofstream sessions_out(sessions_file, std::ios::binary);
    if (!sessions_out.is_open()) return false;

    size_t count = detailed_sessions.size();
    sessions_out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& session : detailed_sessions) {
        // Serialize session data (simplified - would need proper serialization)
        auto start_time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
        auto end_time_t = std::chrono::system_clock::to_time_t(session.wake_up);

        sessions_out.write(reinterpret_cast<const char*>(&start_time_t), sizeof(start_time_t));
        sessions_out.write(reinterpret_cast<const char*>(&end_time_t), sizeof(end_time_t));
        sessions_out.write(reinterpret_cast<const char*>(&session.sleep_efficiency), sizeof(session.sleep_efficiency));
        sessions_out.write(reinterpret_cast<const char*>(&session.perceived_quality), sizeof(session.perceived_quality));
        sessions_out.write(reinterpret_cast<const char*>(&session.is_nap), sizeof(session.is_nap));
        sessions_out.write(reinterpret_cast<const char*>(&session.is_complete), sizeof(session.is_complete));

        // Save notes
        size_t notes_length = session.notes.length();
        sessions_out.write(reinterpret_cast<const char*>(&notes_length), sizeof(notes_length));
        sessions_out.write(session.notes.c_str(), notes_length);
    }
    sessions_out.close();

    // Save daily summaries
    std::ofstream summaries_out(summaries_file, std::ios::binary);
    if (!summaries_out.is_open()) return false;

    count = daily_summaries.size();
    summaries_out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& summary : daily_summaries) {
        auto date_time_t = std::chrono::system_clock::to_time_t(summary.date);
        summaries_out.write(reinterpret_cast<const char*>(&date_time_t), sizeof(date_time_t));
        summaries_out.write(reinterpret_cast<const char*>(&summary.total_sleep_time), sizeof(summary.total_sleep_time));
        summaries_out.write(reinterpret_cast<const char*>(&summary.average_sleep_efficiency), sizeof(summary.average_sleep_efficiency));
        summaries_out.write(reinterpret_cast<const char*>(&summary.met_sleep_goal), sizeof(summary.met_sleep_goal));
    }
    summaries_out.close();

    // Save user goals
    std::ofstream goals_out(goals_file, std::ios::binary);
    if (!goals_out.is_open()) return false;

    goals_out.write(reinterpret_cast<const char*>(&user_goals.target_sleep_duration), sizeof(user_goals.target_sleep_duration));
    goals_out.write(reinterpret_cast<const char*>(&user_goals.preferred_bedtime), sizeof(user_goals.preferred_bedtime));
    goals_out.write(reinterpret_cast<const char*>(&user_goals.preferred_wake_time), sizeof(user_goals.preferred_wake_time));
    goals_out.write(reinterpret_cast<const char*>(&user_goals.target_sleep_efficiency), sizeof(user_goals.target_sleep_efficiency));
    goals_out.write(reinterpret_cast<const char*>(&user_goals.weekend_schedule_differs), sizeof(user_goals.weekend_schedule_differs));
    goals_out.close();

    return true;
}

bool DescansaCoreManager::load_all_data() {
    // Load detailed sessions
    std::ifstream sessions_in(sessions_file, std::ios::binary);
    if (sessions_in.is_open()) {
        size_t count;
        sessions_in.read(reinterpret_cast<char*>(&count), sizeof(count));

        detailed_sessions.clear();
        detailed_sessions.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            DetailedSleepSession session;

            std::time_t start_time_t, end_time_t;
            sessions_in.read(reinterpret_cast<char*>(&start_time_t), sizeof(start_time_t));
            sessions_in.read(reinterpret_cast<char*>(&end_time_t), sizeof(end_time_t));

            session.sleep_start = std::chrono::system_clock::from_time_t(start_time_t);
            session.wake_up = std::chrono::system_clock::from_time_t(end_time_t);
            session.total_sleep_duration = std::chrono::duration_cast<Duration>(
                    session.wake_up - session.sleep_start);

            sessions_in.read(reinterpret_cast<char*>(&session.sleep_efficiency), sizeof(session.sleep_efficiency));
            sessions_in.read(reinterpret_cast<char*>(&session.perceived_quality), sizeof(session.perceived_quality));
            sessions_in.read(reinterpret_cast<char*>(&session.is_nap), sizeof(session.is_nap));
            sessions_in.read(reinterpret_cast<char*>(&session.is_complete), sizeof(session.is_complete));

            // Load notes
            size_t notes_length;
            sessions_in.read(reinterpret_cast<char*>(&notes_length), sizeof(notes_length));
            if (notes_length > 0 && notes_length < 10000) { // Sanity check
                session.notes.resize(notes_length);
                sessions_in.read(&session.notes[0], notes_length);
            }

            detailed_sessions.push_back(session);
        }
        sessions_in.close();
    }

    // Load daily summaries
    std::ifstream summaries_in(summaries_file, std::ios::binary);
    if (summaries_in.is_open()) {
        size_t count;
        summaries_in.read(reinterpret_cast<char*>(&count), sizeof(count));

        daily_summaries.clear();
        daily_summaries.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            DailySleepSummary summary;

            std::time_t date_time_t;
            summaries_in.read(reinterpret_cast<char*>(&date_time_t), sizeof(date_time_t));
            summary.date = std::chrono::system_clock::from_time_t(date_time_t);

            summaries_in.read(reinterpret_cast<char*>(&summary.total_sleep_time), sizeof(summary.total_sleep_time));
            summaries_in.read(reinterpret_cast<char*>(&summary.average_sleep_efficiency), sizeof(summary.average_sleep_efficiency));
            summaries_in.read(reinterpret_cast<char*>(&summary.met_sleep_goal), sizeof(summary.met_sleep_goal));

            daily_summaries.push_back(summary);
        }
        summaries_in.close();
    }

    // Load user goals
    std::ifstream goals_in(goals_file, std::ios::binary);
    if (goals_in.is_open()) {
        goals_in.read(reinterpret_cast<char*>(&user_goals.target_sleep_duration), sizeof(user_goals.target_sleep_duration));
        goals_in.read(reinterpret_cast<char*>(&user_goals.preferred_bedtime), sizeof(user_goals.preferred_bedtime));
        goals_in.read(reinterpret_cast<char*>(&user_goals.preferred_wake_time), sizeof(user_goals.preferred_wake_time));
        goals_in.read(reinterpret_cast<char*>(&user_goals.target_sleep_efficiency), sizeof(user_goals.target_sleep_efficiency));
        goals_in.read(reinterpret_cast<char*>(&user_goals.weekend_schedule_differs), sizeof(user_goals.weekend_schedule_differs));
        goals_in.close();
    }

    return true;
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

    // Verify daily summaries correspond to sessions
    for (const auto& summary : daily_summaries) {
        bool found_session = false;
        for (const auto& session : detailed_sessions) {
            if (is_same_calendar_day(summary.date, session.wake_up)) {
                found_session = true;
                break;
            }
        }

        if (!found_session && summary.total_sleep_time.count() > 0) {
            return false; // Summary without corresponding session
        }
    }

    return true;
}

bool DescansaCoreManager::run_diagnostics() const {
    std::vector<std::string> issues;

    // Check data integrity
    if (!validate_data_integrity()) {
        issues.push_back("Data integrity check failed");
    }

    // Check for reasonable sleep durations
    for (const auto& session : detailed_sessions) {
        double hours = session.total_sleep_duration.count() / 3600.0;
        if (hours < 1.0 || hours > 20.0) {
            issues.push_back("Unrealistic sleep duration detected: " + std::to_string(hours) + " hours");
        }
    }

    // Check for data gaps
    if (detailed_sessions.size() > 1) {
        auto recent_sessions = get_sessions(7);
        if (recent_sessions.size() < 3) {
            issues.push_back("Insufficient recent data for analysis");
        }
    }

    return issues.empty();
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

    // Check for very short or long sessions
    int unusual_sessions = 0;
    for (const auto& session : detailed_sessions) {
        double hours = session.total_sleep_duration.count() / 3600.0;
        if (hours < 2.0 || hours > 15.0) {
            unusual_sessions++;
        }
    }

    if (unusual_sessions > 0) {
        warnings.push_back(std::to_string(unusual_sessions) + " sessions with unusual durations");
    }

    return warnings;
}

std::vector<SleepPhase> DescansaCoreManager::detect_sleep_phases() const {
    // Placeholder for future sensor integration
    // This would analyze movement, heart rate, etc. to detect sleep phases
    std::vector<SleepPhase> phases;

    if (enhanced_session_active) {
        // For now, just create basic phases based on time
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
    // This would analyze historical data to improve detection accuracy
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

void DescansaCoreManager::update_weekly_patterns() {
    // Update weekly patterns based on current daily summaries
    // This would be called periodically to maintain weekly analysis
}

void DescansaCoreManager::analyze_sleep_trends() {
    // Analyze trends across time periods
    // This would identify patterns and changes in sleep behavior
}

void DescansaCoreManager::generate_recommendations() {
    // Generate personalized recommendations based on all available data
    // This would be called to update recommendation lists
}

TimePoint DescansaCoreManager::get_day_start(const TimePoint& tp) const {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    auto tm = *std::localtime(&time_t);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

bool DescansaCoreManager::is_same_calendar_day(const TimePoint& t1, const TimePoint& t2) const {
    auto time1 = std::chrono::system_clock::to_time_t(t1);
    auto time2 = std::chrono::system_clock::to_time_t(t2);
    auto tm1 = *std::localtime(&time1);
    auto tm2 = *std::localtime(&time2);

    return (tm1.tm_year == tm2.tm_year && tm1.tm_yday == tm2.tm_yday);
}

void DescansaCoreManager::set_session_completed_callback(std::function<void(const DetailedSleepSession&)> callback) {
    session_completed_callback = std::move(callback);
}

void DescansaCoreManager::set_daily_summary_callback(std::function<void(const DailySleepSummary&)> callback) {
    daily_summary_callback = std::move(callback);
}

// SleepTrendAnalyzer Implementation
SleepTrendAnalyzer::SleepTrendAnalyzer(const std::vector<DailySleepSummary>& data)
        : daily_data(data) {}

SleepStatistics::Trend SleepTrendAnalyzer::analyze_trend(TrendType type, int days) const {
    if (daily_data.size() < static_cast<size_t>(days) || days < 4) {
        return SleepStatistics::Trend::STABLE;
    }

    std::vector<double> values;
    size_t start_idx = daily_data.size() - days;

    for (size_t i = start_idx; i < daily_data.size(); ++i) {
        const auto& summary = daily_data[i];

        switch (type) {
            case TrendType::DURATION:
                values.push_back(summary.total_sleep_time.count() / 3600.0);
                break;
            case TrendType::QUALITY:
                values.push_back(summary.get_sleep_score());
                break;
            case TrendType::EFFICIENCY:
                values.push_back(summary.average_sleep_efficiency);
                break;
            case TrendType::CONSISTENCY:
                // Calculate day-to-day variance (simplified)
                if (i > start_idx) {
                    double prev_duration = daily_data[i-1].total_sleep_time.count() / 3600.0;
                    double curr_duration = summary.total_sleep_time.count() / 3600.0;
                    values.push_back(std::abs(curr_duration - prev_duration));
                }
                break;
            default:
                values.push_back(0.0);
                break;
        }
    }

    if (values.size() < 4) return SleepStatistics::Trend::STABLE;

    // Simple linear regression to detect trend
    size_t n = values.size();
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;

    for (size_t i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        double y = values[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);

    // Determine trend based on slope and significance
    double threshold = 0.1; // Adjust based on data type
    if (type == TrendType::DURATION) threshold = 0.1; // 0.1 hours per day
    else if (type == TrendType::EFFICIENCY) threshold = 1.0; // 1% per day
    else if (type == TrendType::QUALITY) threshold = 2.0; // 2 points per day

    if (slope > threshold) {
        return SleepStatistics::Trend::IMPROVING;
    } else if (slope < -threshold) {
        return SleepStatistics::Trend::DECLINING;
    } else {
        return SleepStatistics::Trend::STABLE;
    }
}

double SleepTrendAnalyzer::calculate_trend_strength(TrendType type, int days) const {
    // Returns correlation coefficient (0-1) indicating trend strength
    if (daily_data.size() < static_cast<size_t>(days) || days < 4) {
        return 0.0;
    }

    std::vector<double> x_values, y_values;
    size_t start_idx = daily_data.size() - days;

    for (size_t i = start_idx; i < daily_data.size(); ++i) {
        x_values.push_back(static_cast<double>(i - start_idx));

        const auto& summary = daily_data[i];
        switch (type) {
            case TrendType::DURATION:
                y_values.push_back(summary.total_sleep_time.count() / 3600.0);
                break;
            case TrendType::QUALITY:
                y_values.push_back(summary.get_sleep_score());
                break;
            case TrendType::EFFICIENCY:
                y_values.push_back(summary.average_sleep_efficiency);
                break;
            default:
                y_values.push_back(0.0);
                break;
        }
    }

    // Calculate Pearson correlation coefficient
    size_t n = x_values.size();
    if (n < 2) return 0.0;

    double sum_x = std::accumulate(x_values.begin(), x_values.end(), 0.0);
    double sum_y = std::accumulate(y_values.begin(), y_values.end(), 0.0);
    double mean_x = sum_x / n;
    double mean_y = sum_y / n;

    double numerator = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double dx = x_values[i] - mean_x;
        double dy = y_values[i] - mean_y;
        numerator += dx * dy;
        sum_x2 += dx * dx;
        sum_y2 += dy * dy;
    }

    double denominator = std::sqrt(sum_x2 * sum_y2);
    return (denominator > 0.0) ? std::abs(numerator / denominator) : 0.0;
}

std::vector<std::string> SleepTrendAnalyzer::generate_trend_insights() const {
    std::vector<std::string> insights;

    auto duration_trend = analyze_trend(TrendType::DURATION, 14);
    auto quality_trend = analyze_trend(TrendType::QUALITY, 14);
    auto efficiency_trend = analyze_trend(TrendType::EFFICIENCY, 14);

    // Duration insights
    if (duration_trend == SleepStatistics::Trend::IMPROVING) {
        insights.push_back("Your sleep duration is improving over the past 2 weeks");
    } else if (duration_trend == SleepStatistics::Trend::DECLINING) {
        insights.push_back("Your sleep duration has been decreasing - consider adjusting your schedule");
    }

    // Quality insights
    if (quality_trend == SleepStatistics::Trend::IMPROVING) {
        insights.push_back("Your sleep quality scores are trending upward - keep up the good habits!");
    } else if (quality_trend == SleepStatistics::Trend::DECLINING) {
        insights.push_back("Sleep quality has been declining - review your sleep environment and routine");
    }

    // Efficiency insights
    if (efficiency_trend == SleepStatistics::Trend::IMPROVING) {
        insights.push_back("Sleep efficiency is improving - you're spending more time actually sleeping");
    } else if (efficiency_trend == SleepStatistics::Trend::DECLINING) {
        insights.push_back("Sleep efficiency is declining - consider factors affecting your ability to stay asleep");
    }

    return insights;
}

bool SleepTrendAnalyzer::detect_pattern_changes() const {
    if (daily_data.size() < 14) return false;

    // Compare first week vs second week
    size_t mid_point = daily_data.size() / 2;

    double first_half_avg = 0.0, second_half_avg = 0.0;

    for (size_t i = 0; i < mid_point; ++i) {
        first_half_avg += daily_data[i].total_sleep_time.count() / 3600.0;
    }
    first_half_avg /= mid_point;

    for (size_t i = mid_point; i < daily_data.size(); ++i) {
        second_half_avg += daily_data[i].total_sleep_time.count() / 3600.0;
    }
    second_half_avg /= (daily_data.size() - mid_point);

    // Detect significant change (more than 1 hour difference)
    return std::abs(second_half_avg - first_half_avg) > 1.0;
}

// SleepScheduleOptimizer Implementation
SleepScheduleOptimizer::SleepScheduleOptimizer(const SleepGoals& user_goals,
                                               const std::vector<DetailedSleepSession>& history)
        : goals(user_goals), historical_data(history) {}

SleepScheduleOptimizer::OptimalSchedule SleepScheduleOptimizer::calculate_optimal_schedule() const {
    OptimalSchedule optimal;
    optimal.confidence_score = 0.0;

    if (historical_data.size() < 7) {
        // Insufficient data - return current goals
        optimal.recommended_bedtime = goals.preferred_bedtime;
        optimal.recommended_wake_time = goals.preferred_wake_time;
        optimal.recommended_sleep_duration = goals.target_sleep_duration;
        optimal.confidence_score = 0.3; // Low confidence
        optimal.reasoning.push_back("Insufficient historical data - using current goals");
        return optimal;
    }

    // Analyze best performing sessions
    std::vector<std::pair<double, const DetailedSleepSession*>> scored_sessions;

    for (const auto& session : historical_data) {
        if (!session.is_complete || session.is_nap) continue;

        double score = 0.0;

        // Score based on efficiency
        score += session.sleep_efficiency * 0.4;

        // Score based on quality
        score += static_cast<double>(session.perceived_quality) * 25.0 * 0.3;

        // Score based on goal adherence
        double duration_ratio = session.total_sleep_duration.count() / goals.target_sleep_duration.count();
        score += std::min(1.0, duration_ratio) * 100.0 * 0.3;

        scored_sessions.emplace_back(score, &session);
    }

    // Sort by score and take top 30%
    std::sort(scored_sessions.begin(), scored_sessions.end(),
              std::greater<std::pair<double, const DetailedSleepSession*>>());
    size_t top_count = std::max(size_t(1), scored_sessions.size() * 3 / 10);

    // Calculate optimal times from best sessions
    std::vector<int> bedtime_hours, wake_hours;
    Duration total_duration(0);

    for (size_t i = 0; i < top_count; ++i) {
        const auto* session = scored_sessions[i].second;

        auto bed_time_t = std::chrono::system_clock::to_time_t(session->sleep_start);
        auto wake_time_t = std::chrono::system_clock::to_time_t(session->wake_up);

        auto bed_tm = *std::localtime(&bed_time_t);
        auto wake_tm = *std::localtime(&wake_time_t);

        bedtime_hours.push_back(bed_tm.tm_hour);
        wake_hours.push_back(wake_tm.tm_hour);
        total_duration += session->total_sleep_duration;
    }

    // Calculate averages
    double avg_bedtime = std::accumulate(bedtime_hours.begin(), bedtime_hours.end(), 0.0) / bedtime_hours.size();
    double avg_wake_time = std::accumulate(wake_hours.begin(), wake_hours.end(), 0.0) / wake_hours.size();

    optimal.recommended_bedtime = std::chrono::hours(static_cast<int>(std::round(avg_bedtime)));
    optimal.recommended_wake_time = std::chrono::hours(static_cast<int>(std::round(avg_wake_time)));
    optimal.recommended_sleep_duration = Duration(total_duration.count() / top_count);

    // Calculate confidence based on consistency of top sessions
    double bedtime_variance = 0.0;
    for (int hour : bedtime_hours) {
        bedtime_variance += (hour - avg_bedtime) * (hour - avg_bedtime);
    }
    bedtime_variance /= bedtime_hours.size();

    optimal.confidence_score = std::max(0.0, 1.0 - (bedtime_variance / 4.0)); // Normalize

    // Generate reasoning
    optimal.reasoning.push_back("Based on analysis of your " + std::to_string(top_count) + " best sleep sessions");
    if (!scored_sessions.empty()) {
        optimal.reasoning.push_back("Average sleep efficiency in top sessions: " +
                                    std::to_string(static_cast<int>(scored_sessions[0].first)) + "%");
    }

    if (optimal.confidence_score > 0.7) {
        optimal.reasoning.push_back("High confidence recommendation based on consistent patterns");
    } else if (optimal.confidence_score > 0.4) {
        optimal.reasoning.push_back("Moderate confidence - some variation in optimal times");
    } else {
        optimal.reasoning.push_back("Low confidence - consider gathering more consistent data");
    }

    return optimal;
}

std::vector<std::string> SleepScheduleOptimizer::get_schedule_adjustment_tips() const {
    std::vector<std::string> tips;

    auto optimal = calculate_optimal_schedule();

    int bedtime_diff = static_cast<int>(optimal.recommended_bedtime.count() - goals.preferred_bedtime.count());
    int wake_diff = static_cast<int>(optimal.recommended_wake_time.count() - goals.preferred_wake_time.count());

    if (std::abs(bedtime_diff) > 0) {
        if (bedtime_diff > 0) {
            tips.push_back("Consider going to bed " + std::to_string(bedtime_diff) + " hour(s) later");
        } else {
            tips.push_back("Consider going to bed " + std::to_string(-bedtime_diff) + " hour(s) earlier");
        }
    }

    if (std::abs(wake_diff) > 0) {
        if (wake_diff > 0) {
            tips.push_back("Consider waking " + std::to_string(wake_diff) + " hour(s) later");
        } else {
            tips.push_back("Consider waking " + std::to_string(-wake_diff) + " hour(s) earlier");
        }
    }

    if (optimal.confidence_score < 0.5) {
        tips.push_back("Track more consistent sleep data to improve recommendations");
    }

    return tips;
}

bool SleepScheduleOptimizer::should_adjust_current_schedule() const {
    auto optimal = calculate_optimal_schedule();

    // Only suggest adjustment if confidence is reasonable and change is significant
    if (optimal.confidence_score < 0.4) return false;

    int bedtime_diff = std::abs(static_cast<int>(optimal.recommended_bedtime.count() - goals.preferred_bedtime.count()));
    int wake_diff = std::abs(static_cast<int>(optimal.recommended_wake_time.count() - goals.preferred_wake_time.count()));

    return (bedtime_diff >= 1 || wake_diff >= 1); // At least 1 hour difference
}

Duration SleepScheduleOptimizer::calculate_adjustment_period() const {
    // Suggest gradual adjustment over 1-2 weeks
    auto optimal = calculate_optimal_schedule();

    int max_diff = std::max(
            std::abs(static_cast<int>(optimal.recommended_bedtime.count() - goals.preferred_bedtime.count())),
            std::abs(static_cast<int>(optimal.recommended_wake_time.count() - goals.preferred_wake_time.count()))
    );

    // 3-7 days per hour of adjustment
    return Duration((max_diff * 5 + 2) * 24 * 3600);
}

// SleepEnvironmentAnalyzer Implementation
SleepEnvironmentAnalyzer::SleepEnvironmentAnalyzer(const std::vector<DetailedSleepSession>& session_data)
        : sessions(session_data) {}

std::vector<SleepEnvironmentAnalyzer::EnvironmentCorrelation> SleepEnvironmentAnalyzer::analyze_environment_impact() const {
    std::vector<EnvironmentCorrelation> correlations;

    if (sessions.size() < 10) {
        EnvironmentCorrelation insufficient;
        insufficient.factor = "Overall";
        insufficient.correlation_strength = 0.0;
        insufficient.impact_description = "Insufficient data for environmental analysis";
        insufficient.recommendations.push_back("Collect more sleep sessions with environmental data");
        correlations.push_back(insufficient);
        return correlations;
    }

    // Analyze temperature correlation
    std::vector<double> temperatures, efficiency_scores;
    for (const auto& session : sessions) {
        if (session.room_temperature > 0) {
            temperatures.push_back(session.room_temperature);
            efficiency_scores.push_back(session.sleep_efficiency);
        }
    }

    if (temperatures.size() >= 5) {
        // Calculate correlation (simplified)
        double temp_correlation = calculate_correlation(temperatures, efficiency_scores);

        EnvironmentCorrelation temp_corr;
        temp_corr.factor = "Room Temperature";
        temp_corr.correlation_strength = temp_correlation;

        if (std::abs(temp_correlation) > 0.3) {
            if (temp_correlation > 0) {
                temp_corr.impact_description = "Higher temperatures correlate with better sleep efficiency";
                temp_corr.recommendations.push_back("Consider slightly increasing room temperature");
            } else {
                temp_corr.impact_description = "Lower temperatures correlate with better sleep efficiency";
                temp_corr.recommendations.push_back("Consider cooling your room before sleep");
            }
        } else {
            temp_corr.impact_description = "Room temperature shows minimal impact on sleep efficiency";
        }

        correlations.push_back(temp_corr);
    }

    // Analyze noise correlation
    std::vector<double> noise_levels;
    efficiency_scores.clear();

    for (const auto& session : sessions) {
        if (session.noise_level >= 0) {
            noise_levels.push_back(session.noise_level);
            efficiency_scores.push_back(session.sleep_efficiency);
        }
    }

    if (noise_levels.size() >= 5) {
        double noise_correlation = calculate_correlation(noise_levels, efficiency_scores);

        EnvironmentCorrelation noise_corr;
        noise_corr.factor = "Noise Level";
        noise_corr.correlation_strength = noise_correlation;

        if (noise_correlation < -0.2) {
            noise_corr.impact_description = "Higher noise levels correlate with reduced sleep efficiency";
            noise_corr.recommendations.push_back("Consider using earplugs or white noise");
            noise_corr.recommendations.push_back("Identify and eliminate noise sources");
        } else {
            noise_corr.impact_description = "Noise level shows minimal impact on sleep efficiency";
        }

        correlations.push_back(noise_corr);
    }

    return correlations;
}

double SleepEnvironmentAnalyzer::calculate_correlation(const std::vector<double>& x, const std::vector<double>& y) const {
    if (x.size() != y.size() || x.size() < 2) return 0.0;

    size_t n = x.size();
    double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
    double sum_y = std::accumulate(y.begin(), y.end(), 0.0);
    double mean_x = sum_x / n;
    double mean_y = sum_y / n;

    double numerator = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double dx = x[i] - mean_x;
        double dy = y[i] - mean_y;
        numerator += dx * dy;
        sum_x2 += dx * dx;
        sum_y2 += dy * dy;
    }

    double denominator = std::sqrt(sum_x2 * sum_y2);
    return (denominator > 0.0) ? (numerator / denominator) : 0.0;
}

SleepEnvironment SleepEnvironmentAnalyzer::get_optimal_environment() const {
    SleepEnvironment optimal;

    // Find sessions with highest efficiency and extract their environment
    auto best_sessions = sessions;
    std::sort(best_sessions.begin(), best_sessions.end(),
              [](const DetailedSleepSession& a, const DetailedSleepSession& b) {
                  return a.sleep_efficiency > b.sleep_efficiency;
              });

    if (best_sessions.size() >= 3) {
        // Average the environment of top 3 sessions
        optimal.temperature = (best_sessions[0].room_temperature +
                               best_sessions[1].room_temperature +
                               best_sessions[2].room_temperature) / 3.0;
        optimal.noise_level = (best_sessions[0].noise_level +
                               best_sessions[1].noise_level +
                               best_sessions[2].noise_level) / 3;
        optimal.light_level = (best_sessions[0].light_level +
                               best_sessions[1].light_level +
                               best_sessions[2].light_level) / 3;
    } else {
        // Use general recommendations
        optimal.temperature = 19.0; // 19°C optimal
        optimal.noise_level = 20;   // Low noise
        optimal.light_level = 5;    // Minimal light
        optimal.humidity = 50.0;    // 50% humidity
    }

    return optimal;
}

std::vector<std::string> SleepEnvironmentAnalyzer::get_environment_improvements() const {
    std::vector<std::string> improvements;
    auto optimal = get_optimal_environment();

    improvements.push_back("Optimal room temperature: " +
                           std::to_string(static_cast<int>(optimal.temperature)) + "°C");
    improvements.push_back("Keep noise levels below " +
                           std::to_string(optimal.noise_level) + " (use earplugs if needed)");
    improvements.push_back("Minimize light sources (blackout curtains recommended)");
    improvements.push_back("Maintain humidity around 40-60%");
    improvements.push_back("Ensure good air circulation");

    return improvements;
}

bool SleepEnvironmentAnalyzer::detect_environmental_sleep_disruptors() const {
    // Look for patterns where environment might be disrupting sleep

    int high_noise_sessions = 0;
    int extreme_temp_sessions = 0;
    int bright_light_sessions = 0;

    for (const auto& session : sessions) {
        if (session.noise_level > 40) high_noise_sessions++;
        if (session.room_temperature < 16.0 || session.room_temperature > 24.0) extreme_temp_sessions++;
        if (session.light_level > 20) bright_light_sessions++;
    }

    double total_sessions = static_cast<double>(sessions.size());

    return (high_noise_sessions / total_sessions > 0.3) ||
           (extreme_temp_sessions / total_sessions > 0.3) ||
           (bright_light_sessions / total_sessions > 0.3);
}

} // namespace descansa#include "DescansaCoreManager.h"
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

        // Weekend vs weekday analysis
        double weekday_avg = 0, weekend_avg = 0;
        int weekday_count = 0, weekend_count = 0;

        for (const auto& session : recent_sessions) {
            auto time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
            auto tm = *std::localtime(&time_t);

            double duration_hours = session.total_sleep_duration.count() / 3600.0;

            if (tm.tm_wday == 0 || tm.tm_wday == 6) { // Sunday or Saturday
                weekend_avg += duration_hours;
                weekend_count++;
            } else {
                weekday_avg += duration_hours;
                weekday_count++;
            }
        }

        if (weekday_count > 0 && weekend_count > 0) {
            weekday_avg /= weekday_count;
            weekend_avg /= weekend_count;

            if (weekend_avg - weekday_avg > 1.0) {
                patterns.push_back("Significant weekend sleep extension pattern (social jet lag risk)");
            } else if (std::abs(weekend_avg - weekday_avg) < 0.5) {
                patterns.push_back("Consistent weekday/weekend sleep schedule");
            }
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

        // Consistency suggestions
        if (recent_stats.bedtime_variance.count() > 3600) { // More than 1 hour variance
            suggestions.push_back("Try to maintain a more consistent bedtime schedule");
        }

        // Quality-based suggestions
        int poor_quality_days = 0;
        for (const auto& summary : recent_summaries) {
            if (summary.main_sleep.perceived_quality <= SleepQuality::FAIR) {
                poor_quality_days++;
            }
        }

        if (poor_quality_days > 2) {
            suggestions.push_back("Focus on sleep quality - consider factors like room temperature, noise, and comfort");
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

    bool DescansaCoreManager::export_summary_csv(const std::string& export_path) const {
        std::ofstream file(export_path);
        if (!file.is_open()) return false;

        file << "Date,Total Sleep (hours),Sleep Efficiency (%),Sleep Score,Met Goal,Sleep Debt (hours)\n";

        for (const auto& summary : daily_summaries) {
            auto date_time_t = std::chrono::system_clock::to_time_t(summary.date);

            file << std::put_time(std::localtime(&date_time_t), "%Y-%m-%d") << ","
                 << std::fixed << std::setprecision(2) << (summary.total_sleep_time.count() / 3600.0) << ","
                 << std::setprecision(1) << summary.average_sleep