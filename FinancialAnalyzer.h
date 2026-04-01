#pragma once
#include <vector>
#include <map>
#include <string>
#include <utility>

class Record;

/**
 * @brief Represents the distribution information of a financial category.
 *
 * This structure stores the category name, the total amount for that category,
 * and its percentage contribution to the overall expense distribution.
 */
struct CategoryDistItem
{
    /** @brief The name of the category. */
    std::string category;

    /** @brief The total amount associated with this category. */
    double amount;

    /** @brief The percentage of this category relative to total expenses. */
    double percentage;
};

/**
 * @brief Provides functions for analyzing financial records.
 *
 * This class supports summary calculation, category distribution analysis,
 * monthly trend analysis, and separate monthly income/expense analysis.
 */
class FinancialAnalyzer
{
public:
    /**
     * @brief Calculates a summary of the given financial records.
     *
     * This function computes total income, total expense, net balance,
     * and grouped totals for income and expense categories.
     *
     * @param records A vector of financial records to analyze.
     * @return A map containing summary values and category totals.
     */
    std::map<std::string, double> calculateSummary(const std::vector<Record> &records);

    /**
     * @brief Analyzes the expense distribution by category.
     *
     * This function groups expense records by category and calculates
     * the amount and percentage for each category.
     *
     * @param records A vector of financial records to analyze.
     * @return A vector of CategoryDistItem objects representing
     *         the expense distribution.
     */
    std::vector<CategoryDistItem> analyzeDistribution(const std::vector<Record> &records);

    /**
     * @brief Analyzes monthly financial trends.
     *
     * This function groups record amounts by month and returns
     * the total amount for each month.
     *
     * @param records A vector of financial records to analyze.
     * @return A map where the key is the month and the value is
     *         the total amount for that month.
     */
    std::map<std::string, double> analyzeTrend(const std::vector<Record> &records);

    /**
     * @brief Separately analyzes monthly income and expense totals.
     *
     * This function groups records by month and stores income and
     * expense totals in a pair for each month.
     *
     * The first value in the pair represents income.
     * The second value in the pair represents expense.
     *
     * @param records A vector of financial records to analyze.
     * @return A map where the key is the month and the value is
     *         a pair of (income, expense).
     */
    std::map<std::string, std::pair<double, double>> analyzeIncomeExpense(const std::vector<Record> &records);
};