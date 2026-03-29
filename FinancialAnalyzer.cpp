#include "FinancialAnalyzer.h"
#include "Record.h"
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

static double roundToTwoDecimals(double value)
{
    return std::round(value * 100.0) / 100.0;
}

static std::string extractMonthKey(const std::string &date)
{
    if (date.size() >= 7)
    {
        return date.substr(0, 7);
    }
    return "Unknown";
}

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
