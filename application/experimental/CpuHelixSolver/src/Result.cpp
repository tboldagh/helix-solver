#include "CpuHelixSolver/Result.h"

Result::Result()
{
    solutionHitCounts_ = new uint8_t[MaxSolutions];
    solutionRs_ = new float[MaxSolutions];
    solutionPhis_ = new float[MaxSolutions];
}

Result::~Result()
{
    delete[] solutionHitCounts_;
    delete[] solutionRs_;
    delete[] solutionPhis_;
}

