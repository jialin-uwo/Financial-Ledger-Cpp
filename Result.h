

#pragma once
#include <string>

// Status codes for controller operations
enum class StatusCode
{
    SUCCESS,
    VALIDATION_ERROR,
    IO_ERROR,
    NOT_FOUND,
    DUPLICATE,
    UNKNOWN
};

// Result struct for controller method returns
struct Result
{
    StatusCode code;
    std::string message;
    int reassignedCount;
    Result(StatusCode c = StatusCode::UNKNOWN, const std::string &msg = "", int reassigned = -1)
        : code(c), message(msg), reassignedCount(reassigned) {}
    bool ok() const { return code == StatusCode::SUCCESS; }
};
