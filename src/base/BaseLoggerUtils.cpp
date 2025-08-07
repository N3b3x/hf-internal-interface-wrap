/**
 * @file BaseLoggerUtils.cpp
 * @brief Utility functions for BaseLogger string conversions
 *
 * This file contains utility functions for converting logger enums to strings
 * for debugging and logging purposes.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "base/BaseLogger.h"

const char* HfLoggerErrToString(hf_logger_err_t error) noexcept {
  switch (error) {
    case hf_logger_err_t::LOGGER_SUCCESS: return "Success";
    case hf_logger_err_t::LOGGER_ERR_FAILURE: return "General failure";
    case hf_logger_err_t::LOGGER_ERR_NOT_INITIALIZED: return "Not initialized";
    case hf_logger_err_t::LOGGER_ERR_ALREADY_INITIALIZED: return "Already initialized";
    case hf_logger_err_t::LOGGER_ERR_INVALID_PARAMETER: return "Invalid parameter";
    case hf_logger_err_t::LOGGER_ERR_NULL_POINTER: return "Null pointer";
    case hf_logger_err_t::LOGGER_ERR_OUT_OF_MEMORY: return "Out of memory";
    case hf_logger_err_t::LOGGER_ERR_INVALID_CONFIGURATION: return "Invalid configuration";
    case hf_logger_err_t::LOGGER_ERR_UNSUPPORTED_OPERATION: return "Unsupported operation";
    case hf_logger_err_t::LOGGER_ERR_RESOURCE_BUSY: return "Resource busy";
    case hf_logger_err_t::LOGGER_ERR_RESOURCE_UNAVAILABLE: return "Resource unavailable";
    case hf_logger_err_t::LOGGER_ERR_WRITE_FAILURE: return "Write failure";
    case hf_logger_err_t::LOGGER_ERR_OUTPUT_BUFFER_FULL: return "Output buffer full";
    case hf_logger_err_t::LOGGER_ERR_FORMAT_ERROR: return "Format error";
    case hf_logger_err_t::LOGGER_ERR_ENCODING_ERROR: return "Encoding error";
    case hf_logger_err_t::LOGGER_ERR_SYSTEM_ERROR: return "System error";
    case hf_logger_err_t::LOGGER_ERR_PERMISSION_DENIED: return "Permission denied";
    case hf_logger_err_t::LOGGER_ERR_OPERATION_ABORTED: return "Operation aborted";
    case hf_logger_err_t::LOGGER_ERR_NOT_SUPPORTED: return "Operation not supported";
    case hf_logger_err_t::LOGGER_ERR_DRIVER_ERROR: return "Driver error";
    case hf_logger_err_t::LOGGER_ERR_INVALID_STATE: return "Invalid state";
    case hf_logger_err_t::LOGGER_ERR_INVALID_ARG: return "Invalid argument";
    case hf_logger_err_t::LOGGER_ERR_TIMEOUT: return "Timeout";
    case hf_logger_err_t::LOGGER_ERR_BUFFER_OVERFLOW: return "Buffer overflow";
    default: return "Unknown error";
  }
}

const char* HfLogLevelToString(hf_log_level_t level) noexcept {
  switch (level) {
    case hf_log_level_t::LOG_LEVEL_NONE: return "NONE";
    case hf_log_level_t::LOG_LEVEL_ERROR: return "ERROR";
    case hf_log_level_t::LOG_LEVEL_WARN: return "WARN";
    case hf_log_level_t::LOG_LEVEL_INFO: return "INFO";
    case hf_log_level_t::LOG_LEVEL_DEBUG: return "DEBUG";
    case hf_log_level_t::LOG_LEVEL_VERBOSE: return "VERBOSE";
    default: return "UNKNOWN";
  }
}

const char* HfLogLevelToShortString(hf_log_level_t level) noexcept {
  switch (level) {
    case hf_log_level_t::LOG_LEVEL_NONE: return "N";
    case hf_log_level_t::LOG_LEVEL_ERROR: return "E";
    case hf_log_level_t::LOG_LEVEL_WARN: return "W";
    case hf_log_level_t::LOG_LEVEL_INFO: return "I";
    case hf_log_level_t::LOG_LEVEL_DEBUG: return "D";
    case hf_log_level_t::LOG_LEVEL_VERBOSE: return "V";
    default: return "?";
  }
}