#pragma once

namespace HelixSolver {
struct Options{
    float ACC_X_PRECISION = 0.01;
    float ACC_PT_PRECISION = 0.1; // this is simplified approach, in reality it could be modified it depending on q/pt
};
}