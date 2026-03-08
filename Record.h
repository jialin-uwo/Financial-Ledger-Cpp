#ifndef RECORD_H
#define RECORD_H

#include <string>
using namespace std;

/**
 * @struct Record
 * @brief Represents one financial record in the ledger system.
 */
struct Record {
    int id;             ///< Unique record ID
    string date;        ///< Record date
    double amount;      ///< Transaction amount
    bool isExpense;     ///< True if expense, false if income
    string category;    ///< Category name
    string note;        ///< Additional note or description
};

#endif