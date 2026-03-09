#include "MenuSystem.hpp"
#include <iostream>
#include <string>
#include <iomanip>

MenuSystem::MenuSystem(LedgerController &ctrl) : controller(ctrl) {}

void MenuSystem::run()
{
    bool isRunning = true;
    std::string choice;

    std::cout << "\n=== Welcome to C-The-Cash ===" << std::endl;
    while (isRunning)
    {
        displayMainMenu();

        std::cout << "\nPlease enter your choice (0-7): ";
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

void MenuSystem::displayMainMenu()
{
    std::cout << "\n-----------------------------------" << std::endl;
    std::cout << "1. Add a New Record" << std::endl;
    std::cout << "2. Add Records by File (Batch Import)" << std::endl;
    std::cout << "3. View/Search Records" << std::endl;
    std::cout << "4. Update a Record" << std::endl;
    std::cout << "5. Delete a Record" << std::endl;
    std::cout << "6. Financial Summary (All-in-one)" << std::endl;
    std::cout << "7. Simple Total" << std::endl;
    std::cout << "0. Exit and Save" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

void MenuSystem::handleCommand(std::string cmd)
{
    if (cmd == "1")
    {
        handleAddRecord();
    }
    else if (cmd == "2")
    {
        handleAddRecordByFile();
    }
    else if (cmd == "3")
    {
        handleSearchRecords();
    }
    else if (cmd == "4")
    {
        handleUpdateRecord();
    }
    else if (cmd == "5")
    {
        handleDeleteRecord();
    }
    else if (cmd == "6")
    {
        handleFinancialSummary();
    }
    else if (cmd == "7")
    {
        handleSimpleTotal();
    }
    else
    {
        std::cout << "Invalid Input! Please select a valid option (0-7)." << std::endl;
    }
}

void MenuSystem::exitMenu()
{
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

    std::string result = controller.addRecord(date, amount, isExpense, "Other");

    std::cout << "\n> " << result << std::endl;
}

void MenuSystem::handleAddRecordByFile()
{
    std::cout << "\n--- Add Records by File (Batch Import) ---" << std::endl;
    std::string filePath = getValidatedInput("Enter import file path: ");

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

    // Step 4: Get minimum amount (optional)
    double minAmount = getValidatedAmount(true);

    std::vector<Record> results = controller.getRecords(start, end, isExpense, "", minAmount);

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

    // Call controller with all filters
    std::string result = controller.getTotal(start, end, isExpense, "");
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

    std::string result = controller.updateRecord(id, date, amount, isExpense, "Other");
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
        std::cout << "\nBreakdown by Category:" << std::endl;
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
            if (allowEmpty)
            {
                return "";
            }
            else
            {
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
        std::cout << "Enter amount (positive number" << (allowEmpty ? ", or press Enter to skip" : "") << "): ";
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
