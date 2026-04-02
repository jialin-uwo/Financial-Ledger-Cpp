/**
 * @file FinancialAnalyzer.h
 * @brief Declaration of the FinancialAnalyzer class and the CategoryDistItem structure.
 *
 * This file defines the analysis layer for the financial ledger system.
 * It declares:
 * - the @c CategoryDistItem structure, which stores expense distribution
 *   information for a category, and
 * - the @c FinancialAnalyzer class, which provides functions for computing
 *   summaries, category distributions, monthly trends, and monthly
 *   income-versus-expense comparisons from financial records.
 *
 * The analysis results declared in this file are intended to support
 * reporting, visualization, and high-level financial insights based on
 * collections of @c Record objects.
 *
 * @author Peiyong Owen Wang
 */

#pragma once
#include <vector>
#include <map>
#include <string>
#include <utility>

class Record;

/**
 * @struct CategoryDistItem
 * @brief Represents distribution information for one financial category.
 *
 * This structure is used to store the result of expense distribution
 * analysis for a single category. It contains the category name, the
 * total expense amount accumulated for that category, and the percentage
 * that this amount contributes relative to the overall expense total.
 *
 * It serves as a lightweight data container for reporting and displaying
 * category-based financial breakdowns.
 *
 * @author Peiyong Owen Wang
 */
struct CategoryDistItem
{
    /**
     * @brief The name of the category.
     *
     * This field identifies which category the distribution item represents.
     */
    std::string category;

    /**
     * @brief The total amount associated with this category.
     *
     * This value stores the accumulated expense amount for the category.
     */
    double amount;

    /**
     * @brief The percentage of this category relative to total expenses.
     *
     * This value indicates what fraction of total expenses belongs to the
     * category, expressed as a percentage.
     */
    double percentage;
};

/**
 * @class FinancialAnalyzer
 * @brief Provides functions for analyzing financial records.
 *
 * The FinancialAnalyzer class offers high-level analytical operations over
 * collections of @c Record objects. It is responsible for computing key
 * financial summaries and grouped statistics that can be used for reporting,
 * budgeting, and insight generation.
 *
 * The class supports:
 * - overall income, expense, and balance summaries,
 * - expense distribution analysis by category,
 * - monthly trend aggregation, and
 * - separate monthly income and expense analysis.
 *
 * The class operates on in-memory record collections and returns aggregated
 * results using standard library containers and small supporting structures.
 *
 * @author Peiyong Owen Wang
 */
class FinancialAnalyzer
{
public:
    /**
     * @brief Calculates a summary of the given financial records.
     *
     * This function computes high-level financial totals from the supplied
     * records, including:
     * - total income,
     * - total expense,
     * - net balance,
     * - grouped totals for expense categories, and
     * - grouped totals for income categories.
     *
     * The returned map contains both overall summary entries and category-
     * specific totals identified by string keys.
     *
     * @param records A vector of financial records to analyze.
     * @return A map containing summary values and category totals.
     *
     * @author Peiyong Owen Wang
     */
    std::map<std::string, double> calculateSummary(const std::vector<Record> &records);

    /**
     * @brief Analyzes the expense distribution by category.
     *
     * This function groups expense records by category and calculates both
     * the total amount and the relative percentage for each category.
     *
     * The returned vector contains one @c CategoryDistItem per category
     * found among the expense records.
     *
     * @param records A vector of financial records to analyze.
     * @return A vector of @c CategoryDistItem objects representing the
     * expense distribution.
     *
     * @author Peiyong Owen Wang
     */
    std::vector<CategoryDistItem> analyzeDistribution(const std::vector<Record> &records);

    /**
     * @brief Analyzes monthly financial trends.
     *
     * This function groups record amounts by month and computes the total
     * amount associated with each month.
     *
     * The returned map uses the month string as the key and the aggregated
     * monthly total as the value.
     *
     * @param records A vector of financial records to analyze.
     * @return A map where the key is the month and the value is the total
     * amount for that month.
     *
     * @author Peiyong Owen Wang
     */
    std::map<std::string, double> analyzeTrend(const std::vector<Record> &records);

    /**
     * @brief Separately analyzes monthly income and expense totals.
     *
     * This function groups records by month and stores the monthly income
     * total and monthly expense total together in a pair.
     *
     * The first value in the pair represents income.
     * The second value in the pair represents expense.
     *
     * @param records A vector of financial records to analyze.
     * @return A map where the key is the month and the value is a pair
     * of @c (income, expense).
     *
     * @author Peiyong Owen Wang
     */
    std::map<std::string, std::pair<double, double>> analyzeIncomeExpense(const std::vector<Record> &records);
};