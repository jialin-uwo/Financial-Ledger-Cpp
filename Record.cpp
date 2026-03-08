#include "Record.h"

#include <cmath>
#include <cstdio>
#include <regex>

Record::Record(int id,
               const std::string& date,
               double amount,
               bool isExpense,
               const std::string& category)
    : id(id), date(date), amount(amount), isExpense(isExpense), category(category) {
}

int Record::getId() const {
    return id;
}

std::string Record::getDate() const {
    return date;
}

double Record::getAmount() const {
    return amount;
}

bool Record::getIsExpense() const {
    return isExpense;
}

std::string Record::getCategory() const {
    return category;
}

bool Record::validateData(const std::string& date,
                          double amount,
                          std::string& errorMsg) {
    if (date.empty()) {
        errorMsg = "Date is required.";
        return false;
    }

    std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, pattern)) {
        errorMsg = "Date must be in YYYY-MM-DD format.";
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;

    if (std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        errorMsg = "Invalid date format.";
        return false;
    }

    if (month < 1 || month > 12) {
        errorMsg = "Invalid month.";
        return false;
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leapYear) {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1]) {
        errorMsg = "Invalid day.";
        return false;
    }

    if (std::isnan(amount)) {
        errorMsg = "Amount cannot be NaN.";
        return false;
    }

    if (amount == 0.0) {
        errorMsg = "Amount cannot be zero.";
        return false;
    }

    errorMsg = "";
    return true;
}