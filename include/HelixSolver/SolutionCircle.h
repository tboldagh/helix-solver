#pragma once

namespace HelixSolver
{
    struct SolutionCircle
    {
        static constexpr float INVALID_PT = 0;
        static constexpr float INVALID_PHI = -1000;
        static constexpr float INVALID_ETA = -1000;
        static constexpr float INVALID_Z = -1000;
        static constexpr float INVALID_D0 = -1000;
        static constexpr float INVALID_NHITS = -1000;
        static constexpr float INVALID_Q = -1000;

        float pt = INVALID_PT;
        float phi = INVALID_PHI;
        float eta = INVALID_ETA;
        float z = INVALID_Z;
        float d0 = INVALID_D0;
        int nhits = INVALID_NHITS;
        float q = INVALID_Q;        
        bool invalid() const {  return pt == INVALID_PT or phi == INVALID_PHI; }
    };
} // namespace HelixSolver
