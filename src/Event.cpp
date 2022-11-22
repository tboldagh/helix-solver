#include <fstream>
#include <iostream>
#include <TFile.h>
#include <TTree.h>

#include "HelixSolver/AccumulatorHelper.h"
#include "HelixSolver/Event.h"

namespace HelixSolver
{
    Event::Event(EventId id, const std::vector<Stub>& stubs)
    : id(id)
    {
        for (const auto& stub : stubs) this->stubs.push_back(stub);
        buildStubsFunctions();
    }
    
    
    void Event::loadFromFile(std::string& path, std::string& fileType)
    {
        if(fileType == std::string("root")) loadFromRootFile(path);
        else if(fileType == std::string("txt")) loadFromTxtFile(path);
    }

    void Event::print() const
    {
        std::cout.precision(64);
        for (auto &stub : stubs) {
            std::cout << stub.x << " " << stub.y << " " << stub.z << std::endl;
        }
    }

    const std::vector<Stub>& Event::getStubs() const
    {
        return stubs;
    }

    const std::vector<std::function<float(float)>>&  Event::getStubsFuncs() const
    {
        return stubsFunctions;
    }

    void Event::buildStubsFunctions()
    {
        for (const auto& stub : stubs)
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

    void Event::loadFromTxtFile(std::string filePath)
    {
        // TODO: Optimize by allocating memory for the vectors before loading data

        try
        {
            std::ifstream file(filePath);
            float x, y, z;
            uint32_t layer;
            while (file >> x >> y >> z >> layer)
            {
                stubs.push_back(Stub{x, y, z, static_cast<uint8_t>(layer)});
            }
        }
        catch (std::exception &exc)
        {
            std::cerr << exc.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Event::loadFromRootFile(std::string path)
    {
        // TODO: add exception handling

        // TODO: Optimize by allocating memory for the vectors before loading data

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
            stubs.push_back(Stub{x, y, z, static_cast<uint8_t>(layer)});
        }
    }
} // namespace HelixSolver
