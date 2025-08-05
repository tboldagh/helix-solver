#include "CpuHelixSolver/Result.h"

#include <algorithm>

Result::Result(uint32_t resultId)
: resultId_(resultId)
{
    solutionHitCounts_ = new uint8_t[MaxSolutions];
    solutionRs_ = new float[MaxSolutions];
    solutionPhis_ = new float[MaxSolutions];
    xAngleMins_ = new float[MaxSolutions];
    xAngleMaxs_ = new float[MaxSolutions];
}

Result::Result(Result&& other)
: resultId_(other.resultId_)
, solutionHitCounts_(std::exchange(other.solutionHitCounts_, nullptr))
, solutionRs_(std::exchange(other.solutionRs_, nullptr))
, solutionPhis_(std::exchange(other.solutionPhis_, nullptr))
, xAngleMins_(std::exchange(other.xAngleMins_, nullptr))
, xAngleMaxs_(std::exchange(other.xAngleMaxs_, nullptr)) {}

Result::~Result()
{
    if (solutionHitCounts_ == nullptr)
    {
        return;
    }

    delete[] solutionHitCounts_;
    delete[] solutionRs_;
    delete[] solutionPhis_;
    delete[] xAngleMins_;
    delete[] xAngleMaxs_;
}

Result& Result::operator=(Result&& other)
{
    if (this == &other)
    {
        return *this;
    }   

    resultId_ = other.resultId_;
    solutionHitCounts_ = std::exchange(other.solutionHitCounts_, nullptr);
    solutionRs_ = std::exchange(other.solutionRs_, nullptr);
    solutionPhis_ = std::exchange(other.solutionPhis_, nullptr);
    xAngleMins_ = std::exchange(other.xAngleMins_, nullptr);
    xAngleMaxs_ = std::exchange(other.xAngleMaxs_, nullptr);

    return *this;
}