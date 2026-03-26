#pragma once
#include <vector>
#include <map>
#include <string>
#include <utility>
using namespace std;

class Record;

struct CategoryDistItem
{
    string category;
    double amount;
    double percentage;
};

class FinancialAnalyzer
{
public:
    map<string, double> calculateSummary(const vector<Record> &records);

    vector<CategoryDistItem> analyzeDistribution(const vector<Record> &records);

    map<string, double> analyzeTrend(const vector<Record> &records);

    map<string, pair<double, double>> analyzeIncomeExpense(const vector<Record> &records);
};
