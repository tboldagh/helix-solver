#pragma once
#include <stdint.h>
#include <cmath>
static constexpr uint8_t THRESHOLD = 6;

static constexpr uint8_t THRESHOLD_DIVISION_LEVEL_COUNT_HITS_ORDER_CHECK = 10;

static constexpr bool USE_GAUSS_FILTERING  = 1;
static constexpr uint8_t MIN_COUNT_CHANGES = 3;

static constexpr uint8_t N_SIGMA_PHI = 3;
static constexpr uint8_t N_SIGMA_GAUSS = 2;
static constexpr uint8_t MAX_COUNT_PER_SECTION = 16;
static constexpr float STDEV_CORRECTION = 0.6;

// Suggested values: N_PHI_WEDGE = 8, N_ETA_WEDGE = 39, z.centre = 0, z.width = 200
static constexpr float wedge_z_center = 0;
static constexpr float wedge_z_width = 200;
static constexpr float excess_wedge_phi_width = 0.15;
static constexpr float excess_wedge_eta_width = 0.15;

static_assert(THRESHOLD < MAX_COUNT_PER_SECTION, "Require threshold that is higher than max per cell");

static constexpr float Q_OVER_PT_BEGIN  = -1.05;
static constexpr float Q_OVER_PT_END = 1.05;
static constexpr float PHI_BEGIN = -3.1416;
static constexpr float PHI_END = 3.1416;
static constexpr float ETA_WEDGE_MIN = -4.;
static constexpr float ETA_WEDGE_MAX =  4.;
static constexpr float MAX_PT = 10.5;

// Accumulator size parameters
static constexpr float ACC_X_SIZE = PHI_END - PHI_BEGIN;
static constexpr float ACC_Y_SIZE = Q_OVER_PT_END - Q_OVER_PT_BEGIN;

static constexpr uint32_t MAX_SPACEPOINTS = 100000;
static constexpr uint32_t MAX_SOLUTIONS   = 10000; // an arbitrary size, need to get it experimentally (ideally configurable)

static int32_t TO_DISPLAY_PRECISION_PAIR_ONCE = 1;

// Initial division parameters
//static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL = 20; // this gives parallelism
static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISIONS = 1; // this is easier to debug

static constexpr uint32_t MAX_SECTIONS_BUFFER_SIZE = 100; // need to be checked experimentally

// Additional parameters
static constexpr float MAGNETIC_INDUCTION = 2.0;
static constexpr float INVERSE_A = 1.0/3.0e-4;