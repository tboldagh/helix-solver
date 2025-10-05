#pragma once

#include <cstdint>
#include <cmath>

class AccumulatorRegion
{
public:
    AccumulatorRegion() = default;

    AccumulatorRegion(float qOverPtMin, float qOverPtMax, float phi0Min, float phi0Max, u_int8_t qOverPtDivisionLevel = 0, u_int8_t phi0DivisionLevel = 0)
        : qOverPtMin_(qOverPtMin)
        , qOverPtMax_(qOverPtMax)
        , phi0Min_(phi0Min)
        , phi0Max_(phi0Max)
        , qOverPtDivisionLevel_(qOverPtDivisionLevel)
        , phi0DivisionLevel_(phi0DivisionLevel) {}

    bool operator==(const AccumulatorRegion& other) const
    {
        // Note: not comparing pointListBegin and pointListEnd
        constexpr float epsilon = 1e-9f;
        return std::abs(qOverPtMin_ - other.qOverPtMin_) < epsilon
            && std::abs(qOverPtMax_ - other.qOverPtMax_) < epsilon
            && std::abs(phi0Min_ - other.phi0Min_) < epsilon
            && std::abs(phi0Max_ - other.phi0Max_) < epsilon
            && qOverPtDivisionLevel_ == other.qOverPtDivisionLevel_
            && phi0DivisionLevel_ == other.phi0DivisionLevel_;
    }

    inline AccumulatorRegion subregionQOverPtMinPhi0Min() const
    {
        return AccumulatorRegion(qOverPtMin_, 0.5f * (qOverPtMin_ + qOverPtMax_), phi0Min_, 0.5f * (phi0Min_ + phi0Max_), qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
    }

    inline AccumulatorRegion subregionQOverPtMinPhi0Max() const
    {
        return AccumulatorRegion(qOverPtMin_, 0.5f * (qOverPtMin_ + qOverPtMax_), 0.5f * (phi0Min_ + phi0Max_), phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
    }

    inline AccumulatorRegion subregionQOverPtMaxPhi0Min() const
    {
        return AccumulatorRegion(0.5f * (qOverPtMin_ + qOverPtMax_), qOverPtMax_, phi0Min_, 0.5f * (phi0Min_ + phi0Max_), qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
    }

    inline AccumulatorRegion subregionQOverPtMaxPhi0Max() const
    {
        return AccumulatorRegion(0.5f * (qOverPtMin_ + qOverPtMax_), qOverPtMax_, 0.5f * (phi0Min_ + phi0Max_), phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_ + 1);
    }

    inline AccumulatorRegion subregionQOverPtMin() const
    {
        return AccumulatorRegion(qOverPtMin_, 0.5f * (qOverPtMin_ + qOverPtMax_), phi0Min_, phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_);
    }

    inline AccumulatorRegion subregionQOverPtMax() const
    {
        return AccumulatorRegion(0.5f * (qOverPtMin_ + qOverPtMax_), qOverPtMax_, phi0Min_, phi0Max_, qOverPtDivisionLevel_ + 1, phi0DivisionLevel_);
    }

    inline AccumulatorRegion subregionPhi0Min() const
    {
        return AccumulatorRegion(qOverPtMin_, qOverPtMax_, phi0Min_, 0.5f * (phi0Min_ + phi0Max_), qOverPtDivisionLevel_, phi0DivisionLevel_ + 1);
    }

    inline AccumulatorRegion subregionPhi0Max() const
    {
        return AccumulatorRegion(qOverPtMin_, qOverPtMax_, 0.5f * (phi0Min_ + phi0Max_), phi0Max_, qOverPtDivisionLevel_, phi0DivisionLevel_ + 1);
    }

    float qOverPtMin_;
    float qOverPtMax_;
    float phi0Min_;
    float phi0Max_;
    u_int8_t qOverPtDivisionLevel_;
    u_int8_t phi0DivisionLevel_;
    u_int32_t pointListBegin_;
    u_int32_t pointListEnd_;
};