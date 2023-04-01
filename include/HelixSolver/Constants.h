#pragma once
#include <stdint.h>
#include <cmath>
static constexpr uint8_t THRESHOLD = 6;
static constexpr float Q_OVER_PT_BEGIN  = -0.2857142857142857;
static constexpr float Q_OVER_PT_END = 0.2857142857142857;
static constexpr float PHI_BEGIN = 0.1;
static constexpr float PHI_END = 0.6;
static constexpr uint32_t MAX_STUB_NUM = 100000;

// Accumulator size parameters
static constexpr float ACC_WIDTH = Q_OVER_PT_END - Q_OVER_PT_BEGIN;
static constexpr float ACC_HEIGHT = PHI_END - PHI_BEGIN;

static constexpr uint16_t MAX_SOLUTIONS=1000; // an arbitrary size, need to get it experimentally (ideally configurable)
// Precision of soliution estimate -> criterion used for ending the loop
static constexpr float PRECISION_WIDTH = 0.001;
static constexpr float PRECISION_HEIGHT = 0.001;
static constexpr double ACC_WIDTH_PRECISION = PRECISION_WIDTH * ACC_WIDTH;
static constexpr double ACC_HEIGHT_PRECISION = PRECISION_HEIGHT * ACC_HEIGHT;

// Division levels
// static constexpr uint8_t Q_OVER_PT_MAX_GRID_DIVISION_LEVEL = ceil(std::log2(ACC_WIDTH / ACC_WIDTH_PRECISION));
// static constexpr uint8_t PHI_MAX_GRID_DIVISION_LEVEL = ceil(std::log2(ACC_HEIGHT / ACC_HEIGHT_PRECISION));
// static constexpr uint8_t MAX_DIVISION_LEVEL = Q_OVER_PT_MAX_GRID_DIVISION_LEVEL > PHI_MAX_GRID_DIVISION_LEVEL ? Q_OVER_PT_MAX_GRID_DIVISION_LEVEL : PHI_MAX_GRID_DIVISION_LEVEL;

// Max number of cells for width and height
// static constexpr uint32_t MAX_WIDTH_CELL_N = pow(2, Q_OVER_PT_MAX_GRID_DIVISION_LEVEL);
// static constexpr uint32_t MAX_HEIGHT_CELL_N = pow(2, PHI_MAX_GRID_DIVISION_LEVEL);
// static constexpr uint32_t ACC_SIZE = MAX_WIDTH_CELL_N * MAX_HEIGHT_CELL_N;

// Initial division parameters
//static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL = 20;
static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISIONS = 20;

// Additional parameters
static constexpr float MagneticInduction = 2.0;
// constexpr uint8_t MAX_STUB_LISTS_NUM = MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL + 2;
// constexpr uint32_t MAX_STUB_LIST_ELEMENTS_NUM = MAX_STUB_NUM * MAX_STUB_LISTS_NUM;
