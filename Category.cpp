/**
 * @file Category.cpp
 * @brief Implementation of the Category class.
 *
 * This file contains the implementation of the Category data model.
 * It provides logic for constructing and validating category objects,
 * accessing and mutating category attributes, retrieving date-related
 * information, and calculating budget analysis results for a category.
 *
 * In particular, this implementation supports:
 * - validation of category names and budget-related values,
 * - automatic derivation of a default warning threshold,
 * - safe state-preserving updates through setter methods,
 * - date utilities for month-based budget tracking, and
 * - generation of a BudgetStatus summary object.
 *
 * The class is intended to support financial tracking features in which
 * each category may optionally define a monthly budget and warning level.
 *
 * @author Xinyan Cai
 */

#include "Category.h"
#include <regex>

#include <cmath>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

/**
 * @brief Constructs a Category object.
 *
 * This constructor initializes a Category instance using the supplied
 * name, expense flag, budget, and warning threshold values.
 *
 * The constructor first stores the non-name attributes in the initializer
 * list, then validates and normalizes the category name by trimming leading
 * and trailing whitespace and ensuring that the final name satisfies the
 * required format constraints. After that, it validates the full object
 * state using the static @c valid function.
 *
 * If a budget is provided but the warning threshold is not explicitly set,
 * the constructor automatically derives the warning threshold as 70% of the
 * budget value.
 *
 * An exception is thrown if the provided data does not satisfy the required
 * constraints.
 *
 * @param name The category name.
 * @param isExpense True if this category represents an expense category;
 * false if it represents an income category.
 * @param budget The budget limit for this category. A value of -1.0 means
 * no budget is set.
 * @param warningThreshold The warning threshold for this category. A value
 * of -1.0 means no warning threshold is explicitly set.
 *
 * @return None.
 *
 * @throws std::invalid_argument Thrown if the category name is invalid or
 * if the overall category state fails validation.
 *
 * @author Xinyan Cai
 */
Category::Category(const std::string &name,
                   bool isExpense,
                   double budget,
                   double warningThreshold)
    : isExpense(isExpense), budget(budget), warningThreshold(warningThreshold)
{
    /**
     * @brief Trims leading and trailing whitespace from a string.
     *
     * This local helper lambda removes spaces, tabs, and newline-related
     * whitespace characters from both ends of the input string.
     *
     * @param s The input string to trim.
     * @return A new string with leading and trailing whitespace removed.
     */
    auto trim = [](const std::string &s) -> std::string
    {
        size_t start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
            return "";
        size_t end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    };

    /// Trim the input name before validation and storage.
    std::string trimmedName = trim(name);

    /// Enforce the category naming rule: 1-32 allowed characters.
    std::regex namePattern(R"(^[A-Za-z0-9 _-]{1,32}$)");

    /// Reject an empty or whitespace-only name.
    if (trimmedName.empty())
        throw std::invalid_argument("Category ctor: name is required.");

    /// Reject names that do not satisfy the allowed character/length rule.
    if (!std::regex_match(trimmedName, namePattern))
        throw std::invalid_argument("Category ctor: name must be 1-32 chars and only contain letters, numbers, space, _, -");

    /// Store the validated and normalized name.
    this->name = trimmedName;

    /// Validate the complete object state after normalization.
    std::string err;
    if (!Category::valid(this->name, budget, warningThreshold, err))
        throw std::invalid_argument("Category ctor: " + err);

    /// Auto-derive the warning threshold when a budget exists but the warning threshold is unset.
    if (this->budget >= 0.0 && this->warningThreshold == -1.0)
        this->warningThreshold = 0.7 * this->budget;
}

/**
 * @brief Validates category input data.
 *
 * This static helper function checks whether the provided category data
 * satisfies all business rules required by the Category model.
 *
 * Validation rules include:
 * - the name must not be empty or whitespace only,
 * - the budget must be either -1.0 or a non-negative number,
 * - the warning threshold must be either -1.0 or a non-negative number,
 * - a warning threshold cannot be set if the budget is not set, and
 * - if both budget and warning threshold are set, the warning threshold
 *   must not exceed the budget.
 *
 * If validation fails, an explanatory error message is written to the
 * output parameter.
 *
 * @param name The category name to validate.
 * @param budget The budget value to validate.
 * @param warningThreshold The warning threshold value to validate.
 * @param errorMsg Output parameter that receives the validation error
 * message if validation fails. It is cleared if validation succeeds.
 *
 * @return True if the supplied data is valid; false otherwise.
 *
 * @author Xinyan Cai
 */
bool Category::valid(const std::string &name,
                     double budget,
                     double warningThreshold,
                     std::string &errorMsg)
{
    /**
     * @brief Trims leading and trailing whitespace from a string.
     *
     * This local helper lambda is used to normalize the category name
     * before checking whether it is empty.
     *
     * @param s The input string to trim.
     * @return The trimmed string.
     */
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

    /// Normalize the input name before validation.
    std::string trimmedName = trim(name);

    /// The category name must contain at least one non-whitespace character.
    if (trimmedName.empty())
    {
        errorMsg = "Category name is required.";
        return false;
    }

    /// Reject invalid numeric input for the budget.
    if (std::isnan(budget))
    {
        errorMsg = "Budget cannot be NaN.";
        return false;
    }

    /// Reject invalid numeric input for the warning threshold.
    if (std::isnan(warningThreshold))
    {
        errorMsg = "Warning threshold cannot be NaN.";
        return false;
    }

    /// Budget must either be unset (-1.0) or non-negative.
    if (budget < 0.0 && budget != -1.0)
    {
        errorMsg = "Budget must be -1.0 or a non-negative value.";
        return false;
    }

    /// Warning threshold must either be unset (-1.0) or non-negative.
    if (warningThreshold < 0.0 && warningThreshold != -1.0)
    {
        errorMsg = "Warning threshold must be -1.0 or a non-negative value.";
        return false;
    }

    /// A warning threshold cannot exist unless a budget exists.
    if (budget == -1.0 && warningThreshold != -1.0)
    {
        errorMsg = "Warning threshold cannot be set when budget is not set.";
        return false;
    }

    /// The warning threshold cannot be greater than the budget.
    if (budget >= 0.0 && warningThreshold >= 0.0 && warningThreshold > budget)
    {
        errorMsg = "Warning threshold cannot exceed budget.";
        return false;
    }

    /// Clear any previous error because validation succeeded.
    errorMsg.clear();
    return true;
}

/**
 * @brief Gets the category name.
 *
 * This accessor returns the stored category name exactly as maintained
 * by the Category object.
 *
 * @return The category name.
 *
 * @author Xinyan Cai
 */
std::string Category::getName() const
{
    return name;
}

/**
 * @brief Gets whether this is an expense category.
 *
 * This accessor indicates whether the category represents expenses
 * rather than income.
 *
 * @return True if this is an expense category; false otherwise.
 *
 * @author Xinyan Cai
 */
bool Category::getIsExpense() const
{
    return isExpense;
}

/**
 * @brief Gets the category budget.
 *
 * This accessor returns the current budget value associated with the
 * category. A value of -1.0 indicates that no budget is set.
 *
 * @return The budget value.
 *
 * @author Xinyan Cai
 */
double Category::getBudget() const
{
    return budget;
}

/**
 * @brief Gets the warning threshold.
 *
 * This accessor returns the warning threshold currently associated
 * with the category. A value of -1.0 indicates that no warning
 * threshold is set.
 *
 * @return The warning threshold value.
 *
 * @author Xinyan Cai
 */
double Category::getWarningThreshold() const
{
    return warningThreshold;
}

/**
 * @brief Sets the category name.
 *
 * This mutator attempts to update the category name while preserving
 * object validity. The new name is checked together with the current
 * budget and warning threshold values. If the resulting state is valid,
 * the name is updated; otherwise, no change is applied.
 *
 * @param name The new category name.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Category::setName(const std::string &name)
{
    /// Validate the proposed new name against the current object state.
    std::string errorMsg;
    if (Category::valid(name, budget, warningThreshold, errorMsg))
    {
        /// Apply the change only when the full state remains valid.
        this->name = name;
    }
}

/**
 * @brief Sets whether this is an expense category.
 *
 * This mutator updates the category type flag indicating whether the
 * category is used for expenses or income.
 *
 * @param isExpense True if this category should represent expenses;
 * false if it should represent income.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Category::setIsExpense(bool isExpense)
{
    this->isExpense = isExpense;
}

/**
 * @brief Sets the budget for the category.
 *
 * This mutator updates the category budget while preserving internal
 * consistency between the budget and warning threshold fields.
 *
 * The following consistency rules are applied:
 * - if the new budget is -1.0, the warning threshold is also reset to -1.0;
 * - if the new budget is non-negative and the current warning threshold is
 *   unset, a default warning threshold of 70% of the new budget is derived;
 * - if the resulting object state would be invalid, no change is applied.
 *
 * @param budget The new budget value.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Category::setBudget(double budget)
{
    /// Work with temporary values first so the object is only changed if valid.
    double newBudget = budget;
    double newWarningThreshold = this->warningThreshold;

    /// Removing the budget also removes the warning threshold.
    if (newBudget == -1.0)
    {
        newWarningThreshold = -1.0;
    }
    /// If a budget is newly available and no warning threshold exists, derive one automatically.
    else if (newBudget >= 0.0 && newWarningThreshold == -1.0)
    {
        newWarningThreshold = 0.7 * newBudget;
    }

    /// Validate the proposed state before committing changes.
    std::string errorMsg;
    if (Category::valid(this->name, newBudget, newWarningThreshold, errorMsg))
    {
        /// Apply both values together only if the combined state is valid.
        this->budget = newBudget;
        this->warningThreshold = newWarningThreshold;
    }
}

/**
 * @brief Sets the warning threshold for the category.
 *
 * This mutator attempts to update the warning threshold while preserving
 * overall object validity. If the proposed threshold is incompatible with
 * the current category name or budget, the object remains unchanged.
 *
 * @param warningThreshold The new warning threshold value.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Category::setWarningThreshold(double warningThreshold)
{
    /// Validate the proposed warning threshold against the current state.
    std::string errorMsg;
    if (Category::valid(this->name, this->budget, warningThreshold, errorMsg))
    {
        /// Apply the change only when it does not invalidate the object.
        this->warningThreshold = warningThreshold;
    }
}

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
bool Category::hasBudget() const
{
    return budget >= 0.0;
}

/**
 * @brief Gets the current system date in YYYY-MM-DD format.
 *
 * This utility function retrieves the current local system date and
 * formats it as a string in ISO-like YYYY-MM-DD format.
 *
 * @return The current date as a string in YYYY-MM-DD format.
 *
 * @author Xinyan Cai
 */
std::string Category::getCurrentDate() const
{
    /// Retrieve the current system time.
    std::time_t now = std::time(nullptr);

    /// Convert the time value to local calendar time.
    std::tm *localTime = std::localtime(&now);

    /// Format the date as YYYY-MM-DD.
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d");
    return oss.str();
}

/**
 * @brief Calculates the number of remaining days in the month.
 *
 * This function parses a date string in YYYY-MM-DD format and returns
 * the number of days remaining in that month, including the current day.
 *
 * The function validates the parsed year, month, and day values. If the
 * date string is malformed or contains an invalid calendar date, the
 * function returns -1.
 *
 * Leap years are handled for February.
 *
 * @param currentDate The date string in YYYY-MM-DD format.
 *
 * @return The number of remaining days in the month, including the current
 * day, or -1 if the input date is invalid.
 *
 * @author Xinyan Cai
 */
int Category::getDaysRemainingInMonth(const std::string &currentDate) const
{
    /// Parsed date components.
    int year = 0;
    int month = 0;
    int day = 0;

    /// Parse the input string into year, month, and day components.
    if (std::sscanf(currentDate.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
    {
        return -1;
    }

    /// Reject an invalid month value.
    if (month < 1 || month > 12)
    {
        return -1;
    }

    /// Default day counts for each month in a non-leap year.
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /// Determine whether the year is a leap year.
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

    /// Adjust February for leap years.
    if (leapYear)
    {
        daysInMonth[1] = 29;
    }

    /// Reject an invalid day value for the given month.
    if (day < 1 || day > daysInMonth[month - 1])
    {
        return -1;
    }

    /// Return the number of days remaining, including today.
    return daysInMonth[month - 1] - day + 1;
}

/**
 * @brief Computes the budget status for the current month.
 *
 * This function creates and returns a @c BudgetStatus object summarizing
 * the category's current budget condition based on the provided amount
 * spent in the current month.
 *
 * The returned status includes:
 * - the category name,
 * - the actual amount spent,
 * - the budget limit,
 * - the number of days remaining in the current month,
 * - the remaining budget amount,
 * - the estimated daily available spending for the rest of the month, and
 * - the overall budget health classification.
 *
 * If the category does not have a budget, budget-related fields remain
 * unset where appropriate and the budget health is marked as Unknown.
 *
 * @param amount The total amount spent in this category for the current month.
 *
 * @return A @c BudgetStatus object describing the category's current
 * budget condition.
 *
 * @author Xinyan Cai
 */
BudgetStatus Category::getBudgetStatus(double amount) const
{
    /// Create the status result object to be populated and returned.
    BudgetStatus status;

    /// Populate the identifying and input-based fields.
    status.categoryName = name;
    status.actualSpent = amount;
    status.budgetLimit = budget;

    /// Determine today's date and compute the remaining days in the month.
    std::string currentDate = getCurrentDate();
    status.daysRemaining = getDaysRemainingInMonth(currentDate);

    /// If no budget exists, return an unknown budget status with unset numeric analysis fields.
    if (!hasBudget())
    {
        status.remaining = -1.0;
        status.dailyAvailable = -1.0;
        status.budgetHealth = BudgetHealth::Unknown;
        return status;
    }

    /// Compute the remaining budget amount.
    status.remaining = budget - amount;

    /// Compute the average available spending per remaining day when possible.
    if (status.daysRemaining > 0)
    {
        status.dailyAvailable = status.remaining / status.daysRemaining;
    }
    else
    {
        status.dailyAvailable = -1.0;
    }

    /// Classify the budget condition based on the spent amount and thresholds.
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

    /// Return the fully computed budget analysis result.
    return status;
}