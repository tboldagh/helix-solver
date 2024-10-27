#pragma once

#include "SplitterUsm/SplitterSettings.h"
#include "EventUsm/EventUsm.h"

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
        static constexpr u_int32_t MaxRegionsPerPoint = 16;

        PointsWithRegionIds(std::unique_ptr<EventUsm>&& event, std::unique_ptr<std::unordered_map<u_int32_t, std::array<u_int16_t, MaxRegionsPerPoint>>>&& regionIdsByPointIndex);

        std::unique_ptr<EventUsm> event;
        std::unique_ptr<std::unordered_map<u_int32_t, std::array<uint16_t, MaxRegionsPerPoint>>> regionIdsByPointIndex;
    };

    static std::optional<SplitterSettings> readSplitterSettings(const std::string& path);
    static bool writeSplitterSettings(const std::string& path, const SplitterSettings& settings);
    static std::optional<PointsWithRegionIds> readPointsWithRegionIds(const std::string& path, EventUsm::EventId eventId);
};