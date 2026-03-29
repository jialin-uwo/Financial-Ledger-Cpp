

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
    Result(StatusCode c = StatusCode::UNKNOWN, const std::string &msg = "") : code(c), message(msg) {}
    bool ok() const { return code == StatusCode::SUCCESS; }
};
