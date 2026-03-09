#include "DataAccess.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/**
 * @brief Loads record data from a CSV file into memory.
 *
 * This function reads record data line by line from a CSV file.
 * Each valid row should contain five columns:
 * id, date, amount, isExpense, and category.
 *
 * @param path Optional custom file path. If empty, the default file is used.
 * @return A reference to the static vector containing loaded records.
 */
vector<Record> DataAccess::loadRecords(string path) {
    vector<Record> records;
    records.clear();

    string filePath = path.empty() ? RECORD_FILE : path;
    ifstream file(filePath);

    if (!file.is_open()) {
        cerr << "Failed to open record file: " << filePath << endl;
        return records;
    }

    auto parseRecordLine = [&](const string& row) -> bool {
        stringstream ss(row);
        string token;
        int id = 0;
        string date;
        double amount = 0.0;
        bool isExpense = true;
        string category = "Other";
        string errorMsg;

        try {
            if (!getline(ss, token, ',')) {
                return false;
            }
            id = stoi(token);

            if (!getline(ss, date, ',')) {
                return false;
            }

            if (!getline(ss, token, ',')) {
                return false;
            }
            amount = stod(token);

            if (!getline(ss, token, ',')) {
                return false;
            }
            isExpense = (token == "1" || token == "true" || token == "True");

            if (getline(ss, token, ',')) {
                category = token.empty() ? "Other" : token;
            }

            if (!Record::validateData(date, amount, errorMsg)) {
                cerr << "Invalid record line skipped: " << row
                     << " (" << errorMsg << ")" << endl;
                return false;
            }

            records.emplace_back(id, date, amount, isExpense, category);
            return true;
        } catch (...) {
            return false;
        }
    };

    string line;

    // Read first line and skip header if necessary
    if (getline(file, line)) {
        if (line.find("id") == string::npos && !parseRecordLine(line)) {
            cerr << "Invalid record line skipped: " << line << endl;
        }
    }

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (!parseRecordLine(line)) {
            cerr << "Invalid record line skipped: " << line << endl;
        }
    }

    file.close();
    return records;
}

/**
 * @brief Saves all records to the default CSV file.
 *
 * This function writes all records into a CSV file using five columns:
 * id, date, amount, isExpense, and category.
 *
 * @param data The collection of records to save.
 */
void DataAccess::saveRecords(const vector<Record>& data) {
    ofstream file(RECORD_FILE);

    if (!file.is_open()) {
        cerr << "Failed to write record file: " << RECORD_FILE << endl;
        return;
    }

    file << "id,date,amount,isExpense,category\n";

    for (const auto& record : data) {
        file << record.getId() << ","
             << record.getDate() << ","
             << record.getAmount() << ","
             << (record.getIsExpense() ? 1 : 0) << ","
             << record.getCategory() << "\n";
    }

    file.close();
}
