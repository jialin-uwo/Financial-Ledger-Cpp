/**
 * @file Record.cpp
 * @brief Implementation of the Record class.
 *
 * This file contains the implementation of the Record data model.
 * It defines construction logic, attribute accessors, mutator methods,
 * category normalization behavior, and validation rules for transaction
 * date and amount values.
 *
 * The implementation ensures that records maintain a consistent internal
 * state, including automatic handling of default categories for income
 * and expense records.
 *
 * @author Xinyan Cai
 */

#include "Record.h"

#include <cmath>
#include <cstdio>
#include <regex>

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * This helper function removes whitespace characters from the beginning
 * and end of a string. It is used to normalize category input before
 * storing it in a Record object.
 *
 * @param s The input string.
 * @return A copy of the string without leading or trailing whitespace.
 *
 * @author Xinyan Cai
 */
static std::string trimString(const std::string &s)
{
    /// Find the first non-whitespace character.
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        /// Return an empty string if the input contains only whitespace.
        return "";
    }

    /// Find the last non-whitespace character.
    size_t end = s.find_last_not_of(" \t\n\r");

    /// Return the trimmed substring.
    return s.substr(start, end - start + 1);
}

/**
 * @brief Constructs a Record object.
 *
 * This constructor initializes a financial record using the provided
 * identifier, date, amount, transaction type, and category.
 *
 * Before storing the date and amount, the constructor validates them
 * using @c Record::validateData. If validation fails, an exception is
 * thrown. The category is then assigned through @c setCategory so that
 * whitespace trimming and default-category rules are applied consistently.
 *
 * @param id The unique identifier of the record.
 * @param date The transaction date in YYYY-MM-DD format.
 * @param amount The transaction amount.
 * @param isExpense True if the record is an expense; false if it is income.
 * @param category The category name for the transaction.
 *
 * @return None.
 *
 * @throws std::invalid_argument Thrown if the date or amount is invalid.
 *
 * @author Xinyan Cai
 */
Record::Record(int id,
               const std::string &date,
               double amount,
               bool isExpense,
               const std::string &category)
    : id(id), isExpense(isExpense)
{
    /// Validate the date and amount before storing them.
    std::string err;
    if (!Record::validateData(date, amount, err))
        throw std::invalid_argument("Record ctor: " + err);

    /// Store the validated date.
    this->date = date;

    /// Store the validated amount.
    this->amount = amount;

    /// Normalize and assign the category.
    setCategory(category);
}

/**
 * @brief Gets the record ID.
 *
 * This accessor returns the unique identifier associated with the record.
 *
 * @return The unique identifier of the record.
 *
 * @author Xinyan Cai
 */
int Record::getId() const
{
    return id;
}

/**
 * @brief Gets the record date.
 *
 * This accessor returns the transaction date stored in the record.
 *
 * @return The transaction date string.
 *
 * @author Xinyan Cai
 */
std::string Record::getDate() const
{
    return date;
}

/**
 * @brief Gets the record amount.
 *
 * This accessor returns the monetary amount stored in the record.
 *
 * @return The transaction amount.
 *
 * @author Xinyan Cai
 */
double Record::getAmount() const
{
    return amount;
}

/**
 * @brief Gets whether the record is an expense.
 *
 * This accessor indicates whether the record represents an expense
 * transaction or an income transaction.
 *
 * @return True if the record is an expense, false if it is income.
 *
 * @author Xinyan Cai
 */
bool Record::getIsExpense() const
{
    return isExpense;
}

/**
 * @brief Gets the record category.
 *
 * This accessor returns the category currently assigned to the record.
 *
 * @return The category name of the transaction.
 *
 * @author Xinyan Cai
 */
std::string Record::getCategory() const
{
    return category;
}

/**
 * @brief Sets the record ID.
 *
 * This mutator updates the unique identifier of the record.
 *
 * @param id The new unique identifier of the record.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Record::setId(int id)
{
    this->id = id;
}

/**
 * @brief Sets the record date.
 *
 * This mutator updates the stored transaction date.
 * The method directly assigns the new value without performing validation.
 *
 * @param date The new transaction date string.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Record::setDate(const std::string &date)
{
    this->date = date;
}

/**
 * @brief Sets the record amount.
 *
 * This mutator updates the stored transaction amount.
 * The method directly assigns the new value without performing validation.
 *
 * @param amount The new transaction amount.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Record::setAmount(double amount)
{
    this->amount = amount;
}

/**
 * @brief Sets whether the record is an expense.
 *
 * This mutator updates the transaction type of the record.
 * If the current category is one of the system default categories
 * ("Other Expense" or "Other Income"), the category is automatically
 * updated to stay consistent with the new transaction type.
 *
 * @param isExpense True if the record is an expense, false if it is income.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Record::setIsExpense(bool isExpense)
{
    /// Check whether the current category is one of the auto-managed defaults.
    bool wasDefaultCategory =
        (category == "Other Expense" || category == "Other Income");

    /// Update the transaction type flag.
    this->isExpense = isExpense;

    /// Keep the default category aligned with the new transaction type.
    if (wasDefaultCategory)
    {
        category = isExpense ? "Other Expense" : "Other Income";
    }
}

/**
 * @brief Sets the record category.
 *
 * This mutator trims leading and trailing whitespace from the provided
 * category string before applying it.
 *
 * If the resulting category is empty, or if it equals "other" or "Other",
 * the method assigns a default category based on whether the record
 * represents an expense or income transaction.
 *
 * @param category The new category name.
 *
 * @return None.
 *
 * @author Xinyan Cai
 */
void Record::setCategory(const std::string &category)
{
    /// Normalize the category string by trimming surrounding whitespace.
    std::string trimmedCategory = trimString(category);

    /// Assign a default category when the input is empty or generic.
    if (trimmedCategory.empty() || trimmedCategory == "other" || trimmedCategory == "Other")
    {
        this->category = isExpense ? "Other Expense" : "Other Income";
    }
    else
    {
        /// Otherwise, store the normalized category name as provided.
        this->category = trimmedCategory;
    }
}

/**
 * @brief Validates the date and amount for a financial record.
 *
 * This function checks whether the supplied date and amount satisfy
 * the rules required for a valid Record object.
 *
 * Validation rules include:
 * - the date must not be empty,
 * - the date must match YYYY-MM-DD format,
 * - the date must represent a valid calendar date,
 * - the amount must not be NaN,
 * - the amount must not be negative,
 * - the amount must not be zero.
 *
 * If validation fails, an explanatory error message is written into
 * the output parameter @p errorMsg.
 *
 * @param date The date string to validate.
 * @param amount The amount value to validate.
 * @param errorMsg Output parameter that stores the validation error message.
 *
 * @return True if both the date and amount are valid; false otherwise.
 *
 * @author Xinyan Cai
 */
bool Record::validateData(const std::string &date,
                          double amount,
                          std::string &errorMsg)
{
    /// The date field is required.
    if (date.empty())
    {
        errorMsg = "Date is required.";
        return false;
    }

    // First parse a relaxed Y-M-D shape so we can return calendar-specific errors.
    std::smatch match;
    std::regex loosePattern(R"(^(\d{4})-(\d{1,2})-(\d{1,2})$)");
    if (!std::regex_match(date, match, loosePattern))
    {
        errorMsg = "Date must be in YYYY-MM-DD format.";
        return false;
    }

    /// Parsed calendar components.
    int year = std::stoi(match[1]);
    int month = std::stoi(match[2]);
    int day = std::stoi(match[3]);

    /// Validate the month range.
    if (month < 1 || month > 12)
    {
        errorMsg = "Invalid month.";
        return false;
    }

    /// Day counts for each month in a non-leap year.
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /// Determine whether the parsed year is a leap year.
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

    /// Adjust February for leap years.
    if (leapYear)
    {
        daysInMonth[1] = 29;
    }

    /// Validate the day range for the parsed month and year.
    if (day < 1 || day > daysInMonth[month - 1])
    {
        errorMsg = "Invalid date.";
        return false;
    }

    // Keep canonical storage format for reliable lexical date comparisons.
    std::regex strictPattern(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, strictPattern))
    {
        errorMsg = "Date must use zero-padded YYYY-MM-DD format (e.g., 2026-02-28).";
        return false;
    }

    /// Reject NaN values for the amount.
    if (std::isnan(amount))
    {
        errorMsg = "Amount cannot be NaN.";
        return false;
    }

    /// Reject negative amounts.
    if (amount < 0.0)
    {
        errorMsg = "Amount cannot be negative.";
        return false;
    }

    /// Reject zero amounts.
    if (amount == 0.0)
    {
        errorMsg = "Amount cannot be zero.";
        return false;
    }

    /// Clear any previous error because validation succeeded.
    errorMsg.clear();
    return true;
}