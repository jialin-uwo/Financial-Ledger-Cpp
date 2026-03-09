#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <string>
#include <vector>
#include "Record.h"

using namespace std;

/**
 * @class DataAccess
 * @brief Handles loading and saving record data from and to CSV files.
 *
 * This class manages file persistence for Record objects.
 * Each record is stored in CSV format with five columns:
 * id, date, amount, isExpense, and category.
 */
class DataAccess {
private:
    /**
     * @brief Default CSV file path for record storage.
     */
    const string RECORD_FILE = "records.csv";

public:
    /**
     * @brief Loads record data from a CSV file.
     *
     * If no path is provided, the default record file is used.
     * Each row is parsed into a Record object with five fields:
     * id, date, amount, isExpense, and category.
     *
     * @param path Optional custom file path.
     * @return A reference to the loaded record list.
     */
    vector<Record> loadRecords(string path = "");

    /**
     * @brief Saves record data to the default CSV file.
     *
     * The file is overwritten and all records are written in
     * five-column CSV format:
     * id, date, amount, isExpense, and category.
     *
     * @param data The list of records to save.
     */
    void saveRecords(const vector<Record>& data);
};

#endif
