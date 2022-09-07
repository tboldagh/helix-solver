#pragma once

#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

#include "HelixSolver/Stub.h"

namespace HelixSolver {

    class Event {
    public:
        Event() = default;

        Event(std::string p_filePath);

        void LoadFromFile(std::string p_filePath); 
        void loadFromRootFile(std::string path);

        const std::vector<Stub> &GetStubs() const;
        const std::vector<std::function<float(float)>> &GetStubsFuncs() const;
        
        std::vector<float> GetR() const;
        std::vector<float> GetPhi() const;
        std::vector<uint8_t> GetLayers() const;

        void BuildStubsFunctions(const nlohmann::json& config);

        void Print() const;

    private:
        std::vector<Stub> m_stubs;
        std::vector<std::function<float(float)>> stubsFunctions;
        std::vector<float> rs;
        std::vector<float> phis;
        std::vector<uint8_t> layers;
    };

} // HelixSolver
