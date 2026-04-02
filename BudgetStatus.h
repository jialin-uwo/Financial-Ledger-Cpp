/**
 * @file BudgetStatus.h
 * @brief Declaration of the BudgetStatus structure.
 *
 * This file defines the @c BudgetStatus structure, which stores the
 * result of a budget analysis for a category. It is intended to group
 * together all computed budget-related summary values in a single data
 * object so they can be passed, stored, or displayed consistently
 * throughout the program.
 *
 * The structure includes identifying information for the category,
 * spending and limit values, remaining budget calculations, time-based
 * budgeting information, and an overall budget health classification.
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
 * This structure serves as a lightweight container for the results of
 * evaluating a category's budget state. It records the category name,
 * the amount already spent, the budget limit, the remaining amount,
 * the number of days left in the budget period, the recommended daily
 * spending amount for the rest of the period, and the corresponding
 * overall budget health.
 *
 * Numeric fields are initialized to @c -1 or @c -1.0 by default to
 * represent an unset, unavailable, or not-yet-computed state. The
 * @c budgetHealth field is initialized to @c BudgetHealth::Unknown
 * until a valid analysis is performed.
 *
 * @author Xinyan Cai
 */
struct BudgetStatus
{
    /** @brief Name of the category associated with this budget status. */
    std::string categoryName = "";

    /** @brief Actual amount spent in this category so far. */
    double actualSpent = -1.0;

    /** @brief Budget limit assigned to this category. */
    double budgetLimit = -1.0;

    /** @brief Remaining budget amount after subtracting spending from the limit. */
    double remaining = -1.0;

    /** @brief Number of days remaining in the current budget period. */
    int daysRemaining = -1;

    /** @brief Recommended daily available spending for the remaining period. */
    double dailyAvailable = -1.0;

    /** @brief Overall budget health classification for this category. */
    BudgetHealth budgetHealth = BudgetHealth::Unknown;
};

#endif