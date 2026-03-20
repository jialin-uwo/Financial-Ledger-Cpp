/**
* @file BudgetStatus.h
 * @brief Declaration of the BudgetStatus structure.
 *
 * This file defines the BudgetStatus structure, which stores
 * the result of a budget analysis for a category.
 *
 * @author Xinyan Cai
 */

#ifndef BUDGETSTATUS_H
#define BUDGETSTATUS_H

#include <string>
#include "BudgetHealth.h"

/**
 * @struct BudgetStatus
 * @brief Stores the budget analysis result for a category.
 *
 * This structure contains summary information about a category's
 * current budget condition, including spending, remaining amount,
 * and overall budget health.
 */
struct BudgetStatus
{
    /** @brief Name of the category. */
    std::string categoryName;

    /** @brief Actual amount spent in this category. */
    double actualSpent;

    /** @brief Budget limit assigned to this category. */
    double budgetLimit;

    /** @brief Remaining budget amount. */
    double remaining;

    /** @brief Number of days remaining in the current budget period. */
    int daysRemaining;

    /** @brief Recommended daily available spending for the remaining period. */
    double dailyAvailable;

    /** @brief Overall budget health status. */
    BudgetHealth budgetHealth;
};

#endif