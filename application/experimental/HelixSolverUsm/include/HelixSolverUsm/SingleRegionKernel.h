#pragma once

#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"

#include <CL/sycl.hpp>
#include <cmath>


class SingleRegionKernel
{
public:
    SingleRegionKernel(const Splitter* splitter, const EventUsm* event, const ResultUsm* result);

    SYCL_EXTERNAL void operator()(sycl::id<1> regionIdIdx) const;

protected:
    class AccumulatorRegion
    {
    public:
        AccumulatorRegion() = default;
        AccumulatorRegion(float qOverPtMin, float qOverPtMax, float phi0Min, float phi0Max)
            : qOverPtMin(qOverPtMin)
            , qOverPtMax(qOverPtMax)
            , phi0Min(phi0Min)
            , phi0Max(phi0Max) {}

        inline AccumulatorRegion subregionQOverPtMinPhi0Min() const
        {
            return AccumulatorRegion(qOverPtMin, 0.5f * (qOverPtMin + qOverPtMax), phi0Min, 0.5f * (phi0Min + phi0Max));
        }

        inline AccumulatorRegion subregionQOverPtMinPhi0Max() const
        {
            return AccumulatorRegion(qOverPtMin, 0.5f * (qOverPtMin + qOverPtMax), 0.5f * (phi0Min + phi0Max), phi0Max);
        }

        inline AccumulatorRegion subregionQOverPtMaxPhi0Min() const
        {
            return AccumulatorRegion(0.5f * (qOverPtMin + qOverPtMax), qOverPtMax, phi0Min, 0.5f * (phi0Min + phi0Max));
        }

        inline AccumulatorRegion subregionQOverPtMaxPhi0Max() const
        {
            return AccumulatorRegion(0.5f * (qOverPtMin + qOverPtMax), qOverPtMax, 0.5f * (phi0Min + phi0Max), phi0Max);
        }

        float qOverPtMin;
        float qOverPtMax;
        float phi0Min;
        float phi0Max;
    };

    const Splitter* splitter_;
    const u_int32_t* deviceNumPoints_;
    const float* deviceXs_;
    const float* deviceYs_;
    const float* deviceZs_;
    const EventUsm::LayerNumber* deviceLayers_;

    u_int32_t* deviceNumSolutions_;
    float* deviceSomeSolutionParameters_;

    const EventUsm* event_; // TODO: Remove
    const ResultUsm* result_;    // TODO: Remove

    static constexpr u_int16_t MaxPointsInRegion = 10000;   // TODO: Tune
    // Based on thesis p. 22 Phi_0 is in range dependent on region
    // Phi_min and Phi_max are region dependent
    static constexpr float SpaceMaxPhiPhi0AbsDiff = 0.42f;  // TODO: Tune
    static constexpr float SpaceMinQOverPt = -2.137f;   // TODO: I have no idea what this value should be, tune
    static constexpr float SpaceMaxQOverPt = 2.137f;    // TODO: I have no idea what this value should be, tune
    static constexpr u_int8_t Phi0MaxDivisionLevel = 4;   // TODO: Tune
    static constexpr u_int8_t QOverPtMaxDivisionLevel = 5;   // TODO: Tune
    static constexpr u_int8_t MaxDivisionLevel = std::max(Phi0MaxDivisionLevel, QOverPtMaxDivisionLevel);
    static constexpr u_int8_t MaxAccumulatorRegionStackSize = MaxDivisionLevel * 4;
    static constexpr u_int8_t MaxPointListsNum = MaxDivisionLevel + 2;
    static constexpr u_int32_t MaxPointListsPointsNum = MaxPointsInRegion * MaxPointListsNum;    // TODO: This is max possible number of points in all lists combined. Can be tuned

    // Auxiliary functions, move somewhere else later
    static float wrapMinusPiToPi(float angle)
    {
        return std::fmod(angle + M_PI, 2.0f * M_PI) + (angle < -M_PI ? 1.0f : -1.0f) * M_PI;
    }

    static float angleWrap2Pi(float angle)
    {
        return std::fmod(angle, 2 * M_PI) + (angle >= 0 ? 0 : 2 * M_PI);
    }

    static float atan2Wrap2Pi(float y, float x)
    {
        return angleWrap2Pi(std::atan2(y, x));
    }

    SYCL_EXTERNAL void filterPointsInRegion(u_int16_t regionId, u_int32_t& numPoints, u_int32_t* indexes, float* xs, float* ys, float* zs, EventUsm::LayerNumber* layers) const;
    void convertToPolarCoordinates(float* phis, float* rs, const float* xs, const float* ys, u_int32_t numPoints) const;
    void rotateRegionAndPoints(float& regionPhi0Min, float& regionPhi0Max, float* phis, u_int32_t numPoints) const;
    void processNextAccumulatorRegion(AccumulatorRegion* accumulatorRegions, u_int8_t& accumulatorRegionStackSize, u_int32_t* pointLists, u_int32_t* pointListsSizes) const;
    void rotateSolutions() const;
};