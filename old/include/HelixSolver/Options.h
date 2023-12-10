#pragma once


namespace HelixSolver {
    struct Options{
        float ACC_X_PRECISION = 0.01;
        float ACC_PT_PRECISION = 0.1; // this is simplified approach, in reality it could be modified depending on q/pt

        uint8_t N_PHI_WEDGE = 16; // wedge to process
        uint8_t N_ETA_WEDGE = 53;

        uint8_t THRESHOLD_PT_THRESHOLD = 2;
        uint8_t LOW_PT_THRESHOLD = 6;
        uint8_t HIGH_PT_THRESHOLD = 7;

        float N_SIGMA_GAUSS = 2.;
        float STDEV_CORRECTION = 0.6;
        uint8_t MIN_LINES_GAUSS = 4;

        float THRESHOLD_X_PRECISION = 0.002;
        float THRESHOLD_PT_PRECISION = 0.02;
        uint8_t THRESHOLD_COUNTER = 10;
    };
}

#ifdef USE_SYCL
    #include <CL/sycl.hpp>
    using OptionsBuffer=sycl::buffer<HelixSolver::Options, 1>;
#else
    #include <vector>
    using OptionsBuffer=std::vector<HelixSolver::Options>;
#endif
