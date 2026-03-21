#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <map>
#include <string>
#include <vector>
#include "Record.h"
#include "Category.h"

using namespace std;

/**
 * @struct LoadReport
 * @brief Stores the summary of one CSV loading operation.
 *
 * processedRows counts all non-header, non-empty rows that were attempted.
 * successRows counts rows successfully converted into objects.
 * errorRows counts rows that failed parsing or validation.
 * errorsByMessage groups line numbers by error message.
 */
struct LoadReport
{
    int processedRows = 0;
    int successRows = 0;
    int errorRows = 0;
    map<string, vector<int>> errorsByMessage;
};

/**
 * @class DataAccess
 * @brief Handles loading and saving ledger data in CSV format.
 *
 * This class supports persistence for both Record objects and Category objects.
 * Records and categories are stored in separate CSV files.
 */
class DataAccess
{
private:
    /**
     * @brief Default CSV file path for transaction records.
     */
    const string RECORD_FILE = "records.csv";

    /**
     * @brief Default CSV file path for categories.
     */
    const string CATEGORY_FILE = "categories.csv";

public:
    /**
     * @brief Loads records from the default storage file, where each row already contains an ID.
     *
     * Expected CSV format:
     * id,date,amount,isExpense,category
     *
     * @param report Output report that stores row-level statistics and errors.
     * @return A list of successfully loaded records.
     */
    vector<Record> loadRecordsWithId(LoadReport &report);

    /**
     * @brief Loads records from an external CSV file without an ID column.
     *
     * Expected CSV format:
     * date,amount,isExpense,category
     *
     * @param path The CSV file path to import from.
     * @param report Output report that stores row-level statistics and errors.
     * @return A list of successfully loaded records.
     */
    vector<Record> loadRecordsWithoutId(const string &path, LoadReport &report);

    /**
     * @brief Saves records to a CSV file.
     *
     * Output CSV format:
     * id,date,amount,isExpense,category
     *
     * @param data The list of records to save.
     * @param path Optional custom output path. If empty, the default file is used.
     * @return True if saving succeeds; otherwise false.
     */
    bool saveRecords(const vector<Record> &data, string path = "");

    /**
     * @brief Loads categories from a CSV file.
     *
     * Expected CSV format:
     * name,isExpense,budget,warningThreshold
     *
     * If budget is empty, it defaults to -0.1.
     * If warningThreshold is empty, it defaults to -1.0.
     *
     * @param report Output report that stores row-level statistics and errors.
     * @param path Optional custom input path. If empty, the default file is used.
     * @return A list of successfully loaded categories.
     */
    vector<Category> loadCategories(LoadReport &report, string path = "");

    /**
     * @brief Saves categories to a CSV file.
     *
     * Output CSV format:
     * name,isExpense,budget,warningThreshold
     *
     * @param data The list of categories to save.
     * @param path Optional custom output path. If empty, the default file is used.
     * @return True if saving succeeds; otherwise false.
     */
    bool saveCategories(const vector<Category> &data, string path = "");
};

#endif