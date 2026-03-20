/**
* @file BudgetHealth.h
 * @brief Declaration of the BudgetHealth enumeration.
 *
 * This file defines the BudgetHealth enum, which represents the
 * health status of a category budget.
 *
 * @author Xinyan Cai
 */

#ifndef BUDGETHEALTH_H
#define BUDGETHEALTH_H

/**
 * @enum BudgetHealth
 * @brief Represents the health condition of a budget.
 */
enum class BudgetHealth
{
    Safe,
    Warning,
    Exceeded
};

#endif