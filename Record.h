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
           bool isExpense,
           const std::string& category = "Other");

    int getId() const;
    std::string getDate() const;
    double getAmount() const;
    bool getIsExpense() const;
    std::string getCategory() const;

    static bool validateRequiredFields(const std::string& date,
                                       const std::string& category,
                                       std::string& errorMsg);

    static bool validateDateFormat(const std::string& date,
                                   std::string& errorMsg);

    static bool validateRealDate(const std::string& date,
                                 std::string& errorMsg);

    static bool validateAmount(double amount,
                               std::string& errorMsg);
};

#endif