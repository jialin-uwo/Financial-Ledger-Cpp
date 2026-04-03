/**
 * @file DataAccess.h
 * @brief Declaration of the DataAccess class and the LoadReport structure.
 *
 * This file defines the data access layer for the ledger system. It provides
 * the declarations needed to load and save financial records and categories
 * using CSV files as persistent storage.
 *
 * The file contains:
 * - the @c LoadReport structure, which stores statistics and detailed error
 *   information for one CSV loading operation, and
 * - the @c DataAccess class, which provides methods for loading and saving
 *   @c Record and @c Category objects.
 *
 * This module supports both internal system files and external user-imported
 * CSV files, and is designed to serve as the persistence boundary between
 * in-memory objects and file-based storage.
 *
 * @author Zhixian Wang
 */

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
 * The LoadReport structure records the outcome of a CSV parsing and loading
 * operation. It tracks how many non-header, non-empty rows were processed,
 * how many rows were successfully converted into valid objects, how many
 * rows failed validation or parsing, and which line numbers correspond to
 * each success or error.
 *
 * Error line numbers are grouped by message in the @c errorsByMessage map,
 * allowing callers to summarize failures by type. Successfully parsed line
 * numbers are stored separately so accepted input rows can be traced back
 * to the original source file.
 *
 * @author Zhixian Wang
 */
struct LoadReport
{
    /**
     * @brief Number of non-header, non-empty rows that were processed.
     */
    int processedRows = 0;

    /**
     * @brief Number of rows that were successfully parsed and accepted.
     */
    int successRows = 0;

    /**
     * @brief Number of rows that failed parsing or validation.
     */
    int errorRows = 0;

    /**
     * @brief Groups original source line numbers by error message.
     *
     * Each key is an error description and each value is a list of line
     * numbers that triggered that error.
     */
    std::map<std::string, std::vector<int>> errorsByMessage;

    /**
     * @brief Stores the original line numbers of successfully parsed rows.
     */
    std::vector<int> successLineNumbers;

    /**
     * @brief Records one processed input row.
     *
     * This method increments the number of processed rows in the report.
     *
     * @return None.
     *
     * @author Zhixian Wang
     */
    void addProcessedRow()
    {
        /// Increase the count of attempted non-header, non-empty rows.
        processedRows++;
    }

    /**
     * @brief Records one successful parse result and its original source line.
     *
     * This method increments the success count and stores the original CSV
     * line number of the accepted row.
     *
     * @param lineNumber The original CSV line number.
     * @return None.
     *
     * @author Zhixian Wang
     */
    void addSuccessLine(int lineNumber)
    {
        /// Increase the count of successful rows.
        successRows++;

        /// Record the original source line number for later reference.
        successLineNumbers.push_back(lineNumber);
    }

    /**
     * @brief Replaces the current success count with a final accepted count.
     *
     * This method directly overwrites the current success total. It can be
     * used when a later stage of processing decides how many successfully
     * parsed rows were ultimately accepted.
     *
     * @param count The final number of successfully accepted records.
     * @return None.
     *
     * @author Zhixian Wang
     */
    void setSuccessCount(int count)
    {
        /// Replace the recorded success count with the supplied value.
        successRows = count;
    }

    /**
     * @brief Adds one error entry into the report.
     *
     * This method increments the error count and records the original line
     * number under the supplied error message.
     *
     * @param message The error message.
     * @param lineNumber The original CSV line number.
     * @return None.
     *
     * @author Zhixian Wang
     */
    void addError(const std::string &message, int lineNumber)
    {
        /// Increase the count of failed rows.
        errorRows++;

        /// Append the source line number to the group for this error message.
        errorsByMessage[message].push_back(lineNumber);
    }

    /**
     * @brief Returns the original CSV line number for one successfully parsed record.
     *
     * This method looks up the stored original line number corresponding to
     * the given success index.
     *
     * @param index The index in the successfully parsed record list.
     * @return The corresponding original CSV line number, or @c 0 if the
     * index is out of range.
     *
     * @author Zhixian Wang
     */
    int getSuccessLineNumber(size_t index) const
    {
        /// Return the stored line number when available; otherwise return 0.
        return index < successLineNumbers.size() ? successLineNumbers[index] : 0;
    }

    /**
     * @brief Checks whether the report contains any successful records.
     *
     * @return True if at least one record succeeded; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool hasSuccesses() const
    {
        /// A report has successes when the success count is greater than zero.
        return successRows > 0;
    }
};

/**
 * @class DataAccess
 * @brief Handles loading and saving ledger data in CSV format.
 *
 * The DataAccess class provides the persistence interface for the ledger
 * system. It is responsible for reading and writing @c Record and
 * @c Category objects to CSV files, supporting both default internal
 * storage files and external user-provided import files.
 *
 * This class separates storage concerns from business logic by encapsulating
 * file paths, parsing expectations, and save/load operations in one module.
 * It supports:
 * - loading categories from the default category file,
 * - loading records with pre-existing IDs from the default record file,
 * - importing records without IDs from external CSV files, and
 * - saving records and categories back to CSV storage.
 *
 * @author Zhixian Wang
 */
class DataAccess
{
private:
    /**
     * @brief Default CSV file path for transaction records.
     *
     * This file is used as the standard persistent storage location for
     * ledger records.
     */
    const std::string RECORD_FILE = "records.csv";

    /**
     * @brief Default CSV file path for categories.
     *
     * This file is used as the standard persistent storage location for
     * ledger categories.
     */
    const std::string CATEGORY_FILE = "categories.csv";

public:
    /**
     * @brief Loads categories from the default category storage file.
     *
     * This method reads category data from the system's default category CSV
     * file and converts valid rows into @c Category objects.
     *
     * Expected CSV format:
     * @code
     * name,isExpense,budget,warningThreshold
     * @endcode
     *
     * If the budget field is empty, it defaults to @c -1.0.
     * If the warning threshold field is empty, it defaults to @c -1.0.
     *
     * This method reads only from the default system file and does not
     * generate a @c LoadReport.
     *
     * @return A list of successfully loaded categories.
     *
     * @author Zhixian Wang
     */
    std::vector<Category> loadCategories();

    /**
     * @brief Loads records from the default record storage file, where each row already contains an ID.
     *
     * This method reads record data from the system's default record CSV
     * file and converts valid rows into @c Record objects.
     *
     * Expected CSV format:
     * @code
     * id,date,amount,isExpense,category
     * @endcode
     *
     * This method reads only from the default system file and does not
     * generate a @c LoadReport.
     *
     * @return A list of successfully loaded records.
     *
     * @author Zhixian Wang
     */
    std::vector<Record> loadRecordsWithId();

    /**
     * @brief Loads records from an external CSV file without an ID column.
     *
     * This method is intended for importing user-provided CSV files whose
     * rows do not contain record IDs. It validates each row and stores
     * row-level success and error information in the supplied @c LoadReport.
     *
     * Expected CSV format:
     * @code
     * date,amount,isExpense,category
     * @endcode
     *
     * @param path The CSV file path to import from.
     * @param report Output report that stores row-level statistics and errors.
     * @return A list of successfully loaded records.
     *
     * @author Zhixian Wang
     */
    std::vector<Record> loadRecordsWithoutId(const std::string &path, LoadReport &report);

    /**
     * @brief Saves records to a CSV file.
     *
     * This method serializes a collection of @c Record objects and writes
     * them to CSV output.
     *
     * Output CSV format:
     * @code
     * id,date,amount,isExpense,category
     * @endcode
     *
     * If @p path is empty, the default record file is used.
     *
     * @param data The list of records to save.
     * @param path Optional custom output path. If empty, the default file is used.
     * @return True if saving succeeds; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool saveRecords(const std::vector<Record> &data, std::string path = "");

    /**
     * @brief Saves categories to a CSV file.
     *
     * This method serializes a collection of @c Category objects and writes
     * them to CSV output.
     *
     * Output CSV format:
     * @code
     * name,isExpense,budget,warningThreshold
     * @endcode
     *
     * If @p path is empty, the default category file is used.
     *
     * @param data The list of categories to save.
     * @param path Optional custom output path. If empty, the default file is used.
     * @return True if saving succeeds; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool saveCategories(const std::vector<Category> &data, std::string path = "");
};

#endif