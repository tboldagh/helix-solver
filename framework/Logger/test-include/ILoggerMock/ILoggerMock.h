#pragma once

#include "Logger/ILogger.h"

#include <gmock/gmock.h>

namespace Logger
{
class ILoggerMock : public ILogger
{
public:
    ILoggerMock() = default;
    ~ILoggerMock() override = default;

    MOCK_METHOD(void, log, (LogMessage&& message), (override));
};
} // namespace Logger