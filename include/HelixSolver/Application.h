#pragma once

#include <HelixSolver/IApplication.h>

namespace HelixSolver
{

class Application : public IApplication
{
public:
    void Run() override;
    ~Application() override;
};

} // HelixSolver
