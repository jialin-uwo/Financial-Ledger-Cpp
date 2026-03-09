/**
 * @file LedgerController.h
 * @brief Orchestrates financial data flow between CSV storage and the User Interface.
 * * This controller manages a collection of financial records, providing services for
 * persistent storage via DataAccess and complex analytics via FinancialAnalyzer.
 * It supports cumulative filtering and comprehensive reporting (Income, Expense, Balance)
 * to satisfy user requirements for financial health monitoring.
 * @author Jialin Li
 */

#ifndef LEDGERCONTROLLER_H
#define LEDGERCONTROLLER_H

#include <vector>
#include <string>
#include "Record.h"
#include "FinancialAnalyzer.h"
#include "DataAccess.h"

class LedgerController
{
private:
    std::vector<Record> records; ///< In-memory storage of historical data
    FinancialAnalyzer analyzer;  ///< Component for summary and total calculations
    DataAccess dataAccess;       ///< Component for CSV reading and writing
    std::string lastError;       ///< Stores error descriptions
    int nextRecordId;            ///< Tracks the next available unique identifier

public:
    /**
     * @brief Constructor: Initializes internal components via default constructors.
     * @note Components like analyzer and dataAccess are instantiated here,
     * but no data is loaded yet.
     */
    LedgerController();

    /**
     * @brief Destructor: Ensures data is saved and provides a safety net for system shutdown.
     */
    ~LedgerController();

    /**
     * @brief Initialization: Opens the CSV file and populates the records vector.
     * @return A status message for the UI:
     * - "System initialized. No existing records found." : Case for new users or empty CSV files.
     * - "System initialized. Successfully loaded and re-indexed [N] records." : Normal case where data exists and is re-indexed.
     * @note The UI should call this immediately after construction.
     */
    std::string init();

    /**
     * @brief Saves data and closes the file stream.
     * @return std::string Status of the shutdown.
     */
    std::string shutDown();

    /**
     * @brief Validates and adds a new record to the collection.
     * @param date Transaction date (YYYY-MM-DD).
     * @param amount Monetary value (> 0).
     * @param isExpense True for Expense, False for Income.
     * @param cat Transaction category. Pass empty string to disable category filtering.
     * @return "SUCCESS: Record added (ID: N)" or "FAIL: [Error details]".
     */
    std::string addRecord(std::string date, double amount, bool isExpense, std::string cat = "");

    /**
     * @brief Batch imports financial records from an external CSV file.
     * @param filePath The path to the external CSV file to be imported.
     * @return "SUCCESS: Imported [N] records from file." or "FAIL: [Error details]".
     * @note This method appends new data to the existing in-memory records without clearing them.
     */
    std::string addRecordsByFile(std::string filePath);

    /**
     * @brief Updates all fields of an existing financial record.
     * @param recordId The unique identifier of the record to be modified.
     * @param date New transaction date in "YYYY-MM-DD" format.
     * @param amount New transaction amount (must be positive).
     * @param isExpense True if the transaction is an expense, false for income.
     * @param cat New category name. Pass empty string to keep category unchanged or disable filtering.
     * @return "SUCCESS: Record ID #[id] is updated." if successful, "FAIL: [Error details]" otherwise.
     */
    std::string updateRecord(int recordId, std::string date, double amount, bool isExpense, std::string cat = "");

    /**
     * @brief Deletes a specific financial record by its ID.
     * @param id The unique identifier of the record.
     * @return "SUCCESS: Record #[id] deleted." if successful, "FAIL: Record ID #[id] does not exist." otherwise.
     */
    std::string removeRecord(int id);

    /**
     * @brief Retrieves the last error message recorded by the controller.
     * @return std::string The error description; returns an empty string if the last operation succeeded.
     */
    std::string getLastError();

    /**
     * @brief Fetches a list of records based on cumulative filters.
     * All filters are optional and default to no filtering:
     * - start: Default "" (no date lower bound).
     * - end: Default "" (no date upper bound).
     * - isExpense: Default -1 (no transaction type filtering; -1=no filter, 0=income only, 1=expense only).
     * - cat: Pass empty string to disable category filtering.
     * - minAmount: Default 0.0 (no amount filtering).
     * @param start Optional start date (YYYY-MM-DD). Empty string means no lower bound.
     * @param end Optional end date (YYYY-MM-DD). Empty string means no upper bound.
     * @param isExpense Optional transaction type filter. Defaults to -1 (no filtering, returns both income and expense). Use 0 for income only, 1 for expense only.
     * @param cat Optional category filter. Pass empty string to return all categories.
     * @param minAmount Optional minimum amount threshold. Defaults to 0.0 (no filtering).
     * @return std::vector<Record> A list of matching records. Returns an empty vector if no matches are found or if inputs are invalid (check lastError).
     */
    std::vector<Record> getRecords(std::string start = "", std::string end = "",
                                   int isExpense = -1, std::string cat = "", double minAmount = 0.0);

    /**
     * @brief Generates a comprehensive financial summary for a specified period.
     * Calculates Total Income, Total Expense, Net Balance, and a detailed breakdown by category.
     * If no parameters are provided, it summarizes all historical data in the system.
     * @param start Optional start date in "YYYY-MM-DD" format. If empty, the summary starts from the earliest record.
     * @param end Optional end date in "YYYY-MM-DD" format. If empty, the summary goes up to the latest record.
     * @return std::map<std::string, double> A map containing:
     *         - "Total Income": Sum of all income records in the period.
     *         - "Total Expense": Sum of all expense records in the period.
     *         - "Net Balance": Total Income minus Total Expense.
     *         - "Category: [category_name]": Total amount for each category (e.g., "Category: Food", "Category: Rent", "Category: Other").
     *         Returns an empty map and updates lastError if the date range is logically invalid or no records found.
     */
    std::map<std::string, double> getPeriodSummary(std::string start = "", std::string end = "");

    /**
     * @brief Calculates the total income and/or expense for a specific category and/or time period.
     * Provides a detailed breakdown of spending habits. The filtering is cumulative based on optional parameters:
     * - Date range: Filters records within the specified date range.
     * - Transaction type: Can filter to income only, expense only, or both (default).
     * - Category filter: When cat is "" (default), filters to that category. Otherwise filters to the specified category.
     * @param start Optional start date in "YYYY-MM-DD" format. Empty string means no lower bound.
     * @param end Optional end date in "YYYY-MM-DD" format. Empty string means no upper bound.
     * @param isExpense Optional transaction type filter. Defaults to -1 (no filter, returns both income and expense).
     *                   Use 0 for income only, 1 for expense only.
     * @param cat The name of the category to filter (e.g., "Food", "Rent"). Defaults to "Other".
     * @return std::string A formatted summary message:
     *         - For isExpense=-1: "Total Income: $X.XX, Total Expense: $Y.YY, Balance: $Z.ZZ"
     *         - For isExpense=0: "Total Income: $X.XX"
     *         - For isExpense=1: "Total Expense: $X.XX"
     *         Returns error message if no records match the criteria.
     */
    std::string getTotal(std::string start = "", std::string end = "", int isExpense = -1, std::string cat = "");
};

#endif // LEDGERCONTROLLER_H