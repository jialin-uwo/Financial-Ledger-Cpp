/**
 * @file Category.h
 * @brief Declaration of the Category class.
 *
 * This file defines the Category class, which represents a transaction
 * category in the system. A category has a unique name, a transaction type,
 * an optional budget, and a warning threshold for budget monitoring.
 *
 * @author Xinyan Cai
 */

#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>
#include <unordered_set>
#include "BudgetStatus.h"

/**
 * @class Category
 * @brief Represents a financial category for transaction records.
 *
 * The Category class stores the basic information of a category, including
 * its unique name, whether it is an expense category, its budget amount,
 * and its warning threshold.
 *
 * Rules:
 * - name is required and must be unique
 * - isExpense defaults to true
 * - budget defaults to -0.1
 * - warningThreshold defaults to 70% of budget if not explicitly provided
 */
class Category
{
private:
    /** @brief Unique category name. */
    std::string name;

    /** @brief True if this category is for expenses; false if for income. */
    bool isExpense;

    /** @brief Budget limit for the category. */
    double budget;

    /** @brief Warning threshold for the category. */
    double warningThreshold;

    /**
     * @brief Tracks all used category names to enforce uniqueness.
     */
    static std::unordered_set<std::string> usedNames;

public:
    /**
     * @brief Constructs a Category object.
     *
     * Creates a category with a required unique name.
     * If @p warningThreshold is not explicitly provided, it is set to 70%
     * of @p budget during construction.
     *
     * @param name The category name. Must not be empty or duplicated.
     * @param isExpense True if this category is for expenses; false if for income.
     *                  Defaults to true.
     * @param budget The budget limit for this category. Defaults to -0.1.
     * @param warningThreshold The warning threshold for this category.
     *        If set to -1.0, it will be initialized to 70% of @p budget.
     *
     * @throws std::invalid_argument If the name is empty.
     * @throws std::runtime_error If the name already exists.
     */
    Category(const std::string &name,
             bool isExpense = true,
             double budget = -0.1,
             double warningThreshold = -1.0);

    /**
     * @brief Destroys the Category object.
     *
     * Removes the category name from the uniqueness registry.
     */
    ~Category();

    /**
     * @brief Gets the category name.
     *
     * @return The category name.
     */
    std::string getName() const;

    /**
     * @brief Gets whether this is an expense category.
     *
     * @return True if this is an expense category; false otherwise.
     */
    bool getIsExpense() const;

    /**
     * @brief Gets the category budget.
     *
     * @return The budget value.
     */
    double getBudget() const;

    /**
     * @brief Gets the warning threshold.
     *
     * @return The warning threshold value.
     */
    double getWarningThreshold() const;

    /**
     * @brief Sets the category name.
     *
     * The new name must not be empty and must remain unique.
     *
     * @param name The new category name.
     *
     * @throws std::invalid_argument If the name is empty.
     * @throws std::runtime_error If the name already exists.
     */
    void setName(const std::string &name);

    /**
     * @brief Sets whether this is an expense category.
     *
     * @param isExpense True if this is an expense category; false otherwise.
     */
    void setIsExpense(bool isExpense);

    /**
     * @brief Sets the budget for the category.
     *
     * @param budget The new budget value.
     */
    void setBudget(double budget);

    /**
     * @brief Sets the warning threshold for the category.
     *
     * @param warningThreshold The new warning threshold value.
     */
    void setWarningThreshold(double warningThreshold);

    /**
     * @brief Checks whether the category has a valid budget.
     *
     * @return True if the budget is non-negative; false otherwise.
     */
    bool hasBudget() const;

    /**
     * @brief Gets the current system date in YYYY-MM-DD format.
     *
     * This method retrieves the current local system date and returns it
     * as a formatted string.
     *
     * @return The current date as a string in YYYY-MM-DD format.
     */
    std::string getCurrentDate() const;

    /**
     * @brief Calculates the number of remaining days in the month.
     *
     * This method takes a date string in YYYY-MM-DD format and computes
     * how many days remain in that month.
     *
     * @param currentDate The date string in YYYY-MM-DD format.
     * @return The number of remaining days in the current month.
     */
    int getDaysRemainingInMonth(const std::string &currentDate) const;

    /**
     * @brief Computes the budget status for the current month.
     *
     * This method evaluates the given monthly spending amount against the
     * category budget and warning threshold. It also obtains the current
     * system date, calculates the number of remaining days in the current
     * month, and fills a BudgetStatus object with the computed results.
     *
     * @param amount The total amount spent in this category for the current month.
     * @return A BudgetStatus object describing the current budget condition.
     */
    BudgetStatus getBudgetStatus(double amount) const;

    /**
     * @brief Checks whether a category name already exists.
     *
     * @param name The category name to check.
     * @return True if the name already exists; false otherwise.
     */
    static bool exists(const std::string &name);
};

#endif