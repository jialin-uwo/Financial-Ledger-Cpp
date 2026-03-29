#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <map>
#include <string>
#include <vector>
#include "Record.h"
#include "Category.h"

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
    std::map<std::string, std::vector<int>> errorsByMessage;
    std::vector<int> successLineNumbers;

    /** @brief Records one processed input row. */
    void addProcessedRow()
    {
        processedRows++;
    }

    /**
     * @brief Records one successful parse result and its original source line.
     * @param lineNumber The original CSV line number.
     */
    void addSuccessLine(int lineNumber)
    {
        successRows++;
        successLineNumbers.push_back(lineNumber);
    }

    /**
     * @brief Replaces the current success count with a final accepted count.
     * @param count The final number of successfully accepted records.
     */
    void setSuccessCount(int count)
    {
        successRows = count;
    }

    /**
     * @brief Adds one error entry into the report.
     * @param message The error message.
     * @param lineNumber The original CSV line number.
     */
    void addError(const std::string &message, int lineNumber)
    {
        errorRows++;
        errorsByMessage[message].push_back(lineNumber);
    }

    /**
     * @brief Returns the original CSV line number for one successfully parsed record.
     * @param index The index in the successfully parsed record list.
     * @return The corresponding original CSV line number, or 0 if unavailable.
     */
    int getSuccessLineNumber(size_t index) const
    {
        return index < successLineNumbers.size() ? successLineNumbers[index] : 0;
    }

    /**
     * @brief Checks whether the report contains any successful records.
     * @return True if at least one record succeeded; otherwise false.
     */
    bool hasSuccesses() const
    {
        return successRows > 0;
    }
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
    const std::string RECORD_FILE = "records.csv";

    /**
     * @brief Default CSV file path for categories.
     */
    const std::string CATEGORY_FILE = "categories.csv";

public:
    /**
     * @brief Loads categories from the default category storage file.
     *
     * Expected CSV format:
     * name,isExpense,budget,warningThreshold
     *
     * If budget is empty, it defaults to -0.1.
     * If warningThreshold is empty, it defaults to -1.0.
     *
     * This method reads from the system file only and does not generate a load report.
     *
     * @return A list of successfully loaded categories.
     */
    std::vector<Category> loadCategories();

    /**
     * @brief Loads records from the default record storage file, where each row already contains an ID.
     *
     * Expected CSV format:
     * id,date,amount,isExpense,category
     *
     * This method reads from the system file only and does not generate a load report.
     *
     * @return A list of successfully loaded records.
     */
    std::vector<Record> loadRecordsWithId();

    /**
     * @brief Loads records from an external CSV file without an ID column.
     *
     * Expected CSV format:
     * date,amount,isExpense,category
     *
     * This method is intended for user-uploaded files. It validates each row and
     * records detailed error information in the provided report.
     *
     * @param path The CSV file path to import from.
     * @param report Output report that stores row-level statistics and errors.
     * @return A list of successfully loaded records.
     */
    std::vector<Record> loadRecordsWithoutId(const std::string &path, LoadReport &report);

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
    bool saveRecords(const std::vector<Record> &data, std::string path = "");

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
    bool saveCategories(const std::vector<Category> &data, std::string path = "");
};

#endif