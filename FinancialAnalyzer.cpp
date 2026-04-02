/**
 * @file FinancialAnalyzer.cpp
 * @brief Implementation of the FinancialAnalyzer class and internal analysis helpers.
 *
 * This file contains the implementation of the FinancialAnalyzer module,
 * which provides summary and reporting functions for financial records.
 * It includes helper utilities for rounding values and extracting month
 * keys from date strings, as well as analysis functions for computing:
 * - overall income, expense, and balance summaries,
 * - expense distribution by category,
 * - monthly financial trends, and
 * - monthly income-versus-expense comparisons.
 *
 * The functions in this file operate on collections of Record objects
 * and return aggregated results in standard library containers and
 * lightweight data structures.
 *
 * @author Peiyong Owen Wang
 */

#include "FinancialAnalyzer.h"
#include "Record.h"
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Rounds a numeric value to two decimal places.
 *
 * This helper function is used to normalize financial values such as
 * income totals, expense totals, category percentages, and balances
 * so that analysis results are presented in a standard currency-style
 * format.
 *
 * @param value The input value to round.
 * @return The value rounded to two decimal places.
 *
 * @author Peiyong Owen Wang
 */
static double roundToTwoDecimals(double value)
{
    /// Multiply by 100, round to the nearest integer, and scale back.
    return std::round(value * 100.0) / 100.0;
}

/**
 * @brief Extracts the year-month portion from a date string.
 *
 * This helper function derives a monthly grouping key from a date string
 * by taking its first seven characters, which are expected to match the
 * format @c YYYY-MM. If the input string is too short to contain a valid
 * month key, the function returns @c "Unknown".
 *
 * @param date The full date string.
 * @return A string representing the month key in @c "YYYY-MM" format,
 * or @c "Unknown" if the input is too short.
 *
 * @author Peiyong Owen Wang
 */
static std::string extractMonthKey(const std::string &date)
{
    /// Return the leading YYYY-MM portion when available.
    if (date.size() >= 7)
    {
        return date.substr(0, 7);
    }

    /// Fall back to a placeholder key for invalid or incomplete dates.
    return "Unknown";
}

/**
 * @brief Calculates an overall financial summary from a list of records.
 *
 * This function aggregates the supplied financial records and computes:
 * - total income,
 * - total expense,
 * - net balance,
 * - expense totals grouped by category, and
 * - income totals grouped by category.
 *
 * Expense category totals are stored using keys in the form:
 * @code
 * category:<category_name>
 * @endcode
 *
 * Income category totals are stored using keys in the form:
 * @code
 * income_category:<category_name>
 * @endcode
 *
 * In addition, the map contains the summary keys:
 * - @c total_income
 * - @c total_expense
 * - @c net_balance
 *
 * All returned numeric values are rounded to two decimal places.
 *
 * @param records A vector of financial records to analyze.
 * @return A map containing summary statistics and category totals.
 *
 * @author Peiyong Owen Wang
 */
std::map<std::string, double> FinancialAnalyzer::calculateSummary(const std::vector<Record> &records)
{
    /// Store all computed summary values and grouped totals.
    std::map<std::string, double> summary;

    /// Running totals for income and expenses.
    double totalIncome = 0.0;
    double totalExpense = 0.0;

    /// Traverse all records and accumulate totals by type and category.
    for (const Record &record : records)
    {
        double amount = record.getAmount();

        if (record.getIsExpense())
        {
            totalExpense += amount;
            summary["category:" + record.getCategory()] += amount;
        }
        else
        {
            totalIncome += amount;
            summary["income_category:" + record.getCategory()] += amount;
        }
    }

    /// Round category subtotal values before adding final summary fields.
    for (auto &entry : summary)
    {
        entry.second = roundToTwoDecimals(entry.second);
    }

    /// Store the final top-level summary values.
    summary["total_income"] = roundToTwoDecimals(totalIncome);
    summary["total_expense"] = roundToTwoDecimals(totalExpense);
    summary["net_balance"] = roundToTwoDecimals(totalIncome - totalExpense);

    return summary;
}

/**
 * @brief Analyzes the distribution of expenses by category.
 *
 * This function considers only expense records, groups them by category,
 * and calculates both:
 * - the total expense amount for each category, and
 * - the percentage that each category contributes to total expenses.
 *
 * Percentages are rounded to two decimal places. If there are no expense
 * records, all category percentages default to @c 0.0.
 *
 * @param records A vector of financial records to analyze.
 * @return A vector of CategoryDistItem objects representing the expense
 * distribution by category.
 *
 * @author Peiyong Owen Wang
 */
std::vector<CategoryDistItem> FinancialAnalyzer::analyzeDistribution(const std::vector<Record> &records)
{
    /// Store raw expense totals by category.
    std::map<std::string, double> expenseByCategory;

    /// Running total of all expenses across categories.
    double totalExpense = 0.0;

    /// Accumulate expense values for each category.
    for (const Record &record : records)
    {
        if (record.getIsExpense())
        {
            expenseByCategory[record.getCategory()] += record.getAmount();
            totalExpense += record.getAmount();
        }
    }

    /// Output container for category distribution results.
    std::vector<CategoryDistItem> result;

    /// Convert grouped expense totals into CategoryDistItem objects.
    for (const auto &entry : expenseByCategory)
    {
        CategoryDistItem item;
        item.category = entry.first;
        item.amount = roundToTwoDecimals(entry.second);

        /// Compute the category's percentage of total expenses when possible.
        if (totalExpense > 0.0)
        {
            item.percentage = roundToTwoDecimals(entry.second * 100.0 / totalExpense);
        }
        else
        {
            item.percentage = 0.0;
        }

        result.push_back(item);
    }

    return result;
}

/**
 * @brief Analyzes monthly financial trends based on record dates.
 *
 * This function groups record amounts by month using the @c "YYYY-MM"
 * portion of each record's date. All amounts are summed into a single
 * monthly total regardless of whether the record represents income or
 * expense.
 *
 * The resulting totals are rounded to two decimal places.
 *
 * @param records A vector of financial records to analyze.
 * @return A map where each key is a month and each value is the total
 * amount for that month.
 *
 * @author Peiyong Owen Wang
 */
std::map<std::string, double> FinancialAnalyzer::analyzeTrend(const std::vector<Record> &records)
{
    /// Store cumulative totals for each month.
    std::map<std::string, double> monthlyTotals;

    /// Group record amounts by extracted month key.
    for (const Record &record : records)
    {
        std::string month = extractMonthKey(record.getDate());
        monthlyTotals[month] += record.getAmount();
    }

    /// Round each monthly total before returning the result.
    for (auto &entry : monthlyTotals)
    {
        entry.second = roundToTwoDecimals(entry.second);
    }

    return monthlyTotals;
}

/**
 * @brief Separately analyzes monthly income and expense totals.
 *
 * This function groups all records by month and stores two aggregated values
 * for each month:
 * - the income total in the first element of the pair,
 * - the expense total in the second element of the pair.
 *
 * All returned values are rounded to two decimal places.
 *
 * @param records A vector of financial records to analyze.
 * @return A map where each key is a month and each value is a pair
 * containing @c (income, expense).
 *
 * @author Peiyong Owen Wang
 */
std::map<std::string, std::pair<double, double>> FinancialAnalyzer::analyzeIncomeExpense(const std::vector<Record> &records)
{
    /// Store monthly income and expense totals as pairs.
    std::map<std::string, std::pair<double, double>> monthlyData;

    /// Group records by month and accumulate income and expense separately.
    for (const Record &record : records)
    {
        std::string month = extractMonthKey(record.getDate());

        if (record.getIsExpense())
        {
            monthlyData[month].second += record.getAmount();
        }
        else
        {
            monthlyData[month].first += record.getAmount();
        }
    }

    /// Round each month's income and expense totals.
    for (auto &entry : monthlyData)
    {
        entry.second.first = roundToTwoDecimals(entry.second.first);
        entry.second.second = roundToTwoDecimals(entry.second.second);
    }

    return monthlyData;
}