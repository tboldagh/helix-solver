#include <iostream>
#include <memory>

#include <HelixSolver/Application.h>

int main(const int argc, const char** argv)
{
    std::unique_ptr<HelixSolver::Application> app =
            std::make_unique<HelixSolver::Application>();

    app->Run();

    return 0;
}
