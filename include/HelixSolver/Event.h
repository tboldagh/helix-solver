#pragma once

#include <vector>

#include <HelixSolver/Stub.h>

namespace HelixSolver
{

class Event
{
public:
    Event() = default;
    Event(std::string p_filePath);
    void LoadFromFile(std::string p_filePath);
    // TODO: loadFromRootFile

    const std::vector<Stub>& GetStubs() const;
    void Print() const;
private:
    std::vector<Stub> m_stubs;
};

} // HelixSolver
