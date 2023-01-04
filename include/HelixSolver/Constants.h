#pragma once

static constexpr uint8_t THRESHOLD = 6;
static constexpr float Q_OVER_P_BEGIN  = -0.2857142857142857;
static constexpr float Q_OVER_P_END = 0.2857142857142857;
static constexpr float PHI_BEGIN = 0.1;
static constexpr float PHI_END = 0.6;

static constexpr uint8_t NUM_OF_LAYERS = 8;
static constexpr uint32_t MAX_STUB_NUM = 100000;
// static constexpr uint32_t MAX_STUB_NUM = 8;
// TODO: rename
// * Must be a power of 2
static constexpr uint32_t ACC_WIDTH = 64;
// TODO: rename
// * Must be a power of 2
static constexpr uint32_t ACC_HEIGHT = 256;
// TODO: rename
static constexpr uint32_t ACC_SIZE = ACC_WIDTH * ACC_HEIGHT;
// TODO: rename
static constexpr float ACC_CELL_WIDTH = (Q_OVER_P_END - Q_OVER_P_BEGIN) / (ACC_WIDTH - 1);
// TODO: rename
static constexpr float ACC_CELL_HEIGHT = (PHI_END - PHI_BEGIN) / (ACC_HEIGHT - 1);

static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL = 2;
// * Must be 2 to power ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL
static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISIONS = 4;

// * Must be log2 of ACC_WIDTH
static constexpr uint8_t Q_OVER_PT_MAX_GRID_DIVISION_LEVEL = 6;
// * Must be log2 of ACC_HEIGHT
static constexpr uint8_t PHI_MAX_GRID_DIVISION_LEVEL = 8;
// * Must be at least max(Q_OVER_PT_MAX_GRID_DIVISION_LEVEL, PHI_MAX_GRID_DIVISION_LEVEL) * 4
static constexpr uint16_t ACCUMULATOR_SECTION_STACK_MAX_HEIGHT = PHI_MAX_GRID_DIVISION_LEVEL * 4;

// TODO: rename
static constexpr float B = 2.0;
