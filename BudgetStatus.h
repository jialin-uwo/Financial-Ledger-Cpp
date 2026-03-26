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
 *
 * Numeric fields are initialized to -1 by default to represent
 * an unset or uncomputed state.
 */
struct BudgetStatus
{
    /** @brief Name of the category. */
    std::string categoryName = "";

    /** @brief Actual amount spent in this category. */
    double actualSpent = -1.0;

    /** @brief Budget limit assigned to this category. */
    double budgetLimit = -1.0;

    /** @brief Remaining budget amount. */
    double remaining = -1.0;

    /** @brief Number of days remaining in the current budget period. */
    int daysRemaining = -1;

    /** @brief Recommended daily available spending for the remaining period. */
    double dailyAvailable = -1.0;

    /** @brief Overall budget health status. */
    BudgetHealth budgetHealth = BudgetHealth::Unknown;
};

#endif