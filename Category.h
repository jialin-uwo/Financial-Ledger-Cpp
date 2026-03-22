/**
 * @file Category.h
 * @brief Declaration of the Category class.
 *
 * This file defines the Category class, which represents a transaction
 * category in the system. A category has a name, a transaction type,
 * an optional budget, and an optional warning threshold for budget monitoring.
 *
 * @author Xinyan Cai
 */

#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>
#include "BudgetStatus.h"

/**
 * @class Category
 * @brief Represents a financial category for transaction records.
 *
 * The Category class stores the basic information of a category, including
 * its name, whether it is an expense category, its budget amount,
 * and its warning threshold.
 *
 * Rules:
 * - name is required
 * - isExpense defaults to true
 * - budget defaults to -1.0
 * - warningThreshold defaults to -1.0
 *
 * In this class, -1.0 means "not set".
 *
 * If budget >= 0 and warningThreshold == -1.0, the warning threshold
 * is automatically derived as 70% of the budget during construction.
 *
 * Category does not perform duplicate-name validation. Name uniqueness
 * should be managed by outer logic such as the controller.
 *
 * Validation should be performed by calling Category::valid(...)
 * before constructing a Category object.
 */
class Category
{
private:
    /** @brief Category name. */
    std::string name;

    /** @brief True if this category is for expenses; false if for income. */
    bool isExpense;

    /**
     * @brief Budget limit for the category.
     *
     * A value of -1.0 means the budget is not set.
     */
    double budget;

    /**
     * @brief Warning threshold for the category.
     *
     * A value of -1.0 means the warning threshold is not set.
     */
    double warningThreshold;

public:
    /**
     * @brief Constructs a Category object.
     *
     * Creates a category with the given values. Validation should be done
     * before construction by calling Category::valid(...).
     *
     * If budget >= 0 and warningThreshold == -1.0, the warning threshold
     * is automatically set to 70% of the budget.
     *
     * Invalid input is expected to be intercepted by Category::valid(...)
     * before construction. The constructor does not handle the main
     * validation/error-reporting workflow.
     *
     * @param name The category name.
     * @param isExpense True if this category is for expenses; false if for income.
     *                  Defaults to true.
     * @param budget The budget limit for this category. Defaults to -1.0.
     *               A value of -1.0 means "not set".
     * @param warningThreshold The warning threshold for this category.
     *                         Defaults to -1.0. A value of -1.0 means "not set".
     */
    Category(const std::string &name,
             bool isExpense = true,
             double budget = -1.0,
             double warningThreshold = -1.0);

    /**
     * @brief Validates category input data.
     *
     * This method checks whether the given category data is valid for
     * constructing a Category object. If validation fails, an explanatory
     * message is stored in @p errorMsg.
     *
     * This method does not check duplicate names. Name uniqueness is managed
     * by outer logic.
     *
     * Validation includes:
     * - name must not be empty
     * - budget and warningThreshold must satisfy the system rules
     * - -1.0 is treated as "not set"
     *
     * @param name The category name to validate.
     * @param budget The budget value to validate.
     * @param warningThreshold The warning threshold value to validate.
     * @param errorMsg Output parameter for the validation error message.
     * @return True if the data is valid; false otherwise.
     */
    static bool valid(const std::string &name,
                      double budget,
                      double warningThreshold,
                      std::string &errorMsg);

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
     * @return The budget value. A value of -1.0 means "not set".
     */
    double getBudget() const;

    /**
     * @brief Gets the warning threshold.
     *
     * @return The warning threshold value. A value of -1.0 means "not set".
     */
    double getWarningThreshold() const;

    /**
     * @brief Sets the category name.
     *
     * @param name The new category name.
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
};

#endif