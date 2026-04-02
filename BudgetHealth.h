/**
 * @file BudgetHealth.h
 * @brief Declaration of the BudgetHealth enumeration.
 *
 * This header file defines the @c BudgetHealth scoped enumeration used to
 * represent the current health status of a category budget. The enumeration
 * can be used by other parts of the program to indicate whether a budget
 * is unavailable, within a safe range, approaching its limit, or already
 * exceeded.
 *
 * @author Xinyan Cai
 */

#ifndef BUDGETHEALTH_H
#define BUDGETHEALTH_H

/**
 * @enum BudgetHealth
 * @brief Represents the health condition of a budget.
 *
 * This scoped enumeration provides a simple and type-safe way to describe
 * the status of a budget. It is typically used when comparing current
 * spending against a planned or assigned budget amount.
 *
 * The possible states are:
 * - @c Unknown: No budget status can be determined.
 * - @c Safe: Spending is comfortably within the budget.
 * - @c Warning: Spending is close to the budget limit.
 * - @c Exceeded: Spending has gone beyond the budget.
 *
 * @author Xinyan Cai
 */
enum class BudgetHealth
{
    /**< Budget status is unknown or cannot be determined. */
    Unknown,

    /**< Budget usage is within a safe range. */
    Safe,

    /**< Budget usage is approaching the limit. */
    Warning,

    /**< Budget usage has exceeded the allowed limit. */
    Exceeded
};

#endif