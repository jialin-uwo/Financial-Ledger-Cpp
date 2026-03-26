/**
 * @file Category.cpp
 * @brief Implementation of the Category class.
 *
 * This file contains the implementation of the Category data model,
 * including construction, validation, date utilities, and budget
 * status calculation.
 *
 * @author Xinyan Cai
 */

#include "Category.h"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

/**
 * @brief Constructs a Category object.
 *
 * If budget is set and warningThreshold is not set, the warning threshold
 * is automatically derived as 70% of the budget.
 *
 * @param name The category name.
 * @param isExpense True if this category is for expenses; false if for income.
 * @param budget The budget limit for this category.
 * @param warningThreshold The warning threshold for this category.
 */
Category::Category(const std::string &name,
                   bool isExpense,
                   double budget,
                   double warningThreshold)
    : name(name), isExpense(isExpense), budget(budget), warningThreshold(warningThreshold)
{
    if (this->budget >= 0.0 && this->warningThreshold == -1.0)
    {
        this->warningThreshold = 0.7 * this->budget;
    }
}

/**
 * @brief Validates category input data.
 *
 * Validation rules:
 * - name must not be empty or whitespace only
 * - budget must be -1.0 or non-negative
 * - warningThreshold must be -1.0 or non-negative
 * - if budget is not set, warningThreshold cannot be set
 * - if both are set, warningThreshold must not exceed budget
 *
 * @param name The category name to validate.
 * @param budget The budget value to validate.
 * @param warningThreshold The warning threshold value to validate.
 * @param errorMsg Output parameter for the validation error message.
 * @return True if the data is valid; false otherwise.
 */
bool Category::valid(const std::string &name,
                     double budget,
                     double warningThreshold,
                     std::string &errorMsg)
{
    auto trim = [](const std::string &s) -> std::string
    {
        size_t start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
        {
            return "";
        }

        size_t end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    };

    std::string trimmedName = trim(name);

    if (trimmedName.empty())
    {
        errorMsg = "Category name is required.";
        return false;
    }

    if (std::isnan(budget))
    {
        errorMsg = "Budget cannot be NaN.";
        return false;
    }

    if (std::isnan(warningThreshold))
    {
        errorMsg = "Warning threshold cannot be NaN.";
        return false;
    }

    if (budget < 0.0 && budget != -1.0)
    {
        errorMsg = "Budget must be -1.0 or a non-negative value.";
        return false;
    }

    if (warningThreshold < 0.0 && warningThreshold != -1.0)
    {
        errorMsg = "Warning threshold must be -1.0 or a non-negative value.";
        return false;
    }

    if (budget == -1.0 && warningThreshold != -1.0)
    {
        errorMsg = "Warning threshold cannot be set when budget is not set.";
        return false;
    }

    if (budget >= 0.0 && warningThreshold >= 0.0 && warningThreshold > budget)
    {
        errorMsg = "Warning threshold cannot exceed budget.";
        return false;
    }

    errorMsg.clear();
    return true;
}

/**
 * @brief Gets the category name.
 *
 * @return The category name.
 */
std::string Category::getName() const
{
    return name;
}

/**
 * @brief Gets whether this is an expense category.
 *
 * @return True if this is an expense category; false otherwise.
 */
bool Category::getIsExpense() const
{
    return isExpense;
}

/**
 * @brief Gets the category budget.
 *
 * @return The budget value.
 */
double Category::getBudget() const
{
    return budget;
}

/**
 * @brief Gets the warning threshold.
 *
 * @return The warning threshold value.
 */
double Category::getWarningThreshold() const
{
    return warningThreshold;
}

/**
 * @brief Sets the category name.
 *
 * This method applies consistency protection. If the new name would make
 * the object invalid, the name is not changed.
 *
 * @param name The new category name.
 */
void Category::setName(const std::string &name)
{
    std::string errorMsg;
    if (Category::valid(name, budget, warningThreshold, errorMsg))
    {
        this->name = name;
    }
}

/**
 * @brief Sets whether this is an expense category.
 *
 * @param isExpense True if this is an expense category; false otherwise.
 */
void Category::setIsExpense(bool isExpense)
{
    this->isExpense = isExpense;
}

/**
 * @brief Sets the budget for the category.
 *
 * Consistency protection rules:
 * - If the new budget is -1.0, the warning threshold is also reset to -1.0.
 * - If the new budget is non-negative and the current warning threshold is -1.0,
 *   the warning threshold is automatically derived as 70% of the new budget.
 * - If the resulting state is invalid, no change is applied.
 *
 * @param budget The new budget value.
 */
void Category::setBudget(double budget)
{
    double newBudget = budget;
    double newWarningThreshold = this->warningThreshold;

    if (newBudget == -1.0)
    {
        newWarningThreshold = -1.0;
    }
    else if (newBudget >= 0.0 && newWarningThreshold == -1.0)
    {
        newWarningThreshold = 0.7 * newBudget;
    }

    std::string errorMsg;
    if (Category::valid(this->name, newBudget, newWarningThreshold, errorMsg))
    {
        this->budget = newBudget;
        this->warningThreshold = newWarningThreshold;
    }
}

/**
 * @brief Sets the warning threshold for the category.
 *
 * This method applies consistency protection. If the new warning threshold
 * would make the object invalid, no change is applied.
 *
 * @param warningThreshold The new warning threshold value.
 */
void Category::setWarningThreshold(double warningThreshold)
{
    std::string errorMsg;
    if (Category::valid(this->name, this->budget, warningThreshold, errorMsg))
    {
        this->warningThreshold = warningThreshold;
    }
}

/**
 * @brief Checks whether the category has a valid budget.
 *
 * @return True if the budget is non-negative; false otherwise.
 */
bool Category::hasBudget() const
{
    return budget >= 0.0;
}

/**
 * @brief Gets the current system date in YYYY-MM-DD format.
 *
 * @return The current date as a string in YYYY-MM-DD format.
 */
std::string Category::getCurrentDate() const
{
    std::time_t now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d");
    return oss.str();
}

/**
 * @brief Calculates the number of remaining days in the month.
 *
 * This implementation includes the current day in the remaining day count.
 *
 * @param currentDate The date string in YYYY-MM-DD format.
 * @return The number of remaining days in the current month, or -1 if the date is invalid.
 */
int Category::getDaysRemainingInMonth(const std::string &currentDate) const
{
    int year = 0;
    int month = 0;
    int day = 0;

    if (std::sscanf(currentDate.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
    {
        return -1;
    }

    if (month < 1 || month > 12)
    {
        return -1;
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

    if (leapYear)
    {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1])
    {
        return -1;
    }

    return daysInMonth[month - 1] - day + 1;
}

/**
 * @brief Computes the budget status for the current month.
 *
 * If the category budget is not set, budget-related numeric fields remain
 * unset where appropriate and the budget health is set to Unknown.
 *
 * @param amount The total amount spent in this category for the current month.
 * @return A BudgetStatus object describing the current budget condition.
 */
BudgetStatus Category::getBudgetStatus(double amount) const
{
    BudgetStatus status;

    status.categoryName = name;
    status.actualSpent = amount;
    status.budgetLimit = budget;

    std::string currentDate = getCurrentDate();
    status.daysRemaining = getDaysRemainingInMonth(currentDate);

    if (!hasBudget())
    {
        status.remaining = -1.0;
        status.dailyAvailable = -1.0;
        status.budgetHealth = BudgetHealth::Unknown;
        return status;
    }

    status.remaining = budget - amount;

    if (status.daysRemaining > 0)
    {
        status.dailyAvailable = status.remaining / status.daysRemaining;
    }
    else
    {
        status.dailyAvailable = -1.0;
    }

    if (amount > budget)
    {
        status.budgetHealth = BudgetHealth::Exceeded;
    }
    else if (warningThreshold >= 0.0 && amount >= warningThreshold)
    {
        status.budgetHealth = BudgetHealth::Warning;
    }
    else
    {
        status.budgetHealth = BudgetHealth::Safe;
    }

    return status;
}