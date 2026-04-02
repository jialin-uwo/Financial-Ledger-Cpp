/**
 * @file Record.h
 * @brief Declaration of the Record class.
 *
 * This file defines the Record class, which represents a single financial
 * transaction record in the system. A record stores the core attributes
 * associated with one transaction, including its unique identifier, date,
 * monetary amount, transaction type, and category name.
 *
 * The class provides accessor and mutator methods for reading and updating
 * these attributes, as well as a static validation function used to check
 * whether date and amount inputs satisfy the system's data rules.
 *
 * Default-category behavior is also supported through the implementation:
 * when a category is empty or generic, the record can automatically assign
 * a default category based on whether the transaction is an expense or income.
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
 * The Record class models one transaction entry in the financial system.
 * Each record contains an identifier, a transaction date, a monetary amount,
 * a boolean flag indicating whether the transaction is an expense, and a
 * category name describing the type of transaction.
 *
 * This class supports controlled access and modification of record data
 * through getter and setter methods. It also provides a static validation
 * method for checking whether the date and amount values are valid before
 * a record is constructed or otherwise used.
 *
 * The class is designed to be a lightweight data model for financial records
 * while still enforcing core input constraints through validation logic.
 *
 * @author Xinyan Cai
 */
class Record
{
private:
    /**
     * @brief Unique identifier for the record.
     *
     * This value is used to distinguish one transaction record from another.
     */
    int id;

    /**
     * @brief Transaction date in YYYY-MM-DD format.
     *
     * The date is stored as a string and is expected to follow the
     * standard YYYY-MM-DD format.
     */
    std::string date;

    /**
     * @brief Monetary amount of the transaction.
     *
     * This field stores the financial value associated with the record.
     */
    double amount;

    /**
     * @brief Indicates whether the transaction is an expense.
     *
     * A value of @c true means the transaction is an expense.
     * A value of @c false means the transaction is income.
     */
    bool isExpense;

    /**
     * @brief Category name associated with the transaction.
     *
     * This field stores the category label assigned to the record.
     */
    std::string category;

public:
    /**
     * @brief Constructs a Record object.
     *
     * This constructor initializes a Record instance using the provided
     * identifier, date, amount, transaction type, and category.
     *
     * All input values are expected to satisfy the class rules. In particular,
     * the date and amount are subject to validation, and invalid input may
     * result in an exception being thrown by the implementation.
     *
     * @param id The unique identifier of the record.
     * @param date The transaction date in YYYY-MM-DD format.
     * @param amount The monetary amount of the transaction.
     * @param isExpense True if the record is an expense; false if it is income.
     * Defaults to true.
     * @param category The category name of the transaction. Defaults to an
     * empty string.
     *
     * @return None.
     *
     * @throws std::invalid_argument Thrown if the provided data fails validation.
     *
     * @author Xinyan Cai
     */
    Record(int id,
           const std::string &date,
           double amount,
           bool isExpense = true,
           const std::string &category = "");

    /**
     * @brief Gets the record ID.
     *
     * This accessor returns the unique identifier of the record.
     *
     * @return The unique identifier of the record.
     *
     * @author Xinyan Cai
     */
    int getId() const;

    /**
     * @brief Gets the transaction date.
     *
     * This accessor returns the stored date string for the transaction.
     *
     * @return The date string of the record.
     *
     * @author Xinyan Cai
     */
    std::string getDate() const;

    /**
     * @brief Gets the transaction amount.
     *
     * This accessor returns the monetary amount stored in the record.
     *
     * @return The monetary amount of the record.
     *
     * @author Xinyan Cai
     */
    double getAmount() const;

    /**
     * @brief Gets whether the record is an expense.
     *
     * This accessor indicates whether the transaction is categorized as
     * an expense or as income.
     *
     * @return True if the record is an expense; false if it is income.
     *
     * @author Xinyan Cai
     */
    bool getIsExpense() const;

    /**
     * @brief Gets the category of the transaction.
     *
     * This accessor returns the category label assigned to the record.
     *
     * @return The category string of the record.
     *
     * @author Xinyan Cai
     */
    std::string getCategory() const;

    /**
     * @brief Sets the record ID.
     *
     * This mutator updates the identifier associated with the record.
     *
     * @param id The new unique identifier of the record.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setId(int id);

    /**
     * @brief Sets the transaction date.
     *
     * This mutator updates the transaction date stored in the record.
     *
     * @param date The new date string in YYYY-MM-DD format.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setDate(const std::string &date);

    /**
     * @brief Sets the transaction amount.
     *
     * This mutator updates the monetary amount stored in the record.
     *
     * @param amount The new monetary amount of the record.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setAmount(double amount);

    /**
     * @brief Sets whether the record is an expense.
     *
     * This mutator updates the transaction type of the record.
     * If the current category is one of the default categories
     * ("Other Expense" or "Other Income"), the category is updated
     * automatically to remain consistent with the new transaction type.
     *
     * @param isExpense True if the record is an expense; false if it is income.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setIsExpense(bool isExpense);

    /**
     * @brief Sets the category of the transaction.
     *
     * This mutator updates the category label associated with the record.
     * If the category is empty or equals "other" or "Other", a default
     * category is assigned based on whether the record represents an
     * expense or income.
     *
     * @param category The new category string.
     *
     * @return None.
     *
     * @author Xinyan Cai
     */
    void setCategory(const std::string &category);

    /**
     * @brief Validates the date and amount of a record.
     *
     * This static function checks whether the given date and amount values
     * satisfy the rules required for valid record data.
     *
     * If validation fails, an explanatory message is written into
     * @p errorMsg. This function can be used before constructing a Record
     * object or when checking proposed updates to record data.
     *
     * @param date The date string to validate.
     * @param amount The amount value to validate.
     * @param errorMsg Output parameter for the validation error message.
     *
     * @return True if the data is valid; false otherwise.
     *
     * @author Xinyan Cai
     */
    static bool validateData(const std::string &date,
                             double amount,
                             std::string &errorMsg);
};

#endif