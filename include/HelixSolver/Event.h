#pragma once

#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

#include <HelixSolver/Stub.h>

namespace HelixSolver {

    class Event {
    public:
        Event() = default;

        Event(std::string p_filePath);

        void LoadFromFile(std::string p_filePath);
        // TODO: loadFromRootFile

        const std::vector<Stub> &GetStubs() const;
        const std::vector<std::function<double(double)>> &GetStubsFuncs() const;

        void BuildStubsFunctions(const nlohmann::json& config);

        void Print() const;

    private:
        std::vector<Stub> m_stubs;
        std::vector<std::function<double(double)>> m_stubsFunctions;
    };

} // HelixSolver
