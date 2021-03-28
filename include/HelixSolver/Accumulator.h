#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include <HelixSolver/Stub.h>

namespace HelixSolver
{

class Accumulator
{
public:
    Accumulator(nlohmann::json& p_config, const std::vector<Stub>& p_stubs);
private:
    nlohmann::json& m_config;
    const std::vector<Stub>& m_stubs;

    std::vector<double> m_X;
    std::vector<std::vector<uint8_t>> m_map;
};

} // namespace HelixSolver