#pragma once
#include <vector>
#include <map>
#include <string>
using namespace std;

class Record;

class FinancialAnalyzer {
public:
    map<string, double> calculateSummary(const vector<Record>& records);
};
