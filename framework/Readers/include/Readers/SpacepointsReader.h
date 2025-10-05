#pragma once

#include "DataTypes/Spacepoint.h"

#include <string>
#include <vector>


namespace Readers
{
class SpacepointsReader
{
public:
    SpacepointsReader(const std::string& path);

    std::vector<DataTypes::Spacepoint> readRootAll();

private:
    std::string path_;
};
}   // namespace Readers