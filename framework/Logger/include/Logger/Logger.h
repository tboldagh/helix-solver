#pragma once

#include "Logger/ILogger.h"

#define LOG(severity, message) \
    Logger::ILogger::getGlobalInstance()->log(Logger::LogMessage(severity, message, std::chrono::system_clock::now(), __FILE__, __LINE__, __func__))

#define LOG_DEBUG(message) LOG(Logger::LogMessage::Severity::Debug, message)
#define LOG_INFO(message) LOG(Logger::LogMessage::Severity::Info, message)
#define LOG_WARNING(message) LOG(Logger::LogMessage::Severity::Warning, message)
#define LOG_ERROR(message) LOG(Logger::LogMessage::Severity::Error, message)
#define LOG_FATAL(message) LOG(Logger::LogMessage::Severity::Fatal, message)