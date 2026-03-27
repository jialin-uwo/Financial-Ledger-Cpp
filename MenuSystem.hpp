#ifndef MENUSYSTEM_HPP
#define MENUSYSTEM_HPP

#include "LedgerController.hpp"
#include <string>

class MenuSystem
{
private:
    LedgerController &controller;

    void displayMainMenu();
    void handleCommand(std::string cmd);

    void handleAddRecord();
    void handleAddRecordByFile();
    void handleCategoryManagement();
    void displayCategoryMenu();
    void handleAddCategory();
    void handleListCategories();
    void handleUpdateCategory();
    void handleDeleteCategory();
    void handleSearchRecords();
    void handleSimpleTotal();
    void handleUpdateRecord();
    void handleDeleteRecord();
    void handleReportGeneration();
    void handleFinancialSummary();
    void displayMessage(std::string msg);

    void renderRecordTable(const std::vector<Record> &records);
    // void renderDistribution(CategoryDistribution &distribution);
    void renderTrend(std::map<std::string, double> &data);
    void renderIncomeExpense(std::map<std::string, std::pair<double, double>> &data);

    // Helpers
    std::string getValidatedInput(std::string prompt, bool allowEmpty = false);
    double getValidatedAmount(bool allowEmpty = false);
    std::string getValidatedDate();

    void exitMenu();

public:
    // Constructor: accepts a LedgerController reference
    MenuSystem(LedgerController &ctrl);

    // The main entry point for the UI loop, called from main.cpp
    void run();
};

#endif // MENUSYSTEM_HPP