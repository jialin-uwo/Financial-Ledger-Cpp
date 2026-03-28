#include "MenuSystem.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include "BudgetStatus.h"
#include "BudgetHealth.h"
#include "LedgerController.hpp"
#include "FinancialAnalyzer.h"
#include "Record.h"
#include <map>

MenuSystem::MenuSystem(LedgerController &ctrl) : controller(ctrl) {}

void MenuSystem::run()
{
    bool isRunning = true;
    std::string choice;

    std::cout << "\n=== Welcome to C-The-Cash ===" << std::endl;
    while (isRunning)
    {
        displayMainMenu();

        std::cout << "\nPlease enter your choice (0-13): ";
        std::getline(std::cin, choice);

        if (choice == "0")
        {
            isRunning = false;
            exitMenu();
        }
        else
        {
            handleCommand(choice);
        }
    }
}

void MenuSystem::displayMainMenu() {
    std::cout << "\n-----------------------------------" << std::endl;
    std::cout << "1. Add a New Record" << std::endl;
    std::cout << "2. Add Records by File (Batch Import)" << std::endl;
    std::cout << "3. View/Search Records" << std::endl;
    std::cout << "4. Update a Record" << std::endl;
    std::cout << "5. Delete a Record" << std::endl;
    std::cout << "6. Financial Summary (All-in-one)" << std::endl;
    std::cout << "7. Simple Total" << std::endl;
    std::cout << "8. Category Management" << std::endl;
    std::cout << "9. Current Budget Status" << std::endl;
    std::cout << "10. Expense Distribution" << std::endl;
    std::cout << "11. Trend Analysis" << std::endl;
    std::cout << "12. Income vs Expense Comparison" << std::endl;
    std::cout << "13. Visualize Records" << std::endl;
    std::cout << "0. Exit and Save" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

void MenuSystem::handleCommand(std::string cmd) {
    if (cmd == "1") {
        handleAddRecord();
    }
    else if (cmd == "2") {
        handleAddRecordByFile();
    }
    else if (cmd == "3") {
        handleSearchRecords();
    }
    else if (cmd == "4") {
        handleUpdateRecord();
    }
    else if (cmd == "5") {
        handleDeleteRecord();
    }
    else if (cmd == "6") {
        handleFinancialSummary();
    }
    else if (cmd == "7") {
        handleSimpleTotal();
    }
    else if (cmd == "8") {
        handleCategoryManagement();
    }
    else if (cmd == "9") {
        handleCurrentBudgetStatus();
    }
    else if (cmd == "10") {
        handleDistribution();
    }
    else if (cmd == "11") {
        handleTrend();
    }
    else if (cmd == "12") {
        handleIncomeExpense();
    }
    else if (cmd == "13") {
        handleViewAllRecords();
    }
    else {
        std::cout << "Invalid Input! Please select a valid option (0-13)." << std::endl;
    }
}

void MenuSystem::exitMenu() const {
    std::cout << "\nSaving your ledger data..." << std::endl;
    std::string result = controller.shutDown();
    std::cout << result << std::endl;
    std::cout << "See you next time!" << std::endl;
}

void MenuSystem::handleAddRecord()
{
    std::cout << "\n--- Add New Record ---" << std::endl;

    std::string date = getValidatedInput("Enter date (YYYY-MM-DD): ");

    double amount = getValidatedAmount();

    std::string typeStr = getValidatedInput("Is it an expense? (y/n): ");

    bool isExpense = (typeStr == "y" || typeStr == "Y");

    std::string category = getValidatedInput("Enter category (optional, press Enter to auto-assign): ", true);

    std::string result = controller.addRecord(date, amount, isExpense, category);

    std::cout << "\n> " << result << std::endl;
}

void MenuSystem::handleAddRecordByFile()
{
    std::cout << "\n--- Add Records by File (Batch Import) ---" << std::endl;
    std::cout << R"(====================================================
              IMPORT DATA FROM CSV FILE
====================================================

[ 1. CSV FORMAT GUIDE ]
    Format: date,amount,isExpense,category
    Example: 2024-01-01,4150.11,false,Salary
             2024-01-02,22.22,true,Groceries

[ 2. ABOUT THE 'isExpense' COLUMN ]
    - Use true/false(case-insensitive) or 1/0
    - true / 1 means expense; false / 0 means income

[ 3. ABOUT THE 'Category' COLUMN ]
    - Category can be left blank
    - Blank category will auto-assign:
      * "Other Income" (for income)
      * "Other Expense" (for expense)

[ 4. SYSTEM REQUIREMENTS ]
    - Date format: YYYY-MM-DD
    - File format: .csv (Comma Separated)

Please enter the FULL path to your CSV file:
> _
====================================================)";
    std::string filePath = getValidatedInput("\nEnter import file path: ");

    std::string result = controller.addRecordsByFile(filePath);
    std::cout << "\n> " << result << std::endl;

    if (!controller.getLastError().empty())
    {
        std::cout << "> Reason: " << controller.getLastError() << std::endl;
    }
}

void MenuSystem::handleSearchRecords()
{
    std::cout << "\n--- View/Search Records ---" << std::endl;
    std::cout << "(Press Enter to skip any filter and view all)" << std::endl;

    // Step 1: Get start date (optional)
    std::string start = getValidatedInput("Enter Start Date (YYYY-MM-DD): ", true);

    // Step 2: Get end date (optional)
    std::string end = getValidatedInput("Enter End Date (YYYY-MM-DD): ", true);

    // Step 3: Get expense/income filter (optional)
    std::cout << "Filter by type:" << std::endl;
    std::cout << "  e - Expense only" << std::endl;
    std::cout << "  i - Income only" << std::endl;
    std::cout << "  Press Enter - No filter (both)" << std::endl;

    std::string filterChoice = getValidatedInput("Enter your choice: ", true);

    int isExpense = -1; // Default: no filter
    if (filterChoice == "e" || filterChoice == "E")
    {
        isExpense = 1; // Expense only
    }
    else if (filterChoice == "i" || filterChoice == "I")
    {
        isExpense = 0; // Income only
    }

    // Step 4: Optional category filter
    std::string category = getValidatedInput("Enter Category (optional, press Enter for all categories): ", true);

    // Step 5: Get minimum amount (optional)
    double minAmount = getValidatedAmount(true);

    std::vector<Record> results = controller.getRecords(start, end, isExpense, category, minAmount);

    if (results.empty())
    {
        std::cout << "\n> No records found or invalid criteria." << std::endl;
        if (!controller.getLastError().empty())
        {
            std::cout << "> Reason: " << controller.getLastError() << std::endl;
        }
    }
    else
    {
        std::cout << "\n--- Found " << results.size() << " Records ---" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        for (const auto &rec : results)
        {
            std::string type = rec.getIsExpense() ? "[Expense]" : "[Income ]";
            std::cout << "ID: " << rec.getId() << " | Date: " << rec.getDate()
                      << " | Category: " << rec.getCategory()
                      << " | " << type << " | Amount: $" << rec.getAmount() << std::endl;
        }
    }
}

void MenuSystem::handleSimpleTotal()
{
    std::cout << "\n--- Simple Total ---" << std::endl;

    // Step 1: Get start date (optional)
    std::string start = getValidatedInput("Enter Start Date (YYYY-MM-DD, or press Enter to skip): ", true);

    // Step 2: Get end date (optional)
    std::string end = getValidatedInput("Enter End Date (YYYY-MM-DD, or press Enter to skip): ", true);

    // Step 3: Get expense/income filter (optional)
    std::cout << "Filter by type:" << std::endl;
    std::cout << "  e - Expense only" << std::endl;
    std::cout << "  i - Income only" << std::endl;
    std::cout << "  Press Enter - No filter (both)" << std::endl;

    std::string filterChoice = getValidatedInput("Enter your choice: ", true);

    int isExpense = -1; // Default: no filter
    if (filterChoice == "e" || filterChoice == "E")
    {
        isExpense = 1; // Expense only
    }
    else if (filterChoice == "i" || filterChoice == "I")
    {
        isExpense = 0; // Income only
    }

    std::string category = getValidatedInput("Enter Category (optional, press Enter for all categories): ", true);

    // Call controller with all filters
    std::string result = controller.getTotal(start, end, isExpense, category);
    std::cout << "\n> " << result << std::endl;
}

void MenuSystem::handleUpdateRecord()
{
    std::cout << "\n--- Update a Record ---" << std::endl;
    std::string idStr = getValidatedInput("Enter the Record ID to update: ");
    int id = 0;

    try
    {
        id = std::stoi(idStr);
    }
    catch (...)
    {
        std::cout << "> Error: Invalid ID format! Must be an integer." << std::endl;
        return;
    }

    std::cout << "Enter new details below:" << std::endl;
    std::string date = getValidatedInput("Enter new date (YYYY-MM-DD): ");
    double amount = getValidatedAmount();

    std::string typeStr = getValidatedInput("Is it an expense? (y/n): ");
    bool isExpense = (typeStr == "y" || typeStr == "Y");

    std::string category = getValidatedInput("Enter new category (optional, press Enter to keep current): ", true);

    std::string result = controller.updateRecord(id, date, amount, isExpense, category);
    std::cout << "\n> " << result << std::endl;
}

void MenuSystem::handleDeleteRecord()
{
    std::cout << "\n--- Delete a Record ---" << std::endl;
    std::string idStr = getValidatedInput("Enter the Record ID to delete: ");
    int id = 0;

    try
    {
        id = std::stoi(idStr);
    }
    catch (...)
    {
        std::cout << "> Error: Invalid ID format! Must be an integer." << std::endl;
        return;
    }

    std::string result = controller.removeRecord(id);
    std::cout << "\n> " << result << std::endl;
}

void MenuSystem::handleFinancialSummary()
{
    std::cout << "\n--- Financial Summary (All-in-one) ---" << std::endl;
    std::cout << "(Time filter only. Press Enter to skip Start/End date)" << std::endl;

    std::string start = getValidatedInput("Enter Start Date (YYYY-MM-DD): ", true);
    std::string end = getValidatedInput("Enter End Date (YYYY-MM-DD): ", true);

    std::map<std::string, double> summary = controller.getPeriodSummary(start, end);

    if (summary.empty())
    {
        std::cout << "> No data available to generate summary for the selected period." << std::endl;
        if (!controller.getLastError().empty())
        {
            std::cout << "> Reason: " << controller.getLastError() << std::endl;
        }
        return;
    }

    std::cout << "-----------------------------------" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    // Print total income, expense, net balance first
    if (summary.count("total_income"))
        std::cout << "Total Income: $" << summary["total_income"] << std::endl;
    if (summary.count("total_expense"))
        std::cout << "Total Expense: $" << summary["total_expense"] << std::endl;
    if (summary.count("net_balance"))
        std::cout << "Net Balance: $" << summary["net_balance"] << std::endl;

    // Print breakdown by category if any
    bool hasCategory = false;
    for (const auto &pair : summary)
    {
        if (pair.first.rfind("category:", 0) == 0)
        {
            hasCategory = true;
            break;
        }
    }
    if (hasCategory)
    {
        std::cout << "\nBreakdown expense by category:" << std::endl;
        for (const auto &pair : summary)
        {
            if (pair.first.rfind("category:", 0) == 0)
            {
                std::string catName = pair.first.substr(9); // after "category:"
                std::cout << "  • " << catName << ": $" << pair.second << std::endl;
            }
        }
    }
    std::cout << "-----------------------------------" << std::endl;
}




void MenuSystem::displayCategoryMenu() {
    std::cout << "\n--- Category Management ---" << std::endl;
    std::cout << "1. List Categories" << std::endl;
    std::cout << "2. Add Category" << std::endl;
    std::cout << "3. Update Category" << std::endl;
    std::cout << "4. Delete Category" << std::endl;
    std::cout << "0. Back to Main Menu" << std::endl;
}

void MenuSystem::handleCategoryManagement()
{
    while (true)
    {
        displayCategoryMenu();
        std::string choice = getValidatedInput("Enter your choice (0-4): ");

        if (choice == "0")
        {
            return;
        }
        else if (choice == "1")
        {
            handleListCategories();
        }
        else if (choice == "2")
        {
            handleAddCategory();
        }
        else if (choice == "3")
        {
            handleUpdateCategory();
        }
        else if (choice == "4")
        {
            handleDeleteCategory();
        }
        else
        {
            std::cout << "Invalid Input! Please select a valid option (0-4)." << std::endl;
        }
    }
}

void MenuSystem::handleAddCategory() {
    std::cout << "\n--- Add Category ---" << std::endl;

    // 1. Category name (required)
    std::string name = getValidatedInput("Enter category name: ");
    // 2. Category type
    std::cout << "Category type:\n"
              << "  e - Expense\n"
              << "  i - Income\n";

    std::string typeStr;

    while (true) {
        typeStr = getValidatedInput("Enter type (e/i): ");
        if (typeStr == "e" || typeStr == "E" || typeStr == "i" || typeStr == "I") {
            break;
        }

        std::cout << "Invalid type. Please enter 'e' or 'i'." << std::endl;
    }

    bool isExpense = (typeStr == "e" || typeStr == "E");

    // 3. Optional budget and warning threshold
    std::cout << "Optional fields (press Enter to skip):" << std::endl;
    std::string budgetStr = getValidatedInput("Enter budget (>= 0, optional): ", true);
    std::string warningStr = getValidatedInput("Enter warning threshold (>= 0, optional): ", true);

    double budget = -1.0;
    double warningThreshold = -1.0;

    if (!budgetStr.empty()) {
        try
        {
            budget = std::stod(budgetStr);
            if (budget < 0.0)
            {
                std::cout << "> Error: Budget must be >= 0." << std::endl;
                return;
            }
        }
        catch (...)
        {
            std::cout << "> Error: Invalid budget format." << std::endl;
            return;
        }
    }

    if (!warningStr.empty()) {
        try
        {
            warningThreshold = std::stod(warningStr);
            if (warningThreshold < 0.0)
            {
                std::cout << "> Error: Warning threshold must be >= 0." << std::endl;
                return;
            }
        }
        catch (...)
        {
            std::cout << "> Error: Invalid warning threshold format." << std::endl;
            return;
        }
    }

    // UI-level friendly check before calling controller
    if (budget < 0.0 && warningThreshold >= 0.0)
    {
        std::cout << "> Error: You set warning threshold but no budget." << std::endl;
        return;
    }

    // 4. Call controller
    std::string result = controller.addCategory(name, isExpense, budget, warningThreshold);
    std::cout << "\n> " << result << std::endl;

    if (!controller.getLastError().empty())
    {
        std::cout << "> Reason: " << controller.getLastError() << std::endl;
    }
}

void MenuSystem::handleListCategories() const
{
    std::cout << "\n--- Category List ---" << std::endl;

    std::vector<Category> categories = controller.getCategories();
    if (categories.empty()) {
        std::cout << "> No categories found." << std::endl;
        if (!controller.getLastError().empty()) {
            std::cout << "> Reason: " << controller.getLastError() << std::endl;
        }
        return;
    }

    std::cout << std::fixed << std::setprecision(2);
    for (const auto &cat : categories)
    {
        std::string type = cat.getIsExpense() ? "Expense" : "Income";
        std::cout << "- Name: " << cat.getName()
                  << " | Type: " << type;

        if (cat.hasBudget()) {
            std::cout << " | Budget: $" << cat.getBudget()
                      << " | Warning: $" << cat.getWarningThreshold();
        }
        else {
            std::cout << " | Budget: N/A | Warning: N/A";
        }

        std::cout << std::endl;
    }
}

void MenuSystem::handleUpdateCategory()
{
    std::cout << "\n--- Update Category ---" << std::endl;

    std::string oldName = getValidatedInput("Enter category name to update: ");

    std::cout << "Press Enter to skip any field." << std::endl;
    std::string newName = getValidatedInput("Enter new name (optional): ", true);

    int isExpense = -1;
    while (true) {
        std::string typeStr = getValidatedInput("Enter new type (e/i, optional): ", true);
        if (typeStr.empty()) {
            break; // keep unchanged
        }
        if (typeStr == "e" || typeStr == "E") {
            isExpense = 1;
            break;
        }
        if (typeStr == "i" || typeStr == "I") {
            isExpense = 0;
            break;
        }
        std::cout << "Invalid type. Please enter e, i, or press Enter to skip." << std::endl;
    }

    double budget = -1.0;
    std::string budgetStr = getValidatedInput("Enter new budget (>= 0, optional): ", true);
    if (!budgetStr.empty())
    {
        try {
            budget = std::stod(budgetStr);
            if (budget < 0.0) {
                std::cout << "> Error: Budget must be >= 0." << std::endl;
                return;
            }
        }
        catch (...) {
            std::cout << "> Error: Invalid budget format." << std::endl;
            return;
        }
    }

    double warningThreshold = -1.0;
    std::string warningStr = getValidatedInput("Enter new warning threshold (>= 0, optional): ", true);
    if (!warningStr.empty()) {
        try {
            warningThreshold = std::stod(warningStr);
            if (warningThreshold < 0.0) {
                std::cout << "> Error: Warning threshold must be >= 0." << std::endl;
                return;
            }
        }
        catch (...) {
            std::cout << "> Error: Invalid warning threshold format." << std::endl;
            return;
        }
    }

    std::string result = controller.updateCategory(oldName, newName, isExpense, budget, warningThreshold);
    std::cout << "\n> " << result << std::endl;

    if (!controller.getLastError().empty()) {
        std::cout << "> Reason: " << controller.getLastError() << std::endl;
    }
}

void MenuSystem::handleDeleteCategory()
{
    std::cout << "\n--- Delete Category ---" << std::endl;

    std::string name = getValidatedInput("Enter category name to delete: ");
    std::string confirm = getValidatedInput("Are you sure? (y/n): ");

    if (!(confirm == "y" || confirm == "Y"))
    {
        std::cout << "> Deletion cancelled." << std::endl;
        return;
    }

    std::string result = controller.removeCategory(name);
    std::cout << "\n> " << result << std::endl;

    if (!controller.getLastError().empty())
    {
        std::cout << "> Reason: " << controller.getLastError() << std::endl;
    }
}



void MenuSystem::handleCurrentBudgetStatus() const {
    std::cout << "\n=== Budget Status Report ===\n";

    std::vector<BudgetStatus> statuses = controller.getCurrentBudgetStatus();

    if (statuses.empty()) {
        std::cout << "No active budgets found or no expense categories exist.\n";
        return;
    }
    renderBudgetStatus(statuses);
}

void MenuSystem::renderBudgetStatus(const std::vector<BudgetStatus>& statuses){
    const std::string COLOR_RESET   = "\033[0m";
    const std::string COLOR_GREEN   = "\033[32m";  // Safe
    const std::string COLOR_YELLOW  = "\033[33m";  // Warning
    const std::string COLOR_RED     = "\033[31m";  // Exceeded

    std::cout << std::left
              << std::setw(15) << "Category"
              << std::setw(10) << "Health"
              << std::setw(12) << "Budget"
              << std::setw(12) << "Spent"
              << std::setw(12) << "Remaining"
              << "Daily Rec." << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& status : statuses) {
        std::string colorCode = COLOR_RESET;
        std::string healthStr = "Unknown";

        switch (status.budgetHealth) {
            case BudgetHealth::Safe:
                colorCode = COLOR_GREEN;
                healthStr = "Safe";
                break;
            case BudgetHealth::Warning:
                colorCode = COLOR_YELLOW;
                healthStr = "Warning";
                break;
            case BudgetHealth::Exceeded:
                colorCode = COLOR_RED;
                healthStr = "Exceeded";
                break;
            case BudgetHealth::Unknown:
            default:
                colorCode = COLOR_RESET;
                healthStr = "Unknown";
                break;
        }

        std::cout << colorCode
                  << std::left << std::setw(15) << status.categoryName
                  << std::setw(10) << healthStr
                  << "$" << std::setw(11) << std::fixed << std::setprecision(2) << status.budgetLimit
                  << "$" << std::setw(11) << status.actualSpent
                  << "$" << std::setw(11) << status.remaining;

        if (status.dailyAvailable >= 0.0) {
            std::cout << "$" << status.dailyAvailable << " (" << status.daysRemaining << " days left)";
        } else {
            std::cout << "N/A";
        }

        std::cout << COLOR_RESET << "\n";
    }

    std::cout << std::string(75, '-') << "\n";
}

void MenuSystem::handleDistribution() const {
    std::cout << "\n=== Expense Distribution Analysis ===\n";

    std::string start, end;
    while (true)
    {
        std::cout << "Enter start date (YYYY-MM-DD) or leave blank for all: ";
        start = getValidatedDate();
        std::cout << "Enter end date (YYYY-MM-DD) or leave blank for all: ";
        end = getValidatedDate();

        if (!start.empty() && !end.empty() && start > end)
        {
            std::cout << "Start date cannot be later than end date. Please re-enter.\n";
            continue;
        }
        break;
    }

    auto distribution = controller.getExpenseDistribution(start, end);

    if (distribution.second.empty()) {
        std::string error = controller.getLastError();
        if (!error.empty()) {
            std::cout << "Notice: " << error << "\n";
        } else {
            std::cout << "No expense records found for the specified period.\n";
        }
        return;
    }

    renderDistribution(distribution);
}

void MenuSystem::renderDistribution(const std::pair<double, std::vector<CategoryDistItem>>& distribution) {
    double totalExpense = distribution.first;
    const auto& items = distribution.second;

    std::cout << "\nTotal Expense for period: $" << std::fixed << std::setprecision(2) << totalExpense << "\n";
    std::cout << std::string(75, '-') << "\n";

    std::cout << std::left
              << std::setw(20) << "Category"
              << std::setw(15) << "Amount"
              << std::setw(10) << "Percent"
              << "Visual Distribution\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& item : items) {
        int barLength = static_cast<int>((item.percentage / 100.0) * 25.0);
        std::string bar(barLength, '#');

        std::cout << std::left << std::setw(20) << item.category
                  << "$" << std::setw(14) << item.amount
                  << std::setw(6) << item.percentage << "%   "
                  << "|" << bar << "\n";
    }

    std::cout << std::string(75, '-') << "\n";
}

void MenuSystem::handleTrend() const {
    std::cout << "\n=== Trend Analysis ===\n";

    std::string start, end, cat, typeStr;
    int isExpense = -1;

    while (true)
    {
        std::cout << "Enter start date (YYYY-MM-DD) or leave blank for all: ";
        start = getValidatedDate();
        std::cout << "Enter end date (YYYY-MM-DD) or leave blank for all: ";
        end = getValidatedDate();

        if (!start.empty() && !end.empty() && start > end)
        {
            std::cout << "Start date cannot be later than end date. Please re-enter.\n";
            continue;
        }
        break;
    }

    std::cout << "Filter by type? (1 = Expense, 0 = Income, or leave blank for All): ";
    std::getline(std::cin, typeStr);
    if (!typeStr.empty()) {
        try {
            isExpense = std::stoi(typeStr);
            if (isExpense != 0 && isExpense != 1) {
                isExpense = -1;
            }
        } catch (...) {
            isExpense = -1;
        }
    }

    std::cout << "Enter category name or leave blank for all categories: ";
    std::getline(std::cin, cat);

    std::map<std::string, double> trendData = controller.getTrend(start, end, isExpense, cat);

    if (trendData.empty()) {
        std::string error = controller.getLastError();
        if (!error.empty()) {
            std::cout << "Notice: " << error << "\n";
        } else {
            std::cout << "No records found for the specified criteria.\n";
        }
        return;
    }

    renderTrend(trendData);
}

void MenuSystem::renderTrend(const std::map<std::string, double>& trendData) {
    double maxAmount = 0.0;
    for (const auto& pair : trendData) {
        if (pair.second > maxAmount) {
            maxAmount = pair.second;
        }
    }

    std::cout << "\nTrend Report (Monthly)\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left
              << std::setw(12) << "Month"
              << std::setw(15) << "Total Amount"
              << "Trend\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto& pair : trendData) {
        std::string month = pair.first;
        double amount = pair.second;

        int barLength = 0;
        if (maxAmount > 0) {
            barLength = static_cast<int>((amount / maxAmount) * 25.0);
        }
        std::string bar(barLength, '#');

        std::cout << std::left << std::setw(12) << month
                  << "$" << std::setw(14) << std::fixed << std::setprecision(2) << amount
                  << "|" << bar << "\n";
    }

    std::cout << std::string(60, '-') << "\n";
}

void MenuSystem::handleIncomeExpense() const {
    std::cout << "\n=== Income vs Expense Analysis ===\n";

    std::string start, end;

    while (true)
    {
        std::cout << "Enter start date (YYYY-MM-DD) or leave blank for all: ";
        start = getValidatedDate();
        std::cout << "Enter end date (YYYY-MM-DD) or leave blank for all: ";
        end = getValidatedDate();

        if (!start.empty() && !end.empty() && start > end)
        {
            std::cout << "Start date cannot be later than end date. Please re-enter.\n";
            continue;
        }
        break;
    }

    std::map<std::string, std::pair<double, double>> data = controller.getIncomeExpense(start, end);

    if (data.empty()) {
        std::string error = controller.getLastError();
        if (!error.empty()) {
            std::cout << "Notice: " << error << "\n";
        } else {
            std::cout << "No records found for the specified period.\n";
        }
        return;
    }

    renderIncomeExpense(data);
}

void MenuSystem::renderIncomeExpense(const std::map<std::string, std::pair<double, double>>& data) {
    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m"; // Positive balance
    const std::string COLOR_RED   = "\033[31m"; // Negative balance

    std::cout << "\nIncome vs Expense Report\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left
              << std::setw(12) << "Month"
              << std::setw(15) << "Income"
              << std::setw(15) << "Expense"
              << "Net Balance\n";
    std::cout << std::string(60, '-') << "\n";

    double totalIncome = 0.0;
    double totalExpense = 0.0;

    for (const auto& pair : data) {
        std::string month = pair.first;
        double income = pair.second.first;
        double expense = pair.second.second;
        double balance = income - expense;

        totalIncome += income;
        totalExpense += expense;

        std::string color = (balance >= 0) ? COLOR_GREEN : COLOR_RED;
        std::string sign = (balance >= 0) ? "+$" : "-$";

        std::cout << std::left << std::setw(12) << month
                  << "$" << std::setw(14) << std::fixed << std::setprecision(2) << income
                  << "$" << std::setw(14) << expense
                  << color << sign << std::abs(balance) << COLOR_RESET << "\n";
    }

    std::cout << std::string(60, '-') << "\n";
    double overallBalance = totalIncome - totalExpense;
    std::string overallColor = (overallBalance >= 0) ? COLOR_GREEN : COLOR_RED;
    std::string overallSign = (overallBalance >= 0) ? "+$" : "-$";

    std::cout << std::left << std::setw(12) << "TOTAL"
              << "$" << std::setw(14) << totalIncome
              << "$" << std::setw(14) << totalExpense
              << overallColor << overallSign << std::abs(overallBalance) << COLOR_RESET << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void MenuSystem::handleViewAllRecords() const {
    std::cout << "\n=== All Transactions ===\n";

    std::vector<Record> allRecords = controller.getRecords("", "", -1, "", 0.0);

    renderRecordTable(allRecords);
}

void MenuSystem::renderRecordTable(const std::vector<Record> &records) {
    if (records.empty()) {
        std::cout << "No records to display.\n";
        return;
    }

    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m"; // income
    const std::string COLOR_RED   = "\033[31m"; // expense

    std::cout << "\n" << std::string(75, '=') << "\n";
    std::cout << std::left
              << std::setw(8)  << "ID"
              << std::setw(14) << "Date"
              << std::setw(20) << "Category"
              << std::setw(12) << "Type"
              << "Amount\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& record : records) {
        bool isExp = record.getIsExpense();
        std::string typeStr = isExp ? "Expense" : "Income";
        std::string color = isExp ? COLOR_RED : COLOR_GREEN;
        std::string sign = isExp ? "-$" : "+$";

        std::cout << std::left
                  << std::setw(8)  << record.getId()
                  << std::setw(14) << record.getDate()
                  << std::setw(20) << record.getCategory()
                  << std::setw(12) << typeStr
                  << color << sign << std::fixed << std::setprecision(2) << record.getAmount()
                  << COLOR_RESET << "\n";
    }

    std::cout << std::string(75, '=') << "\n";
    std::cout << "Total records listed: " << records.size() << "\n";
}




// Helpers
std::string MenuSystem::getValidatedInput(std::string prompt, bool allowEmpty)
{
    std::string input;
    while (true)
    {
        std::cout << prompt;
        std::getline(std::cin, input);

        if (input.empty())
        {
            if (allowEmpty) {
                return "";
            }
            else {
                std::cout << "Input cannot be empty. Please try again." << std::endl;
                continue;
            }
        }

        return input;
    }
}

double MenuSystem::getValidatedAmount(bool allowEmpty)
{
    std::string input;
    double amount = 0.0;

    while (true)
    {
        std::cout << "Enter amount (positive number(Example: 50.00)" << (allowEmpty ? ", or press Enter to skip" : "") << "): ";
        std::getline(std::cin, input);

        // When users press Enter, it returns the default value 0.0.
        if (input.empty() && allowEmpty)
        {
            return 0.0;
        }

        try
        {
            amount = std::stod(input);
            if (amount > 0)
            {
                break;
            }
            else
            {
                std::cout << "Amount must be greater than 0." << std::endl;
            }
        }
        catch (...)
        {
            std::cout << "Invalid number format. Please try again." << std::endl;
        }
    }

    return amount;
}

std::string MenuSystem::getValidatedDate() const
{
    std::vector<Record> allRecords = controller.getRecords("", "", -1, "", 0.0);

    std::string minDate;
    std::string maxDate;
    if (!allRecords.empty())
    {
        minDate = allRecords.front().getDate();
        maxDate = allRecords.front().getDate();
        for (const auto &rec : allRecords)
        {
            if (rec.getDate() < minDate)
            {
                minDate = rec.getDate();
            }
            if (rec.getDate() > maxDate)
            {
                maxDate = rec.getDate();
            }
        }
    }

    while (true)
    {
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) {
            return "";
        }

        std::string error;
        if (!Record::validateData(input, 1.0, error))
        {
            std::cout << "Invalid date: " << error << std::endl;
            std::cout << "Please enter date again (YYYY-MM-DD), or press Enter to skip: ";
            continue;
        }

        if (!allRecords.empty() && (input < minDate || input > maxDate))
        {
            std::cout << "Date must be within existing data range [" << minDate << " to " << maxDate << "]." << std::endl;
            std::cout << "Please enter date again (YYYY-MM-DD), or press Enter to skip: ";
            continue;
        }

        return input;
    }
}
