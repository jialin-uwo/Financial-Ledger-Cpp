/**
 * @file DataAccess.cpp
 * @brief Implementation of the DataAccess class and internal CSV parsing helpers.
 *
 * This file contains the implementation of the DataAccess module, which is
 * responsible for loading and saving financial records and categories in CSV
 * format. It provides the persistence layer for the ledger system and includes
 * helper utilities for string normalization, header detection, token parsing,
 * row parsing, and file serialization.
 *
 * The implementation supports:
 * - loading categories from the default category file,
 * - loading records from the default record file when IDs are already present,
 * - importing records from an external CSV file that does not contain IDs,
 * - saving records and categories back to CSV files safely using a temporary
 *   file replacement strategy, and
 * - skipping malformed rows while preserving successfully parsed data.
 *
 * Anonymous-namespace helper functions are used to keep parsing utilities
 * local to this translation unit.
 *
 * @author Zhixian Wang
 */

#include "DataAccess.h"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace
{
    /**
     * @brief Removes leading and trailing whitespace from a string.
     *
     * This helper function normalizes a string by removing whitespace
     * characters from its beginning and end. It is used throughout the CSV
     * parsing logic to clean tokens before validation or conversion.
     *
     * @param value Input string.
     * @return A trimmed copy of the input string.
     *
     * @author Zhixian Wang
     */
    string trim(const string &value)
    {
        /// Locate the first non-whitespace character.
        size_t start = value.find_first_not_of(" \t\n\r");
        if (start == string::npos)
        {
            /// Return an empty string if the input is entirely whitespace.
            return "";
        }

        /// Locate the last non-whitespace character.
        size_t end = value.find_last_not_of(" \t\n\r");

        /// Return the substring bounded by the non-whitespace range.
        return value.substr(start, end - start + 1);
    }

    /**
     * @brief Converts a string to lowercase.
     *
     * This helper function returns a lowercase copy of the supplied string.
     * It is used to perform case-insensitive comparisons when parsing CSV
     * tokens such as boolean flags and header labels.
     *
     * @param value Input string.
     * @return A lowercase copy of the input string.
     *
     * @author Zhixian Wang
     */
    string toLowerCopy(const string &value)
    {
        /// Start with a copy so the original string remains unchanged.
        string result = value;

        /// Convert each character to lowercase safely.
        for (char &ch : result)
        {
            ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        }

        return result;
    }

    /**
     * @brief Checks whether a line is empty or contains only whitespace.
     *
     * This helper function is used when reading CSV files to ignore blank
     * lines that do not carry meaningful data.
     *
     * @param line Input line.
     * @return True if the line is blank; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool isBlankLine(const string &line)
    {
        /// A line is considered blank if its trimmed form is empty.
        return trim(line).empty();
    }

    /**
     * @brief Parses an isExpense token into a boolean value.
     *
     * This helper function interprets the textual value of an @c isExpense
     * CSV field and converts it into a boolean. The comparison is
     * case-insensitive after trimming surrounding whitespace.
     *
     * Accepted true values:
     * - @c 1
     * - @c true
     *
     * Accepted false values:
     * - @c 0
     * - @c false
     *
     * @param token Input token.
     * @param isExpense Output parameter that receives the parsed boolean value.
     * @return True if parsing succeeds; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool parseIsExpenseToken(const string &token, bool &isExpense)
    {
        /// Normalize the token before comparison.
        string normalized = toLowerCopy(trim(token));

        /// Recognize supported true literals.
        if (normalized == "1" || normalized == "true")
        {
            isExpense = true;
            return true;
        }

        /// Recognize supported false literals.
        if (normalized == "0" || normalized == "false")
        {
            isExpense = false;
            return true;
        }

        /// Reject unsupported token values.
        return false;
    }

    /**
     * @brief Checks whether a line appears to be a record header.
     *
     * This helper function examines the first column of a CSV row to decide
     * whether the row looks like a record header rather than actual data.
     * The expected first header token depends on whether the format includes
     * an ID column.
     *
     * @param line Input line.
     * @param hasId True if the record format includes an ID column; false if
     * the record format begins with the date column.
     * @return True if the line appears to be a record header; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool isRecordHeaderLine(const string &line, bool hasId)
    {
        /// Tokenize the input line as CSV.
        stringstream ss(line);
        string firstToken;

        /// If the line has no first token, it cannot be a valid header.
        if (!getline(ss, firstToken, ','))
        {
            return false;
        }

        /// Normalize the first token for case-insensitive comparison.
        firstToken = toLowerCopy(trim(firstToken));

        /// Match the expected leading column name for the chosen format.
        if (hasId)
        {
            return firstToken == "id";
        }

        return firstToken == "date";
    }

    /**
     * @brief Checks whether a line appears to be a category header.
     *
     * This helper function examines the first column of a category CSV row
     * and determines whether it matches the expected header token.
     *
     * @param line Input line.
     * @return True if the line appears to be a category header; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool isCategoryHeaderLine(const string &line)
    {
        /// Tokenize the line and inspect the first column.
        stringstream ss(line);
        string firstToken;

        /// Reject lines that do not contain even one token.
        if (!getline(ss, firstToken, ','))
        {
            return false;
        }

        /// Normalize the token before testing against the expected header.
        firstToken = toLowerCopy(trim(firstToken));
        return firstToken == "name";
    }

    /**
     * @brief Parses a record row that contains an ID column.
     *
     * This helper function parses one CSV row representing a record in the
     * format:
     * @code
     * id,date,amount,isExpense,category
     * @endcode
     *
     * The function validates the presence and format of each required field,
     * converts the ID and amount to numeric values, parses the boolean type
     * token, and validates the resulting record data using
     * @c Record::validateData.
     *
     * @param line Input CSV row.
     * @param id Output parameter that receives the parsed record ID.
     * @param date Output parameter that receives the parsed date string.
     * @param amount Output parameter that receives the parsed amount.
     * @param isExpense Output parameter that receives the parsed transaction type.
     * @param category Output parameter that receives the parsed category name.
     * @param errorMsg Output parameter that receives an explanatory error message
     * if parsing fails.
     * @return True if parsing succeeds; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool parseRecordRowWithId(const string &line,
                              int &id,
                              string &date,
                              double &amount,
                              bool &isExpense,
                              string &category,
                              string &errorMsg)
    {
        /// Use a string stream to parse CSV columns sequentially.
        stringstream ss(line);
        string token;

        /// Read and validate the ID column.
        if (!getline(ss, token, ','))
        {
            errorMsg = "Missing ID column.";
            return false;
        }

        token = trim(token);
        if (token.empty())
        {
            errorMsg = "Missing ID column.";
            return false;
        }

        /// Convert the ID token to an integer.
        try
        {
            id = stoi(token);
        }
        catch (...)
        {
            errorMsg = "Invalid ID.";
            return false;
        }

        /// Read and normalize the date column.
        if (!getline(ss, date, ','))
        {
            errorMsg = "Missing date column.";
            return false;
        }
        date = trim(date);

        /// Read and validate the amount column.
        if (!getline(ss, token, ','))
        {
            errorMsg = "Missing amount column.";
            return false;
        }

        token = trim(token);
        if (token.empty())
        {
            errorMsg = "Missing amount column.";
            return false;
        }

        /// Convert the amount token to a floating-point value.
        try
        {
            amount = stod(token);
        }
        catch (...)
        {
            errorMsg = "Invalid amount.";
            return false;
        }

        /// Read and parse the isExpense column.
        if (!getline(ss, token, ','))
        {
            errorMsg = "Missing isExpense column.";
            return false;
        }

        if (!parseIsExpenseToken(token, isExpense))
        {
            errorMsg = "Invalid isExpense value.";
            return false;
        }

        /// Read the optional category field if present.
        if (getline(ss, category))
        {
            category = trim(category);
        }
        else
        {
            category = "";
        }

        /// Validate the parsed date and amount values using Record rules.
        if (!Record::validateData(date, amount, errorMsg))
        {
            return false;
        }

        return true;
    }

    /**
     * @brief Parses a record row that does not contain an ID column.
     *
     * This helper function parses one CSV row representing a record in the
     * format:
     * @code
     * date,amount,isExpense,category
     * @endcode
     *
     * This form is intended for imported records whose IDs will be assigned
     * later by outer logic.
     *
     * @param line Input CSV row.
     * @param date Output parameter that receives the parsed date string.
     * @param amount Output parameter that receives the parsed amount.
     * @param isExpense Output parameter that receives the parsed transaction type.
     * @param category Output parameter that receives the parsed category name.
     * @param errorMsg Output parameter that receives an explanatory error message
     * if parsing fails.
     * @return True if parsing succeeds; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool parseRecordRowWithoutId(const string &line,
                                 string &date,
                                 double &amount,
                                 bool &isExpense,
                                 string &category,
                                 string &errorMsg)
    {
        /// Use a string stream to parse CSV columns in order.
        stringstream ss(line);
        string token;

        /// Read and normalize the date column.
        if (!getline(ss, date, ','))
        {
            errorMsg = "Missing date column.";
            return false;
        }
        date = trim(date);

        /// Read and validate the amount column.
        if (!getline(ss, token, ','))
        {
            errorMsg = "Missing amount column.";
            return false;
        }

        token = trim(token);
        if (token.empty())
        {
            errorMsg = "Missing amount column.";
            return false;
        }

        /// Convert the amount token to a floating-point value.
        try
        {
            amount = stod(token);
        }
        catch (...)
        {
            errorMsg = "Invalid amount.";
            return false;
        }

        /// Read and parse the isExpense column.
        if (!getline(ss, token, ','))
        {
            errorMsg = "Missing isExpense column.";
            return false;
        }

        if (!parseIsExpenseToken(token, isExpense))
        {
            errorMsg = "Invalid isExpense value.";
            return false;
        }

        /// Read the optional category field if present.
        if (getline(ss, category))
        {
            category = trim(category);
        }
        else
        {
            category = "";
        }

        /// Validate the parsed date and amount values using Record rules.
        if (!Record::validateData(date, amount, errorMsg))
        {
            return false;
        }

        return true;
    }

    /**
     * @brief Parses one category row.
     *
     * This helper function parses a CSV row representing a category in the
     * format:
     * @code
     * name,isExpense,budget,warningThreshold
     * @endcode
     *
     * Empty optional numeric fields are interpreted using the module's
     * default placeholder conventions:
     * - @c budget defaults to @c -1.0 when omitted,
     * - @c warningThreshold defaults to @c -1.0 when omitted.
     *
     * The parsed values are returned through output parameters. Validation
     * of the final category object is handled later by the @c Category
     * constructor.
     *
     * @param line Input CSV row.
     * @param name Output parameter that receives the category name.
     * @param isExpense Output parameter that receives the category type flag.
     * @param budget Output parameter that receives the parsed budget value.
     * @param warningThreshold Output parameter that receives the parsed
     * warning threshold value.
     * @param errorMsg Output parameter that receives an explanatory error
     * message if parsing fails.
     * @return True if parsing succeeds; otherwise false.
     *
     * @author Zhixian Wang
     */
    bool parseCategoryRow(const string &line,
                          string &name,
                          bool &isExpense,
                          double &budget,
                          double &warningThreshold,
                          string &errorMsg)
    {
        /// Use a string stream to parse category CSV columns sequentially.
        stringstream ss(line);
        string token;

        /// Read and validate the category name.
        if (!getline(ss, name, ','))
        {
            errorMsg = "Missing category name.";
            return false;
        }
        name = trim(name);

        if (name.empty())
        {
            errorMsg = "Missing category name.";
            return false;
        }

        /// Read and parse the isExpense column.
        if (!getline(ss, token, ','))
        {
            errorMsg = "Missing isExpense column.";
            return false;
        }

        if (!parseIsExpenseToken(token, isExpense))
        {
            errorMsg = "Invalid isExpense value.";
            return false;
        }

        /// Initialize the budget to the module's placeholder default.
        budget = -1.0;
        if (getline(ss, token, ','))
        {
            token = trim(token);
            if (!token.empty())
            {
                try
                {
                    budget = stod(token);
                }
                catch (...)
                {
                    errorMsg = "Invalid category budget.";
                    return false;
                }
            }
        }

        /// Initialize the warning threshold to the module's placeholder default.
        warningThreshold = -1.0;
        if (getline(ss, token))
        {
            token = trim(token);
            if (!token.empty())
            {
                try
                {
                    warningThreshold = stod(token);
                }
                catch (...)
                {
                    errorMsg = "Invalid category warning threshold.";
                    return false;
                }
            }
        }

        return true;
    }
}

/**
 * @brief Loads categories from the default category storage file.
 *
 * This function reads the default category CSV file defined by
 * @c CATEGORY_FILE, skips blank lines and an optional header row, and
 * attempts to parse each remaining line into a @c Category object.
 * Invalid rows are ignored so that successfully parsed categories can still
 * be returned.
 *
 * @return A list of successfully loaded categories.
 *
 * @author Zhixian Wang
 */
vector<Category> DataAccess::loadCategories()
{
    /// Container that accumulates successfully parsed categories.
    vector<Category> categories;

    /// Open the default category file for reading.
    ifstream file(CATEGORY_FILE);
    if (!file.is_open())
    {
        /// Return an empty result if the file cannot be opened.
        return categories;
    }

    string line;
    int lineNumber = 0;
    bool firstNonEmptyLineHandled = false;

    /// Process the category file line by line.
    while (getline(file, line))
    {
        ++lineNumber;

        /// Skip blank lines entirely.
        if (isBlankLine(line))
        {
            continue;
        }

        /// Handle a possible header on the first non-empty line.
        if (!firstNonEmptyLineHandled)
        {
            firstNonEmptyLineHandled = true;
            if (isCategoryHeaderLine(line))
            {
                continue;
            }
        }

        /// Parsed category fields.
        string name;
        bool isExpense = true;
        double budget = -1.0;
        double warningThreshold = -1.0;
        string errorMsg;

        /// Skip malformed rows and continue processing the file.
        if (!parseCategoryRow(line, name, isExpense, budget, warningThreshold, errorMsg))
        {
            continue;
        }

        /// Construct the category object; skip rows rejected by Category validation.
        try
        {
            categories.emplace_back(name, isExpense, budget, warningThreshold);
        }
        catch (...)
        {
            continue;
        }
    }

    return categories;
}

/**
 * @brief Loads records from the default record storage file when rows already contain IDs.
 *
 * This function reads the default record CSV file defined by @c RECORD_FILE,
 * skips blank lines and an optional header row, parses each valid record row
 * that includes an ID column, and returns a collection of successfully loaded
 * @c Record objects.
 *
 * Malformed rows are ignored rather than terminating the load process.
 *
 * @return A list of successfully loaded records.
 *
 * @author Zhixian Wang
 */
vector<Record> DataAccess::loadRecordsWithId()
{
    /// Container that accumulates successfully parsed records.
    vector<Record> records;

    /// Open the default record file for reading.
    ifstream file(RECORD_FILE);
    if (!file.is_open())
    {
        /// Return an empty result if the file cannot be opened.
        return records;
    }

    string line;
    bool firstNonEmptyLineHandled = false;

    /// Process the record file line by line.
    while (getline(file, line))
    {
        /// Ignore blank lines.
        if (isBlankLine(line))
        {
            continue;
        }

        /// Skip the header if the first non-empty row appears to be one.
        if (!firstNonEmptyLineHandled)
        {
            firstNonEmptyLineHandled = true;
            if (isRecordHeaderLine(line, true))
            {
                continue;
            }
        }

        /// Parsed record fields.
        int id = 0;
        string date;
        double amount = 0.0;
        bool isExpense = true;
        string category;
        string errorMsg;

        /// Parse and store only valid rows.
        if (parseRecordRowWithId(line, id, date, amount, isExpense, category, errorMsg))
        {
            records.emplace_back(id, date, amount, isExpense, category);
        }
    }

    return records;
}

/**
 * @brief Loads records from an external CSV file without an ID column.
 *
 * This function imports record data from the specified CSV file path, where
 * rows are expected to omit the record ID column. Row-level processing
 * statistics and parsing errors are written into the supplied @c LoadReport.
 *
 * Successfully parsed rows are converted into @c Record objects with a
 * placeholder ID of @c 0, allowing outer logic to assign final IDs later.
 *
 * @param path The CSV file path to import from.
 * @param report Output report that stores row-level statistics, successful
 * line numbers, and parsing errors.
 * @return A list of successfully loaded records.
 *
 * @author Zhixian Wang
 */
vector<Record> DataAccess::loadRecordsWithoutId(const string &path, LoadReport &report)
{
    /// Reset the output report before processing begins.
    report = LoadReport{};
    vector<Record> records;

    /// Reject an empty import path.
    if (path.empty())
    {
        report.addError("Input path is required.", 0);
        return records;
    }

    /// Open the external CSV file for reading.
    ifstream file(path);
    if (!file.is_open())
    {
        report.addError("Failed to open file.", 0);
        return records;
    }

    string line;
    int lineNumber = 0;
    bool firstNonEmptyLineHandled = false;

    /// Process the import file one line at a time.
    while (getline(file, line))
    {
        ++lineNumber;

        /// Ignore blank lines.
        if (isBlankLine(line))
        {
            continue;
        }

        /// Skip the header if the first non-empty line appears to be one.
        if (!firstNonEmptyLineHandled)
        {
            firstNonEmptyLineHandled = true;
            if (isRecordHeaderLine(line, false))
            {
                continue;
            }
        }

        /// Count the current non-header, non-blank row as processed.
        report.addProcessedRow();

        /// Parsed record fields.
        string date;
        double amount = 0.0;
        bool isExpense = true;
        string category;
        string errorMsg;

        /// Store successful rows and log failures in the import report.
        if (parseRecordRowWithoutId(line, date, amount, isExpense, category, errorMsg))
        {
            records.emplace_back(0, date, amount, isExpense, category);
            report.addSuccessLine(lineNumber);
        }
        else
        {
            report.addError(errorMsg, lineNumber);
        }
    }

    return records;
}

/**
 * @brief Saves records to a CSV file using a temporary-file replacement strategy.
 *
 * This function serializes the supplied list of records into CSV format and
 * writes the result to a temporary file. If the write succeeds, the temporary
 * file replaces the target file using @c std::rename. This reduces the risk
 * of leaving a partially written output file if an error occurs during saving.
 *
 * If @p path is empty, the default record file path defined by @c RECORD_FILE
 * is used.
 *
 * @param data The list of records to save.
 * @param path Optional custom output path. If empty, the default record file
 * path is used.
 * @return True if saving succeeds; otherwise false.
 *
 * @author Zhixian Wang
 */
bool DataAccess::saveRecords(const vector<Record> &data, string path)
{
    /// Resolve the destination file path.
    string filePath = path.empty() ? RECORD_FILE : path;

    /// Build the temporary output file path.
    string tempPath = filePath + ".tmp";

    /// Open the temporary file for writing.
    ofstream file(tempPath);
    if (!file.is_open())
    {
        cerr << "Failed to write record file: " << tempPath << endl;
        return false;
    }

    /// Write the CSV header row.
    file << "id,date,amount,isExpense,category\n";

    /// Serialize each record in CSV format.
    for (const auto &record : data)
    {
        file << record.getId() << ","
             << record.getDate() << ","
             << record.getAmount() << ","
             << (record.getIsExpense() ? 1 : 0) << ","
             << record.getCategory() << "\n";
    }

    /// Flush and close the temporary file before replacement.
    file.close();

    /// Replace the destination file with the completed temporary file.
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0)
    {
        cerr << "Failed to replace record file: " << filePath << endl;
        return false;
    }

    return true;
}

/**
 * @brief Saves categories to a CSV file using a temporary-file replacement strategy.
 *
 * This function serializes the supplied list of categories into CSV format
 * and writes the result to a temporary file. If writing succeeds, the
 * temporary file replaces the final destination file using @c std::rename.
 *
 * If @p path is empty, the default category file path defined by
 * @c CATEGORY_FILE is used.
 *
 * @param data The list of categories to save.
 * @param path Optional custom output path. If empty, the default category
 * path is used.
 * @return True if saving succeeds; otherwise false.
 *
 * @author Zhixian Wang
 */
bool DataAccess::saveCategories(const vector<Category> &data, string path)
{
    /// Resolve the destination file path.
    string filePath = path.empty() ? CATEGORY_FILE : path;

    /// Build the temporary output file path.
    string tempPath = filePath + ".tmp";

    /// Open the temporary file for writing.
    ofstream file(tempPath);
    if (!file.is_open())
    {
        cerr << "Failed to write category file: " << tempPath << endl;
        return false;
    }

    /// Write the CSV header row.
    file << "name,isExpense,budget,warningThreshold\n";

    /// Serialize each category in CSV format.
    for (const auto &category : data)
    {
        file << category.getName() << ","
             << (category.getIsExpense() ? 1 : 0) << ","
             << category.getBudget() << ","
             << category.getWarningThreshold() << "\n";
    }

    /// Flush and close the temporary file before replacement.
    file.close();

    /// Replace the destination file with the completed temporary file.
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0)
    {
        cerr << "Failed to replace category file: " << filePath << endl;
        return false;
    }

    return true;
}