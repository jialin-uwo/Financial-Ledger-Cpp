#include "FinancialAnalyzer.h"
#include "Record.h"
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

static double roundToTwoDecimals(double value) {
    return std::round(value * 100.0) / 100.0;
}

static string extractMonthKey(const string& date) {
    if (date.size() >= 7) {
        return date.substr(0, 7);
    }
    return "Unknown";
}

map<string, double> FinancialAnalyzer::calculateSummary(const vector<Record>& records) {
    map<string, double> summary;

    double totalIncome = 0.0;
    double totalExpense = 0.0;

    for (const Record& record : records) {
        double amount = record.getAmount();

        if (record.getIsExpense()) {
            totalExpense += amount;
            summary["category:" + record.getCategory()] += amount;
        } else {
            totalIncome += amount;
        }
    }

    for (auto& entry : summary) {
        entry.second = roundToTwoDecimals(entry.second);
    }

    summary["total_income"] = roundToTwoDecimals(totalIncome);
    summary["total_expense"] = roundToTwoDecimals(totalExpense);
    summary["net_balance"] = roundToTwoDecimals(totalIncome - totalExpense);

    return summary;
}

vector<CategoryDistItem> FinancialAnalyzer::analyzeDistribution(const vector<Record>& records) {
    map<string, double> expenseByCategory;
    double totalExpense = 0.0;

    for (const Record& record : records) {
        if (record.getIsExpense()) {
            expenseByCategory[record.getCategory()] += record.getAmount();
            totalExpense += record.getAmount();
        }
    }

    vector<CategoryDistItem> result;

    for (const auto& entry : expenseByCategory) {
        CategoryDistItem item;
        item.category = entry.first;
        item.amount = roundToTwoDecimals(entry.second);

        if (totalExpense > 0.0) {
            item.percentage = roundToTwoDecimals(entry.second * 100.0 / totalExpense);
        } else {
            item.percentage = 0.0;
        }

        result.push_back(item);
    }

    return result;
}

map<string, double> FinancialAnalyzer::analyzeTrend(const vector<Record>& records) {
    map<string, double> monthlyNet;

    for (const Record& record : records) {
        string month = extractMonthKey(record.getDate());

        if (record.getIsExpense()) {
            monthlyNet[month] -= record.getAmount();
        } else {
            monthlyNet[month] += record.getAmount();
        }
    }

    for (auto& entry : monthlyNet) {
        entry.second = roundToTwoDecimals(entry.second);
    }

    return monthlyNet;
}

map<string, pair<double, double>> FinancialAnalyzer::analyzeIncomeExpense(const vector<Record>& records) {
    map<string, pair<double, double>> monthlyData;

    for (const Record& record : records) {
        string month = extractMonthKey(record.getDate());

        if (record.getIsExpense()) {
            monthlyData[month].second += record.getAmount();
        } else {
            monthlyData[month].first += record.getAmount();
        }
    }

    for (auto& entry : monthlyData) {
        entry.second.first = roundToTwoDecimals(entry.second.first);
        entry.second.second = roundToTwoDecimals(entry.second.second);
    }

    return monthlyData;
}
