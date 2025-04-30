#pragma once

#include <cstdint>

class Result
{
public:
    Result();
    ~Result();

    static constexpr uint32_t MaxSolutions = 4e6;

    uint32_t resultId_;
    uint32_t numSolutions_ = 0;
    uint8_t* solutionHitCounts_ = nullptr;
    float* solutionRs_ = nullptr;
    float* solutionPhis_ = nullptr;
};