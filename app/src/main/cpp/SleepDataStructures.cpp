#include "SleepDataStructures.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <cmath>
#include <iomanip>

namespace descansa {

// DetailedSleepSession Implementation
    DetailedSleepSession::DetailedSleepSession()
            : total_sleep_duration(0), time_in_bed(0), sleep_efficiency(0.0),
              perceived_quality(SleepQuality::UNKNOWN), awakenings_count(0),
              total_awake_time(0), room_temperature(20.0), noise_level(0),
              light_level(0), light_sleep_duration(0), deep_sleep_duration(0),
              rem_sleep_duration(0), is_nap(false), is_complete(false),
              data_validated(false), created_timestamp(std::chrono::system_clock::now()),
              modified_timestamp(std::chrono::system_clock::now()) {}

    DetailedSleepSession::DetailedSleepSession(TimePoint start, TimePoint end)
            : DetailedSleepSession() {
        sleep_start = start;
        wake_up = end;
        total_sleep_duration = std::chrono::duration_cast<Duration>(end - start);
        time_in_bed = total_sleep_duration;
        is_complete = true;
        sleep_efficiency = calculate_sleep_efficiency();
    }

    double DetailedSleepSession::calculate_sleep_efficiency() const {
        if (time_in_bed.count() <= 0) return 0.0;
        return (total_sleep_duration.count() / time_in_bed.count()) * 100.0;
    }

    Duration DetailedSleepSession::get_sleep_latency() const {
        // Time from getting in bed to actually falling asleep
        // For now, assume immediate sleep - can be enhanced with sensor data
        return Duration(0);
    }

    Duration DetailedSleepSession::get_wake_after_sleep_onset() const {
        return total_awake_time;
    }

    bool DetailedSleepSession::is_sleep_debt() const {
        // Consider 7-9 hours as normal range
        double hours = total_sleep_duration.count() / 3600.0;
        return hours < 7.0;
    }

    std::string DetailedSleepSession::get_quality_description() const {
        switch (perceived_quality) {
            case SleepQuality::POOR: return "Poor";
            case SleepQuality::FAIR: return "Fair";
            case SleepQuality::GOOD: return "Good";
            case SleepQuality::EXCELLENT: return "Excellent";
            default: return "Unknown";
        }
    }

// DailySleepSummary Implementation
    DailySleepSummary::DailySleepSummary()
            : total_sleep_time(0), total_time_in_bed(0), total_awakenings(0),
              average_sleep_efficiency(0.0), daily_steps(0), daily_screen_time_minutes(0),
              stress_level(5), sleep_debt(0), cumulative_sleep_debt(0),
              target_sleep_duration(Duration(8 * 3600)), met_sleep_goal(false) {
        date = std::chrono::system_clock::now();
    }

    DailySleepSummary::DailySleepSummary(const std::chrono::system_clock::time_point& day)
            : DailySleepSummary() {
        date = day;
    }

    void DailySleepSummary::calculate_daily_totals() {
        total_sleep_time = main_sleep.total_sleep_duration;
        total_time_in_bed = main_sleep.time_in_bed;
        total_awakenings = main_sleep.awakenings_count;

        // Add nap data
        for (const auto& nap : naps) {
            total_sleep_time += nap.total_sleep_duration;
            total_time_in_bed += nap.time_in_bed;
            total_awakenings += nap.awakenings_count;
        }

        // Calculate average efficiency
        if (total_time_in_bed.count() > 0) {
            average_sleep_efficiency = (total_sleep_time.count() / total_time_in_bed.count()) * 100.0;
        }

        // Calculate sleep debt
        sleep_debt = target_sleep_duration - total_sleep_time;
        met_sleep_goal = total_sleep_time >= target_sleep_duration;
    }

    double DailySleepSummary::get_sleep_score() const {
        double score = 0.0;

        // Duration score (40% of total)
        double duration_hours = total_sleep_time.count() / 3600.0;
        double duration_score = std::min(100.0, (duration_hours / 8.0) * 100.0);
        score += duration_score * 0.4;

        // Efficiency score (30% of total)
        score += average_sleep_efficiency * 0.3;

        // Quality score (20% of total)
        double quality_score = static_cast<double>(main_sleep.perceived_quality) * 25.0;
        score += quality_score * 0.2;

        // Consistency score (10% of total)
        // This would need historical data to calculate properly
        score += 75.0 * 0.1; // Default decent consistency score

        return std::min(100.0, std::max(0.0, score));
    }

// WeeklySleepPattern Implementation
    WeeklySleepPattern::WeeklySleepPattern()
            : average_sleep_duration(0), average_bedtime_variance(0),
              average_wake_time_variance(0), average_sleep_efficiency(0.0),
              average_sleep_score(0.0), has_consistent_schedule(false),
              weekend_schedule_shift_minutes(0) {
        week_start = std::chrono::system_clock::now();
    }

    WeeklySleepPattern::WeeklySleepPattern(const std::chrono::system_clock::time_point& start)
            : WeeklySleepPattern() {
        week_start = start;
    }

    void WeeklySleepPattern::analyze_patterns() {
        if (daily_summaries.empty()) return;

        // Calculate averages
        Duration total_sleep(0);
        double total_efficiency = 0.0;
        double total_score = 0.0;

        for (const auto& day : daily_summaries) {
            total_sleep += day.total_sleep_time;
            total_efficiency += day.average_sleep_efficiency;
            total_score += day.get_sleep_score();
        }

        size_t count = daily_summaries.size();
        average_sleep_duration = Duration(total_sleep.count() / count);
        average_sleep_efficiency = total_efficiency / count;
        average_sleep_score = total_score / count;

        // Analyze schedule consistency
        has_consistent_schedule = calculate_schedule_consistency() > 0.8;

        // Identify problem days
        problem_days.clear();
        for (size_t i = 0; i < daily_summaries.size(); ++i) {
            if (daily_summaries[i].get_sleep_score() < 60.0) {
                problem_days.push_back(static_cast<int>(i));
            }
        }
    }

    void WeeklySleepPattern::generate_recommendations() {
        recommendations.clear();

        if (average_sleep_duration.count() < 7 * 3600) {
            recommendations.push_back("Consider going to bed earlier to increase sleep duration");
        }

        if (average_sleep_efficiency < 85.0) {
            recommendations.push_back("Improve sleep efficiency by optimizing sleep environment");
        }

        if (!has_consistent_schedule) {
            recommendations.push_back("Try to maintain consistent bedtime and wake time");
        }

        if (problem_days.size() > 2) {
            recommendations.push_back("Identify patterns in poor sleep days and address underlying causes");
        }
    }

    double WeeklySleepPattern::calculate_schedule_consistency() const {
        if (daily_summaries.size() < 2) return 1.0;

        // Calculate variance in bedtimes and wake times
        std::vector<double> bedtimes, wake_times;

        for (const auto& day : daily_summaries) {
            if (day.has_main_sleep()) {
                auto bedtime_t = std::chrono::system_clock::to_time_t(day.main_sleep.sleep_start);
                auto wake_time_t = std::chrono::system_clock::to_time_t(day.main_sleep.wake_up);

                auto bed_tm = *std::localtime(&bedtime_t);
                auto wake_tm = *std::localtime(&wake_time_t);

                double bed_minutes = bed_tm.tm_hour * 60 + bed_tm.tm_min;
                double wake_minutes = wake_tm.tm_hour * 60 + wake_tm.tm_min;

                bedtimes.push_back(bed_minutes);
                wake_times.push_back(wake_minutes);
            }
        }

        if (bedtimes.empty()) return 1.0;

        // Calculate standard deviation
        auto calc_std_dev = [](const std::vector<double>& values) {
            double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
            double sq_sum = 0.0;
            for (double value : values) {
                sq_sum += (value - mean) * (value - mean);
            }
            return std::sqrt(sq_sum / values.size());
        };

        double bed_std_dev = calc_std_dev(bedtimes);
        double wake_std_dev = calc_std_dev(wake_times);

        // Consistency score: lower std dev = higher consistency
        // 30 minutes std dev = 0.5 consistency, 0 minutes = 1.0 consistency
        double bed_consistency = std::max(0.0, 1.0 - (bed_std_dev / 60.0));
        double wake_consistency = std::max(0.0, 1.0 - (wake_std_dev / 60.0));

        return (bed_consistency + wake_consistency) / 2.0;
    }

// SleepEnvironment Implementation
    SleepEnvironment::SleepEnvironment()
            : temperature(20.0), humidity(50.0), noise_level(0), light_level(0),
              screen_time_minutes(0), used_sleep_aid(false) {
        measurement_time = std::chrono::system_clock::now();
    }

    bool SleepEnvironment::is_environment_optimal() const {
        return (temperature >= 18.0 && temperature <= 22.0) &&
               (humidity >= 40.0 && humidity <= 60.0) &&
               (noise_level < 30) &&
               (light_level < 10) &&
               (screen_time_minutes < 60);
    }

    std::vector<std::string> SleepEnvironment::get_environment_recommendations() const {
        std::vector<std::string> recommendations;

        if (temperature < 18.0 || temperature > 22.0) {
            recommendations.push_back("Optimize room temperature (18-22°C ideal)");
        }
        if (noise_level > 30) {
            recommendations.push_back("Reduce noise levels or use white noise");
        }
        if (light_level > 10) {
            recommendations.push_back("Minimize light sources in bedroom");
        }
        if (screen_time_minutes > 60) {
            recommendations.push_back("Reduce screen time before bed");
        }

        return recommendations;
    }

// SleepGoals Implementation
    SleepGoals::SleepGoals()
            : target_sleep_duration(Duration(8 * 3600)),
              preferred_bedtime(std::chrono::hours(22)),
              preferred_wake_time(std::chrono::hours(6)),
              bedtime_tolerance(Duration(30 * 60)),
              wake_time_tolerance(Duration(30 * 60)),
              target_sleep_efficiency(85.0),
              max_acceptable_awakenings(2),
              max_acceptable_sleep_latency(Duration(30 * 60)),
              weekend_schedule_differs(true),
              weekend_sleep_extension(Duration(60 * 60)),
              allow_naps(true),
              max_nap_duration(Duration(30 * 60)),
              latest_nap_time(std::chrono::hours(15)) {}

    bool SleepGoals::is_within_tolerance(const DetailedSleepSession& session) const {
        // Check duration
        Duration duration_diff = session.total_sleep_duration - target_sleep_duration;
        if (std::abs(duration_diff.count()) > bedtime_tolerance.count()) {
            return false;
        }

        // Check efficiency
        if (session.sleep_efficiency < target_sleep_efficiency) {
            return false;
        }

        // Check awakenings
        if (session.awakenings_count > max_acceptable_awakenings) {
            return false;
        }

        return true;
    }

    double SleepGoals::calculate_goal_adherence(const DailySleepSummary& summary) const {
        double score = 0.0;

        // Duration adherence (40%)
        double duration_ratio = summary.total_sleep_time.count() / target_sleep_duration.count();
        double duration_score = std::min(1.0, duration_ratio);
        score += duration_score * 0.4;

        // Efficiency adherence (30%)
        double efficiency_score = std::min(1.0, summary.average_sleep_efficiency / target_sleep_efficiency);
        score += efficiency_score * 0.3;

        // Quality adherence (30%)
        double quality_score = static_cast<double>(summary.main_sleep.perceived_quality) / 4.0;
        score += quality_score * 0.3;

        return score * 100.0;
    }

// SleepStatistics Implementation
    SleepStatistics::SleepStatistics()
            : total_sessions(0), average_sleep_duration(0), median_sleep_duration(0),
              shortest_sleep(Duration::max()), longest_sleep(Duration::zero()),
              sleep_duration_std_dev(0), average_bedtime(std::chrono::hours(0)),
              average_wake_time(std::chrono::hours(0)), bedtime_variance(0),
              wake_time_variance(0), average_sleep_efficiency(0.0),
              average_sleep_score(0.0), most_common_quality(SleepQuality::UNKNOWN),
              total_awakenings(0), sleep_duration_trend(Trend::STABLE),
              sleep_quality_trend(Trend::STABLE), schedule_consistency_trend(Trend::STABLE),
              total_sleep_debt(0), average_daily_sleep_debt(0), days_with_sleep_debt(0) {
        analysis_period_start = std::chrono::system_clock::now();
        analysis_period_end = std::chrono::system_clock::now();
    }

    void SleepStatistics::calculate_from_sessions(const std::vector<DetailedSleepSession>& sessions) {
        if (sessions.empty()) return;

        total_sessions = static_cast<int>(sessions.size());

        // Duration statistics
        std::vector<double> durations;
        Duration total_duration(0);
        double total_efficiency = 0.0;

        for (const auto& session : sessions) {
            double hours = session.total_sleep_duration.count() / 3600.0;
            durations.push_back(hours);
            total_duration += session.total_sleep_duration;
            total_efficiency += session.sleep_efficiency;
            total_awakenings += session.awakenings_count;

            if (session.total_sleep_duration < shortest_sleep) {
                shortest_sleep = session.total_sleep_duration;
            }
            if (session.total_sleep_duration > longest_sleep) {
                longest_sleep = session.total_sleep_duration;
            }
        }

        average_sleep_duration = Duration(total_duration.count() / total_sessions);
        average_sleep_efficiency = total_efficiency / total_sessions;

        // Median calculation
        std::sort(durations.begin(), durations.end());
        if (durations.size() % 2 == 0) {
            median_sleep_duration = Duration((durations[durations.size()/2 - 1] +
                                              durations[durations.size()/2]) * 3600.0 / 2.0);
        } else {
            median_sleep_duration = Duration(durations[durations.size()/2] * 3600.0);
        }

        // Standard deviation
        double mean_hours = average_sleep_duration.count() / 3600.0;
        double variance = 0.0;
        for (double duration : durations) {
            variance += (duration - mean_hours) * (duration - mean_hours);
        }
        sleep_duration_std_dev = Duration(std::sqrt(variance / durations.size()) * 3600.0);
    }

    void SleepStatistics::calculate_trends(const std::vector<DailySleepSummary>& daily_data) {
        if (daily_data.size() < 7) return; // Need at least a week for trend analysis

        // Simple trend calculation: compare first half vs second half
        size_t mid_point = daily_data.size() / 2;

        // Duration trend
        Duration first_half_avg(0), second_half_avg(0);
        for (size_t i = 0; i < mid_point; ++i) {
            first_half_avg += daily_data[i].total_sleep_time;
        }
        for (size_t i = mid_point; i < daily_data.size(); ++i) {
            second_half_avg += daily_data[i].total_sleep_time;
        }

        first_half_avg = Duration(first_half_avg.count() / mid_point);
        second_half_avg = Duration(second_half_avg.count() / (daily_data.size() - mid_point));

        double duration_change = (second_half_avg.count() - first_half_avg.count()) / first_half_avg.count();

        if (duration_change > 0.05) {
            sleep_duration_trend = Trend::IMPROVING;
        } else if (duration_change < -0.05) {
            sleep_duration_trend = Trend::DECLINING;
        } else {
            sleep_duration_trend = Trend::STABLE;
        }
    }

    std::string SleepStatistics::generate_summary_report() const {
        std::ostringstream report;

        report << "Sleep Statistics Summary\n";
        report << "========================\n\n";

        report << "Total Sessions: " << total_sessions << "\n";
        report << "Average Sleep Duration: " << std::fixed << std::setprecision(1)
               << (average_sleep_duration.count() / 3600.0) << " hours\n";
        report << "Average Sleep Efficiency: " << std::setprecision(1)
               << average_sleep_efficiency << "%\n";
        report << "Average Sleep Score: " << std::setprecision(1)
               << average_sleep_score << "/100\n\n";

        report << "Duration Range: " << std::setprecision(1)
               << (shortest_sleep.count() / 3600.0) << " - "
               << (longest_sleep.count() / 3600.0) << " hours\n";

        report << "Total Awakenings: " << total_awakenings << "\n";
        report << "Days with Sleep Debt: " << days_with_sleep_debt << "\n\n";

        std::string trend_desc;
        switch (sleep_duration_trend) {
            case Trend::IMPROVING: trend_desc = "Improving"; break;
            case Trend::DECLINING: trend_desc = "Declining"; break;
            default: trend_desc = "Stable"; break;
        }
        report << "Sleep Duration Trend: " << trend_desc << "\n";

        return report.str();
    }

} // namespace descansa