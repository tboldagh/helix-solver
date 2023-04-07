#include <fstream>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include "HelixSolver/Debug.h"
#include "HelixSolver/Event.h"

namespace HelixSolver
{
    Event::Event(EventId id, std::unique_ptr<std::vector<Point>> Points)
    : id(id)
    , Points(std::move(Points))
    {
        buildPointsFunctions();
    }

    Event::Event(const Event& other)
    : id(other.id)
    {
        Points = std::make_unique<std::vector<Point>>();
        for(const Point& Point : *other.Points)
        {
            Points->push_back(Point);
        }
        buildPointsFunctions();
    }

    void Event::print() const
    {
        std::cout.precision(64);
        for (const Point& Point : *Points) {
            std::cout << Point.x << " " << Point.y << " " << Point.z << std::endl;
        }
    }

    const std::vector<std::function<float(float)>>&  Event::getPointsFuncs() const
    {
        return PointsFunctions;
    }

    void Event::buildPointsFunctions()
    {
        for (const Point& Point : *Points)
        {
            const float r = sqrt(Point.x * Point.x + Point.y * Point.y);
            const float phi = atan2(Point.y, Point.x);
            rs.push_back(r);
            phis.push_back(phi);
            z.push_back(Point.z);
            layers.push_back(Point.layer);
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
    std::vector<float>& Event::getZ()
    {
        return z;
    }

    std::vector<uint8_t>& Event::getLayers()
    {
        return layers;
    }
} // namespace HelixSolver
