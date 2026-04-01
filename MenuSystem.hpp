/**
 * @file MenuSystem.hpp
 * @brief Header file for the MenuSystem class in the Ledger System.
 * @details This file defines the terminal-based user interface, handling menu displays,
 * user input validation, and communication with the core controllers.
 * @author Junqi Zheng
 */
#ifndef MENUSYSTEM_HPP
#define MENUSYSTEM_HPP

#include "LedgerController.hpp"
#include <string>

/**
 * @class MenuSystem
 * @brief Manages the terminal-based Command Line Interface (CLI) for the Ledger System.
 * @details This class is responsible for displaying menus, capturing and validating user
 * input, and routing user actions to the appropriate LedgerController methods. It also
 * handles the formatted rendering of financial data and statistics.
 */
class MenuSystem
{
private:
    LedgerController &controller;       ///< Reference to the core controller managing business logic and data access.

    /**
     * @brief Displays the main application menu.
     */
    static void displayMainMenu();

    /**
     * @brief Handles the interactive workflow for adding a single new record.
     */
    void handleAddRecord();

    /**
     * @brief Handles the workflow for batch importing records from a CSV file.
     */
    void handleAddRecordByFile();

    /**
     * @brief Displays the report and statistics menu.
     */
    static void displayReportMenu();

    /**
     * @brief Routes user input for report and statistics generation.
     */
    void handleReportManagement();

    /**
     * @brief Displays the category management menu.
     */
    static void displayCategoryMenu();

    /**
     * @brief Routes user input for the category management sub-menu.
     */
    void handleCategoryManagement();

    /**
     * @brief Prompts the user for details to create a new category.
     */
    void handleAddCategory();

    /**
     * @brief Retrieves and displays a list of all existing categories.
     */
    void handleListCategories();

    /**
     * @brief Prompts the user to update an existing category's details.
     */
    void handleUpdateCategory();

    /**
     * @brief Prompts the user to delete an existing category.
     */
    void handleDeleteCategory();

    /**
     * @brief Handles searching for records based on various criteria (e.g., date, category).
     */
    void handleSearchRecords();

    /**
     * @brief Prompts the user for a specific Record ID and displays it.
     */
    void handleSearchRecordById();

    /**
     * @brief Calculates and displays a simple total of income vs. expenses.
     */
    void handleSimpleTotal();

    /**
     * @brief Prompts the user for a Record ID and the new details to update it.
     */
    void handleUpdateRecord();

    /**
     * @brief Prompts the user for a Record ID to permanently delete.
     */
    void handleDeleteRecord();

    /**
     * @brief Handles the generation and display of a comprehensive financial summary.
     */
    void handleFinancialSummary();

    /**
     * @brief Retrieves and displays the current status of category budgets.
     */
    void handleCurrentBudgetStatus();

    /**
     * @brief Renders the budget status data into a formatted terminal view.
     * @param statuses A vector containing the budget status for various categories.
     * @param showRemaining If true, prominently displays the remaining budget amount.
     */
    static void renderBudgetStatus(const std::vector<BudgetStatus> &statuses, bool showRemaining);

    /**
     * @brief Calculates and displays the distribution of expenses/income across categories.
     */
    void handleDistribution();

    /**
     * @brief Renders the category distribution data into a formatted terminal view.
     * @param distribution A pair containing the total amount and a vector of distribution items (percentages/amounts per category).
     */
    static void renderDistribution(const std::pair<double, std::vector<CategoryDistItem>> &distribution);

    /**
     * @brief Calculates and displays the financial trend over a specific period.
     */
    void handleTrend() const;

    /**
     * @brief Renders trend data into a formatted terminal view.
     * @param trendData A map of time periods (e.g., months) to their corresponding financial amounts.
     * @param category The specific category being analyzed (empty string for overall trend).
     * @param trendTypeStr The type of trend being rendered (e.g., "Expense" or "Income").
     */
    static void renderTrend(const std::map<std::string, double> &trendData, const std::string &category = "", const std::string &trendTypeStr = "Expense");

    /**
     * @brief Calculates and displays the comparison between income and expenses over time.
     */
    void handleIncomeExpense() const;

    /**
     * @brief Renders the income vs. expense comparison data into a formatted terminal view.
     * @param data A map of time periods to a pair of doubles representing {Income, Expense}.
     */
    static void renderIncomeExpense(const std::map<std::string, std::pair<double, double>> &data);

    /**
     * @brief Renders a list of records into a formatted, aligned terminal table.
     * @param records A vector of Record objects to be displayed.
     */
    static void renderRecordTable(const std::vector<Record> &records);

    // --- Input Validation Helpers ---

    /**
     * @brief Prompts the user for string input and validates it.
     * @param prompt The text displayed to the user.
     * @param allowEmpty If true, the user can submit an empty string (e.g., for optional fields). Defaults to false.
     * @return The validated string input from the user.
     */
    std::string getValidatedInput(std::string prompt, bool allowEmpty = false);

    /**
     * @brief Prompts the user for a numeric amount and ensures it is a valid double.
     * @param allowEmpty If true, allows the user to skip input (returns a default/unchanged value). Defaults to false.
     * @return The validated numeric amount.
     */
    double getValidatedAmount(bool allowEmpty = false);

    /**
     * @brief Prompts the user for a date and ensures it matches the required format (e.g., YYYY-MM-DD).
     * @return The validated date string.
     */
    std::string getValidatedDate() const;

public:
    /**
     * @brief Constructs a new MenuSystem.
     * @param ctrl A reference to the LedgerController that will handle backend operations.
     */
    MenuSystem(LedgerController &ctrl);

    /**
     * @brief Starts the main UI loop.
     * @details This is the main entry point for the UI, called from main.cpp. It keeps the
     * application running and listening for user commands until the user chooses to exit.
     */
    void run();
};

#endif // MENUSYSTEM_HPP
