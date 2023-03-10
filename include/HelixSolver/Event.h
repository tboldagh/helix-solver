#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <stdint.h>
#include "HelixSolver/Stub.h"

namespace HelixSolver
{
    class Event
    {
    public:
        using EventId = uint32_t;

        Event(EventId id, std::unique_ptr<std::vector<Stub>> stubs);
        Event(const Event& other);

        const std::vector<std::function<float(float)>>& getStubsFuncs() const;

        // TODO: fix building process and make getters inline
        EventId getId() const;
        std::vector<float>& getR();
        std::vector<float>& getPhi();
        std::vector<uint8_t>& getLayers();

        void buildStubsFunctions();

        void print() const;

    private:
        EventId id;

        std::unique_ptr<std::vector<Stub>> stubs;
        std::vector<std::function<float(float)>> stubsFunctions;
        std::vector<float> rs;
        std::vector<float> phis;
        std::vector<uint8_t> layers;
    };

} // HelixSolver
