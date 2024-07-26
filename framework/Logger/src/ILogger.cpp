#include "Logger/ILogger.h"

namespace Logger
{
ILogger* ILogger::globalInstance_ = nullptr;

ILogger* ILogger::getGlobalInstance()
{
    return globalInstance_;
}

void ILogger::setGlobalInstance(ILogger* instance)
{
    globalInstance_ = instance;
}
} // namespace Logger