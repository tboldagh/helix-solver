#pragma once


namespace HelixSolver {
    struct Options{
        float ACC_X_PRECISION = 0.01;
        float ACC_PT_PRECISION = 0.1; // this is simplified approach, in reality it could be modified depending on q/pt

        uint8_t N_PHI_WEDGE = 16; // wedge to process
        uint8_t N_ETA_WEDGE = 53;
    };
}

#ifdef USE_SYCL
    #include <CL/sycl.hpp>
    using OptionsBuffer=sycl::buffer<HelixSolver::Options, 1>;
#else
    #include <vector>
    using OptionsBuffer=std::vector<HelixSolver::Options>;
#endif
