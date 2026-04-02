/**
 * @file Result.h
 * @brief Declaration of the StatusCode enumeration and the Result structure.
 *
 * This file defines the common status and result types used by controller
 * operations in the ledger system. It provides:
 * - the @c StatusCode enumeration, which classifies the outcome of an
 *   operation, and
 * - the @c Result structure, which stores a status code, a descriptive
 *   message, and optional additional result information.
 *
 * These types are intended to provide a lightweight and consistent way
 * for controller methods to report success, validation failures, I/O
 * problems, missing data, duplicates, and unknown errors.
 *
 * @author Jialin Li
 */

#pragma once
#include <string>

/**
 * @enum StatusCode
 * @brief Represents the status of a controller operation.
 *
 * This scoped enumeration defines the possible outcome categories returned
 * by controller methods. It allows higher-level logic to distinguish among
 * successful execution, validation issues, file I/O problems, missing
 * records, duplicate data, and unspecified failures.
 *
 * @author Jialin Li
 */
enum class StatusCode
{
    /** @brief The operation completed successfully. */
    SUCCESS,

    /** @brief The operation failed because input validation did not pass. */
    VALIDATION_ERROR,

    /** @brief The operation failed because of an input/output error. */
    IO_ERROR,

    /** @brief The requested item or resource could not be found. */
    NOT_FOUND,

    /** @brief The operation failed because duplicate data was detected. */
    DUPLICATE,

    /** @brief The operation ended with an unspecified or unknown error. */
    UNKNOWN
};

/**
 * @struct Result
 * @brief Stores the outcome of a controller method.
 *
 * The Result structure is a lightweight return type used by controller
 * operations to communicate both status and explanatory information.
 * It contains:
 * - a @c StatusCode value describing the type of result,
 * - a human-readable message giving further detail, and
 * - an optional reassigned count used when an operation needs to report
 *   how many items were reassigned or updated.
 *
 * This structure is intended to standardize controller responses and
 * simplify error handling at higher layers of the application.
 *
 * @author Jialin Li
 */
struct Result
{
    /**
     * @brief The status code describing the operation result.
     */
    StatusCode code;

    /**
     * @brief A human-readable message describing the result.
     */
    std::string message;

    /**
     * @brief Optional count of reassigned items related to the operation.
     *
     * A value of @c -1 indicates that no reassignment count is applicable
     * or available for the result.
     */
    int reassignedCount;

    /**
     * @brief Constructs a Result object.
     *
     * This constructor initializes a Result instance with the supplied
     * status code, descriptive message, and optional reassignment count.
     *
     * @param c The status code of the result. Defaults to @c StatusCode::UNKNOWN.
     * @param msg The message describing the result. Defaults to an empty string.
     * @param reassigned The reassigned item count. Defaults to @c -1.
     *
     * @return None.
     *
     * @author Jialin Li
     */
    Result(StatusCode c = StatusCode::UNKNOWN, const std::string &msg = "", int reassigned = -1)
        : code(c), message(msg), reassignedCount(reassigned) {}

    /**
     * @brief Checks whether the result indicates success.
     *
     * This helper function returns true only when the stored status code
     * is @c StatusCode::SUCCESS.
     *
     * @return True if the result represents success; otherwise false.
     *
     * @author Jialin Li
     */
    bool ok() const { return code == StatusCode::SUCCESS; }
};