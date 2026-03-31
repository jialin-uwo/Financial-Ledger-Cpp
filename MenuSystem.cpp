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
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <cstdlib> // getenv
#include <cstdio>  // popen, pclose
#include <cmath>
void MenuSystem::handleTrend() const
{
    std::cout << "\n=== Trend Analysis (Monthly) ===\n";
    std::cout << "This feature shows the monthly trend of your income or expenses over time, helping you identify patterns and changes in your financial activity." << std::endl;

    std::string start, end, cat, typeStr;
    int isExpense = 1;
    std::string trendTypeStr = "Expense";

    while (true)
    {
        std::cout << "Enter start date (YYYY-MM-DD, or press Enter to include from the earliest record): ";
        start = getValidatedDate();
        std::cout << "Enter end date (YYYY-MM-DD, or press Enter to include up to the latest record): ";
        end = getValidatedDate();

        if (!start.empty() && !end.empty() && start > end)
        {
            std::cout << "Start date cannot be later than end date. Please re-enter.\n";
            continue;
        }
        break;
    }

    std::cout << "Filter by type? (1 = Expense, 0 = Income, or press Enter for Expense): ";
    std::getline(std::cin, typeStr);
    if (!typeStr.empty())
    {
        try
        {
            isExpense = std::stoi(typeStr);
            if (isExpense == 0)
                trendTypeStr = "Income";
            else
                trendTypeStr = "Expense";
        }
        catch (...)
        {
            isExpense = 1;
            trendTypeStr = "Expense";
        }
    }

    std::cout << "Enter category name (or press Enter for all categories): ";
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

    renderTrend(trendData, cat, trendTypeStr);
}

void MenuSystem::renderTrend(const std::map<std::string, double> &trendData, const std::string &category, const std::string &trendTypeStr)
{
    double maxAmount = 0.0;
    for (const auto &pair : trendData)
    {
        if (pair.second > maxAmount)
            maxAmount = pair.second;
    }

    // Compose chart title with type and category
    std::string chartTitle = "Monthly Trend Analysis [" + trendTypeStr + "]";
    if (!category.empty())
    {
        chartTitle += " [Category: " + category + "]";
    }

    // --- Gnuplot Rendering Branch ---
    if (shouldUseGnuplot())
    {
        FILE *pipe = openGnuplotPipe();
        if (pipe)
        {
            fprintf(pipe, "set title '%s'\n", chartTitle.c_str());
            fprintf(pipe, "set label 1 at graph 0.01,0.97 'Type: %s' left font ',10'\n", trendTypeStr.c_str());
            if (!category.empty())
            {
                fprintf(pipe, "set label 2 at graph 0.01,0.93 'Category: %s' left font ',10'\n", category.c_str());
            }
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
    std::cout << "\n"
              << chartTitle << "\n";
    std::cout << std::string(75, '-') << "\n";
    std::cout << std::left
              << std::setw(12) << "Month"
              << std::setw(15) << "Total Amount"
              << "Trend\n";
    std::cout << std::string(75, '-') << "\n";

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
    std::cout << std::string(75, '-') << "\n";
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

    // Use getRecords to search all records
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
    // std::cout << "Type 'h' at any menu for help or instructions." << std::endl;
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
            // Record Management
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
            // Financial Analytics & Reports
            while (true)
            {

                std::cout << "\n--- Financial Analytics & Reports ---" << std::endl;
                std::cout << "This section provides various analysis and report features to help you understand your financial situation, spending habits, and trends.\n";
                std::cout << "Available analyses include: summary, totals, budget status, expense distribution, monthly trends, and income vs expense comparison." << std::endl;
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
            // Category Management
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

    double amount = 0.0;
    while (true)
    {
        std::cout << "Enter amount (e.g. 123.45): ";
        std::string input;
        std::getline(std::cin, input);
        try
        {
            amount = std::stod(input);
            break;
        }
        catch (...)
        {
            std::cout << "Invalid number format. Please enter a valid number like 123.45." << std::endl;
        }
    }

    std::string typeStr = getValidatedInput("Is it an expense? (y/n): ");

    bool isExpense = (typeStr == "y" || typeStr == "Y");

    std::string category = getValidatedInput("Enter category (optional, press Enter to assign to 'Other Income/Expense'): ", true);

    Result result = controller.addRecord(date, amount, isExpense, category);
    std::cout << "\n> " << result.message << std::endl;

    // if add success and date is in current month, show updated budget status for that category
    if (result.ok())
    {
        // if category is empty, it means it will be auto-assigned to "Other Income" or "Other Expense", so we should check both cases in the budget status display
        if (date.size() >= 7)
        {
            time_t t = time(nullptr);
            tm *now = localtime(&t);
            char buf[8];
            strftime(buf, sizeof(buf), "%Y-%m", now);
            std::string currentMonth(buf);
            if (date.substr(0, 7) == currentMonth)
            {
                // After adding a record, if it's in the current month, show the updated budget status for the relevant category
                std::vector<BudgetStatus> statuses = controller.getCurrentBudgetStatus();
                std::vector<BudgetStatus> filtered;
                for (const auto &s : statuses)
                {
                    if (s.categoryName == category || (category.empty() && s.categoryName.find("Other") != std::string::npos))
                        filtered.push_back(s);
                }
                if (!filtered.empty())
                {
                    std::cout << "\n[Updated Budget Status for Category]" << std::endl;
                    renderBudgetStatus(filtered, true);
                }
            }
        }
    }
}

void MenuSystem::handleAddRecordByFile()
{
    std::cout << "\n--- Add Records by File (Batch Import) ---" << std::endl;
    std::cout << R"(====================================================
                            IMPORT DATA FROM CSV FILE
====================================================

[ CSV FORMAT REQUIREMENTS ]
    Fields order: date,amount,isExpense,category
    Example: 2024-01-01,4150.11,1,Salary
                     2024-01-02,22.22,0,Groceries

    - isExpense: 1/true means expense, 0/false means income (case-insensitive)
    - category can be left blank; blank will auto-assign "Other Income" or "Other Expense"
    - Date format: YYYY-MM-DD
    - File format: .csv (comma separated)

Please enter the FULL path to your CSV file:
> _
====================================================)";
    std::string filePath = getValidatedInput("\nEnter import file path: ");

    Result result = controller.addRecordsByFile(filePath);

    size_t pos = result.message.find("Error details:");
    if (pos != std::string::npos)
    {
        std::string summary = result.message.substr(0, pos);
        std::string details = result.message.substr(pos + 14); // 14 = strlen("Error details:")

        std::cout << std::endl;
        size_t autoPos = summary.find("Auto-created");
        if (autoPos != std::string::npos)
        {
            std::string first = summary.substr(0, autoPos);
            std::string second = summary.substr(autoPos);

            while (!first.empty() && (first.front() == ' '))
                first.erase(0, 1);
            while (!first.empty() && (first.back() == ' '))
                first.pop_back();
            while (!second.empty() && (second.front() == ' '))
                second.erase(0, 1);
            while (!second.empty() && (second.back() == ' '))
                second.pop_back();
            if (!first.empty())
                std::cout << "> " << first << std::endl;
            if (!second.empty())
                std::cout << "> " << second << std::endl;
        }
        else
        {
            // 没有 Auto-created 就整句一行
            std::string line = summary;
            while (!line.empty() && (line.front() == ' '))
                line.erase(0, 1);
            while (!line.empty() && (line.back() == ' '))
                line.pop_back();
            if (!line.empty())
                std::cout << "> " << line << std::endl;
        }
        std::cout << std::string(50, '-') << std::endl;
        std::cout << "> Error details (grouped, with lines):" << std::endl;
        // 1. Split details by "] [" to get individual error blocks
        size_t start = 0;
        while (start < details.size())
        {
            size_t open = details.find('[', start);
            size_t close = details.find(']', open);
            if (open == std::string::npos || close == std::string::npos)
                break;
            std::string errBlock = details.substr(open + 1, close - open - 1);

            while (!errBlock.empty() && (errBlock.front() == ' '))
                errBlock.erase(0, 1);
            while (!errBlock.empty() && (errBlock.back() == ' '))
                errBlock.pop_back();
            if (!errBlock.empty())
            {

                std::string toFind = "Category '";
                size_t catPos = errBlock.find(toFind);
                size_t typePos = errBlock.find("type does not match record type.");
                if (catPos != std::string::npos && typePos != std::string::npos)
                {
                    // Extract category name for clearer error message
                    size_t colonPos = errBlock.find(":");
                    std::string msg = "Category expense/income type does not match record type.";
                    std::string lineNums = (colonPos != std::string::npos) ? errBlock.substr(colonPos + 1) : "";
                    if (!lineNums.empty())
                    {
                        std::cout << "> " << msg << " (lines: " << lineNums << ")" << std::endl;
                    }
                    else
                    {
                        std::cout << "> " << msg << std::endl;
                    }
                }
                else
                {
                    size_t colonPos = errBlock.find(":");
                    if (colonPos != std::string::npos)
                    {
                        std::string msg = errBlock.substr(0, colonPos);
                        std::string lineNums = errBlock.substr(colonPos + 1);
                        std::cout << "> " << msg << " (lines: " << lineNums << ")" << std::endl;
                    }
                    else
                    {
                        std::cout << "> " << errBlock << std::endl;
                    }
                }
            }
            start = close + 1;
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "\n> " << result.message << std::endl;
    }
}

void MenuSystem::handleSearchRecords()
{
    std::cout << "\n--- View/Search Records ---" << std::endl;
    std::cout << "(Press Enter to skip any filter and view all)" << std::endl;

    // Step 1: Get start date (optional)
    std::string start = getValidatedInput("Enter Start Date (YYYY-MM-DD, or press Enter to include from the earliest record): ", true);

    // Step 2: Get end date (optional)
    std::string end = getValidatedInput("Enter End Date (YYYY-MM-DD, or press Enter to include up to the latest record): ", true);

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
    std::string category = getValidatedInput("Enter category (optional, press Enter to select all categories): ", true);

    // Step 5: Get minimum amount (optional)
    double minAmount = getValidatedAmount(true);

    std::vector<Record> results = controller.getRecords(start, end, isExpense, category, minAmount);

    if (results.empty())
    {
        std::string error = controller.getLastError().message;
        std::cout << "\n> No records found or invalid criteria.";
        if (!error.empty())
        {
            std::cout << "\n> Reason: " << error;
        }
        std::cout << std::endl;
    }
    else
    {
        renderRecordTable(results);
    }
}

void MenuSystem::handleSimpleTotal()
{
    std::cout << "\n--- Simple Total ---" << std::endl;
    std::cout << "This feature calculates the total income or expense for a selected period, with optional type filtering." << std::endl;

    // Step 1: Get start date (optional)
    std::string start = getValidatedInput("Enter Start Date (YYYY-MM-DD, or press Enter to include from the earliest record): ", true);

    // Step 2: Get end date (optional)
    std::string end = getValidatedInput("Enter End Date (YYYY-MM-DD, or press Enter to include up to the latest record): ", true);

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

    std::string category = getValidatedInput("Enter category (optional, press Enter to select all categories): ", true);

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
    double amount = 0.0;
    while (true)
    {
        std::cout << "Enter new amount (e.g. 123.45): ";
        std::string input;
        std::getline(std::cin, input);
        try
        {
            amount = std::stod(input);
            break;
        }
        catch (...)
        {
            std::cout << "Invalid number format. Please enter a valid number like 123.45." << std::endl;
        }
    }

    std::string typeStr = getValidatedInput("Is it an expense? (y/n): ");
    bool isExpense = (typeStr == "y" || typeStr == "Y");

    std::string category = getValidatedInput("Enter new category (optional, press Enter to keep current): ", true);

    Result result = controller.updateRecord(id, date, amount, isExpense, category);
    std::cout << "\n> " << result.message << std::endl;

    if (result.ok())
    {

        if (date.size() >= 7)
        {
            time_t t = time(nullptr);
            tm *now = localtime(&t);
            char buf[8];
            strftime(buf, sizeof(buf), "%Y-%m", now);
            std::string currentMonth(buf);
            if (date.substr(0, 7) == currentMonth)
            {

                // After updating a record, if it's in the current month, show the updated budget status for the relevant category
                std::vector<BudgetStatus> statuses = controller.getCurrentBudgetStatus();
                std::vector<BudgetStatus> filtered;
                for (const auto &s : statuses)
                {
                    if (s.categoryName == category || (category.empty() && s.categoryName.find("Other") != std::string::npos))
                        filtered.push_back(s);
                }
                if (!filtered.empty())
                {
                    std::cout << "\n[Updated Budget Status for Category]" << std::endl;
                    renderBudgetStatus(filtered, true);
                }
            }
        }
    }
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
    std::cout << "(Time filter only. Press Enter to include from the earliest record or up to the latest record)" << std::endl;

    std::string start = getValidatedInput("Enter Start Date (YYYY-MM-DD, or press Enter to include from the earliest record): ", true);
    std::string end = getValidatedInput("Enter End Date (YYYY-MM-DD, or press Enter to include up to the latest record): ", true);

    std::map<std::string, double> summary = controller.getPeriodSummary(start, end);

    if (summary.empty())
    {
        std::cout << "> No data available to generate summary for the selected period." << std::endl;

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
    // Removed duplicate tip; will show only before relevant prompts
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

    // 1. Category name (required)
    std::string name;
    while (true)
    {
        name = getValidatedInput("Enter category name: ");
        if (name == "h" || name == "H")
        {
            std::cout << "Tip: Category name is recommended to be within 16 characters and cannot be empty." << std::endl;
            continue;
        }
        if (name.empty())
        {
            std::cout << "> Category name cannot be empty. Please re-enter." << std::endl;
            continue;
        }
        break;
    }

    // 2. Category type
    std::cout << "Category type:\n"
              << "  e - Expense\n"
              << "  i - Income\n";

    std::string typeStr;
    bool isExpense = true;
    while (true)
    {
        typeStr = getValidatedInput("Enter type (e/i): ");
        if (typeStr == "h" || typeStr == "H")
        {
            std::cout << "e means Expense category, i means Income category." << std::endl;
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
        std::cout << "Invalid type. Please enter 'e' (Expense) or 'i' (Income), or enter h for help." << std::endl;
    }

    // 3. Optional budget and warning threshold
    double budget = -1.0;
    double warningThreshold = -1.0;
    if (isExpense)
    {
        std::cout << "Note: Only expense categories can set budget and warning threshold." << std::endl;
        std::cout << "Optional fields (press Enter to skip):" << std::endl;
        std::string budgetStr = getValidatedInput("Enter monthly budget for this category (the maximum amount you plan to spend per month, >= 0, optional): ", true);
        if (budgetStr == "h" || budgetStr == "H")
        {
            std::cout << "Budget sets a spending limit for this category each month. If you exceed this amount, you may get a warning. Leave blank if you do not want to set a budget." << std::endl;
            budgetStr.clear();
        }
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
                std::cout << "> Error: Invalid budget format. Please enter a number." << std::endl;
                return;
            }
        }

        if (!budgetStr.empty())
        {
            std::string warningStr = getValidatedInput("Enter warning threshold for this category (the amount that will trigger a warning, default is 70% of budget, >= 0, optional): ", true);
            if (warningStr == "h" || warningStr == "H")
            {
                std::cout << "Warning threshold is the amount at which you will receive a warning before reaching your budget limit. Leave blank to use the default (70% of budget). Must be less than or equal to the budget." << std::endl;
                warningStr.clear();
            }
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
                    std::cout << "> Error: Invalid warning threshold format. Please enter a number." << std::endl;
                    return;
                }
            }
            if (budget < 0.0 && warningThreshold >= 0.0)
            {
                std::cout << "> Error: Warning threshold set but no budget set." << std::endl;
                return;
            }
        }
    }
    // For income categories, skip budget/warning input entirely

    // 4. Call controller
    Result result = controller.addCategory(name, isExpense, budget, warningThreshold);
    if (result.ok())
    {
        std::cout << "\n> " << result.message << std::endl;
    }
    else
    {
        std::cout << "\n> " << result.message << std::endl;
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
            // 只输出一次错误信息，已在其它地方输出
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
    std::cout << std::string(75, '-') << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    // Sort: Income first (by name), then Expense (by name)
    std::vector<Category> incomeCats, expenseCats;
    for (const auto &cat : categories)
    {
        if (cat.getIsExpense())
            expenseCats.push_back(cat);
        else
            incomeCats.push_back(cat);
    }
    auto byName = [](const Category &a, const Category &b)
    {
        return a.getName() < b.getName();
    };
    std::sort(incomeCats.begin(), incomeCats.end(), byName);
    std::sort(expenseCats.begin(), expenseCats.end(), byName);
    int idx = 1;
    for (const auto &cat : incomeCats)
    {
        std::string type = "Income";
        std::string budgetStr = "N/A";
        std::string warnStr = "N/A";
        std::cout << std::setw(4) << idx++
                  << std::setw(18) << cat.getName().substr(0, 16)
                  << std::setw(10) << type
                  << std::setw(14) << budgetStr
                  << std::setw(14) << warnStr << std::endl;
    }
    for (const auto &cat : expenseCats)
    {
        std::string type = "Expense";
        std::string budgetStr = (cat.hasBudget()) ? ("$" + std::to_string(cat.getBudget())) : "N/A";
        std::string warnStr = (cat.hasBudget() && cat.getWarningThreshold() >= 0.0) ? ("$" + std::to_string(cat.getWarningThreshold())) : "N/A";
        std::cout << std::setw(4) << idx++
                  << std::setw(18) << cat.getName().substr(0, 16)
                  << std::setw(10) << type
                  << std::setw(14) << budgetStr
                  << std::setw(14) << warnStr << std::endl;
    }
    std::cout << std::string(75, '=') << std::endl;
    std::cout << "Tip: Use the category name when adding or editing records.\n";
    std::cout << "      For best results, keep names within 16 characters.\n";
}

void MenuSystem::handleUpdateCategory()
{
    std::cout << "\n--- Update Category ---" << std::endl;
    std::string oldName = getValidatedInput("Enter the original category name to update (cannot be skipped): ");
    std::cout << "For each field below, press Enter to skip (skipping means the field will not be changed)." << std::endl;
    std::cout << "You must provide at least one new value to update." << std::endl;
    std::string newName = getValidatedInput("Enter new name (optional): ", true);

    int isExpense = -1;
    std::string typeStr = getValidatedInput("Enter new type (e/i, optional): ", true);
    if (!typeStr.empty())
    {
        if (typeStr == "e" || typeStr == "E")
            isExpense = 1;
        else if (typeStr == "i" || typeStr == "I")
            isExpense = 0;
        else
        {
            std::cout << "Invalid type. Please enter e, i, or press Enter to skip." << std::endl;
            return;
        }
    }

    // Dynamically determine if budget/warning prompts should be shown
    bool willBeExpense = false;
    if (isExpense == 1)
    {
        willBeExpense = true;
    }
    else if (isExpense == 0)
    {
        willBeExpense = false;
    }
    else
    {
        // If type not changed, get original type from controller
        int origType = controller.getCategoryTypeByName(oldName);
        willBeExpense = (origType == 1);
    }

    double budget = -1.0;
    double warningThreshold = -1.0;
    std::string budgetStr, warningStr;
    if (willBeExpense)
    {
        std::cout << "Note: Only expense categories can set budget and warning threshold." << std::endl;
        budgetStr = getValidatedInput("Enter new budget for this category (Default value: 70% of budget)(>= 0, optional): ", true);
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

        warningStr = getValidatedInput("Enter new warning threshold that can trigger an alarm (>= 0, optional): ", true);
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
    }

    // Check if all new fields are skipped
    if (newName.empty() && typeStr.empty() && budgetStr.empty() && warningStr.empty())
    {
        std::cout << "> Please provide at least one new field to update. Skipping all means no change." << std::endl;
        return;
    }

    Result result = controller.updateCategory(oldName, newName, isExpense, budget, warningThreshold);
    std::cout << "\n> " << result.message << std::endl;
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
    if (result.ok())
    {
        if (result.reassignedCount > 0)
        {
            std::cout << "> Note: " << result.reassignedCount << " record" << (result.reassignedCount > 1 ? "s have" : " has") << " been reassigned to the default category ('Other Expense' or 'Other Income')." << std::endl;
        }
        else
        {
            std::cout << "> Note: No records needed reassignment." << std::endl;
        }
    }
}

void MenuSystem::handleCurrentBudgetStatus()
{
    std::cout << "\n=== Budget Status Report ===\n";
    std::cout << "You can view budget status for any month.\n";
    std::cout << "Enter year and month (YYYY-MM) or press Enter for current month: ";
    std::string yearMonth;
    std::getline(std::cin, yearMonth);
    std::vector<BudgetStatus> statuses;
    bool isCurrentMonth = yearMonth.empty();
    if (isCurrentMonth)
    {
        statuses = controller.getCurrentBudgetStatus();
        std::cout << "\n[Current Month]" << std::endl;
    }
    else
    {
        statuses = controller.getBudgetStatusForMonth(yearMonth);
        std::cout << "\n[Selected Month: " << yearMonth << "]" << std::endl;
    }
    if (statuses.empty())
    {
        std::string error = controller.getLastError().message;
        if (!error.empty())
        {
            std::cout << "> Error: " << error << std::endl;
        }
        else
        {
            std::cout << "No active budgets found or no expense categories exist for this month.\n";
        }
        return;
    }
    renderBudgetStatus(statuses, isCurrentMonth);
    std::cout << "\nEnd of budget status report.\n";
}

void MenuSystem::renderBudgetStatus(const std::vector<BudgetStatus> &statuses, bool showRemaining)
{
    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_GREEN = "\033[32m";  // Safe
    const std::string COLOR_YELLOW = "\033[93m"; // Warning (bright yellow)
    const std::string COLOR_RED = "\033[31m";    // Exceeded

    if (showRemaining)
    {
        std::cout << std::left
                  << std::setw(15) << "Category"
                  << std::setw(10) << "Health"
                  << std::setw(12) << "Budget"
                  << std::setw(12) << "Spent"
                  << std::setw(12) << "Remaining"
                  << "Daily Rec." << "\n";
        std::cout << std::string(75, '-') << "\n";
    }
    else
    {
        std::cout << std::left
                  << std::setw(15) << "Category"
                  << std::setw(10) << "Health"
                  << std::setw(12) << "Budget"
                  << std::setw(12) << "Spent" << "\n";
        std::cout << std::string(55, '-') << "\n";
    }

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

        // Warning整行黄色，其余按原色
        if (status.budgetHealth == BudgetHealth::Warning)
        {
            std::cout << COLOR_YELLOW;
        }
        else
        {
            std::cout << colorCode;
        }
        std::cout << std::left << std::setw(15) << status.categoryName
                  << std::setw(10) << healthStr
                  << "$" << std::setw(11) << std::fixed << std::setprecision(2) << status.budgetLimit
                  << "$" << std::setw(11) << status.actualSpent;

        if (showRemaining)
        {
            std::cout << "$" << std::setw(11) << status.remaining;
            if (status.dailyAvailable >= 0.0)
            {
                std::cout << "$" << status.dailyAvailable << " (" << status.daysRemaining << " days left)";
            }
            else
            {
                std::cout << "N/A";
            }
        }

        std::cout << COLOR_RESET << "\n";
    }

    if (showRemaining)
        std::cout << std::string(75, '-') << "\n";
    else
        std::cout << std::string(55, '-') << "\n";
}

void MenuSystem::handleDistribution()
{
    std::cout << "\n=== Expense Distribution Analysis ===\n";
    std::cout << "This feature analyzes the proportion of your expenses by category for the selected period, helping you understand where your money goes." << std::endl;

    std::string start, end;
    while (true)
    {
        std::cout << "Enter start date (YYYY-MM-DD, or press Enter to include from the earliest record): ";
        start = getValidatedDate();
        std::cout << "Enter end date (YYYY-MM-DD, or press Enter to include up to the latest record): ";
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
            if (totalExpense <= 0.0)
                totalExpense = 1.0;

            const double PI = 3.14159265358979323846;

            // 1st data block: Circle slices (radius increased to 2.2)
            double currentAngle = 0.0;
            int colorIndex = 1;
            for (const auto &item : items)
            {
                double ratio = item.amount / totalExpense;
                double sliceAngle = ratio * 360.0;
                if (sliceAngle > 0.0)
                {
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

                if (sliceAngle > 0.0)
                {
                    double midAngle = currentAngle + sliceAngle / 2.0;
                    double rad = midAngle * PI / 180.0;

                    double r_start = 2.2; // Starting at enlarged circle edge
                    double r_end = 2.5;   // Ending vector outside

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

                if (sliceAngle > 0.0)
                {
                    double midAngle = currentAngle + sliceAngle / 2.0;
                    double rad = midAngle * PI / 180.0;

                    // cos(rad) >= 0 -> Right hemisphere
                    if (std::cos(rad) >= 0)
                    {
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
            if (!hasRightSideLabels)
                fprintf(pipe, "0 0 \"\"\n");
            fprintf(pipe, "e\n");

            // 4th data block: Labels on the left side (align right, extend outward)
            bool hasLeftSideLabels = false;
            currentAngle = 0.0;
            for (const auto &item : items)
            {
                double ratio = item.amount / totalExpense;
                double sliceAngle = ratio * 360.0;

                if (sliceAngle > 0.0)
                {
                    double midAngle = currentAngle + sliceAngle / 2.0;
                    double rad = midAngle * PI / 180.0;

                    // cos(rad) < 0 -> Left hemisphere
                    if (std::cos(rad) < 0)
                    {
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
            if (!hasLeftSideLabels)
                fprintf(pipe, "0 0 \"\"\n");
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
    std::cout << "\n=== Income vs Expense Analysis (Monthly) ===\n";
    std::cout << "This feature compares your total income and total expenses for each month, allowing you to see your net savings or overspending trends." << std::endl;

    std::string start, end;

    while (true)
    {
        std::cout << "Enter Start Date (YYYY-MM-DD, or press Enter to include from the earliest record): ";
        start = getValidatedDate();
        std::cout << "Enter End Date (YYYY-MM-DD, or press Enter to include up to the latest record): ";
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
        std::cout << "Enter minimum amount to filter (positive number, e.g. 50.00" << (allowEmpty ? ", or press Enter to skip" : "") << "): ";
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
    std::cout << "[5] Category Management (List, Add, Update, Delete)" << std::endl;
    std::cout << "[0] Exit and Save" << std::endl;
    std::cout << "===========================================\n";
    std::cout << "Enter your choice: ";
}
