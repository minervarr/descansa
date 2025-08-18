// SleepAnalyticsEngine.h - Advanced C++11 Sleep Analysis
#ifndef SLEEP_ANALYTICS_ENGINE_H
#define SLEEP_ANALYTICS_ENGINE_H

#include "SleepDataStructures.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>
#include <string>

namespace descansa {

// Advanced statistical analysis for sleep patterns
    class SleepAnalyticsEngine {
    private:
        const std::vector<DetailedSleepSession>& sessions;
        const std::vector<DailySleepSummary>& daily_summaries;

        // Statistical helper methods
        double calculate_mean(const std::vector<double>& values) const;
        double calculate_median(std::vector<double> values) const;
        double calculate_std_deviation(const std::vector<double>& values) const;
        double calculate_correlation(const std::vector<double>& x, const std::vector<double>& y) const;

        // Pattern detection algorithms
        std::vector<int> detect_outliers(const std::vector<double>& values, double threshold = 2.0) const;
        std::vector<double> apply_moving_average(const std::vector<double>& values, int window_size) const;
        bool detect_trend(const std::vector<double>& values, double& slope, double& confidence) const;

        // Sleep cycle analysis
        std::vector<double> estimate_sleep_cycles(const DetailedSleepSession& session) const;
        double calculate_sleep_consistency_score(const std::vector<DailySleepSummary>& summaries) const;

    public:
        SleepAnalyticsEngine(const std::vector<DetailedSleepSession>& session_data,
                             const std::vector<DailySleepSummary>& summary_data);

        // Advanced pattern recognition
        struct SleepPattern {
            std::string pattern_type;
            double confidence_score;
            std::string description;
            std::vector<std::string> recommendations;

            SleepPattern(const std::string& type, double confidence, const std::string& desc)
                    : pattern_type(type), confidence_score(confidence), description(desc) {}
        };

        std::vector<SleepPattern> identify_advanced_patterns() const;
        std::vector<SleepPattern> detect_sleep_disorders() const;
        std::vector<SleepPattern> analyze_chronotype() const;

        // Predictive modeling
        struct SleepPrediction {
            TimePoint predicted_bedtime;
            TimePoint predicted_wake_time;
            Duration predicted_sleep_duration;
            double prediction_confidence;
            std::string reasoning;
        };

        SleepPrediction predict_optimal_sleep_schedule() const;
        SleepPrediction predict_next_sleep_quality() const;

        // Performance optimization
        struct OptimizationSuggestion {
            std::string category;
            std::string specific_action;
            double expected_improvement;
            int priority_level; // 1-5, 5 being highest
            std::string scientific_basis;
        };

        std::vector<OptimizationSuggestion> generate_optimization_plan() const;
        std::vector<OptimizationSuggestion> analyze_environmental_factors() const;
        std::vector<OptimizationSuggestion> optimize_sleep_timing() const;

        // Comparative analysis
        struct BenchmarkComparison {
            std::string metric_name;
            double user_value;
            double population_average;
            double population_percentile;
            std::string interpretation;
        };

        std::vector<BenchmarkComparison> compare_to_population_norms() const;
        BenchmarkComparison analyze_sleep_debt_trend() const;
        BenchmarkComparison analyze_consistency_improvement() const;

        // Advanced statistics
        struct AdvancedMetrics {
            double sleep_variability_index;
            double circadian_rhythm_strength;
            double sleep_efficiency_trend;
            double recovery_capability_score;
            double lifestyle_impact_score;

            std::string generate_interpretation() const;
        };

        AdvancedMetrics calculate_advanced_metrics() const;

        // Machine learning-style insights (simplified for C++11)
        struct InsightCluster {
            std::string insight_category;
            std::vector<std::string> related_factors;
            double impact_magnitude;
            std::string actionable_advice;
        };

        std::vector<InsightCluster> discover_hidden_insights() const;
        std::vector<InsightCluster> correlate_lifestyle_factors() const;

        // Reporting and visualization data
        struct ReportData {
            std::string report_title;
            std::map<std::string, double> key_metrics;
            std::vector<std::string> trend_descriptions;
            std::vector<std::string> actionable_items;
            std::string overall_assessment;
        };

        ReportData generate_comprehensive_report() const;
        ReportData generate_weekly_progress_report() const;
        ReportData generate_health_impact_assessment() const;
    };

// Specialized algorithms for sleep optimization
    namespace sleep_algorithms {

        // Optimal bedtime calculation using multiple factors
        TimePoint calculate_optimal_bedtime(const std::vector<DetailedSleepSession>& sessions,
                                            const SleepGoals& goals,
                                            const std::vector<double>& quality_scores);

        // Sleep debt recovery planning
        struct RecoveryPlan {
            Duration total_debt;
            std::vector<std::pair<TimePoint, Duration>> recommended_adjustments;
            int estimated_recovery_days;
            std::vector<std::string> recovery_strategies;
        };

        RecoveryPlan calculate_optimal_recovery_plan(const std::vector<DailySleepSummary>& summaries,
                                                     const SleepGoals& goals);

        // Circadian rhythm optimization
        struct CircadianOptimization {
            std::chrono::hours optimal_light_exposure_time;
            std::chrono::hours optimal_meal_cutoff;
            std::chrono::hours optimal_exercise_window;
            std::chrono::hours optimal_caffeine_cutoff;
            std::vector<std::string> phase_shift_recommendations;
        };

        CircadianOptimization optimize_circadian_rhythm(const std::vector<DetailedSleepSession>& sessions);

        // Environmental optimization
        struct EnvironmentalOptimization {
            double optimal_temperature_range_min;
            double optimal_temperature_range_max;
            int max_acceptable_noise_level;
            int max_acceptable_light_level;
            std::vector<std::string> environmental_improvements;
        };

        EnvironmentalOptimization analyze_optimal_environment(const std::vector<DetailedSleepSession>& sessions);

        // Sleep efficiency maximization
        struct EfficiencyOptimization {
            Duration recommended_time_in_bed_adjustment;
            std::vector<std::string> efficiency_improvement_tactics;
            double target_efficiency_achievable;
            int estimated_improvement_weeks;
        };

        EfficiencyOptimization optimize_sleep_efficiency(const std::vector<DetailedSleepSession>& sessions,
                                                         const SleepGoals& goals);

        // Advanced trend analysis
        enum class TrendDirection { IMPROVING, STABLE, DECLINING, VOLATILE };

        struct TrendAnalysis {
            TrendDirection direction;
            double trend_strength; // 0.0 to 1.0
            double volatility_index;
            std::vector<TimePoint> significant_change_points;
            std::string trend_interpretation;
        };

        TrendAnalysis analyze_sleep_quality_trend(const std::vector<DailySleepSummary>& summaries,
                                                  int analysis_window_days = 30);

        TrendAnalysis analyze_duration_consistency_trend(const std::vector<DetailedSleepSession>& sessions,
                                                         int analysis_window_days = 30);

        // Predictive sleep quality modeling
        struct QualityPrediction {
            SleepQuality predicted_quality;
            double confidence_interval;
            std::vector<std::string> influencing_factors;
            std::vector<std::string> mitigation_strategies;
        };

        QualityPrediction predict_sleep_quality(const DetailedSleepSession& upcoming_session_context,
                                                const std::vector<DetailedSleepSession>& historical_sessions);

        // Comprehensive sleep score calculation
        struct ComprehensiveSleepScore {
            double overall_score; // 0-100
            double duration_component;
            double quality_component;
            double consistency_component;
            double efficiency_component;
            double recovery_component;
            std::string grade_letter; // A+ to F
            std::string detailed_breakdown;
        };

        ComprehensiveSleepScore calculate_comprehensive_score(const std::vector<DailySleepSummary>& summaries,
                                                              const SleepGoals& goals,
                                                              int evaluation_period_days = 30);
    }

} // namespace descansa

// Implementation file would be SleepAnalyticsEngine.cpp
// Due to length constraints, showing key method implementations:

namespace descansa {

// Key statistical helper implementations
    double SleepAnalyticsEngine::calculate_mean(const std::vector<double>& values) const {
        if (values.empty()) return 0.0;
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    double SleepAnalyticsEngine::calculate_median(std::vector<double> values) const {
        if (values.empty()) return 0.0;

        std::sort(values.begin(), values.end());
        size_t size = values.size();

        if (size % 2 == 0) {
            return (values[size/2 - 1] + values[size/2]) / 2.0;
        } else {
            return values[size/2];
        }
    }

    double SleepAnalyticsEngine::calculate_std_deviation(const std::vector<double>& values) const {
        if (values.size() < 2) return 0.0;

        double mean = calculate_mean(values);
        double variance = 0.0;

        for (double value : values) {
            variance += (value - mean) * (value - mean);
        }

        variance /= (values.size() - 1);
        return std::sqrt(variance);
    }

    double SleepAnalyticsEngine::calculate_correlation(const std::vector<double>& x,
                                                       const std::vector<double>& y) const {
        if (x.size() != y.size() || x.size() < 2) return 0.0;

        double mean_x = calculate_mean(x);
        double mean_y = calculate_mean(y);

        double numerator = 0.0;
        double sum_sq_x = 0.0;
        double sum_sq_y = 0.0;

        for (size_t i = 0; i < x.size(); ++i) {
            double dx = x[i] - mean_x;
            double dy = y[i] - mean_y;
            numerator += dx * dy;
            sum_sq_x += dx * dx;
            sum_sq_y += dy * dy;
        }

        double denominator = std::sqrt(sum_sq_x * sum_sq_y);
        return (denominator > 0.0) ? (numerator / denominator) : 0.0;
    }

// Advanced pattern recognition implementation
    std::vector<SleepAnalyticsEngine::SleepPattern> SleepAnalyticsEngine::identify_advanced_patterns() const {
        std::vector<SleepPattern> patterns;

        if (sessions.size() < 7) {
            patterns.emplace_back("insufficient_data", 0.9, "Need at least 7 days of data for pattern analysis");
            return patterns;
        }

        // Extract sleep durations for analysis
        std::vector<double> durations;
        std::vector<double> bedtimes;
        std::vector<double> wake_times;

        for (const auto& session : sessions) {
            if (session.is_complete && !session.is_nap) {
                durations.push_back(session.total_sleep_duration.count() / 3600.0);

                auto bedtime_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto wake_t = std::chrono::system_clock::to_time_t(session.wake_up);
                auto bed_tm = *std::localtime(&bedtime_t);
                auto wake_tm = *std::localtime(&wake_t);

                bedtimes.push_back(bed_tm.tm_hour + bed_tm.tm_min / 60.0);
                wake_times.push_back(wake_tm.tm_hour + wake_tm.tm_min / 60.0);
            }
        }

        if (durations.empty()) return patterns;

        // Pattern 1: Sleep duration consistency
        double duration_std = calculate_std_deviation(durations);
        if (duration_std < 0.5) {
            patterns.emplace_back("highly_consistent_duration", 0.95,
                                  "Very consistent sleep duration - excellent sleep hygiene");
        } else if (duration_std > 2.0) {
            patterns.emplace_back("erratic_sleep_duration", 0.90,
                                  "Highly variable sleep duration - consider establishing consistent bedtime");
        }

        // Pattern 2: Bedtime consistency
        double bedtime_std = calculate_std_deviation(bedtimes);
        if (bedtime_std < 0.5) {
            patterns.emplace_back("consistent_bedtime", 0.92,
                                  "Excellent bedtime consistency - strong circadian rhythm support");
        } else if (bedtime_std > 2.0) {
            patterns.emplace_back("irregular_bedtime", 0.88,
                                  "Irregular bedtime pattern detected - may impact sleep quality");
        }

        // Pattern 3: Weekend effect detection
        if (sessions.size() >= 14) {
            std::vector<double> weekday_durations, weekend_durations;

            for (size_t i = 0; i < std::min(sessions.size(), size_t(14)); ++i) {
                const auto& session = sessions[sessions.size() - 1 - i];
                if (!session.is_complete || session.is_nap) continue;

                auto time_t = std::chrono::system_clock::to_time_t(session.wake_up);
                auto tm = *std::localtime(&time_t);

                double duration_hours = session.total_sleep_duration.count() / 3600.0;

                if (tm.tm_wday == 0 || tm.tm_wday == 6) { // Sunday or Saturday
                    weekend_durations.push_back(duration_hours);
                } else {
                    weekday_durations.push_back(duration_hours);
                }
            }

            if (!weekday_durations.empty() && !weekend_durations.empty()) {
                double weekday_avg = calculate_mean(weekday_durations);
                double weekend_avg = calculate_mean(weekend_durations);
                double difference = weekend_avg - weekday_avg;

                if (difference > 1.0) {
                    patterns.emplace_back("weekend_oversleep", 0.85,
                                          "Significant weekend sleep extension detected - social jetlag risk");
                } else if (difference < -0.5) {
                    patterns.emplace_back("weekend_sleep_loss", 0.80,
                                          "Weekend sleep reduction detected - lifestyle impact");
                }
            }
        }

        // Pattern 4: Sleep debt accumulation
        double avg_duration = calculate_mean(durations);
        if (avg_duration < 7.0) {
            double debt_severity = (7.0 - avg_duration) / 7.0;
            patterns.emplace_back("chronic_sleep_restriction", debt_severity,
                                  "Chronic sleep restriction pattern - health implications");
        }

        return patterns;
    }

// Optimization suggestion generation
    std::vector<SleepAnalyticsEngine::OptimizationSuggestion> SleepAnalyticsEngine::generate_optimization_plan() const {
        std::vector<OptimizationSuggestion> suggestions;

        if (sessions.empty()) return suggestions;

        // Analyze recent sleep efficiency
        std::vector<double> recent_efficiency;
        for (size_t i = 0; i < std::min(sessions.size(), size_t(7)); ++i) {
            const auto& session = sessions[sessions.size() - 1 - i];
            if (session.is_complete && !session.is_nap) {
                recent_efficiency.push_back(session.sleep_efficiency);
            }
        }

        if (!recent_efficiency.empty()) {
            double avg_efficiency = calculate_mean(recent_efficiency);

            if (avg_efficiency < 85.0) {
                OptimizationSuggestion suggestion;
                suggestion.category = "Sleep Efficiency";
                suggestion.specific_action = "Implement sleep restriction therapy - limit time in bed to actual sleep time";
                suggestion.expected_improvement = (85.0 - avg_efficiency) * 0.5;
                suggestion.priority_level = 4;
                suggestion.scientific_basis = "Sleep restriction therapy increases sleep pressure and consolidates sleep";
                suggestions.push_back(suggestion);
            }

            if (avg_efficiency < 90.0) {
                OptimizationSuggestion suggestion;
                suggestion.category = "Sleep Hygiene";
                suggestion.specific_action = "Establish 30-minute wind-down routine before bed";
                suggestion.expected_improvement = 3.0;
                suggestion.priority_level = 3;
                suggestion.scientific_basis = "Consistent pre-sleep routines enhance sleep onset and quality";
                suggestions.push_back(suggestion);
            }
        }

        // Analyze sleep timing consistency
        std::vector<double> bedtimes;
        for (const auto& session : sessions) {
            if (session.is_complete && !session.is_nap) {
                auto time_t = std::chrono::system_clock::to_time_t(session.sleep_start);
                auto tm = *std::localtime(&time_t);
                bedtimes.push_back(tm.tm_hour + tm.tm_min / 60.0);
            }
        }

        if (!bedtimes.empty()) {
            double bedtime_std = calculate_std_deviation(bedtimes);

            if (bedtime_std > 1.0) {
                OptimizationSuggestion suggestion;
                suggestion.category = "Circadian Rhythm";
                suggestion.specific_action = "Establish consistent bedtime within 30-minute window";
                suggestion.expected_improvement = 5.0;
                suggestion.priority_level = 5;
                suggestion.scientific_basis = "Consistent sleep timing strengthens circadian rhythms and improves sleep quality";
                suggestions.push_back(suggestion);
            }
        }

        // Sort by priority level (highest first)
        std::sort(suggestions.begin(), suggestions.end(),
                  [](const OptimizationSuggestion& a, const OptimizationSuggestion& b) {
                      return a.priority_level > b.priority_level;
                  });

        return suggestions;
    }

} // namespace descansa

#endif // SLEEP_ANALYTICS_ENGINE_H