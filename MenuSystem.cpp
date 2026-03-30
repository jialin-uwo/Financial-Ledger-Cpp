#include <cstdio>
#ifdef __cplusplus
extern "C"
{
#endif
    namespace
    {
        FILE *openGnuplotPipe();
        bool shouldUseGnuplot();
    }
#ifdef __cplusplus
}
#endif

#include "MenuSystem.hpp"
#include "Record.h"
#include "LedgerController.hpp"
#include "BudgetStatus.h"
#include "BudgetHealth.h"
#include "FinancialAnalyzer.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <cstdlib> // getenv
#include <cstdio>  // popen, pclose
#include <cmath>
void MenuSystem::handleTrend() const
{
    std::cout << "\n=== Trend Analysis ===\n";

    std::string start, end, cat, typeStr;
    int isExpense = 1;

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

    std::cout << "Filter by type? (1 = Expense, 0 = Income, or leave blank for Expense): ";
    std::getline(std::cin, typeStr);
    if (!typeStr.empty())
    {
        try
        {
            isExpense = std::stoi(typeStr);
            if (isExpense != 0 && isExpense != 1)
            {
                isExpense = 1;
            }
        }
        catch (...)
        {
            isExpense = 1;
        }
    }

    std::cout << "Enter category name or leave blank for all categories: ";
    std::getline(std::cin, cat);

    std::map<std::string, double> trendData = controller.getTrend(start, end, isExpense, cat);
    std::string error = controller.getLastError().message;
    if (!error.empty())
    {
        std::cout << "Notice: " << error << "\n";
        return;
    }
    if (trendData.empty())
    {
        std::cout << "No records found for the specified criteria.\n";
        return;
    }

    renderTrend(trendData, cat);
}

void MenuSystem::renderTrend(const std::map<std::string, double> &trendData, const std::string &category)
{
    double maxAmount = 0.0;
    for (const auto &pair : trendData)
    {
        if (pair.second > maxAmount)
            maxAmount = pair.second;
    }

    std::string chartTitle = "Monthly Trend Analysis";
    if (!category.empty()) {
        chartTitle += " (" + category + ")";
    }

    // --- Gnuplot Rendering Branch ---
    if (shouldUseGnuplot())
    {
        FILE *pipe = openGnuplotPipe();
        if (pipe)
        {
            fprintf(pipe, "set title '%s'\n", chartTitle.c_str());
            fprintf(pipe, "set style data linespoints\n");
            fprintf(pipe, "set xtics rotate by -45\n");
            fprintf(pipe, "set grid\n");
            fprintf(pipe, "set ylabel 'Amount ($)'\n");
            fprintf(pipe, "set offsets 0, 0, graph 0.25, 0\n");
            fprintf(pipe, "plot '-' using 0:2:xtic(1) with linespoints title 'Trend' lw 2 pt 7, \\\n");
            fprintf(pipe, "     '-' using 0:2:(sprintf(\"$%%.2f\", $2)) with labels rotate by 70 left offset char 0, 1.0 notitle\n");

            for (const auto &pair : trendData)
            {
                fprintf(pipe, "\"%s\" %f\n", pair.first.c_str(), pair.second);
            }
            fprintf(pipe, "e\n");

            for (const auto &pair : trendData)
            {
                fprintf(pipe, "\"%s\" %f\n", pair.first.c_str(), pair.second);
            }
            fprintf(pipe, "e\n");

            pclose(pipe);

            std::cout << "\n>> Trend chart opened in external gnuplot window.\n";
            return;
        }
    }

    // --- ASCII Rollback Rendering Branch ---
    std::cout << "\n" << chartTitle << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left
              << std::setw(12) << "Month"
              << std::setw(15) << "Total Amount"
              << "Trend\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto &pair : trendData)
    {
        std::string month = pair.first;
        double amount = pair.second;

        int barLength = 0;
        if (maxAmount > 0)
        {
            barLength = static_cast<int>((amount / maxAmount) * 25.0);
        }
        std::string bar(barLength, '#');

        std::cout << std::left << std::setw(12) << month
                  << "$" << std::setw(14) << std::fixed << std::setprecision(2) << amount
                  << "|" << bar << "\n";
    }
    std::cout << std::string(60, '-') << "\n";
}

void MenuSystem::renderRecordTable(const std::vector<Record> &records)
{
    if (records.empty())
    {
        std::cout << "No records to display.\n";
        return;
    }

    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m"; // income
    const std::string COLOR_RED = "\033[31m";   // expense

    std::cout << "\n"
              << std::string(75, '=') << "\n";
    std::cout << std::left
              << std::setw(8) << "ID"
              << std::setw(14) << "Date"
              << std::setw(20) << "Category"
              << std::setw(12) << "Type"
              << "Amount\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto &record : records)
    {
        bool isExp = record.getIsExpense();
        std::string typeStr = isExp ? "Expense" : "Income";
        std::string color = isExp ? COLOR_RED : COLOR_GREEN;
        std::string sign = isExp ? "-$" : "+$";

        std::cout << std::left
                  << std::setw(8) << record.getId()
                  << std::setw(14) << record.getDate()
                  << std::setw(20) << record.getCategory()
                  << std::setw(12) << typeStr
                  << color << sign << std::fixed << std::setprecision(2) << record.getAmount()
                  << COLOR_RESET << "\n";
    }

    std::cout << std::string(75, '=') << "\n";
    std::cout << "Total records listed: " << records.size() << "\n";
}

void MenuSystem::handleSearchRecordById()
{
    std::cout << "\n--- Search Record by ID ---\n";
    std::string idStr = getValidatedInput("Enter the Record ID to search: ");
    int id = 0;
    try
    {
        id = std::stoi(idStr);
    }
    catch (...)
    {
        std::cout << "> Error: Invalid ID format! Must be an integer.\n";
        return;
    }

    // 用getRecords全量查找
    std::vector<Record> all = controller.getRecords();
    auto it = std::find_if(all.begin(), all.end(), [id](const Record &r)
                           { return r.getId() == id; });
    if (it == all.end())
    {
        std::cout << "> No record found with ID " << id << ".\n";
        return;
    }
    std::vector<Record> single{*it};
    renderRecordTable(single);
}

namespace
{
    std::string detectGnuplotExecutable()
    {
        const char *candidates[] = {
            "/opt/homebrew/bin/gnuplot", // Apple Silicon Homebrew default
            "/usr/local/bin/gnuplot",    // Intel Homebrew default
            "gnuplot"                    // PATH fallback
        };

        char buffer[64];
        for (const char *candidate : candidates)
        {
            std::string cmd = std::string(candidate) + " --version 2>/dev/null";
            FILE *pipe = popen(cmd.c_str(), "r");
            if (!pipe)
            {
                continue;
            }

            bool hasOutput = (fgets(buffer, sizeof(buffer), pipe) != nullptr);
            int status = pclose(pipe);
            if (status == 0 && hasOutput)
            {
                return candidate;
            }
        }

        return "";
    }

    const std::string &getGnuplotExecutable();

    // Escape label text so quotes/backslashes do not break gnuplot data parsing.
    std::string escapeForGnuplotLabel(const std::string &input)
    {
        std::string escaped;
        escaped.reserve(input.size());
        for (char ch : input)
        {
            if (ch == '\\' || ch == '"')
            {
                escaped.push_back('\\');
            }
            escaped.push_back(ch);
        }
        return escaped;
    }

    const std::string &getGnuplotExecutable()
    {
        static const std::string executable = detectGnuplotExecutable();
        return executable;
    }

    FILE *openGnuplotPipe()
    {
        const std::string &executable = getGnuplotExecutable();
        if (executable.empty())
        {
            return nullptr;
        }

        const std::string command = executable + " -persist";
        return popen(command.c_str(), "w");
    }

    bool shouldUseGnuplot()
    {
        const char *mode = std::getenv("LEDGER_PLOT_MODE");
        std::string modeStr = mode ? mode : "auto";

        if (modeStr == "ascii")
        {
            return false;
        }

        if (!getGnuplotExecutable().empty())
        {
            return true;
        }

        if (modeStr == "gnuplot")
        {
            std::cout << "\n[Warning] LEDGER_PLOT_MODE is set to 'gnuplot', but gnuplot is not found. Falling back to ASCII.\n";
        }
        return false;
    }
}

MenuSystem::MenuSystem(LedgerController &ctrl) : controller(ctrl) {}

void MenuSystem::run()
{
    bool isRunning = true;
    std::string choice;

    std::cout << "\n===============================================" << std::endl;
    std::cout << "   Welcome to C-The-Cash Personal Ledger!" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "Type 'h' at any menu for help or instructions." << std::endl;
    if (!shouldUseGnuplot())
    {
        std::cout << "\n[Notice] gnuplot is not detected. Some charts will be shown in ASCII mode." << std::endl;
        std::cout << "For better chart visualization, please install gnuplot manually:" << std::endl;
        std::cout << "  - macOS:  brew install gnuplot" << std::endl;
        std::cout << "  - Ubuntu: sudo apt-get install gnuplot" << std::endl;
        std::cout << "  - Windows: download from http://www.gnuplot.info/" << std::endl;
        std::cout << "To force ASCII mode, set the environment variable: LEDGER_PLOT_MODE=ascii" << std::endl;
    }
    while (isRunning)
    {
        displayMainMenu();
        std::getline(std::cin, choice);
        if (choice == "0")
        {
            std::cout << "Exiting and saving..." << std::endl;
            break;
        }
        else if (choice == "1")
        {
            handleSearchRecords();
        }
        else if (choice == "2")
        {
            handleSearchRecordById();
        }
        else if (choice == "3")
        {
            // Record Management 子菜单
            while (true)
            {
                std::cout << "\n--- Record Management ---" << std::endl;
                std::cout << "[1] Add Record" << std::endl;
                std::cout << "[2] Import Records by File" << std::endl;
                std::cout << "[3] Update Record" << std::endl;
                std::cout << "[4] Delete Record" << std::endl;
                std::cout << "[0] Back to Main Menu" << std::endl;
                std::cout << "Enter your choice: ";
                std::string sub;
                std::getline(std::cin, sub);
                if (sub == "0")
                    break;
                else if (sub == "1")
                    handleAddRecord();
                else if (sub == "2")
                    handleAddRecordByFile();
                else if (sub == "3")
                    handleUpdateRecord();
                else if (sub == "4")
                    handleDeleteRecord();
                else
                    std::cout << "Invalid Input!" << std::endl;
            }
        }
        else if (choice == "4")
        {
            // Financial Analytics & Reports 子菜单
            while (true)
            {
                std::cout << "\n--- Financial Analytics & Reports ---" << std::endl;
                std::cout << "[1] Financial Summary (All-in-one)" << std::endl;
                std::cout << "[2] Simple Total" << std::endl;
                std::cout << "[3] Budget Status" << std::endl;
                std::cout << "[4] Expense Distribution" << std::endl;
                std::cout << "[5] Trend Analysis" << std::endl;
                std::cout << "[6] Income vs Expense Comparison" << std::endl;
                std::cout << "[0] Back to Main Menu" << std::endl;
                std::cout << "Enter your choice: ";
                std::string sub;
                std::getline(std::cin, sub);
                if (sub == "0")
                    break;
                else if (sub == "1")
                    handleFinancialSummary();
                else if (sub == "2")
                    handleSimpleTotal();
                else if (sub == "3")
                    handleCurrentBudgetStatus();
                else if (sub == "4")
                    handleDistribution();
                else if (sub == "5")
                    handleTrend();
                else if (sub == "6")
                    handleIncomeExpense();
                else
                    std::cout << "Invalid Input!" << std::endl;
            }
        }
        else if (choice == "5")
        {
            // Category Management 子菜单
            while (true)
            {
                std::cout << "\n--- Category Management ---" << std::endl;
                std::cout << "[1] List Categories" << std::endl;
                std::cout << "[2] Add Category" << std::endl;
                std::cout << "[3] Update Category" << std::endl;
                std::cout << "[4] Delete Category" << std::endl;
                std::cout << "[0] Back to Main Menu" << std::endl;
                std::cout << "Enter your choice: ";
                std::string sub;
                std::getline(std::cin, sub);
                if (sub == "0")
                    break;
                else if (sub == "1")
                    handleListCategories();
                else if (sub == "2")
                    handleAddCategory();
                else if (sub == "3")
                    handleUpdateCategory();
                else if (sub == "4")
                    handleDeleteCategory();
                else
                    std::cout << "Invalid Input!" << std::endl;
            }
        }
        else
        {
            std::cout << "Invalid Input! Please select a valid option." << std::endl;
        }
    }
}

void MenuSystem::displayReportMenu()
{
    std::cout << "\n========== FINANCIAL REPORTS ==========" << std::endl;
    std::cout << "[1] Financial Summary (All-in-one)" << std::endl;
    std::cout << "[2] Simple Total" << std::endl;
    std::cout << "[3] Current Budget Status" << std::endl;
    std::cout << "[4] Expense Distribution" << std::endl;
    std::cout << "[5] Trend Analysis" << std::endl;
    std::cout << "[6] Income vs Expense Comparison" << std::endl;
    std::cout << "[0] Back to Main Menu" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Tip: Enter the number for the report, or 'h' for help." << std::endl;
}

void MenuSystem::handleReportManagement()
{
    while (true)
    {
        displayReportMenu();
        std::string choice = getValidatedInput("Enter your choice (0-6): ");

        if (choice == "0")
        {
            return;
        }
        else if (choice == "1")
        {
            handleFinancialSummary();
        }
        else if (choice == "2")
        {
            handleSimpleTotal();
        }
        else if (choice == "3")
        {
            handleCurrentBudgetStatus();
        }
        else if (choice == "4")
        {
            handleDistribution();
        }
        else if (choice == "5")
        {
            handleTrend();
        }
        else if (choice == "6")
        {
            handleIncomeExpense();
        }
        else
        {
            std::cout << "Invalid Input! Please select a valid option (0-6)." << std::endl;
        }
    }
}

void MenuSystem::handleAddRecord()
{
    std::cout << "\n--- Add New Record ---" << std::endl;

    std::string date = getValidatedInput("Enter date (YYYY-MM-DD): ");

    double amount = getValidatedAmount();

    std::string typeStr = getValidatedInput("Is it an expense? (y/n): ");

    bool isExpense = (typeStr == "y" || typeStr == "Y");

    std::string category = getValidatedInput("Enter category (optional, press Enter to auto-assign): ", true);

    Result result = controller.addRecord(date, amount, isExpense, category);
    std::cout << "\n> " << result.message << std::endl;
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

    Result result = controller.addRecordsByFile(filePath);
    std::cout << "\n> " << result.message << std::endl;
    if (!result.ok())
    {
        std::cout << "> Reason: " << result.message << std::endl;
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
        if (!controller.getLastError().message.empty())
        {
            std::cout << "> Reason: " << controller.getLastError().message << std::endl;
        }
    }
    else
    {
        renderRecordTable(results);
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
    Result result = controller.getTotal(start, end, isExpense, category);
    std::cout << "\n> " << result.message << std::endl;
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

    Result result = controller.updateRecord(id, date, amount, isExpense, category);
    std::cout << "\n> " << result.message << std::endl;
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

    Result result = controller.removeRecord(id);
    std::cout << "\n> " << result.message << std::endl;
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
        if (!controller.getLastError().message.empty())
        {
            std::cout << "> Reason: " << controller.getLastError().message << std::endl;
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

    // Print breakdown by category if any (expense and income)
    bool hasExpenseCategory = false;
    bool hasIncomeCategory = false;
    for (const auto &pair : summary)
    {
        if (pair.first.rfind("category:", 0) == 0)
            hasExpenseCategory = true;
        if (pair.first.rfind("income_category:", 0) == 0)
            hasIncomeCategory = true;
    }
    if (hasExpenseCategory)
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
    if (hasIncomeCategory)
    {
        std::cout << "\nBreakdown income by category:" << std::endl;
        for (const auto &pair : summary)
        {
            if (pair.first.rfind("income_category:", 0) == 0)
            {
                std::string catName = pair.first.substr(15); // after "income_category:"
                std::cout << "  • " << catName << ": $" << pair.second << std::endl;
            }
        }
    }
    std::cout << "-----------------------------------" << std::endl;
}

void MenuSystem::displayCategoryMenu()
{
    std::cout << "\n========== CATEGORY MANAGEMENT ==========" << std::endl;
    std::cout << "[1] List Categories" << std::endl;
    std::cout << "[2] Add Category" << std::endl;
    std::cout << "[3] Update Category" << std::endl;
    std::cout << "[4] Delete Category" << std::endl;
    std::cout << "[0] Back to Main Menu" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Tip: Enter the number for the operation, or 'h' for help." << std::endl;
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

void MenuSystem::handleAddCategory()
{
    std::cout << "\n--- Add Category ---" << std::endl;
    std::cout << "(输入 h 可随时查看帮助)" << std::endl;

    // 1. Category name (required)
    std::string name;
    while (true)
    {
        name = getValidatedInput("Enter category name: ");
        if (name == "h" || name == "H")
        {
            std::cout << "类别名建议不超过16字符，不能留空。" << std::endl;
            continue;
        }
        if (name.empty())
        {
            std::cout << "> 类别名不能为空，请重新输入。" << std::endl;
            continue;
        }
        break;
    }

    // 2. Category type
    std::cout << "Category type:\n"
              << "  e - Expense (支出)\n"
              << "  i - Income (收入)\n";

    std::string typeStr;
    bool isExpense = true;
    while (true)
    {
        typeStr = getValidatedInput("Enter type (e/i): ");
        if (typeStr == "h" || typeStr == "H")
        {
            std::cout << "e 表示支出类别，i 表示收入类别。" << std::endl;
            continue;
        }
        if (typeStr == "e" || typeStr == "E")
        {
            isExpense = true;
            break;
        }
        if (typeStr == "i" || typeStr == "I")
        {
            isExpense = false;
            break;
        }
        std::cout << "无效类型，请输入 'e'（支出）或 'i'（收入），或输入 h 获取帮助。" << std::endl;
    }

    // 3. Optional budget and warning threshold
    double budget = -1.0;
    double warningThreshold = -1.0;
    if (isExpense)
    {
        std::cout << "可选字段（直接回车跳过）：" << std::endl;
        std::string budgetStr = getValidatedInput("Enter budget for this category (>= 0, optional): ", true);
        if (budgetStr == "h" || budgetStr == "H")
        {
            std::cout << "预算为该类别每月可用金额，留空表示不设预算。" << std::endl;
            budgetStr.clear();
        }
        if (!budgetStr.empty())
        {
            try
            {
                budget = std::stod(budgetStr);
                if (budget < 0.0)
                {
                    std::cout << "> 错误：预算必须 >= 0。" << std::endl;
                    return;
                }
            }
            catch (...)
            {
                std::cout << "> 错误：预算格式无效，请输入数字。" << std::endl;
                return;
            }
        }

        std::string warningStr = getValidatedInput("Enter warning threshold that can trigger an alarm (Default value: 70% of budget)(>= 0, optional): ", true);
        if (warningStr == "h" || warningStr == "H")
        {
            std::cout << "预警值为预算的提醒线，留空默认70%，必须小于等于预算。" << std::endl;
            warningStr.clear();
        }
        if (!warningStr.empty())
        {
            try
            {
                warningThreshold = std::stod(warningStr);
                if (warningThreshold < 0.0)
                {
                    std::cout << "> 错误：预警值必须 >= 0。" << std::endl;
                    return;
                }
            }
            catch (...)
            {
                std::cout << "> 错误：预警值格式无效，请输入数字。" << std::endl;
                return;
            }
        }

        if (budget < 0.0 && warningThreshold >= 0.0)
        {
            std::cout << "> 错误：设置了预警值但未设置预算。" << std::endl;
            return;
        }
    }
    else
    {
        std::cout << "收入类别不支持预算和预警设置，直接回车继续。" << std::endl;
    }

    // 4. Call controller
    Result result = controller.addCategory(name, isExpense, budget, warningThreshold);
    if (result.ok())
    {
        std::cout << "\n> 操作成功！" << std::endl;
    }
    else
    {
        std::cout << "\n> 操作失败，原因：" << result.message << std::endl;
    }
}

void MenuSystem::handleListCategories()
{
    std::cout << "\n========== Category List ==========" << std::endl;
    std::vector<Category> categories = controller.getCategories();
    if (categories.empty())
    {
        std::cout << "> No categories found. Please add a category first!" << std::endl;
        if (!controller.getLastError().message.empty())
        {
            std::cout << "> Reason: " << controller.getLastError().message << std::endl;
        }
        return;
    }

    // 表头
    std::cout << std::left
              << std::setw(4) << "No."
              << std::setw(18) << "Name"
              << std::setw(10) << "Type"
              << std::setw(14) << "Budget"
              << std::setw(14) << "Warning" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    int idx = 1;
    for (const auto &cat : categories)
    {
        std::string type = cat.getIsExpense() ? "Expense" : "Income";
        std::string budgetStr = (cat.getIsExpense() && cat.hasBudget()) ? ("$" + std::to_string(cat.getBudget())) : "N/A";
        std::string warnStr = (cat.getIsExpense() && cat.hasBudget() && cat.getWarningThreshold() >= 0.0) ? ("$" + std::to_string(cat.getWarningThreshold())) : "N/A";
        std::cout << std::setw(4) << idx++
                  << std::setw(18) << cat.getName().substr(0, 16)
                  << std::setw(10) << type
                  << std::setw(14) << budgetStr
                  << std::setw(14) << warnStr << std::endl;
    }
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Tip: Use the category name when adding or editing records.\n";
    std::cout << "      For best results, keep names within 16 characters.\n";
}

void MenuSystem::handleUpdateCategory()
{
    std::cout << "\n--- Update Category ---" << std::endl;

    std::string oldName = getValidatedInput("Enter category name to update: ");

    std::cout << "Press Enter to skip any field." << std::endl;
    std::string newName = getValidatedInput("Enter new name (optional): ", true);

    int isExpense = -1;
    while (true)
    {
        std::string typeStr = getValidatedInput("Enter new type (e/i, optional): ", true);
        if (typeStr.empty())
        {
            break; // keep unchanged
        }
        if (typeStr == "e" || typeStr == "E")
        {
            isExpense = 1;
            break;
        }
        if (typeStr == "i" || typeStr == "I")
        {
            isExpense = 0;
            break;
        }
        std::cout << "Invalid type. Please enter e, i, or press Enter to skip." << std::endl;
    }

    double budget = -1.0;
    std::string budgetStr = getValidatedInput("Enter new budget for this category (Default value: 70% of budget)(>= 0, optional): ", true);
    if (!budgetStr.empty())
    {
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

    double warningThreshold = -1.0;
    std::string warningStr = getValidatedInput("Enter new warning threshold that can trigger an alarm (>= 0, optional): ", true);
    if (!warningStr.empty())
    {
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

    Result result = controller.updateCategory(oldName, newName, isExpense, budget, warningThreshold);
    std::cout << "\n> " << result.message << std::endl;
    if (!result.ok())
    {
        std::cout << "> Reason: " << result.message << std::endl;
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

    Result result = controller.removeCategory(name);
    std::cout << "\n> " << result.message << std::endl;
    if (!result.ok())
    {
        std::cout << "> Reason: " << result.message << std::endl;
    }
}

void MenuSystem::handleCurrentBudgetStatus()
{
    std::cout << "\n=== Budget Status Report ===\n";

    std::vector<BudgetStatus> statuses = controller.getCurrentBudgetStatus();

    if (statuses.empty())
    {
        std::cout << "No active budgets found or no expense categories exist.\n";
        return;
    }
    renderBudgetStatus(statuses);
}

void MenuSystem::renderBudgetStatus(const std::vector<BudgetStatus> &statuses)
{
    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m";  // Safe
    const std::string COLOR_YELLOW = "\033[33m"; // Warning
    const std::string COLOR_RED = "\033[31m";    // Exceeded

    std::cout << std::left
              << std::setw(15) << "Category"
              << std::setw(10) << "Health"
              << std::setw(12) << "Budget"
              << std::setw(12) << "Spent"
              << std::setw(12) << "Remaining"
              << "Daily Rec." << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto &status : statuses)
    {
        std::string colorCode = COLOR_RESET;
        std::string healthStr = "Unknown";

        switch (status.budgetHealth)
        {
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

        if (status.dailyAvailable >= 0.0)
        {
            std::cout << "$" << status.dailyAvailable << " (" << status.daysRemaining << " days left)";
        }
        else
        {
            std::cout << "N/A";
        }

        std::cout << COLOR_RESET << "\n";
    }

    std::cout << std::string(75, '-') << "\n";
}

void MenuSystem::handleDistribution()
{
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

    if (distribution.second.empty())
    {
        std::string error = controller.getLastError().message;
        if (!error.empty())
        {
            std::cout << "Notice: " << error << "\n";
        }
        else
        {
            std::cout << "No expense records found for the specified period.\n";
        }
        return;
    }

    renderDistribution(distribution);
}

void MenuSystem::renderDistribution(const std::pair<double, std::vector<CategoryDistItem>> &distribution)
{
    double totalExpense = distribution.first;
    const auto &items = distribution.second;

    std::cout << "\nTotal Expense for period: $" << std::fixed << std::setprecision(2) << totalExpense << "\n";
    std::cout << std::string(75, '-') << "\n";

    // --- Gnuplot Rendering Branch ---
    if (shouldUseGnuplot())
    {
        FILE *pipe = openGnuplotPipe();
        if (pipe)
        {
            fprintf(pipe, "set title 'Expense Distribution (Pie Chart)'\n");
            fprintf(pipe, "set size square\n");
            fprintf(pipe, "unset key\n");
            fprintf(pipe, "unset border\n");
            fprintf(pipe, "unset tics\n");

            // Cover a suitable range to accommodate the larger circle and labels
            fprintf(pipe, "set xrange [-3.5:3.5]\n");
            fprintf(pipe, "set yrange [-3.5:3.5]\n");
            fprintf(pipe, "set style fill solid 1.0 border -1\n");

            fprintf(pipe, "plot '-' using 1:2:3:4:5:6 with circles lc var notitle, \\\n");
            fprintf(pipe, "     '-' using 1:2:3:4 with vectors nohead lc rgb 'black' notitle, \\\n");
            fprintf(pipe, "     '-' using 1:2:3 with labels left notitle, \\\n");
            fprintf(pipe, "     '-' using 1:2:3 with labels right notitle\n");

            double totalExpense = distribution.first;
            if (totalExpense <= 0.0) totalExpense = 1.0;

            const double PI = 3.14159265358979323846;

            // 1st data block: Circle slices (radius increased to 2.2)
            double currentAngle = 0.0;
            int colorIndex = 1;
            for (const auto &item : items)
            {
                double ratio = item.amount / totalExpense;
                double sliceAngle = ratio * 360.0;
                if (sliceAngle > 0.0) {
                    // Third parameter 2.2 is the radius
                    fprintf(pipe, "0 0 2.2 %f %f %d\n", currentAngle, currentAngle + sliceAngle, colorIndex);
                    currentAngle += sliceAngle;
                }
                colorIndex++;
            }
            fprintf(pipe, "e\n");

            // 2nd data block: Vector guidelines
            currentAngle = 0.0;
            for (const auto &item : items)
            {
                double ratio = item.amount / totalExpense;
                double sliceAngle = ratio * 360.0;

                if (sliceAngle > 0.0) {
                    double midAngle = currentAngle + sliceAngle / 2.0;
                    double rad = midAngle * PI / 180.0;

                    double r_start = 2.2;  // Starting at enlarged circle edge
                    double r_end = 2.5;    // Ending vector outside

                    double vx = r_start * std::cos(rad);
                    double vy = r_start * std::sin(rad);
                    double vdx = (r_end - r_start) * std::cos(rad);
                    double vdy = (r_end - r_start) * std::sin(rad);

                    fprintf(pipe, "%f %f %f %f\n", vx, vy, vdx, vdy);
                    currentAngle += sliceAngle;
                }
            }
            fprintf(pipe, "e\n");

            // 3rd data block: Labels on the right side (align left, extend outward)
            bool hasRightSideLabels = false;
            currentAngle = 0.0;
            for (const auto &item : items)
            {
                double ratio = item.amount / totalExpense;
                double sliceAngle = ratio * 360.0;

                if (sliceAngle > 0.0) {
                    double midAngle = currentAngle + sliceAngle / 2.0;
                    double rad = midAngle * PI / 180.0;

                    // cos(rad) >= 0 -> Right hemisphere
                    if (std::cos(rad) >= 0) {
                        double label_r = 2.55;
                        double lx = label_r * std::cos(rad);
                        double ly = label_r * std::sin(rad);

                        double displayPercent = ratio * 100.0;
                        const std::string label = escapeForGnuplotLabel(item.category);
                        fprintf(pipe, "%f %f \"%s\\n$%.2f, %.1f%%\"\n", lx, ly, label.c_str(), item.amount, displayPercent);
                        hasRightSideLabels = true;
                    }
                    currentAngle += sliceAngle;
                }
            }
            if (!hasRightSideLabels) fprintf(pipe, "0 0 \"\"\n");
            fprintf(pipe, "e\n");

            // 4th data block: Labels on the left side (align right, extend outward)
            bool hasLeftSideLabels = false;
            currentAngle = 0.0;
            for (const auto &item : items)
            {
                double ratio = item.amount / totalExpense;
                double sliceAngle = ratio * 360.0;

                if (sliceAngle > 0.0) {
                    double midAngle = currentAngle + sliceAngle / 2.0;
                    double rad = midAngle * PI / 180.0;

                    // cos(rad) < 0 -> Left hemisphere
                    if (std::cos(rad) < 0) {
                        double label_r = 2.55;
                        double lx = label_r * std::cos(rad);
                        double ly = label_r * std::sin(rad);

                        double displayPercent = ratio * 100.0;
                        const std::string label = escapeForGnuplotLabel(item.category);
                        fprintf(pipe, "%f %f \"%s\\n$%.2f, %.1f%%\"\n", lx, ly, label.c_str(), item.amount, displayPercent);
                        hasLeftSideLabels = true;
                    }
                    currentAngle += sliceAngle;
                }
            }
            if (!hasLeftSideLabels) fprintf(pipe, "0 0 \"\"\n");
            fprintf(pipe, "e\n");

            pclose(pipe);

            std::cout << ">> Pie chart opened in external gnuplot window.\n";
            std::cout << std::string(75, '-') << "\n";
            return;
        }
    }

    // --- ASCII Rollback Rendering Branch ---
    std::cout << std::left
              << std::setw(20) << "Category"
              << std::setw(15) << "Amount"
              << std::setw(10) << "Percent"
              << "Visual Distribution\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto &item : items)
    {
        int barLength = static_cast<int>((item.percentage / 100.0) * 25.0);
        std::string bar(barLength, '#');

        std::cout << std::left << std::setw(20) << item.category
                  << "$" << std::setw(14) << item.amount
                  << std::setw(6) << item.percentage << "%   "
                  << "|" << bar << "\n";
    }
    std::cout << std::string(75, '-') << "\n";
}

void MenuSystem::handleIncomeExpense() const
{
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

    if (data.empty())
    {
        std::string error = controller.getLastError().message;
        if (!error.empty())
        {
            std::cout << "Notice: " << error << "\n";
        }
        else
        {
            std::cout << "No records found for the specified period.\n";
        }
        return;
    }

    renderIncomeExpense(data);
}

void MenuSystem::renderIncomeExpense(const std::map<std::string, std::pair<double, double>> &data)
{
    // --- Gnuplot Rendering Branch ---
    if (shouldUseGnuplot())
    {
        FILE *pipe = openGnuplotPipe();
        if (pipe)
        {
            fprintf(pipe, "set title 'Monthly Income vs Expense Comparison'\n");
            fprintf(pipe, "set style data histograms\n");
            fprintf(pipe, "set style histogram clustered gap 1\n");
            fprintf(pipe, "set style fill solid 0.7 border -1\n");
            fprintf(pipe, "set xtics rotate by -45\n");
            fprintf(pipe, "set grid y\n");
            fprintf(pipe, "set ylabel 'Amount ($)'\n");
            fprintf(pipe, "set offsets 0, 0, graph 0.25, 0\n");
            fprintf(pipe, "plot '-' using 2:xtic(1) title 'Income' linecolor rgb '#228B22', \\\n");
            fprintf(pipe, "     '-' using 3 title 'Expense' linecolor rgb '#B22222', \\\n");
            fprintf(pipe, "     '-' using ($0-0.18):2:(sprintf(\"$%%.2f\", $2)) with labels font \",10\" rotate by 90 left offset 0, 0.5 notitle, \\\n");
            fprintf(pipe, "     '-' using ($0+0.18):3:(sprintf(\"$%%.2f\", $3)) with labels font \",10\" rotate by 90 left offset 0, 0.5 notitle\n");

            for (int i = 0; i < 4; i++)
            {
                for (const auto &pair : data)
                {
                    fprintf(pipe, "\"%s\" %f %f\n", pair.first.c_str(), pair.second.first, pair.second.second);
                }
                fprintf(pipe, "e\n");
            }

            pclose(pipe);

            std::cout << "\n>> Income vs Expense chart opened in external gnuplot window.\n";
            return;
        }
    }

    // --- ASCII Rollback Rendering Branch ---
    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m";
    const std::string COLOR_RED = "\033[31m";

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

    for (const auto &pair : data)
    {
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

        if (input.empty())
        {
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

void MenuSystem::displayMainMenu()
{
    std::cout << "\n================ MAIN MENU ================\n";
    std::cout << "[1] View/Search Records" << std::endl;
    std::cout << "[2] Search Record by ID" << std::endl;
    std::cout << "[3] Record Management (Add, Import, Update, Delete)" << std::endl;
    std::cout << "[4] Financial Analytics & Reports" << std::endl;
    std::cout << "[5] Category Management" << std::endl;
    std::cout << "[0] Exit and Save" << std::endl;
    std::cout << "===========================================\n";
    std::cout << "Enter your choice: ";
}
