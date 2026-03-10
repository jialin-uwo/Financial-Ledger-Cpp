/**
 * @file Record.cpp
 * @brief Implementation of the Record class.
 * @author Xinyan Cai
 * This file contains the implementation of the Record data model, including
 * object construction, attribute access, and validation logic for date and amount.
 */

#include "Record.h"

#include <cmath>
#include <cstdio>
#include <regex>

/**
 * @brief Constructs a Record object.
 *
 * Initializes a financial record with the provided id, date, amount,
 * transaction type, and category. The category string is trimmed before use.
 * If the category is empty or equals "other"/"Other", a default category is
 * assigned based on whether the record represents an expense or income.
 *
 * @param id The unique identifier of the record.
 * @param date The transaction date in YYYY-MM-DD format.
 * @param amount The transaction amount.
 * @param isExpense True if the record is an expense; false if it is income.
 * @param category The category name for the transaction.
 */
Record::Record(int id, const std::string &date, double amount, bool isExpense, const std::string &category)
    : id(id), date(date), amount(amount), isExpense(isExpense)
{
    /**
     * @brief Trims leading and trailing whitespace from a string.
     *
     * @param s The input string.
     * @return A copy of the string without leading or trailing whitespace.
     */
    auto trim = [](const std::string &s) -> std::string
    {
        size_t start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
            return "";
        size_t end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    };

    std::string trimmedCategory = trim(category);

    if (trimmedCategory.empty() || trimmedCategory == "other" || trimmedCategory == "Other")
    {
        this->category = isExpense ? "Other Expense" : "Other Income";
    }
    else
    {
        this->category = trimmedCategory;
    }
}

/**
 * @brief Gets the record ID.
 *
 * @return The unique identifier of the record.
 */
int Record::getId() const
{
    return id;
}

/**
 * @brief Gets the record date.
 *
 * @return The transaction date string.
 */
std::string Record::getDate() const
{
    return date;
}

/**
 * @brief Gets the record amount.
 *
 * @return The transaction amount.
 */
double Record::getAmount() const
{
    return amount;
}

/**
 * @brief Gets whether the record is an expense.
 *
 * @return True if the record is an expense, false if it is income.
 */
bool Record::getIsExpense() const
{
    return isExpense;
}

/**
 * @brief Gets the record category.
 *
 * @return The category name of the transaction.
 */
std::string Record::getCategory() const
{
    return category;
}

/**
 * @brief Validates the date and amount for a financial record.
 *
 * This function checks that:
 * - the date is not empty,
 * - the date matches YYYY-MM-DD format,
 * - the date represents a real calendar date,
 * - the amount is not NaN,
 * - the amount is not negative,
 * - the amount is not zero.
 *
 * If validation fails, an explanatory error message is stored in @p errorMsg.
 *
 * @param date The date string to validate.
 * @param amount The amount value to validate.
 * @param errorMsg Output parameter that stores the validation error message.
 * @return True if both date and amount are valid, false otherwise.
 */
bool Record::validateData(const std::string &date,
                          double amount,
                          std::string &errorMsg)
{
    if (date.empty())
    {
        errorMsg = "Date is required.";
        return false;
    }

    std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, pattern))
    {
        errorMsg = "Date must be in YYYY-MM-DD format.";
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;

    if (std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
    {
        errorMsg = "Invalid date format.";
        return false;
    }

    if (month < 1 || month > 12)
    {
        errorMsg = "Invalid month.";
        return false;
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leapYear)
    {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1])
    {
        errorMsg = "Invalid day.";
        return false;
    }

    if (std::isnan(amount))
    {
        errorMsg = "Amount cannot be NaN.";
        return false;
    }

    if (amount < 0.0)
    {
        errorMsg = "Amount cannot be negative.";
        return false;
    }

    if (amount == 0.0)
    {
        errorMsg = "Amount cannot be zero.";
        return false;
    }

    errorMsg.clear();
    return true;
}