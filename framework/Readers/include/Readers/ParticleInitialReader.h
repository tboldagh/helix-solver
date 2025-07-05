#pragma once

#include "DataTypes/ParticleInitial.h"

#include <string>
#include <vector>


namespace Readers
{
class ParticleInitialReader
{
public:
    ParticleInitialReader(const std::string& path);

    std::vector<DataTypes::ParticleInitial> readRootAll();

private:
    std::string path_;
};
}   // namespace Readers