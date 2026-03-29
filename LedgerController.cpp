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
#include <cmath>
#include <iomanip>
#include <sstream>
#include "Result.h"

namespace
{
    bool hasFilterValidationError(const std::string &message)
    {
        return message == "Invalid date range: Start date cannot be after end date." ||
               message == "Invalid isExpense value. Use -1 (all), 0 (income), or 1 (expense).";
    }
}

// --- static helpers moved into LedgerController ---

std::string LedgerController::defaultCategoryName(bool isExpense)
{
    return isExpense ? "Other Expense" : "Other Income";
}

std::string LedgerController::normalizeCategoryInput(const std::string &category, bool isExpense)
{
    std::string normalized = trim(category);
    if (normalized.empty() || normalized == "other" || normalized == "Other")
    {
        return defaultCategoryName(isExpense);
    }
    return normalized;
}

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

Result LedgerController::init()
{
    // 0. Load categories from persistent storage
    this->categories = dataAccess.loadCategories();

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
        if (!dataAccess.saveRecords(this->records))
        {
            this->lastError = "Failed to save normalized records during initialization.";
            return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
        }
    }

    // 6. Generate the UI Message based on the result
    if (this->records.empty())
    {
        this->lastError = "";
        // This could be a new user (empty file) or a potential error
        return Result(StatusCode::SUCCESS, "System initialized. No existing records found. Loaded " +
                                               std::to_string(this->categories.size()) + " categories.");
    }
    else
    {
        this->lastError = "";
        // Success case: show how many records were processed
        return Result(StatusCode::SUCCESS, "System initialized. Successfully loaded " +
                                               std::to_string(this->records.size()) + " records. Loaded " +
                                               std::to_string(this->categories.size()) + " categories.");
    }
}

Result LedgerController::shutDown()
{
    // Attempt to save current records to CSV
    if (!dataAccess.saveRecords(this->records))
    {
        this->lastError = "Failed to save records during shutdown.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    this->lastError = "";
    return Result(StatusCode::SUCCESS, "Data successfully saved. System shutting down.");
}

Result LedgerController::addRecord(std::string date, double amount, bool isExpense, std::string cat)
{
    std::string errorMsg;

    // Call static validation method using Scope Resolution Operator (::)
    if (!Record::validateData(date, amount, errorMsg))
    {
        this->lastError = errorMsg;
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + errorMsg);
    }

    const std::string normalizedCategory = normalizeCategoryInput(cat, isExpense);
    auto categoryIt = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                                   { return trim(category.getName()) == normalizedCategory; });

    const bool categoryWasCreated = (categoryIt == this->categories.end());
    if (!categoryWasCreated && categoryIt->getIsExpense() != isExpense)
    {
        this->lastError = "Category '" + normalizedCategory + "' type does not match the record type.";
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
    }

    const std::vector<Category> originalCategories = this->categories;
    if (categoryWasCreated)
    {
        this->categories.emplace_back(normalizedCategory, isExpense);
    }

    const int newRecordId = this->nextRecordId;
    Record newRec(newRecordId, date, amount, isExpense, normalizedCategory);

    this->records.push_back(newRec);

    if (categoryWasCreated && !dataAccess.saveCategories(this->categories))
    {
        this->records.pop_back();
        this->categories = originalCategories;
        this->lastError = "Failed to save categories.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    if (!dataAccess.saveRecords(this->records))
    {
        this->records.pop_back();
        if (categoryWasCreated)
        {
            this->categories = originalCategories;
            dataAccess.saveCategories(this->categories);
        }
        this->lastError = "Failed to save records.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    this->nextRecordId++;

    // Return success message, noting if a category was auto-created
    std::string successMsg = "SUCCESS: Record added (ID: " + std::to_string(newRec.getId()) + ")";
    if (categoryWasCreated)
    {
        successMsg += ", Category '" + normalizedCategory + "' auto-created";
    }
    successMsg += ".";
    this->lastError = "";
    return Result(StatusCode::SUCCESS, successMsg);
}

Result LedgerController::addRecordsByFile(std::string filePath)
{
    LoadReport report;
    std::vector<Record> importedRecords = dataAccess.loadRecordsWithoutId(filePath, report);
    const std::vector<Record> originalRecords = this->records;
    const std::vector<Category> originalCategories = this->categories;
    const int originalNextRecordId = this->nextRecordId;
    bool categoriesChanged = false;
    int acceptedCount = 0;

    for (size_t index = 0; index < importedRecords.size(); ++index)
    {
        const Record &record = importedRecords[index];
        const int lineNumber = report.getSuccessLineNumber(index);
        const std::string normalizedCategory = normalizeCategoryInput(record.getCategory(), record.getIsExpense());

        auto categoryIt = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                                       { return trim(category.getName()) == normalizedCategory; });

        if (categoryIt != this->categories.end() && categoryIt->getIsExpense() != record.getIsExpense())
        {
            report.addError("Category '" + normalizedCategory + "' type does not match record type.", lineNumber);
            continue;
        }

        if (categoryIt == this->categories.end())
        {
            this->categories.emplace_back(normalizedCategory, record.getIsExpense());
            categoriesChanged = true;
        }

        Record standardizedRec(this->nextRecordId++, record.getDate(), record.getAmount(), record.getIsExpense(), normalizedCategory);
        this->records.push_back(standardizedRec);
        acceptedCount++;
    }

    report.setSuccessCount(acceptedCount);

    if (categoriesChanged && !dataAccess.saveCategories(this->categories))
    {
        this->records = originalRecords;
        this->categories = originalCategories;
        this->nextRecordId = originalNextRecordId;
        this->lastError = "Failed to save categories.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    if (acceptedCount > 0 && !dataAccess.saveRecords(this->records))
    {
        this->records = originalRecords;
        this->categories = originalCategories;
        this->nextRecordId = originalNextRecordId;
        if (categoriesChanged)
        {
            dataAccess.saveCategories(this->categories);
        }
        this->lastError = "Failed to save records.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
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

    if (!report.hasSuccesses())
    {
        this->lastError = oss.str();
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + oss.str());
    }

    this->lastError = "";
    return Result(StatusCode::SUCCESS, "SUCCESS: " + oss.str());
}

Result LedgerController::updateRecord(int recordId, std::string date, double amount, int isExpense, std::string cat)
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
            return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
        }

        if (isExpense != -1 && isExpense != 0 && isExpense != 1)
        {
            this->lastError = "Invalid isExpense value. Use -1 (unchanged), 0 (income), or 1 (expense).";
            return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
        }

        const std::string effectiveDate = date.empty() ? rec.getDate() : date;
        const double effectiveAmount = (amount == -1.0) ? rec.getAmount() : amount;
        const bool effectiveIsExpense = (isExpense == -1) ? rec.getIsExpense() : (isExpense == 1);
        const std::string originalCategory = trim(rec.getCategory());

        std::string effectiveCategory = cat.empty() ? originalCategory : trim(cat);
        if (!cat.empty() && (effectiveCategory.empty() || effectiveCategory == "other" || effectiveCategory == "Other"))
        {
            effectiveCategory = effectiveIsExpense ? "Other Expense" : "Other Income";
        }
        else if (cat.empty() && (originalCategory == "Other Expense" || originalCategory == "Other Income") && effectiveIsExpense != rec.getIsExpense())
        {
            effectiveCategory = effectiveIsExpense ? "Other Expense" : "Other Income";
        }

        std::string errorMsg;
        if (!Record::validateData(effectiveDate, effectiveAmount, errorMsg))
        {
            this->lastError = errorMsg;
            return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + errorMsg);
        }

        auto categoryIt = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                                       { return trim(category.getName()) == effectiveCategory; });

        const bool shouldCreateCategory = (categoryIt == this->categories.end());
        if (!shouldCreateCategory && categoryIt->getIsExpense() != effectiveIsExpense)
        {
            this->lastError = "Category '" + effectiveCategory + "' type does not match the record type.";
            return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
        }

        const Record originalRecord = rec;
        const std::vector<Category> originalCategories = this->categories;

        if (shouldCreateCategory)
        {
            this->categories.emplace_back(effectiveCategory, effectiveIsExpense);
        }

        // Apply partial updates after the final target state has been validated.
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
        if (effectiveCategory != originalCategory)
        {
            rec.setCategory(effectiveCategory);
        }

        if (shouldCreateCategory && !this->dataAccess.saveCategories(this->categories))
        {
            rec = originalRecord;
            this->categories = originalCategories;
            this->lastError = "Failed to save categories.";
            return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
        }

        if (!this->dataAccess.saveRecords(this->records))
        {
            rec = originalRecord;
            if (shouldCreateCategory)
            {
                this->categories = originalCategories;
                this->dataAccess.saveCategories(this->categories);
            }
            this->lastError = "Failed to save records.";
            return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
        }

        this->lastError = "";
        std::ostringstream amountStream;
        amountStream << std::fixed << std::setprecision(2) << rec.getAmount();
        return Result(StatusCode::SUCCESS, "SUCCESS: Record ID #" + std::to_string(recordId) +
                                               " updated to Date: " + rec.getDate() +
                                               ", Amount: $" + amountStream.str() +
                                               ", Expense: " + (rec.getIsExpense() ? "Yes" : "No") +
                                               ", Category: " + rec.getCategory() + ".");
    }

    this->lastError = "Record ID #" + std::to_string(recordId) + " does not exist.";
    return Result(StatusCode::NOT_FOUND, "FAIL: Record ID #" + std::to_string(recordId) + " does not exist.");
}

Result LedgerController::removeRecord(int id)
{
    for (auto it = records.begin(); it != records.end(); ++it)
    {
        if (it->getId() == id)
        {
            const Record removedRecord = *it;
            const auto removedIndex = static_cast<std::vector<Record>::difference_type>(it - this->records.begin());
            this->records.erase(it);

            if (!this->dataAccess.saveRecords(this->records))
            {
                this->records.insert(this->records.begin() + removedIndex, removedRecord);
                this->lastError = "Failed to save records after record deletion.";
                return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
            }

            this->lastError = "";
            return Result(StatusCode::SUCCESS, "SUCCESS: Record #" + std::to_string(id) + " deleted.");
        }
    }
    this->lastError = "Record ID #" + std::to_string(id) + " does not exist.";
    return Result(StatusCode::NOT_FOUND, "FAIL: Record ID #" + std::to_string(id) + " does not exist.");
}

Result LedgerController::getLastError()
{
    if (this->lastError.empty())
    {
        return Result(StatusCode::SUCCESS, "");
    }
    else
    {
        return Result(StatusCode::UNKNOWN, this->lastError);
    }
}

std::string LedgerController::trim(const std::string &value)
{
    size_t start = value.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        return "";
    }

    size_t end = value.find_last_not_of(" \t\n\r");
    return value.substr(start, end - start + 1);
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

    if (isExpense != -1 && isExpense != 0 && isExpense != 1)
    {
        this->lastError = "Invalid isExpense value. Use -1 (all), 0 (income), or 1 (expense).";
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
        if (hasFilterValidationError(this->lastError))
        {
            return {};
        }

        this->lastError = "No records found for the specified period.";
        return {};
    }
    auto summary = this->analyzer.calculateSummary(filteredRecords);
    this->lastError = ""; // Clear last error if we have a valid summary
    return summary;
}

Result LedgerController::getTotal(std::string start, std::string end, int isExpense, std::string cat)
{

    if (!start.empty() && !end.empty() && start > end)
    {
        this->lastError = "Invalid date range: Start date cannot be after end date.";

        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
    }

    std::vector<Record> filteredRecords = getRecords(start, end, isExpense, cat); // Reuse filtering logic for consistency

    if (filteredRecords.empty())
    {
        if (hasFilterValidationError(this->lastError))
        {
            return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
        }

        this->lastError = "No records found for the specified category and period.";
        return Result(StatusCode::NOT_FOUND, "FAIL: " + this->lastError);
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
    return Result(StatusCode::SUCCESS, buffer);
}

Result LedgerController::addCategory(std::string name, bool isExpense, double budget, double warningThreshold)
{
    // 收入类别不允许设置预算和预警线
    if (!isExpense && (budget >= 0.0 || warningThreshold >= 0.0))
    {
        this->lastError = "Income category cannot have budget or warning threshold.";
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
    }
    std::string errorMsg;
    if (!Category::valid(name, budget, warningThreshold, errorMsg))
    {
        this->lastError = errorMsg;
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + errorMsg);
    }

    const std::string normalizedName = trim(name);

    // Check for duplicate category name
    for (const auto &category : this->categories)
    {
        if (trim(category.getName()) == normalizedName)
        {
            this->lastError = "Category '" + normalizedName + "' already exists.";
            return Result(StatusCode::DUPLICATE, "FAIL: " + this->lastError);
        }
    }

    Category newCategory(normalizedName, isExpense, budget, warningThreshold);
    this->categories.push_back(newCategory);
    if (!dataAccess.saveCategories(this->categories))
    {
        this->categories.pop_back();
        this->lastError = "Failed to save categories.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    this->lastError = "";
    return Result(StatusCode::SUCCESS, "SUCCESS: Category '" + normalizedName + "' added successfully.");
}

Result LedgerController::removeCategory(std::string name)
{
    const std::string normalizedName = trim(name);

    // Find the category to remove
    auto it = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                           { return trim(category.getName()) == normalizedName; });

    if (it == this->categories.end())
    {
        this->lastError = "Category '" + normalizedName + "' not found.";
        return Result(StatusCode::NOT_FOUND, "FAIL: " + this->lastError);
    }

    const std::vector<Category> originalCategories = this->categories;
    const std::vector<Record> originalRecords = this->records;

    this->categories.erase(it);
    bool needsExpenseDefault = false;
    bool needsIncomeDefault = false;

    for (Record &record : this->records)
    {
        if (trim(record.getCategory()) != normalizedName)
        {
            continue;
        }

        record.setCategory(defaultCategoryName(record.getIsExpense()));
        if (record.getIsExpense())
        {
            needsExpenseDefault = true;
        }
        else
        {
            needsIncomeDefault = true;
        }
    }

    if (needsExpenseDefault)
    {
        auto expenseDefaultIt = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                                             { return trim(category.getName()) == defaultCategoryName(true); });
        if (expenseDefaultIt == this->categories.end())
        {
            this->categories.emplace_back(defaultCategoryName(true), true);
        }
    }

    if (needsIncomeDefault)
    {
        auto incomeDefaultIt = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                                            { return trim(category.getName()) == defaultCategoryName(false); });
        if (incomeDefaultIt == this->categories.end())
        {
            this->categories.emplace_back(defaultCategoryName(false), false);
        }
    }

    if (!dataAccess.saveCategories(this->categories))
    {
        this->categories = originalCategories;
        this->records = originalRecords;
        this->lastError = "Failed to save categories.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    if (!dataAccess.saveRecords(this->records))
    {
        this->categories = originalCategories;
        this->records = originalRecords;
        dataAccess.saveCategories(this->categories);
        this->lastError = "Failed to save records after category removal.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    this->lastError = "";
    return Result(StatusCode::SUCCESS, "SUCCESS: Category '" + normalizedName + "' removed successfully.");
}

Result LedgerController::updateCategory(std::string oldName, std::string newName, int isExpense, double budget, double warningThreshold)

{
    const std::string normalizedOldName = trim(oldName);

    // Find the category to update
    auto it = std::find_if(this->categories.begin(), this->categories.end(), [&](const Category &category)
                           { return trim(category.getName()) == normalizedOldName; });

    if (it == this->categories.end())
    {
        this->lastError = "Category '" + normalizedOldName + "' not found.";
        return Result(StatusCode::NOT_FOUND, "FAIL: " + this->lastError);
    }

    if (newName.empty() && isExpense == -1 && budget == -1.0 && warningThreshold == -1.0)
    {
        this->lastError = "At least one field must be provided for update.";
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
    }

    if (isExpense != -1 && isExpense != 0 && isExpense != 1)
    {
        this->lastError = "Invalid isExpense value. Use -1 (unchanged), 0 (income), or 1 (expense).";
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
    }

    const std::string effectiveName = newName.empty() ? trim(it->getName()) : trim(newName);
    const bool effectiveIsExpense = (isExpense == -1) ? it->getIsExpense() : (isExpense == 1);
    const double effectiveBudget = (budget == -1.0) ? it->getBudget() : budget;
    const double effectiveWarningThreshold = (warningThreshold == -1.0) ? it->getWarningThreshold() : warningThreshold;
    // 收入类别不允许设置预算和预警线
    if (!effectiveIsExpense && (effectiveBudget >= 0.0 || effectiveWarningThreshold >= 0.0))
    {
        this->lastError = "Income category cannot have budget or warning threshold.";
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + this->lastError);
    }
    const bool shouldRenameRecords = effectiveName != trim(it->getName());
    const bool shouldRetypeRecords = effectiveIsExpense != it->getIsExpense();
    const bool shouldSyncRecords = shouldRenameRecords || shouldRetypeRecords;

    std::string errorMsg;
    if (!Category::valid(effectiveName, effectiveBudget, effectiveWarningThreshold, errorMsg))
    {
        this->lastError = errorMsg;
        return Result(StatusCode::VALIDATION_ERROR, "FAIL: " + errorMsg);
    }

    for (auto categoryIt = this->categories.begin(); categoryIt != this->categories.end(); ++categoryIt)
    {
        if (categoryIt != it && trim(categoryIt->getName()) == effectiveName)
        {
            this->lastError = "Category '" + effectiveName + "' already exists.";
            return Result(StatusCode::DUPLICATE, "FAIL: " + this->lastError);
        }
    }

    const Category originalCategory = *it;
    const std::vector<Record> originalRecords = this->records;

    // Update fields if new values are provided
    if (!newName.empty())
    {
        it->setName(effectiveName);
    }
    if (isExpense != -1)
    {
        it->setIsExpense(effectiveIsExpense);
    }
    if (budget >= 0.0)
    {
        it->setBudget(budget);
    }
    if (warningThreshold >= 0.0)
    {
        it->setWarningThreshold(warningThreshold);
    }

    if (shouldSyncRecords)
    {
        for (Record &record : this->records)
        {
            if (trim(record.getCategory()) == normalizedOldName)
            {
                if (shouldRetypeRecords)
                {
                    record.setIsExpense(effectiveIsExpense);
                }
                if (shouldRenameRecords || trim(record.getCategory()) != effectiveName)
                {
                    record.setCategory(effectiveName);
                }
            }
        }
    }

    if (!dataAccess.saveCategories(this->categories))
    {
        *it = originalCategory;
        this->records = originalRecords;
        this->lastError = "Failed to save categories.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    if (shouldSyncRecords && !dataAccess.saveRecords(this->records))
    {
        *it = originalCategory;
        this->records = originalRecords;
        dataAccess.saveCategories(this->categories);
        this->lastError = "Failed to save records after category update.";
        return Result(StatusCode::IO_ERROR, "FAIL: " + this->lastError);
    }

    this->lastError = "";
    std::ostringstream budgetStream;
    budgetStream << std::fixed << std::setprecision(2) << it->getBudget();
    std::ostringstream thresholdStream;
    thresholdStream << std::fixed << std::setprecision(2) << it->getWarningThreshold();
    return Result(StatusCode::SUCCESS, "SUCCESS: Category '" + normalizedOldName +
                                           "' updated successfully to '" + trim(it->getName()) +
                                           "' (Expense: " + (it->getIsExpense() ? "Yes" : "No") +
                                           ", Budget: $" + budgetStream.str() +
                                           ", Warning Threshold: $" + thresholdStream.str() + ").");
}

std::vector<Category> LedgerController::getCategories()
{
    return this->categories;
}

std::vector<BudgetStatus> LedgerController::getCurrentBudgetStatus()
{
    std::vector<BudgetStatus> budgetStatusList;

    for (const Category &cat : this->categories)
    {
        if (!cat.getIsExpense() || !cat.hasBudget())
        {
            continue;
        }

        const std::string currentDate = cat.getCurrentDate();
        const std::string monthStart = currentDate.substr(0, 7) + "-01";

        std::vector<Record> monthRecords = getRecords(monthStart, currentDate, 1, cat.getName());
        double actualSpent = 0.0;
        for (const auto &record : monthRecords)
        {
            actualSpent += record.getAmount();
        }

        BudgetStatus status = cat.getBudgetStatus(actualSpent);
        budgetStatusList.push_back(status);
    }

    this->lastError = "";
    return budgetStatusList;
}

std::pair<double, std::vector<CategoryDistItem>> LedgerController::getExpenseDistribution(std::string start, std::string end)
{
    std::pair<double, std::vector<CategoryDistItem>> distribution = {0.0, {}};

    std::vector<Record> filteredRecords = getRecords(start, end, 1, ""); // Filter for expenses in the specified period
    if (filteredRecords.empty())
    {
        if (hasFilterValidationError(this->lastError))
        {
            return distribution;
        }

        this->lastError = "No expense records found for the specified period.";
        return distribution;
    }

    distribution.second = this->analyzer.analyzeDistribution(filteredRecords);
    for (const auto &record : filteredRecords)
    {
        distribution.first += record.getAmount();
    }
    distribution.first = std::round(distribution.first * 100.0) / 100.0;

    this->lastError = "";
    return distribution;
}

std::map<std::string, double> LedgerController::getTrend(std::string start, std::string end, int isExpense, std::string cat)
{
    std::map<std::string, double> trend;
    std::vector<Record> filteredRecords = getRecords(start, end, isExpense, cat);
    if (filteredRecords.empty())
    {
        if (hasFilterValidationError(this->lastError))
        {
            return trend;
        }

        this->lastError = "No records found for the specified period.";
        return trend;
    }

    trend = this->analyzer.analyzeTrend(filteredRecords);
    this->lastError = "";
    return trend;
}
std::map<std::string, std::pair<double, double>> LedgerController::getIncomeExpense(std::string start, std::string end)
{
    std::map<std::string, std::pair<double, double>> incomeExpenseMap;
    std::vector<Record> filteredRecords = getRecords(start, end, -1, ""); // Get all records in the period
    if (filteredRecords.empty())
    {
        if (hasFilterValidationError(this->lastError))
        {
            return incomeExpenseMap;
        }

        this->lastError = "No records found for the specified period.";
        return incomeExpenseMap;
    }

    incomeExpenseMap = this->analyzer.analyzeIncomeExpense(filteredRecords);
    this->lastError = "";
    return incomeExpenseMap;
}

Record LedgerController::getRecordById(int id)
{
    for (const auto &rec : this->records)
    {
        if (rec.getId() == id)
            return rec;
    }
    return Record(-1, "", 0.0, true, "");
}