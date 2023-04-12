#pragma once
#include <stdint.h>
#include <cmath>
static constexpr uint8_t THRESHOLD = 6;
static constexpr float Q_OVER_PT_BEGIN  = -1;
static constexpr float Q_OVER_PT_END = 1;
static constexpr float PHI_BEGIN = -3.1416;
static constexpr float PHI_END = 3.1416;
static constexpr uint32_t MAX_Point_NUM = 100000;

// Bool values for CDEBUG, used to surpress output
static constexpr bool DISPLAY_BASIC = 0;
static constexpr bool DISPLAY_BOX_POSITION = 0;
static constexpr bool DISPLAY_SOLUTION_PAIR = 1;
static constexpr bool DISPLAY_RPHI = 1;

// Accumulator size parameters
static constexpr float ACC_X_SIZE = PHI_END - PHI_BEGIN;
static constexpr float ACC_Y_SIZE = Q_OVER_PT_END - Q_OVER_PT_BEGIN;

static constexpr uint16_t MAX_SOLUTIONS = 2000; // an arbitrary size, need to get it experimentally (ideally configurable)

// Precision of solution estimate -> criterion used for ending the loop
static constexpr float ACC_X_PRECISION = 0.003;
static constexpr float ACC_Y_PRECISION = 0.01; // this is simplified approach, in reality it could be modified it depending on q/pt

// Initial division parameters
//static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL = 20; // this gives parallelism
static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISIONS = 1; // this is easier to debug

static constexpr uint32_t MAX_SECTIONS_BUFFER_SIZE = 40; // need to be checked experimentally

// Additional parameters
static constexpr float MAGNETIC_INDUCTION = 2.0;
static constexpr float INVERSE_A = 1.0/3.0e-4;