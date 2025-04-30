#include "CpuHelixSolver/Event.h"

Event::Event()
{
    xs_ = new float[MaxPoints];
    ys_ = new float[MaxPoints];
    zs_ = new float[MaxPoints];
}

Event::~Event()
{
    delete[] xs_;
    delete[] ys_;
    delete[] zs_;
}