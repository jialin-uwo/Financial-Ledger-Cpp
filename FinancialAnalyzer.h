#pragma once
#include <vector>
#include <map>
#include <string>
#include <utility>

class Record;

struct CategoryDistItem
{
    std::string category;
    double amount;
    double percentage;
};

class FinancialAnalyzer
{
public:
    std::map<std::string, double> calculateSummary(const std::vector<Record> &records);
    std::vector<CategoryDistItem> analyzeDistribution(const std::vector<Record> &records);
    std::map<std::string, double> analyzeTrend(const std::vector<Record> &records);
    std::map<std::string, std::pair<double, double>> analyzeIncomeExpense(const std::vector<Record> &records);
};