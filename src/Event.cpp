#include <fstream>
#include <iostream>
#include <TFile.h>
#include <TTree.h>

#include "HelixSolver/AccumulatorHelper.h"
#include "HelixSolver/Event.h"

namespace HelixSolver
{
    Event::Event(EventId id, std::unique_ptr<std::vector<Stub>> stubs)
    : id(id)
    , stubs(std::move(stubs))
    {
        buildStubsFunctions();
    }
    
    void Event::print() const
    {
        std::cout.precision(64);
        for (const Stub& stub : *stubs) {
            std::cout << stub.x << " " << stub.y << " " << stub.z << std::endl;
        }
    }

    const std::vector<std::function<float(float)>>&  Event::getStubsFuncs() const
    {
        return stubsFunctions;
    }

    void Event::buildStubsFunctions()
    {
        for (const Stub& stub : *stubs)
        {
            const auto[rad, ang] = cart2pol(stub.x, stub.y);
            const float r = rad / 1000.0;
            const float phi = ang;

            rs.push_back(r);
            phis.push_back(phi);
            layers.push_back(stub.layer);

            auto fun = [r, phi](float x) { return -r * x + phi; };
            stubsFunctions.push_back(fun);
        }
    }

    Event::EventId Event::getId() const
    {
        return id;
    }

    std::vector<float>& Event::getR()
    {
        return rs;
    }
    
    std::vector<float>& Event::getPhi()
    {
        return phis;
    }

    std::vector<uint8_t>& Event::getLayers()
    {
        return layers;
    }
} // namespace HelixSolver
