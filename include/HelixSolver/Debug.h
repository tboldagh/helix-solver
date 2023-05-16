#pragma once

// Bool values for CDEBUG, used to surpress output
static constexpr bool DISPLAY_BASIC = 0;
static constexpr bool DISPLAY_BOX_POSITION = 0;
static constexpr bool DISPLAY_SOLUTION_PAIR = 0;
static constexpr bool DISPLAY_RPHI = 0;

#define INFO(MSG) std::cout << " . " << MSG << std::endl;

#ifdef PRINT_DEBUG
#ifdef USE_SYCL
    "Can not use DEBUG and other macros when building SYCL"
#endif

#define DEBUG(MSG) std::cout << " ... " << MSG << std::endl;
#else
#define DEBUG(MSG)
#endif

#ifdef PRINT_CDEBUG
#ifdef USE_SYCL
    "Can not use CDEBUG and other macros when building SYCL"
#endif
#define CDEBUG(COND, MSG) { if ( COND == true ) { std::cout << " ... " << MSG << std::endl; } }
#else
#define CDEBUG(MSG)
#endif

#ifdef PRINT_VERBOSE
#define VERBOSE(MSG) std::cout << " ..... " << MSG << std::endl;
#else
#define VERBOSE(MSG)
#endif

#ifdef USE_SYCL
#define ASSURE_THAT(COND, MSG)
#else
#define ASSURE_THAT(COND, MSG) { if ( COND == false ) { throw std::runtime_error(MSG); } }
#endif

#define ACCUMULATOR_DUMP_FILE_PATH "logs/accumulator_dump.log"
#define PRINT_PLATFORM
#define PRINT_DEVICE
#define PRINT_EXECUTION_TIME
#define MULTIPLY_EVENTS