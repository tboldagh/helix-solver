#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <stdint.h>
#include "HelixSolver/Point.h"

namespace HelixSolver
{
    class Event
    {
    public:
        using EventId = uint32_t;

        Event(EventId id, std::unique_ptr<std::vector<Point>> Points);
        Event(const Event& other);

        const std::vector<std::function<float(float)>>& getPointsFuncs() const;

        // TODO: fix building process and make getters inline
        EventId getId() const;
        std::vector<float>& getR();
        std::vector<float>& getPhi();
        std::vector<float>& getZ();
        std::vector<uint8_t>& getLayers();

        void buildPointsFunctions();

        void print() const;

    private:
        EventId id;

        std::unique_ptr<std::vector<Point>> Points;
        std::vector<std::function<float(float)>> PointsFunctions;
        std::vector<float> rs;
        std::vector<float> phis;
        std::vector<float> z;
        std::vector<uint8_t> layers;
    };

} // HelixSolver
