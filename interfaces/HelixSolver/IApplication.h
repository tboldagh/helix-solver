#pragma once

namespace HelixSolver
{

class IApplication
{
public:
    virtual void Run() = 0;
    virtual ~IApplication() {}
};

} // HelixSolver
