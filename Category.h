/**
 * @file Category.h
 * @brief Declaration of the Category class.
 *
 * This file defines the Category class, which represents a financial
 * transaction category in the system. A category stores identifying
 * information such as its name and transaction type, and may also
 * include optional budget-related settings used for budget tracking
 * and health analysis.
 *
 * The class supports validation, controlled state updates, date-based
 * budget calculations, and generation of budget summary results through
 * the BudgetStatus structure.
 *
 * A budget value of -1.0 indicates that no budget is set, and a warning
 * threshold value of -1.0 indicates that no warning threshold is set.
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
 * The Category class stores the basic properties of a transaction category,
 * including its name, whether it represents expenses or income, its optional
 * budget, and its optional warning threshold.
 *
 * This class is intended to support financial record categorization and
 * budget monitoring. It provides accessor and mutator methods for category
 * properties, static validation logic for category input data, utility
 * methods for date-based calculations, and a method for computing an
 * overall BudgetStatus summary.
 *
 * Rules enforced by the class include:
 * - the name is required,
 * - @c isExpense defaults to @c true,
 * - @c budget defaults to @c -1.0,
 * - @c warningThreshold defaults to @c -1.0,
 * - a value of @c -1.0 means "not set",
 * - if a budget is set and the warning threshold is not set, the warning
 *   threshold may be automatically derived as 70% of the budget.
 *
 * This class does not perform duplicate-name validation. Name uniqueness
 * is expected to be enforced by outer program logic such as a controller
 * or manager class.
 *
 * @author Xinyan Cai
 */
class Category
{
private:
    /** @brief Stores the category name. */
    std::string name;

    /**
     * @brief Indicates whether this category is for expenses.
     *
     * A value of @c true means the category is used for expense records.
     * A value of @c false means the category is used for income records.
     */
    bool isExpense;

    /**
     * @brief Stores the budget limit for the category.
     *
     * A value of @c -1.0 indicates that the budget is not set.
     */
    double budget;

    /**
     * @brief Stores the warning threshold for the category.
     *
     * A value of @c -1.0 indicates that the warning threshold is not set.
     */
    double warningThreshold;

public:
    /**
     * @brief Constructs a Category object.
     *
     * This constructor initializes a Category object with the provided
     * name, transaction type, budget, and warning threshold values.
     *
     * If a valid budget is provided and the warning threshold is left
     * unset, the implementation may automatically derive the warning
     * threshold as 70% of the budget.
     *
     * @param name The category name.
     * @param isExpense True if this category is for expenses; false if it
     * is for income. Defaults to true.
     * @param budget The budget limit for this category. Defaults to -1.0.
     * A value of -1.0 means the budget is not set.
     * @param warningThreshold The warning threshold for this category.
     * Defaults to -1.0. A value of -1.0 means the warning threshold is
     * not set.
     *
     * @return None.
     *
     * @throws std::invalid_argument Thrown if the name is invalid or if
     * the provided values fail category validation.
     *
     * @author Xinyan Cai
     */
    Category(const std::string &name,
             bool isExpense = true,
             double budget = -1.0,
             double warningThreshold = -1.0);

    /**
     * @brief Sets the budget for the category.
     *
     * This method attempts to update the category budget while preserving
     * object validity and internal consistency rules.
     *
     * @param budget The new budget value.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setBudget(double budget);

    /**
     * @brief Sets the warning threshold for the category.
     *
     * This method attempts to update the warning threshold while preserving
     * overall category validity.
     *
     * @param warningThreshold The new warning threshold value.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setWarningThreshold(double warningThreshold);

    /**
     * @brief Validates category input data.
     *
     * This static method checks whether the supplied category data satisfies
     * the rules required for a valid Category object.
     *
     * If validation fails, an explanatory message is written into
     * @p errorMsg. This method does not check whether another category
     * already uses the same name; duplicate-name validation is handled
     * by outer logic.
     *
     * Validation includes:
     * - the name must not be empty,
     * - the budget and warning threshold must satisfy the system rules,
     * - a value of @c -1.0 is treated as "not set".
     *
     * @param name The category name to validate.
     * @param budget The budget value to validate.
     * @param warningThreshold The warning threshold value to validate.
     * @param errorMsg Output parameter that receives the validation error
     * message if validation fails.
     *
     * @return True if the data is valid; false otherwise.
     *
     * @author Xinyan Cai
     */
    static bool valid(const std::string &name,
                      double budget,
                      double warningThreshold,
                      std::string &errorMsg);

    /**
     * @brief Gets the category name.
     *
     * @return The category name.
     *
     * @author Xinyan Cai
     */
    std::string getName() const;

    /**
     * @brief Gets whether this is an expense category.
     *
     * @return True if this is an expense category; false otherwise.
     *
     * @author Xinyan Cai
     */
    bool getIsExpense() const;

    /**
     * @brief Gets the category budget.
     *
     * @return The budget value. A value of -1.0 means the budget is not set.
     *
     * @author Xinyan Cai
     */
    double getBudget() const;

    /**
     * @brief Gets the warning threshold.
     *
     * @return The warning threshold value. A value of -1.0 means the warning
     * threshold is not set.
     *
     * @author Xinyan Cai
     */
    double getWarningThreshold() const;

    /**
     * @brief Sets the category name.
     *
     * This method attempts to update the category name while preserving
     * the validity of the overall object state.
     *
     * @param name The new category name.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setName(const std::string &name);

    /**
     * @brief Sets whether this is an expense category.
     *
     * This method updates the transaction type flag for the category.
     *
     * @param isExpense True if this is an expense category; false if this
     * is an income category.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setIsExpense(bool isExpense);

    /**
     * @brief Checks whether the category has a valid budget.
     *
     * A category is considered to have a valid budget if its budget value
     * is non-negative.
     *
     * @return True if the budget is non-negative; false otherwise.
     *
     * @author Xinyan Cai
     */
    bool hasBudget() const;

    /**
     * @brief Gets the current system date in YYYY-MM-DD format.
     *
     * This method retrieves the current local system date and returns it
     * as a formatted string suitable for later date calculations.
     *
     * @return The current date as a string in YYYY-MM-DD format.
     *
     * @author Xinyan Cai
     */
    std::string getCurrentDate() const;

    /**
     * @brief Calculates the number of remaining days in the month.
     *
     * This method takes a date string in YYYY-MM-DD format, validates
     * the date, and computes how many days remain in that month.
     *
     * @param currentDate The date string in YYYY-MM-DD format.
     *
     * @return The number of remaining days in the current month, or -1
     * if the input date is invalid.
     *
     * @author Xinyan Cai
     */
    int getDaysRemainingInMonth(const std::string &currentDate) const;

    /**
     * @brief Computes the budget status for the current month.
     *
     * This method evaluates the given spending amount against the category's
     * budget and warning threshold. It also obtains the current system date,
     * determines how many days remain in the current month, and fills a
     * BudgetStatus object with the resulting analysis.
     *
     * @param amount The total amount spent in this category for the current month.
     *
     * @return A BudgetStatus object describing the current budget condition.
     *
     * @author Xinyan Cai
     */
    BudgetStatus getBudgetStatus(double amount) const;
};

#endif