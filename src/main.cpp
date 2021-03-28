#include <iostream>
#include <memory>

#include <HelixSolver/Application.h>

int main(int argc, char* argv[])
{
    std::vector<std::string> l_argv;
    for (int i = 0; i < argc; ++i) l_argv.push_back(argv[i]);

    HelixSolver::Application app(l_argv);
    return app.Run();
}
