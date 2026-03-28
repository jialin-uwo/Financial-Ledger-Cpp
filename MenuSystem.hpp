#ifndef MENUSYSTEM_HPP
#define MENUSYSTEM_HPP

#include "LedgerController.hpp"
#include <string>

class MenuSystem
{
private:
    LedgerController &controller;

    static void displayMainMenu();
    void handleCommand(std::string cmd);

    void handleAddRecord();
    void handleAddRecordByFile();
    static void displayRecordMenu();
    void handleRecordManagement();
    static void displayReportMenu();
    void handleReportManagement();
    static void displayCategoryMenu();
    void handleCategoryManagement();
    void handleAddCategory();
    void handleListCategories() const;
    void handleUpdateCategory();
    void handleDeleteCategory();
    void handleSearchRecords();
    void handleSimpleTotal();
    void handleUpdateRecord();
    void handleDeleteRecord();
    void handleFinancialSummary();

    void handleCurrentBudgetStatus() const;
    static void renderBudgetStatus(const std::vector<BudgetStatus>& statuses);
    void handleDistribution() const;
    static void renderDistribution(const std::pair<double, std::vector<CategoryDistItem>>& distribution);
    void handleTrend() const;
    static void renderTrend(const std::map<std::string, double>& trendData);
    void handleIncomeExpense() const;
    static void renderIncomeExpense(const std::map<std::string, std::pair<double, double>>& data);
    static void renderRecordTable(const std::vector<Record> &records);

    // Helpers
    std::string getValidatedInput(std::string prompt, bool allowEmpty = false);
    double getValidatedAmount(bool allowEmpty = false);
    std::string getValidatedDate() const;

    void exitMenu() const;

public:
    // Constructor: accepts a LedgerController reference
    MenuSystem(LedgerController &ctrl);

    // The main entry point for the UI loop, called from main.cpp
    void run();
};

#endif // MENUSYSTEM_HPP