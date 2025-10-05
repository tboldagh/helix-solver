#pragma once

#include <cstdint>
#include <chrono>

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
    float* solutionQs_ = nullptr;
    float* solutionRs_ = nullptr;
    float* solutionPhis_ = nullptr;
    float* xAngleMins_ = nullptr;
    float* xAngleMaxs_ = nullptr;

    std::chrono::steady_clock::duration totalExecutionTime_;
    std::chrono::steady_clock::duration splitterExecutionTime_;
    std::chrono::steady_clock::duration solverExecutionTime_;
};