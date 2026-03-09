#ifndef RECORD_H
#define RECORD_H

#include <string>

class Record {
private:
    int id;
    std::string date;
    double amount;
    bool isExpense;
    std::string category;

public:
    Record(int id,
           const std::string& date,
           double amount,
           bool isExpense = true,
           const std::string& category = "Other");

    int getId() const;
    std::string getDate() const;
    double getAmount() const;
    bool getIsExpense() const;
    std::string getCategory() const;

    static bool validateData(const std::string& date,
                             double amount,
                             std::string& errorMsg);
};

#endif