#pragma once

#include "SplitterUsm/SplitterSettings.h"
#include "EventUsm/EventUsm.h"
#include "SplitterUsm/Splitter.h"

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

class TestDataLoader
{
public:
    class PointsWithRegionIds
    {
    public:
        PointsWithRegionIds() = default;
        PointsWithRegionIds(std::unique_ptr<EventUsm>&& event, std::unique_ptr<std::unordered_map<u_int32_t, Splitter::RegionIds>>&& regionIdsByPointIndex);
        PointsWithRegionIds(const PointsWithRegionIds&) = delete;
        PointsWithRegionIds(PointsWithRegionIds&&) = default;
        PointsWithRegionIds& operator=(const PointsWithRegionIds&) = delete;
        PointsWithRegionIds& operator=(PointsWithRegionIds&&) = default;
        ~PointsWithRegionIds() = default;

        std::unique_ptr<EventUsm> event_;
        std::unique_ptr<std::unordered_map<u_int32_t, Splitter::RegionIds>> regionIdsByPointIndex_;
    };

    static std::optional<SplitterSettings> readSplitterSettings(const std::string& path);
    static bool writeSplitterSettings(const std::string& path, const SplitterSettings& settings);
    static std::optional<std::unique_ptr<EventUsm>> readEvent(const std::string& path, EventUsm::EventId eventId);
    static std::optional<PointsWithRegionIds> readPointsWithRegionIds(const std::string& path, EventUsm::EventId eventId);
};