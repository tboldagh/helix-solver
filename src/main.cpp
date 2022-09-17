#include <iostream>

#include "HelixSolver/Application.h"

int main(int argc, char *argv[])
{
    std::vector<std::string> argvVector;
    for (int i = 0; i < argc; ++i) argvVector.push_back(argv[i]);

    HelixSolver::Application app(argvVector);
    app.Run();

    return 0;
}
