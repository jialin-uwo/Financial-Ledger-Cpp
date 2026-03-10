/**
 * @file Record.h
 * @brief Declaration of the Record class.
 *
 * This file defines the Record class, which represents a financial transaction
 * record in the system. It includes core attributes such as id, date, amount,
 * transaction type, and category, as well as accessor methods and validation logic.
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
 * and its category. It also provides getter methods for accessing these
 * attributes and a static validation function for checking date and amount input.
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
     * Initializes a financial transaction record with the provided values.
     *
     * @param id The unique identifier of the record.
     * @param date The transaction date.
     * @param amount The transaction amount.
     * @param isExpense True if the record is an expense; false if it is income.
     * @param category The category of the transaction. Defaults to an empty string.
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