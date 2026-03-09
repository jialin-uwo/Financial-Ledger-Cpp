#ifndef MENUSYSTEM_HPP
#define MENUSYSTEM_HPP

#include "LedgerController.hpp"
#include <string>

class MenuSystem {
private:
    LedgerController& controller;

    void displayMainMenu();
    void handleCommand(std::string cmd);

    // Record CRUD
    void handleAddRecord();
    void handleAddRecordByFile();
    void handleSearchRecords();
    void handleSimpleTotal();
    void handleUpdateRecord();
    void handleDeleteRecord();
    void handleFinancialSummary();

    // Helpers
    std::string getValidatedInput(std::string prompt, bool allowEmpty = false);
    double getValidatedAmount(bool allowEmpty = false);

    void exitMenu();
public:
    // Constructor: accepts a LedgerController reference
    MenuSystem(LedgerController& ctrl);

    // The main entry point for the UI loop, called from main.cpp
    void run();
};

#endif // MENUSYSTEM_HPP