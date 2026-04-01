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
 * This helper function is used to format financial values such as
 * income, expenses, percentages, and balances.
 *
 * @param value The input value to round.
 * @return The value rounded to two decimal places.
 */
static double roundToTwoDecimals(double value)
{
    return std::round(value * 100.0) / 100.0;
}

/**
 * @brief Extracts the year-month portion from a date string.
 *
 * The function assumes the date format begins with "YYYY-MM".
 * If the input string is shorter than 7 characters, "Unknown" is returned.
 *
 * @param date The full date string.
 * @return A string representing the month key in "YYYY-MM" format,
 *         or "Unknown" if the date is invalid.
 */
static std::string extractMonthKey(const std::string &date)
{
    if (date.size() >= 7)
    {
        return date.substr(0, 7);
    }
    return "Unknown";
}

/**
 * @brief Calculates an overall financial summary from a list of records.
 *
 * This function computes:
 * - total income
 * - total expense
 * - net balance
 * - expense totals grouped by category
 * - income totals grouped by category
 *
 * Expense categories are stored with the key format:
 * "category:<category_name>"
 *
 * Income categories are stored with the key format:
 * "income_category:<category_name>"
 *
 * @param records A vector of financial records to analyze.
 * @return A map containing summary statistics and category totals.
 */
std::map<std::string, double> FinancialAnalyzer::calculateSummary(const std::vector<Record> &records)
{
    std::map<std::string, double> summary;

    double totalIncome = 0.0;
    double totalExpense = 0.0;

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

    for (auto &entry : summary)
    {
        entry.second = roundToTwoDecimals(entry.second);
    }

    summary["total_income"] = roundToTwoDecimals(totalIncome);
    summary["total_expense"] = roundToTwoDecimals(totalExpense);
    summary["net_balance"] = roundToTwoDecimals(totalIncome - totalExpense);

    return summary;
}

/**
 * @brief Analyzes the distribution of expenses by category.
 *
 * This function groups all expense records by category and calculates
 * both the total amount and percentage contribution of each category
 * relative to total expenses.
 *
 * @param records A vector of financial records to analyze.
 * @return A vector of CategoryDistItem objects representing the
 *         expense distribution by category.
 */
std::vector<CategoryDistItem> FinancialAnalyzer::analyzeDistribution(const std::vector<Record> &records)
{
    std::map<std::string, double> expenseByCategory;
    double totalExpense = 0.0;

    for (const Record &record : records)
    {
        if (record.getIsExpense())
        {
            expenseByCategory[record.getCategory()] += record.getAmount();
            totalExpense += record.getAmount();
        }
    }

    std::vector<CategoryDistItem> result;

    for (const auto &entry : expenseByCategory)
    {
        CategoryDistItem item;
        item.category = entry.first;
        item.amount = roundToTwoDecimals(entry.second);

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
 * This function groups record amounts by month using the "YYYY-MM"
 * portion of each date. All record amounts are summed together,
 * regardless of whether they are income or expense.
 *
 * @param records A vector of financial records to analyze.
 * @return A map where each key is a month and each value is the
 *         total amount for that month.
 */
std::map<std::string, double> FinancialAnalyzer::analyzeTrend(const std::vector<Record> &records)
{
    std::map<std::string, double> monthlyTotals;

    for (const Record &record : records)
    {
        std::string month = extractMonthKey(record.getDate());
        monthlyTotals[month] += record.getAmount();
    }

    for (auto &entry : monthlyTotals)
    {
        entry.second = roundToTwoDecimals(entry.second);
    }

    return monthlyTotals;
}

/**
 * @brief Separately analyzes monthly income and expense totals.
 *
 * This function groups records by month and stores:
 * - income total in the first element of the pair
 * - expense total in the second element of the pair
 *
 * @param records A vector of financial records to analyze.
 * @return A map where each key is a month and each value is a pair:
 *         (income, expense).
 */
std::map<std::string, std::pair<double, double>> FinancialAnalyzer::analyzeIncomeExpense(const std::vector<Record> &records)
{
    std::map<std::string, std::pair<double, double>> monthlyData;

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

    for (auto &entry : monthlyData)
    {
        entry.second.first = roundToTwoDecimals(entry.second.first);
        entry.second.second = roundToTwoDecimals(entry.second.second);
    }

    return monthlyData;
}