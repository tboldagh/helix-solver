#include "SplitterUsm/TestDataLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <cmath>


TestDataLoader::PointsWithRegionIds::PointsWithRegionIds(std::unique_ptr<EventUsm>&& event, std::unique_ptr<std::unordered_map<u_int32_t, Splitter::RegionIds>>&& regionIdsByPointIndex)
: event_(std::move(event))
, regionIdsByPointIndex_(std::move(regionIdsByPointIndex)) {}


std::optional<SplitterSettings> TestDataLoader::readSplitterSettings(const std::string& path)
{
    try
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return std::nullopt;
        }

        nlohmann::json json;
        file >> json;

        // Detector properties
        const float maxAbsXy = json["detector_properties"]["max_abs_xy"];
        const float maxAbsZ = json["detector_properties"]["max_abs_z"];

        // Splitter properties
        const float minZAngle = json["splitter_properties"]["min_z_angle"];
        const float maxZAngle = json["splitter_properties"]["max_z_angle"];
        const float minXAgle = json["splitter_properties"]["min_x_angle"];
        const float maxXAgle = json["splitter_properties"]["max_x_angle"];
        const float poleRegionAngle = json["splitter_properties"]["pole_region_angle"];
        const float interactionRegionMin = json["splitter_properties"]["interaction_region_min"];
        const float interactionRegionMax = json["splitter_properties"]["interaction_region_max"];
        const float zAngleMargin = json["splitter_properties"]["z_angle_margin"];
        const float xAngleMargin = json["splitter_properties"]["x_angle_margin"];
        const u_int8_t numZRanges = json["splitter_properties"]["num_z_ranges"];
        const u_int8_t numXRanges = json["splitter_properties"]["num_x_ranges"];

        // Wedges
        std::vector<SplitterSettings::Wedge> wedges;
        for (const auto& wedgeJson : json["wedges"])
        {
            const u_int16_t id = wedgeJson["id"];
            const float zAngleMin = wedgeJson["z_angle_min"];
            const float zAngleMax = wedgeJson["z_angle_max"];
            const float xAngleMin = wedgeJson["x_angle_min"];
            const float xAngleMax = wedgeJson["x_angle_max"];
            const float interactionRegionWidth = wedgeJson["interaction_region_width"];
            wedges.emplace_back(id, zAngleMin, zAngleMax, xAngleMin, xAngleMax, interactionRegionWidth);
        }

        // Pole regions
        std::vector<SplitterSettings::PoleRegion> poleRegions;
        for (const auto& poleRegionJson : json["pole_regions"])
        {
            const u_int16_t id = poleRegionJson["id"];
            const float xAngle = poleRegionJson["x_angle"];
            const float interactionRegionWidth = poleRegionJson["interaction_region_width"];
            poleRegions.emplace_back(id, xAngle, interactionRegionWidth);
        }

        return SplitterSettings(
            maxAbsXy, maxAbsZ,
            minZAngle, maxZAngle,
            minXAgle, maxXAgle,
            poleRegionAngle,
            interactionRegionMin, interactionRegionMax,
            zAngleMargin, xAngleMargin,
            numZRanges, numXRanges,
            std::move(wedges),
            std::move(poleRegions)
        );
    }
    catch (const nlohmann::json::exception&)
    {
        return std::nullopt;
    }
}

bool TestDataLoader::writeSplitterSettings(const std::string& path, const SplitterSettings& settings)
{
    // Detector properties
    nlohmann::json json;
    json["detector_properties"]["max_abs_xy"] = settings.maxAbsXy_;
    json["detector_properties"]["max_abs_z"] = settings.maxAbsZ_;

    // Splitter properties
    json["splitter_properties"]["min_z_angle"] = settings.minZAngle_;
    json["splitter_properties"]["max_z_angle"] = settings.maxZAngle_;
    json["splitter_properties"]["min_x_angle"] = settings.minXAgle_;
    json["splitter_properties"]["max_x_angle"] = settings.maxXAgle_;
    json["splitter_properties"]["pole_region_angle"] = settings.poleRegionAngle_;
    json["splitter_properties"]["interaction_region_min"] = settings.interactionRegionMin_;
    json["splitter_properties"]["interaction_region_max"] = settings.interactionRegionMax_;
    json["splitter_properties"]["z_angle_margin"] = settings.zAngleMargin_;
    json["splitter_properties"]["x_angle_margin"] = settings.xAngleMargin_;
    json["splitter_properties"]["num_z_ranges"] = settings.numZRanges_;
    json["splitter_properties"]["num_x_ranges"] = settings.numXRanges_;

    // Wedges
    for (const auto& wedge : settings.wedges_)
    {
        nlohmann::json wedgesJson;
        wedgesJson["id"] = wedge.id_;
        wedgesJson["z_angle_min"] = wedge.zAngleMin_;
        wedgesJson["z_angle_max"] = wedge.zAngleMax_;
        wedgesJson["x_angle_min"] = wedge.xAngleMin_;
        wedgesJson["x_angle_max"] = wedge.xAngleMax_;
        wedgesJson["interaction_region_width"] = wedge.interactionRegionWidth_;
        json["wedges"].push_back(wedgesJson);
    }

    // Pole regions
    for (const auto& poleRegion : settings.poleRegions_)
    {
        nlohmann::json poleRegionJson;
        poleRegionJson["id"] = poleRegion.id_;
        poleRegionJson["x_angle"] = poleRegion.xAngle_;
        poleRegionJson["interaction_region_width"] = poleRegion.interactionRegionWidth_;
        json["pole_regions"].push_back(poleRegionJson);
    }

    std::ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    constexpr int indent = 4;
    file << json.dump(indent);

    return true;
}

std::optional<TestDataLoader::PointsWithRegionIds> TestDataLoader::readPointsWithRegionIds(const std::string& path, EventUsm::EventId eventId)
{
    try
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return std::nullopt;
        }

        // Column names
        std::string columnNames;
        std::getline(file, columnNames);
        static_cast<void>(columnNames);

        auto event = std::make_unique<EventUsm>(eventId);
        auto regionIdsByPointIndex = std::make_unique<std::unordered_map<u_int32_t, Splitter::RegionIds>>();

        // Read the points with region ids
        std::string line;
        while (std::getline(file, line) && line.find(",") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string valueStr;

            std::getline(iss, valueStr, ',');
            event->hostXs_[event->hostNumPoints_] = std::stof(valueStr);
            std::getline(iss, valueStr, ',');
            event->hostYs_[event->hostNumPoints_] = std::stof(valueStr);
            std::getline(iss, valueStr, ',');
            event->hostZs_[event->hostNumPoints_] = std::stof(valueStr);

            unsigned numRegions = 0;
            while (std::getline(iss, valueStr, ','))
            {
                (*regionIdsByPointIndex)[event->hostNumPoints_][numRegions] = std::stoi(valueStr);
                numRegions++;
            }

            for (unsigned i = numRegions; i < Splitter::MaxRegionsPerPoint; ++i)
            {
                (*regionIdsByPointIndex)[event->hostNumPoints_][i] = 0;
            }

            event->hostNumPoints_++;
        }

        return PointsWithRegionIds(std::move(event), std::move(regionIdsByPointIndex));
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}
