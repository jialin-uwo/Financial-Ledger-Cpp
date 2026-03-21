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
     * @param value Input string.
     * @return Trimmed string.
     */
    string trim(const string &value)
    {
        size_t start = value.find_first_not_of(" \t\n\r");
        if (start == string::npos)
        {
            return "";
        }

        size_t end = value.find_last_not_of(" \t\n\r");
        return value.substr(start, end - start + 1);
    }

    /**
     * @brief Converts a string to lowercase.
     *
     * @param value Input string.
     * @return Lowercase copy of the input.
     */
    string toLowerCopy(const string &value)
    {
        string result = value;
        for (char &ch : result)
        {
            ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        }
        return result;
    }

    /**
     * @brief Checks whether a line is empty or only contains whitespace.
     *
     * @param line Input line.
     * @return True if the line is blank; otherwise false.
     */
    bool isBlankLine(const string &line)
    {
        return trim(line).empty();
    }

    /**
     * @brief Adds one error entry into the load report.
     *
     * @param report The report to update.
     * @param message The error message.
     * @param lineNumber The line number where the error happened.
     */
    void addError(LoadReport &report, const string &message, int lineNumber)
    {
        report.errorRows++;
        report.errorsByMessage[message].push_back(lineNumber);
    }

    /**
     * @brief Parses the isExpense token into a boolean value.
     *
     * Accepted true values: 1, true
     * Accepted false values: 0, false
     * Comparison is case-insensitive.
     *
     * @param token Input token.
     * @param isExpense Output parsed value.
     * @return True if parsing succeeds; otherwise false.
     */
    bool parseIsExpenseToken(const string &token, bool &isExpense)
    {
        string normalized = toLowerCopy(trim(token));

        if (normalized == "1" || normalized == "true")
        {
            isExpense = true;
            return true;
        }

        if (normalized == "0" || normalized == "false")
        {
            isExpense = false;
            return true;
        }

        return false;
    }

    /**
     * @brief Checks whether a line looks like a record header.
     *
     * @param line Input line.
     * @param hasId True if the record format includes an ID column.
     * @return True if the line looks like a header; otherwise false.
     */
    bool isRecordHeaderLine(const string &line, bool hasId)
    {
        stringstream ss(line);
        string firstToken;

        if (!getline(ss, firstToken, ','))
        {
            return false;
        }

        firstToken = toLowerCopy(trim(firstToken));

        if (hasId)
        {
            return firstToken == "id";
        }

        return firstToken == "date";
    }

    /**
     * @brief Checks whether a line looks like a category header.
     *
     * @param line Input line.
     * @return True if the line looks like a header; otherwise false.
     */
    bool isCategoryHeaderLine(const string &line)
    {
        stringstream ss(line);
        string firstToken;

        if (!getline(ss, firstToken, ','))
        {
            return false;
        }

        firstToken = toLowerCopy(trim(firstToken));
        return firstToken == "name";
    }

    /**
     * @brief Parses one record row that contains an ID column.
     *
     * Expected format:
     * id,date,amount,isExpense,category
     *
     * @param line Input CSV row.
     * @param id Output ID.
     * @param date Output date.
     * @param amount Output amount.
     * @param isExpense Output type flag.
     * @param category Output category name.
     * @param errorMsg Output error message.
     * @return True if parsing succeeds; otherwise false.
     */
    bool parseRecordRowWithId(const string &line,
                              int &id,
                              string &date,
                              double &amount,
                              bool &isExpense,
                              string &category,
                              string &errorMsg)
    {
        stringstream ss(line);
        string token;

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

        try
        {
            id = stoi(token);
        }
        catch (...)
        {
            errorMsg = "Invalid ID.";
            return false;
        }

        if (!getline(ss, date, ','))
        {
            errorMsg = "Missing date column.";
            return false;
        }
        date = trim(date);

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

        try
        {
            amount = stod(token);
        }
        catch (...)
        {
            errorMsg = "Invalid amount.";
            return false;
        }

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

        if (getline(ss, category))
        {
            category = trim(category);
        }
        else
        {
            category = "";
        }

        if (!Record::validateData(date, amount, errorMsg))
        {
            return false;
        }

        return true;
    }

    /**
     * @brief Parses one record row that does not contain an ID column.
     *
     * Expected format:
     * date,amount,isExpense,category
     *
     * @param line Input CSV row.
     * @param date Output date.
     * @param amount Output amount.
     * @param isExpense Output type flag.
     * @param category Output category name.
     * @param errorMsg Output error message.
     * @return True if parsing succeeds; otherwise false.
     */
    bool parseRecordRowWithoutId(const string &line,
                                 string &date,
                                 double &amount,
                                 bool &isExpense,
                                 string &category,
                                 string &errorMsg)
    {
        stringstream ss(line);
        string token;

        if (!getline(ss, date, ','))
        {
            errorMsg = "Missing date column.";
            return false;
        }
        date = trim(date);

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

        try
        {
            amount = stod(token);
        }
        catch (...)
        {
            errorMsg = "Invalid amount.";
            return false;
        }

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

        if (getline(ss, category))
        {
            category = trim(category);
        }
        else
        {
            category = "";
        }

        if (!Record::validateData(date, amount, errorMsg))
        {
            return false;
        }

        return true;
    }

    /**
     * @brief Parses one category row.
     *
     * Expected format:
     * name,isExpense,budget,warningThreshold
     *
     * budget may be empty, in which case it defaults to -0.1.
     * warningThreshold may be empty, in which case it defaults to -1.0.
     *
     * @param line Input CSV row.
     * @param name Output category name.
     * @param isExpense Output type flag.
     * @param budget Output budget value.
     * @param warningThreshold Output warning threshold value.
     * @param errorMsg Output error message.
     * @return True if parsing succeeds; otherwise false.
     */
    bool parseCategoryRow(const string &line,
                          string &name,
                          bool &isExpense,
                          double &budget,
                          double &warningThreshold,
                          string &errorMsg)
    {
        stringstream ss(line);
        string token;

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

        budget = -0.1;
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
 * @return A list of successfully loaded categories.
 */
vector<Category> DataAccess::loadCategories()
{
    vector<Category> categories;

    ifstream file(CATEGORY_FILE);
    if (!file.is_open())
    {
        return categories;
    }

    string line;
    int lineNumber = 0;
    bool firstNonEmptyLineHandled = false;

    while (getline(file, line))
    {
        ++lineNumber;

        if (isBlankLine(line))
        {
            continue;
        }

        if (!firstNonEmptyLineHandled)
        {
            firstNonEmptyLineHandled = true;
            if (isCategoryHeaderLine(line))
            {
                continue;
            }
        }

        string name;
        bool isExpense = true;
        double budget = -0.1;
        double warningThreshold = -1.0;
        string errorMsg;

        if (!parseCategoryRow(line, name, isExpense, budget, warningThreshold, errorMsg))
        {
            continue;
        }

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
 * @brief Loads records from the default record storage file, where rows already contain IDs.
 *
 * @return A list of successfully loaded records.
 */
vector<Record> DataAccess::loadRecordsWithId()
{
    vector<Record> records;

    ifstream file(RECORD_FILE);
    if (!file.is_open())
    {
        return records;
    }

    string line;
    bool firstNonEmptyLineHandled = false;

    while (getline(file, line))
    {
        if (isBlankLine(line))
        {
            continue;
        }

        if (!firstNonEmptyLineHandled)
        {
            firstNonEmptyLineHandled = true;
            if (isRecordHeaderLine(line, true))
            {
                continue;
            }
        }

        int id = 0;
        string date;
        double amount = 0.0;
        bool isExpense = true;
        string category;
        string errorMsg;

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
 * @param path The CSV file path to import from.
 * @param report Output report that stores row-level statistics and errors.
 * @return A list of successfully loaded records.
 */
vector<Record> DataAccess::loadRecordsWithoutId(const string &path, LoadReport &report)
{
    report = LoadReport{};
    vector<Record> records;

    if (path.empty())
    {
        addError(report, "Input path is required.", 0);
        return records;
    }

    ifstream file(path);
    if (!file.is_open())
    {
        addError(report, "Failed to open file.", 0);
        return records;
    }

    string line;
    int lineNumber = 0;
    bool firstNonEmptyLineHandled = false;

    while (getline(file, line))
    {
        ++lineNumber;

        if (isBlankLine(line))
        {
            continue;
        }

        if (!firstNonEmptyLineHandled)
        {
            firstNonEmptyLineHandled = true;
            if (isRecordHeaderLine(line, false))
            {
                continue;
            }
        }

        report.processedRows++;

        string date;
        double amount = 0.0;
        bool isExpense = true;
        string category;
        string errorMsg;

        if (parseRecordRowWithoutId(line, date, amount, isExpense, category, errorMsg))
        {
            records.emplace_back(0, date, amount, isExpense, category);
            report.successRows++;
        }
        else
        {
            addError(report, errorMsg, lineNumber);
        }
    }

    return records;
}

/**
 * @brief Saves records to a CSV file using a temporary file and rename strategy.
 *
 * @param data The list of records to save.
 * @param path Optional custom output path.
 * @return True if saving succeeds; otherwise false.
 */
bool DataAccess::saveRecords(const vector<Record> &data, string path)
{
    string filePath = path.empty() ? RECORD_FILE : path;
    string tempPath = filePath + ".tmp";

    ofstream file(tempPath);
    if (!file.is_open())
    {
        cerr << "Failed to write record file: " << tempPath << endl;
        return false;
    }

    file << "id,date,amount,isExpense,category\n";

    for (const auto &record : data)
    {
        file << record.getId() << ","
             << record.getDate() << ","
             << record.getAmount() << ","
             << (record.getIsExpense() ? 1 : 0) << ","
             << record.getCategory() << "\n";
    }

    file.close();

    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0)
    {
        cerr << "Failed to replace record file: " << filePath << endl;
        return false;
    }

    return true;
}

/**
 * @brief Saves categories to a CSV file using a temporary file and rename strategy.
 *
 * @param data The list of categories to save.
 * @param path Optional custom output path.
 * @return True if saving succeeds; otherwise false.
 */
bool DataAccess::saveCategories(const vector<Category> &data, string path)
{
    string filePath = path.empty() ? CATEGORY_FILE : path;
    string tempPath = filePath + ".tmp";

    ofstream file(tempPath);
    if (!file.is_open())
    {
        cerr << "Failed to write category file: " << tempPath << endl;
        return false;
    }

    file << "name,isExpense,budget,warningThreshold\n";

    for (const auto &category : data)
    {
        file << category.getName() << ","
             << (category.getIsExpense() ? 1 : 0) << ","
             << category.getBudget() << ","
             << category.getWarningThreshold() << "\n";
    }

    file.close();

    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0)
    {
        cerr << "Failed to replace category file: " << filePath << endl;
        return false;
    }

    return true;
}