#pragma once

namespace HelixSolver
{
    struct SolutionCircle
    {
        static constexpr float INVALID_PT = 0;
        static constexpr float INVALID_PHI = -1000;

        float pt = INVALID_PT;
        float phi = INVALID_PHI; // invalid
    };
} // namespace HelixSolver
