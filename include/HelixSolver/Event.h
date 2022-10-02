#pragma once

#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

#include "HelixSolver/Stub.h"

namespace HelixSolver
{
    class Event
    {
    public:
        void loadFromFile(std::string& path, std::string& fileType);

        const std::vector<Stub>& getStubs() const;
        const std::vector<std::function<float(float)>>& getStubsFuncs() const;
        
        // TODO: fix building process and make getters inline
        std::vector<float>& getR();
        std::vector<float>& getPhi();
        std::vector<uint8_t>& getLayers();

        void buildStubsFunctions();

        void print() const;

    private:
        std::vector<Stub> stubs;
        std::vector<std::function<float(float)>> stubsFunctions;
        std::vector<float> rs;
        std::vector<float> phis;
        std::vector<uint8_t> layers;

        void loadFromTxtFile(std::string filePath); 
        void loadFromRootFile(std::string path); 
    };

} // HelixSolver
