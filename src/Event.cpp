#include <fstream>
#include <iostream>

#include "HelixSolver/AccumulatorHelper.h"
#include "HelixSolver/Event.h"

namespace HelixSolver {

    Event::Event(std::string p_filePath) {
        LoadFromFile(p_filePath);
    }

    void Event::LoadFromFile(std::string p_filePath) {
        try {
            std::ifstream l_pointsFile(p_filePath);
            float l_x, l_y, l_z;
            uint32_t l_layer;
            while (l_pointsFile >> l_x >> l_y >> l_z >> l_layer) {
                m_stubs.push_back(Stub{l_x, l_y, l_z, static_cast<uint8_t>(l_layer)});
            }
        }
        catch (std::exception &l_exc) {
            std::cerr << l_exc.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Event::Print() const {
        std::cout.precision(64);
        for (auto &stub : m_stubs) {
            std::cout << stub.x << " " << stub.y << " " << stub.z << std::endl;
        }
    }

    const std::vector<Stub> &Event::GetStubs() const {
        return m_stubs;
    }

    const std::vector<std::function<float(float)>> &Event::GetStubsFuncs() const {
        return m_stubsFunctions;
    }

    void Event::BuildStubsFunctions(const nlohmann::json& config) {
        for (const auto& stub : m_stubs) {
            const auto[rad, ang] = cart2pol(stub.x, stub.y);
            const float r = rad / 1000.0;
            const float phi = ang;

            m_r.push_back(r);
            m_phi.push_back(phi);
            m_layers.push_back(stub.layer);

            auto fun = [r, phi](float x) { return -r * x + phi; };
            m_stubsFunctions.push_back(fun);
        }
    }

    std::vector<float> Event::GetR() const {
        return m_r;
    }
    
    std::vector<float> Event::GetPhi() const {
        return m_phi;
    }

    std::vector<uint8_t> Event::GetLayers() const {
        return m_layers;
    }

} // namespace HelixSolver
