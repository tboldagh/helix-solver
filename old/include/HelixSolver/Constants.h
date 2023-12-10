#pragma once
#include <stdint.h>
#include <cmath>

static constexpr uint8_t THRESHOLD_DIVISION_LEVEL_COUNT_HITS_ORDER_CHECK = 10;

static constexpr bool USE_GAUSS_FILTERING  = 1;
static constexpr uint8_t MIN_COUNT_CHANGES = 3;

static constexpr uint8_t N_SIGMA_PHI = 2;
static constexpr uint8_t MAX_COUNT_PER_SECTION = 16;

// Suggested values: N_PHI_WEDGE = 8, N_ETA_WEDGE = 39, z.centre = 0, z.width = 200
static constexpr float wedge_z_center = 0;
static constexpr float wedge_z_width = 200;
static constexpr float excess_wedge_phi_width = 0.12;
static constexpr float excess_wedge_eta_width = 0.12;

static constexpr float MIN_R2 = 0.85;
static constexpr float MAX_LINEAR_DISCREPANCY = 0.4;  // expressed as percents
static constexpr uint32_t LINEAR_THRESHOLD = 7;

static constexpr float Q_OVER_PT_BEGIN  = -1.05;
static constexpr float Q_OVER_PT_END = 1.05;
static constexpr float PHI_BEGIN = -3.1416;
static constexpr float PHI_END = 3.1416;
// single - eta range from -4 to 4, pileup - from -3 to 3
static constexpr float ETA_WEDGE_MIN = -1.5;
static constexpr float ETA_WEDGE_MAX =  1.5;
static constexpr float MAX_PT = 10.5;

// Accumulator size parameters
static constexpr float ACC_X_SIZE = PHI_END - PHI_BEGIN;
static constexpr float ACC_Y_SIZE = Q_OVER_PT_END - Q_OVER_PT_BEGIN;

// MAX_SOLUTIONS - maximal number of solutions in a single event, this
// parameter strongly influences program execution time
// pileup: spacepoints = 80,000, solutions = 600,000 (when using division)
// single muon: spacepoints = 8,000, solutions = 6,000 (when using division) - to be determined
// single pion: spacepoints = 20, solutions = 300(0) (when using division), 100 witoud divisions
static constexpr uint32_t MAX_SPACEPOINTS = 50000;
static constexpr uint32_t MAX_SOLUTIONS   = 600000; // an arbitrary size, need to get it experimentally (ideally configurable)

// Initial division parameters
//static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL = 20; // this gives parallelism
static constexpr uint8_t ADAPTIVE_KERNEL_INITIAL_DIVISIONS = 1; // this is easier to debug

static constexpr uint32_t MAX_SECTIONS_BUFFER_SIZE = 100; // need to be checked experimentally

// Additional parameters
static constexpr float MAGNETIC_INDUCTION = 2.0;
static constexpr float INVERSE_A = 1.0/3.0e-4;