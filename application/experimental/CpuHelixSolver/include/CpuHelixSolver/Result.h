#pragma once

#include <cstdint>

class Result
{
public:
    Result(uint32_t resultId = 0);
    Result(const Result& other) = delete;
    Result(Result&& other);
    ~Result();

    Result& operator=(const Result& other) = delete;
    Result& operator=(Result&& other);

    static constexpr uint32_t MaxSolutions = 4e6;

    uint32_t resultId_;
    uint32_t numSolutions_ = 0;
    uint8_t* solutionHitCounts_ = nullptr;
    float* solutionRs_ = nullptr;
    float* solutionPhis_ = nullptr;
};