#include "Readers/SpacepointsReader.h"
#include "Logger/Logger.h"

#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <vector>
#include <string>


namespace Readers
{
SpacepointsReader::SpacepointsReader(const std::string& path)
: path_(path) {}

std::vector<DataTypes::Spacepoint> SpacepointsReader::readRootAll()
{
    LOG_DEBUG("Reading spacepoints from file: " + path_);

    TFile file(path_.c_str());
    if (!file.IsOpen())
    {
        LOG_ERROR("Failed to read spacepoints from file: " + path_ + ", file not found");
        return {};
    }

    TTreeReader reader("spacepoints", &file);
    if (!reader.GetTree())
    {
        LOG_ERROR("Failed to read spacepoints from file: " + path_ + ", no spacepoints tree found");
        return {};
    }

    TTreeReaderValue<unsigned> eventIdReader(reader, "event_id");
    TTreeReaderValue<unsigned long long> measurementIdReader(reader, "measurement_id");
    TTreeReaderValue<unsigned long long> geometryIdReader(reader, "geometry_id");
    TTreeReaderValue<float> xReader(reader, "x");
    TTreeReaderValue<float> yReader(reader, "y");
    TTreeReaderValue<float> zReader(reader, "z");
    TTreeReaderValue<float> varRReader(reader, "var_r");
    TTreeReaderValue<float> varZReader(reader, "var_z");

    std::vector<uint32_t> eventIds;
    std::vector<uint64_t> measurementIds;
    std::vector<uint64_t> geometryIds;
    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> zs;
    std::vector<float> varRs;
    std::vector<float> varZs;

    while (reader.Next())
    {
        eventIds.push_back(*eventIdReader);
        measurementIds.push_back(*measurementIdReader);
        geometryIds.push_back(*geometryIdReader);
        xs.push_back(*xReader);
        ys.push_back(*yReader);
        zs.push_back(*zReader);
        varRs.push_back(*varRReader);
        varZs.push_back(*varZReader);
    }

    std::vector<DataTypes::Spacepoint> spacepoints;
    spacepoints.reserve(eventIds.size());
    for (size_t i = 0; i < eventIds.size(); ++i)
    {
        spacepoints.emplace_back(eventIds[i], measurementIds[i], geometryIds[i], xs[i], ys[i], zs[i], varRs[i], varZs[i]);
    }

    LOG_DEBUG("Read " + std::to_string(spacepoints.size()) + " spacepoints");

    return spacepoints;
}
}   // namespace Readers