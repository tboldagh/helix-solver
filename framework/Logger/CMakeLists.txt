helix_solver_add_library(Logger
TYPE
    STATIC

INCLUDE
    include

SRC
    src/ILogger.cpp
    src/LogMessage.cpp
    src/OstreamLogger.cpp
    src/JsonLogger.cpp
)

helix_solver_add_library(ILoggerMock
TYPE
    STATIC

INCLUDE
    test-include

SRC
    test-src/ILoggerMock.cpp

PRIVATE
    Logger
)

helix_solver_add_library(LogMessageSuite
UNIT_TEST

LOCATION
    framework/Logger

SRC
    test/LogMessageSuite.cpp

PRIVATE
    Logger
)

helix_solver_add_library(OstreamLoggerSuite
UNIT_TEST

LOCATION
    framework/Logger

SRC
    test/OstreamLoggerSuite.cpp

PRIVATE
    Logger
)

helix_solver_add_library(JsonLoggerSuite
UNIT_TEST

LOCATION
    framework/Logger

SRC
    test/JsonLoggerSuite.cpp

PRIVATE
    Logger
)

helix_solver_add_library(LoggerSuite
UNIT_TEST

LOCATION
    framework/Logger

SRC
    test/LoggerSuite.cpp

PRIVATE
    Logger
    ILoggerMock
)
