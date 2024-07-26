#pragma once

#include "Logger/ILogger.h"
#include "Logger/LogMessage.h"

#include <ostream>

namespace Logger
{
class OstreamLogger : public ILogger
{
public:
    OstreamLogger(std::ostream& stream);
    ~OstreamLogger() override = default;

    void log(LogMessage&& message) override;

private:
    std::ostream& ostream_;
};
} // namespace Logger