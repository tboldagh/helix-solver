#pragma once

#include "EventUsm/TransferableData.h"

#include <sycl/sycl.hpp>
#include <memory>


class ResultUsm : public TransferableData
{
public:
    using ResultId = u_int32_t;

    class ResultKernelMemory : public KernelMemory
    {
    public:
        ResultKernelMemory(sycl::queue& queue);
        ~ResultKernelMemory() override = default;

        ResultKernelMemory(const ResultKernelMemory&) = delete;

        u_int32_t* numSolutions_ = nullptr;
        u_int32_t* regionNumSolutions_ = nullptr;
        u_int8_t* solutionHitCounts_ = nullptr;
        float* solutionRs_ = nullptr;
        float* solutionPhis_ = nullptr;

    protected:
        void allocateInternal() override;
        void deallocateInternal() override;
    };

    ResultUsm(ResultId resultId);
    ResultUsm(const ResultUsm&) = delete;
    ~ResultUsm() override;

    void operator=(const ResultUsm&) = delete;

    static constexpr u_int16_t MaxRegions = 256;
    static constexpr u_int16_t MaxSolutionsPerRegion = 1024;
    static constexpr u_int32_t MaxSolutions = MaxRegions * MaxSolutionsPerRegion;

    TransferableData::TransferEvents transferToDevice() override;
    TransferableData::TransferEvents transferToHost() override;

    static void copyHostData(const ResultUsm& source, ResultUsm& destination);

    ResultId resultId_;
    u_int32_t hostNumSolutions_ = 0;
    u_int32_t* hostRegionNumSolutions_;
    u_int8_t* hostSolutionHitCounts_;
    float* hostSolutionRs_;
    float* hostSolutionPhis_;
    ResultKernelMemory* kernelMemory_ = nullptr;

protected:
    void setKernelMemoryInternal(KernelMemory* kernelMemory) override;
};
