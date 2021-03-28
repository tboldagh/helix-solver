#pragma once

#include <nlohmann/json.hpp>

namespace HelixSolver
{

class Application
{
public:
    explicit Application(std::vector<std::string>& p_argv);
    int Run();
    ~Application();

private:
    nlohmann::json m_config;
    
    std::vector<std::string>& m_argv;
};


} // HelixSolver
