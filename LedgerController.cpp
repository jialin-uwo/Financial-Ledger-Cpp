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
#include <sstream>

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
    std::vector<Record> rawRecords = dataAccess.loadRecordsWithId();

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
        this->lastError = "";
        // This could be a new user (empty file) or a potential error
        return "System initialized. No existing records found.";
    }
    else
    {
        this->lastError = "";
        // Success case: show how many records were processed
        return "System initialized. Successfully loaded and re-indexed " +
               std::to_string(this->records.size()) + " records.";
    }
}

std::string LedgerController::shutDown()
{
    // Attempt to save current records to CSV
    dataAccess.saveRecords(this->records);
    this->lastError = "";
    return "Data successfully saved. System shutting down.";
}

std::string LedgerController::addRecord(std::string date, double amount, bool isExpense, std::string cat)
{
    std::string errorMsg;

    // Call static validation method using Scope Resolution Operator (::)
    if (!Record::validateData(date, amount, errorMsg))
    {
        this->lastError = errorMsg;
        return "FAIL: " + errorMsg;
    }

    bool categoryWasCreated = false;

    // Handle category: if empty, use default "Other Expense" or "Other Income"
    if (cat.empty())
    {
        cat = isExpense ? "Other Expense" : "Other Income";
    }
    else
    {
        // If category is specified, check if it exists; if not, create it automatically
        bool categoryExists = false;
        for (const auto &category : this->categories)
        {
            if (category.getName() == cat)
            {
                categoryExists = true;
                break;
            }
        }

        if (!categoryExists)
        {
            // Auto-create the category with the same isExpense flag and no budget (default -1.0)
            std::string addCatResult = this->addCategory(cat, isExpense);
            if (addCatResult.find("FAIL") == 0)
            {
                this->lastError = addCatResult;
                return addCatResult; // Return error if category creation fails, do not add record
            }
            categoryWasCreated = true;
        }
    }

    // Create record on the stack (No 'new' keyword needed)
    Record newRec(this->nextRecordId++, date, amount, isExpense, cat);

    this->records.push_back(newRec);
    dataAccess.saveRecords(this->records);

    // Return success message, noting if a category was auto-created
    std::string successMsg = "SUCCESS: Record added (ID: " + std::to_string(newRec.getId()) + ")";
    if (categoryWasCreated)
    {
        successMsg += ", Category '" + cat + "' auto-created";
    }
    successMsg += ".";
    this->lastError = "";
    return successMsg;
}

std::string LedgerController::addRecordsByFile(std::string filePath)
{
    (void)filePath; // The new DataAccess interface determines source internally.

    LoadReport report;
    std::vector<Record> importedRecords = dataAccess.loadRecordsWithoutId(report);

    for (const auto &r : importedRecords)
    {
        Record standardizedRec(this->nextRecordId++, r.getDate(), r.getAmount(), r.getIsExpense(), r.getCategory());
        this->records.push_back(standardizedRec);
    }

    if (!importedRecords.empty())
    {
        dataAccess.saveRecords(this->records);
    }

    std::ostringstream oss;
    oss << "Processed " << report.processedRows
        << " lines, imported " << report.successRows
        << " records, errors " << report.errorRows << ".";

    if (!report.errorsByMessage.empty())
    {
        oss << " Error details:";
        for (const auto &entry : report.errorsByMessage)
        {
            oss << " [" << entry.first << ": ";
            for (size_t i = 0; i < entry.second.size(); ++i)
            {
                oss << entry.second[i];
                if (i + 1 < entry.second.size())
                {
                    oss << ", ";
                }
            }
            oss << "]";
        }
    }

    if (report.successRows == 0)
    {
        this->lastError = oss.str();
        return "FAIL: " + oss.str();
    }

    this->lastError = "";
    return "SUCCESS: " + oss.str();
}

std::string LedgerController::updateRecord(int recordId, std::string date, double amount, int isExpense, std::string cat)
{
    for (auto &rec : this->records)
    {
        if (rec.getId() != recordId)
        {
            continue;
        }

        if (date.empty() && amount == -1.0 && isExpense == -1 && cat.empty())
        {
            this->lastError = "At least one field must be provided for update.";
            return "FAIL: At least one field must be provided for update.";
        }

        if (isExpense != -1 && isExpense != 0 && isExpense != 1)
        {
            this->lastError = "Invalid isExpense value. Use -1 (unchanged), 0 (income), or 1 (expense).";
            return "FAIL: Invalid isExpense value. Use -1 (unchanged), 0 (income), or 1 (expense).";
        }

        const std::string effectiveDate = date.empty() ? rec.getDate() : date;
        const double effectiveAmount = (amount == -1.0) ? rec.getAmount() : amount;
        const bool effectiveIsExpense = (isExpense == -1) ? rec.getIsExpense() : (isExpense == 1);

        std::string errorMsg;
        if (!Record::validateData(effectiveDate, effectiveAmount, errorMsg))
        {
            this->lastError = errorMsg;
            return "FAIL: " + errorMsg;
        }

        if (!cat.empty())
        {
            bool categoryExists = false;
            for (const auto &category : this->categories)
            {
                if (category.getName() == cat)
                {
                    categoryExists = true;
                    break;
                }
            }

            if (!categoryExists)
            {
                std::string addCatResult = this->addCategory(cat, effectiveIsExpense);
                if (addCatResult.find("FAIL") == 0)
                {
                    this->lastError = addCatResult;
                    return addCatResult;
                }
            }
        }

        // Apply partial updates via Record setters (setter methods implemented in Record class).
        if (!date.empty())
        {
            rec.setDate(effectiveDate);
        }
        if (amount != -1.0)
        {
            rec.setAmount(effectiveAmount);
        }
        if (isExpense != -1)
        {
            rec.setIsExpense(effectiveIsExpense);
        }
        if (!cat.empty())
        {
            rec.setCategory(cat);
        }

        this->dataAccess.saveRecords(this->records);
        this->lastError = "";
        return "SUCCESS: Record ID #" + std::to_string(recordId) + " is updated.";
    }

    this->lastError = "Record ID #" + std::to_string(recordId) + " does not exist.";
    return "FAIL: Record ID #" + std::to_string(recordId) + " does not exist.";
}

std::string LedgerController::removeRecord(int id)
{
    for (auto it = records.begin(); it != records.end(); ++it)
    {
        if (it->getId() == id)
        {
            this->records.erase(it);
            this->dataAccess.saveRecords(this->records);
            this->lastError = "";
            return "SUCCESS: Record #" + std::to_string(id) + " deleted.";
        }
    }
    this->lastError = "Record ID #" + std::to_string(id) + " does not exist.";
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
    std::vector<Record> filteredRecords = getRecords(start, end, -1, ""); // Reuse filtering logic for consistency
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
