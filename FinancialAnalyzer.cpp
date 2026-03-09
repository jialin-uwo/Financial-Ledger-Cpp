#include "FinancialAnalyzer.h"
#include "Record.h"
#include <cmath>

static double roundToTwoDecimals(double value) {
    return std::round(value * 100.0) / 100.0;
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
