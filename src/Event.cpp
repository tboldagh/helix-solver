#include <fstream>
#include <iostream>

#include <HelixSolver/Event.h>

namespace HelixSolver {

    Event::Event(std::string p_filePath) {
        LoadFromFile(p_filePath);
    }

    void Event::LoadFromFile(std::string p_filePath) {
        try {
            std::ifstream l_pointsFile(p_filePath);
            double l_x, l_y, l_z;
            while (l_pointsFile >> l_x >> l_y >> l_z) {
                m_stubs.push_back(Stub{l_x, l_y, l_z});
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

} // namespace HelixSolver
