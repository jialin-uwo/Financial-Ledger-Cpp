/**
 * @file LedgerController.cpp
 * @brief Implementation of the LedgerController class.
 * * This file contains the functional logic for managing financial records,
 * including CSV data parsing, memory management for analytical components,
 * and the implementation of cumulative filtering logic for reports.
 * * @author Jialin Li
 * @date 2026-03-06
 */
#include "LedgerController.hpp"
#include <iostream>
#include <vector>
#include <cstdio>

LedgerController::LedgerController()
{
    // Initialize components and set up initial state
    // analyzer and dataAccess are initialized via default constructors
    lastError = "";
    nextRecordId = 1; // Start IDs from 1 for better readability
}

LedgerController::~LedgerController()
{
    // Safety net: attempt to save if the UI forgot to call shutDown()
    this->shutDown();
    // No need to delete analyzer or dataAccess as they are member objects, not pointers
}

std::string LedgerController::init()
{
    // 1. Retrieve raw data from the persistent storage (CSV)
    std::vector<Record> rawRecords = dataAccess.loadRecords();

    // 2. Clear current memory buffer to prevent duplication
    this->records.clear();

    // 3. Reset the ID tracker (Crucial for Re-indexing consistency)
    this->nextRecordId = 1;

    // 4.Re-assign unique Identifier (UID) to ensure strict sequence
    for (const auto &r : rawRecords)
    {
        // emplace_back constructs the Record directly in the vector's memory
        Record standardizedRec(this->nextRecordId++, r.getDate(), r.getAmount(), r.getIsExpense(), r.getCategory());
        this->records.push_back(standardizedRec);
    }

    // 5. Immediate Persistence: Sync the normalized data back to CSV
    if (!records.empty())
    {
        dataAccess.saveRecords(this->records);
    }

    // 6. Generate the UI Message based on the result
    if (this->records.empty())
    {
        // This could be a new user (empty file) or a potential error
        return "System initialized. No existing records found.";
    }
    else
    {
        // Success case: show how many records were processed
        return "System initialized. Successfully loaded and re-indexed " +
               std::to_string(this->records.size()) + " records.";
    }
}

std::string LedgerController::shutDown()
{
    // Attempt to save current records to CSV
    dataAccess.saveRecords(this->records);
    return "Data successfully saved. System shutting down.";
}

std::string LedgerController::addRecord(std::string date, double amount, bool isExpense, std::string cat)
{
    std::string errorMsg;

    // Call static validation method using Scope Resolution Operator (::)
    if (!Record::validateData(date, amount, errorMsg))
    {
        return "FAIL: " + errorMsg;
    }

    // Create record on the stack (No 'new' keyword needed)
    Record newRec(this->nextRecordId++, date, amount, isExpense, cat);

    this->records.push_back(newRec);
    dataAccess.saveRecords(this->records);

    return "SUCCESS: Record added (ID: " + std::to_string(newRec.getId()) + ").";
}

std::string LedgerController::addRecordsByFile(std::string filePath)
{
    std::vector<Record> importedRecords = dataAccess.loadRecords(filePath);
    if (importedRecords.empty())
    {
        return "FAIL: No records found in the specified file or file access error.";
    }
    for (const auto &r : importedRecords)
    {
        Record standardizedRec(this->nextRecordId++, r.getDate(), r.getAmount(), r.getIsExpense(), r.getCategory());
        this->records.push_back(standardizedRec);
    }
    dataAccess.saveRecords(this->records);
    return "SUCCESS: Imported " + std::to_string(importedRecords.size()) + " records from file.";
}

std::string LedgerController::updateRecord(int recordId, std::string date, double amount, bool isExpense, std::string cat)
{
    std::string message;
    message = removeRecord(recordId);
    if (message.find("FAIL") == 0)
        return message;
    message = addRecord(date, amount, isExpense, cat);
    if (message.find("SUCCESS") == 0)
        return "SUCCESS: Record ID #" + std::to_string(recordId) + " is updated.";
    return message;
}

std::string LedgerController::removeRecord(int id)
{
    for (auto it = records.begin(); it != records.end(); ++it)
    {
        if (it->getId() == id)
        {
            this->records.erase(it);
            this->dataAccess.saveRecords(this->records);
            return "SUCCESS: Record #" + std::to_string(id) + " deleted.";
        }
    }
    return "FAIL: Record ID #" + std::to_string(id) + " does not exist.";
}

std::string LedgerController::getLastError()
{
    return this->lastError;
}

std::vector<Record> LedgerController::getRecords(std::string start, std::string end, int isExpense, std::string cat, double minAmount)
{
    // Implementation of cumulative filtering logic based on provided parameters
    // This will involve iterating through the records and applying the filters in sequence
    // The method will return a vector of records that match all the specified criteria
    if (!start.empty() && !end.empty() && start > end)
    {
        this->lastError = "Invalid date range: Start date cannot be after end date.";
        return {};
    }
    std::vector<Record> filteredRecords;
    for (const auto &rec : this->records)
    {
        if (!start.empty() && rec.getDate() < start)
            continue;
        if (!end.empty() && rec.getDate() > end)
            continue;
        if (!cat.empty() && rec.getCategory() != cat)
            continue;
        if (rec.getAmount() < minAmount)
            continue;
        if (isExpense != -1 && rec.getIsExpense() != (isExpense == 1))
            continue;
        filteredRecords.push_back(rec);
    }
    if (filteredRecords.empty())
    {
        this->lastError = "No records match the specified criteria.";
    }
    else
    {
        this->lastError = ""; // Clear last error if we have results
    }
    return filteredRecords;
}

std::map<std::string, double> LedgerController::getPeriodSummary(std::string start, std::string end)
{

    if (!start.empty() && !end.empty() && start > end)
    {
        this->lastError = "Invalid date range: Start date cannot be after end date.";
        return {};
    }
    std::vector<Record> filteredRecords = getRecords(start, end, -1, "Other"); // Reuse filtering logic for consistency
    if (filteredRecords.empty())
    {
        this->lastError = "No records found for the specified period.";
        return {};
    }
    auto summary = this->analyzer.calculateSummary(filteredRecords);
    this->lastError = ""; // Clear last error if we have a valid summary
    return summary;
}

std::string LedgerController::getTotal(std::string start, std::string end, int isExpense, std::string cat)
{

    if (!start.empty() && !end.empty() && start > end)
    {
        this->lastError = "Invalid date range: Start date cannot be after end date.";

        return "FAIL: " + this->lastError;
    }

    std::vector<Record> filteredRecords = getRecords(start, end, isExpense, cat); // Reuse filtering logic for consistency

    if (filteredRecords.empty())
    {
        this->lastError = "No records found for the specified category and period.";
        return "FAIL: " + this->lastError;
    }

    auto summary = this->analyzer.calculateSummary(filteredRecords);
    double income = summary.at("total_income");
    double expense = summary.at("total_expense");
    double balance = summary.at("net_balance");

    // Format the response based on the isExpense filter
    char buffer[256];
    if (isExpense == -1)
    {
        // No filter: return all totals
        snprintf(buffer, sizeof(buffer), "Total Income: $%.2f, Total Expense: $%.2f, Balance: $%.2f", income, expense, balance);
    }
    else if (isExpense == 0)
    {
        // Income only
        snprintf(buffer, sizeof(buffer), "Total Income: $%.2f", income);
    }
    else // isExpense == 1
    {
        // Expense only
        snprintf(buffer, sizeof(buffer), "Total Expense: $%.2f", expense);
    }

    this->lastError = "";
    // ...existing code...
    return buffer;
}
