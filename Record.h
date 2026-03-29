/**
 * @file Record.h
 * @brief Declaration of the Record class.
 *
 * This file defines the Record class, which represents a financial transaction
 * record in the system. It includes core attributes such as id, date, amount,
 * transaction type, and category, as well as accessor and mutator methods
 * and validation logic.
 *
 * @author Xinyan Cai
 */

#ifndef RECORD_H
#define RECORD_H

#include <string>

/**
 * @class Record
 * @brief Represents a single financial transaction record.
 *
 * The Record class stores the essential information for one transaction,
 * including its identifier, date, amount, whether it is an expense,
 * and its category. It provides getter and setter methods for accessing
 * and modifying these attributes, as well as a static validation function
 * for checking date and amount input.
 */
class Record
{
private:
    /** @brief Unique identifier for the record. */
    int id;

    /** @brief Transaction date in YYYY-MM-DD format. */
    std::string date;

    /** @brief Monetary amount of the transaction. */
    double amount;

    /** @brief Indicates whether the transaction is an expense. */
    bool isExpense;

    /** @brief Category name associated with the transaction. */
    std::string category;

public:
    /**
     * @brief Constructs a Record object.
     *
     * 所有参数都做严格校验，校验不通过抛出std::invalid_argument。
     */
    Record(int id,
           const std::string &date,
           double amount,
           bool isExpense = true,
           const std::string &category = "");

    /**
     * @brief Gets the record ID.
     *
     * @return The unique identifier of the record.
     */
    int getId() const;

    /**
     * @brief Gets the transaction date.
     *
     * @return The date string of the record.
     */
    std::string getDate() const;

    /**
     * @brief Gets the transaction amount.
     *
     * @return The monetary amount of the record.
     */
    double getAmount() const;

    /**
     * @brief Gets whether the record is an expense.
     *
     * @return True if the record is an expense; false if it is income.
     */
    bool getIsExpense() const;

    /**
     * @brief Gets the category of the transaction.
     *
     * @return The category string of the record.
     */
    std::string getCategory() const;

    /**
     * @brief Sets the record ID.
     *
     * @param id The new unique identifier of the record.
     */
    void setId(int id);

    /**
     * @brief Sets the transaction date.
     *
     * @param date The new date string in YYYY-MM-DD format.
     */
    void setDate(const std::string &date);

    /**
     * @brief Sets the transaction amount.
     *
     * @param amount The new monetary amount of the record.
     */
    void setAmount(double amount);

    /**
     * @brief Sets whether the record is an expense.
     *
     * If the current category is one of the default categories
     * ("Other Expense" or "Other Income"), the category is updated
     * automatically to stay consistent with the new transaction type.
     *
     * @param isExpense True if the record is an expense; false if it is income.
     */
    void setIsExpense(bool isExpense);

    /**
     * @brief Sets the category of the transaction.
     *
     * If the category is empty or equals "other"/"Other", a default category
     * is assigned based on whether the record represents an expense or income.
     *
     * @param category The new category string.
     */
    void setCategory(const std::string &category);

    /**
     * @brief Validates the date and amount of a record.
     *
     * This function checks whether the given date and amount values are valid
     * for constructing a Record object. If validation fails, an explanatory
     * message is stored in @p errorMsg.
     *
     * @param date The date string to validate.
     * @param amount The amount value to validate.
     * @param errorMsg Output parameter for the validation error message.
     * @return True if the data is valid; false otherwise.
     */
    static bool validateData(const std::string &date,
                             double amount,
                             std::string &errorMsg);
};

#endif