#include <fstream>
#include <iostream>
#include <TFile.h>
#include <TTree.h>

#include "HelixSolver/AccumulatorHelper.h"
#include "HelixSolver/Event.h"

namespace HelixSolver {

    Event::Event(std::string filePath) {
        LoadFromFile(filePath);
    }

    void Event::LoadFromFile(std::string filePath) {
        try {
            std::ifstream pointsFile(filePath);
            float x, y, z;
            uint32_t layer;
            while (pointsFile >> x >> y >> z >> layer) {
                m_stubs.push_back(Stub{x, y, z, static_cast<uint8_t>(layer)});
            }
        }
        catch (std::exception &exc) {
            std::cerr << exc.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Event::loadFromRootFile(std::string path)
    {
        // TODO add exception handling

        std::unique_ptr<TFile> file(TFile::Open(path.c_str()));
        std::unique_ptr<TTree> hitsTree(file->Get<TTree>("hits"));

        float x;
        float y;
        float z;
        uint32_t layer;
        hitsTree->SetBranchAddress("tx", &x);
        hitsTree->SetBranchAddress("tx", &y);
        hitsTree->SetBranchAddress("tz", &z);
        hitsTree->SetBranchAddress("layer_id", &layer);
        for(int i = 0; hitsTree->LoadTree(i) >= 0; i++)
        {
            hitsTree->GetEntry(i);
            m_stubs.push_back(Stub{x, y, z, static_cast<uint8_t>(layer)});
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
        return stubsFunctions;
    }

    void Event::BuildStubsFunctions(const nlohmann::json& config) {
        for (const auto& stub : m_stubs) {
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

    std::vector<float> Event::GetR() const {
        return rs;
    }
    
    std::vector<float> Event::GetPhi() const {
        return phis;
    }

    std::vector<uint8_t> Event::GetLayers() const {
        return layers;
    }

} // namespace HelixSolver
