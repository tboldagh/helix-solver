#pragma once

static constexpr uint8_t NUM_OF_LAYERS = 8;
static constexpr uint32_t MAX_STUB_NUM = 100000;
static constexpr uint32_t ACC_WIDTH = 64;
static constexpr uint32_t ACC_HEIGHT = 256;
static constexpr uint32_t ACC_SIZE = ACC_WIDTH * ACC_HEIGHT;

static constexpr uint8_t THRESHOLD = 6;
static constexpr float Q_OVER_P_BEGIN  = -0.2857142857142857;
static constexpr float Q_OVER_P_END = 0.2857142857142857;
static constexpr float PHI_BEGIN = 0.1;
static constexpr float PHI_END = 0.6;

static constexpr float NS = 1000000000.0; // nanoseconds in second

static constexpr float B = 2.0;
