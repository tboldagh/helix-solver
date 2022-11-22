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
        using EventId = uint32_t;

        Event(EventId id, const std::vector<Stub>& stubs);

        void loadFromFile(std::string& path, std::string& fileType);

        const std::vector<Stub>& getStubs() const;
        const std::vector<std::function<float(float)>>& getStubsFuncs() const;
        
        // TODO: fix building process and make getters inline
        EventId getId() const;
        std::vector<float>& getR();
        std::vector<float>& getPhi();
        std::vector<uint8_t>& getLayers();

        void buildStubsFunctions();

        void print() const;

    private:
        EventId id;
        std::vector<Stub> stubs;
        std::vector<std::function<float(float)>> stubsFunctions;
        std::vector<float> rs;
        std::vector<float> phis;
        std::vector<uint8_t> layers;

        void loadFromTxtFile(std::string filePath); 
        void loadFromRootFile(std::string path); 
    };

} // HelixSolver
