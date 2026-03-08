#include "DataAccess.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/**
 * @brief Loads record data from a CSV file into memory.
 *
 * This function reads record data line by line from a CSV file.
 * Each valid row should contain six columns:
 * id, date, amount, isExpense, category, and note.
 *
 * @param path Optional custom file path. If empty, the default file is used.
 * @return A reference to the static vector containing loaded records.
 */
vector<Record>& DataAccess::loadRecords(string path) {
    static vector<Record> records;
    records.clear();

    string filePath = path.empty() ? RECORD_FILE : path;
    ifstream file(filePath);

    if (!file.is_open()) {
        cerr << "Failed to open record file: " << filePath << endl;
        return records;
    }

    string line;

    // Read first line and skip header if necessary
    if (getline(file, line)) {
        if (line.find("id") == string::npos) {
            stringstream ss(line);
            string token;
            Record record;

            try {
                getline(ss, token, ',');
                record.id = stoi(token);

                getline(ss, record.date, ',');

                getline(ss, token, ',');
                record.amount = stod(token);

                getline(ss, token, ',');
                record.isExpense = (token == "1" || token == "true" || token == "True");

                getline(ss, record.category, ',');
                getline(ss, record.note);

                records.push_back(record);
            } catch (...) {
                cerr << "Invalid record line skipped: " << line << endl;
            }
        }
    }

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string token;
        Record record;

        try {
            getline(ss, token, ',');
            record.id = stoi(token);

            getline(ss, record.date, ',');

            getline(ss, token, ',');
            record.amount = stod(token);

            getline(ss, token, ',');
            record.isExpense = (token == "1" || token == "true" || token == "True");

            getline(ss, record.category, ',');
            getline(ss, record.note);

            records.push_back(record);
        } catch (...) {
            cerr << "Invalid record line skipped: " << line << endl;
        }
    }

    file.close();
    return records;
}

/**
 * @brief Saves all records to the default CSV file.
 *
 * This function writes all records into a CSV file using six columns:
 * id, date, amount, isExpense, category, and note.
 *
 * @param data The collection of records to save.
 */
void DataAccess::saveRecords(const vector<Record>& data) {
    ofstream file(RECORD_FILE);

    if (!file.is_open()) {
        cerr << "Failed to write record file: " << RECORD_FILE << endl;
        return;
    }

    file << "id,date,amount,isExpense,category,note\n";

    for (const auto& record : data) {
        file << record.id << ","
             << record.date << ","
             << record.amount << ","
             << (record.isExpense ? 1 : 0) << ","
             << record.category << ","
             << record.note << "\n";
    }

    file.close();
}