#pragma once

#define INFO(MSG) std::cout << " . " << MSG << std::endl;


#ifdef PRINT_DEBUG
#define DEBUG(MSG) std::cout << " ... " << MSG << std::endl;
#else
#define DEBUG(MSG)
#endif

#ifdef PRINT_VERBOSE
#define VERBOSE(MSG) std::cout << " ..... " << MSG << std::endl;
#else
#define VERBOSE(MSG)
#endif

#define ACCUMULATOR_DUMP_FILE_PATH "logs/accumulator_dump.log"
#define PRINT_PLATFORM
#define PRINT_DEVICE
#define PRINT_EXECUTION_TIME
#define MULTIPLY_EVENTS