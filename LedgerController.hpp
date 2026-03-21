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
    std::vector<Record> records;      ///< In-memory storage of historical data
    std::vector<Category> categories; ///< List of unique categories for filtering and reporting
    FinancialAnalyzer analyzer;       ///< Component for summary and total calculations
    DataAccess dataAccess;            ///< Component for CSV reading and writing
    std::string lastError;            ///< Stores error descriptions
    int nextRecordId;                 ///< Tracks the next available unique identifier

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
     * @brief Initialization: Loads persisted records and categories into memory.
     * @return A status message for the UI:
     * - "System initialized. No existing records found. Loaded [M] categories." : Case for new users or empty CSV files.
        * - "System initialized. Successfully loaded [N] records. Loaded [M] categories." : Normal case where data exists and has been loaded.
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
     * @param cat Transaction category. If empty, defaults to "Other Income" or "Other Expense" based on isExpense.
     *            If the specified category does not exist in the system, it will be created automatically with no budget set.
     *            If category creation fails, the record will not be added and an error is returned.
     * @return "SUCCESS: Record added (ID: N)" if category exists or was auto-created successfully.
     *         "SUCCESS: Record added (ID: N), Category '[cat]' auto-created." if a new category was created.
     *         "FAIL: [Error details]" if validation fails or category creation fails.
     */
    std::string addRecord(std::string date, double amount, bool isExpense, std::string cat = "");

    /**
     * @brief Batch imports financial records and returns detailed loading report.
     * @param filePath The path to the external CSV file to be imported.
     * @return A result message with report summary, including:
     *         - processed line count,
     *         - successful line count,
     *         - error line count,
     *         - grouped error details in the form: error message -> line number list.
     *         Returns "FAIL: ..." when no rows are imported successfully.
     * @note This method appends imported data to existing in-memory records without clearing them.
     */
    std::string addRecordsByFile(std::string filePath);

    /**
     * @brief Partially updates fields of an existing financial record.
     * @note recordId is required.
     *       At least one updatable field must be provided (date != "", amount != -1.0,
     *       isExpense != -1, or cat != "") for a successful update.
     * @param recordId The unique identifier of the record to be modified.
     * @param date New transaction date in "YYYY-MM-DD" format. Pass empty string to keep unchanged.
     * @param amount New transaction amount (must be positive). Pass -1.0 to keep unchanged.
     * @param isExpense New transaction type: 1 for expense, 0 for income. Pass -1 to keep unchanged.
     * @param cat New category name. Pass empty string to keep unchanged.
     *            If a non-empty category does not exist, the controller attempts to auto-create it.
     * @return "SUCCESS: Record ID #[id] is updated." if successful, "FAIL: [Error details]" otherwise.
     */
    std::string updateRecord(int recordId, std::string date = "", double amount = -1.0, int isExpense = -1, std::string cat = "");

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

    /**
     * @brief Adds a new category to the system with optional budget and warning threshold.
     * @param name The name of the category to be added (e.g., "Food", "Rent"). Must be unique and non-empty.
     * @param isExpense Optional indicator for expense category. Defaults to true (expense). Pass false for income.
     * @param budget Optional budget amount for the category. Defaults to -1.0, indicating no budget set.
     * @param warningThreshold Optional percentage threshold for budget warning. Defaults to -1.0 (no warning).
     *                        If budget is set but warningThreshold is not, defaults to 70% of budget.
     *                        If warningThreshold is set but budget is -1.0 (not set), returns error.
     * @return "SUCCESS: Category '[name]' added..." if successful, "FAIL: [Error details]" otherwise.
     *         Error cases include: duplicate name, empty name, invalid threshold without budget.
     */
    std::string addCategory(std::string name, bool isExpense = true, double budget = -1.0, double warningThreshold = -1.0);

    /**
     * @brief Removes an existing category from the system.
     * @param name The name of the category to be removed. Must exist in the system.
     * @note When a category is removed, all records previously associated with it are reassigned to
     *       the default category: "Other Expense" for expense records and "Other Income" for income records.
     * @return "SUCCESS: Category '[name]' removed." if successful, "FAIL: [Error details]" otherwise.
     */
    std::string removeCategory(std::string name);

    /**
     * @brief Updates an existing category's name, type, budget, and/or warning threshold.
     *        At least one of newName, isExpense, newBudget, or newWarningThreshold must be provided to make a change.
     * @param oldName The current name of the category. Must exist in the system.
     * @param newName The new name for the category. Must be unique and non-empty. Defaults to "" (no change).
     * @param isExpense Category type update flag: 1 for expense, 0 for income, -1 for unchanged.
     * @param newBudget The new budget for the category. Defaults to -1.0 (no change).
     * @param newWarningThreshold The new warning threshold percentage for the category. Defaults to -1.0 (no change).
     * @note Records previously associated with this category remain unchanged and automatically update their category reference
     *       if the category name is modified.
     * @return "SUCCESS: Category '[oldName]' updated..." if successful, "FAIL: [Error details]" otherwise.
     */
    std::string updateCategory(std::string oldName, std::string newName = "", int isExpense = -1, double newBudget = -1.0, double newWarningThreshold = -1.0);

    /**
     * @brief Retrieves all categories currently in the system.
     * @return std::vector<Category> Category objects including name/type/budgetza/warning threshold settings.
     */
    std::vector<Category> getCategories();

    /**
     * @brief Calculates current-month budget status for each category.
     *
     * Each returned BudgetStatus item contains:
     * - categoryName: Category identifier.
     * - actualSpent: Total expense recorded in the current month for the category.
     * - budgetLimit: Configured monthly budget limit for the category.
     * - remaining: budgetLimit - actualSpent.
     * - daysRemaining: Remaining days in the current calendar month.
     * - dailyAvailable: Recommended average spend per remaining day (remaining / daysRemaining).
     * - budgetHealth: Enum indicating budget status:
     *                 - Safe: Actual spending is below warning threshold.
     *                 - Warning: Actual spending has exceeded warning threshold but not budget limit.
     *                 - Exceeded: Actual spending has exceeded the budget limit.
     *
     * @return std::vector<BudgetStatus> One status object per category for the current month.
     */
    std::vector<BudgetStatus> getCurrentBudgetStatus();

    /**
     * @brief Computes category distribution within an optional date range.
     *
     * Each CategoryDistItem contains:
     * - categoryName: Category label.
     * - amount: Total amount accumulated for that category.
     * - percentage: Share of the overall total represented by that category.
     *
     * The returned vector may also include one aggregate item representing the
     * overall total, for use in summary or chart rendering.
     *
     * @param start Optional start date in "YYYY-MM-DD" format. Empty string means no lower bound.
     * @param end Optional end date in "YYYY-MM-DD" format. Empty string means no upper bound.
     * @return std::vector<CategoryDistItem> Distribution items for the filtered period.
     */
    std::vector<CategoryDistItem> getDistribution(std::string start = "", std::string end = "");

    /**
     * @brief Generates a monthly trend series for a category or for all records.
     *
     * The returned map is aggregated by month. Keys use "YYYY-MM" and values are
     * aggregated monthly amounts, suitable for monthly trend charts.
     *
     * @param start Optional start date in "YYYY-MM-DD" format. Empty string means no lower bound.
     * @param end Optional end date in "YYYY-MM-DD" format. Empty string means no upper bound.
     * @param cat Optional category filter. Empty string means all categories.
     * @return std::map<std::string, double> A month-to-amount mapping for the filtered records.
     */
    std::map<std::string, double> getTrend(std::string start = "", std::string end = "", std::string cat = "");

    /**
     * @brief Summarizes monthly income and expense amounts across a date range.
     *
     * The returned map is grouped by month. Keys use "YYYY-MM" and values store
     * paired amounts in the form {income, expense} for each month.
     *
     * @param start Optional start date in "YYYY-MM-DD" format. Empty string means no lower bound.
     * @param end Optional end date in "YYYY-MM-DD" format. Empty string means no upper bound.
     * @return std::map<std::string, std::pair<double, double>> Monthly income and expense totals.
     */
    std::map<std::string, std::pair<double, double>> getIncomeExpense(std::string start = "", std::string end = "");
};

#endif // LEDGERCONTROLLER_H