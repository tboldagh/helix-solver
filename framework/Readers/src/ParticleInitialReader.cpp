#include "Readers/ParticleInitialReader.h"
#include "Logger/Logger.h"

#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <vector>
#include <string>


namespace Readers
{
ParticleInitialReader::ParticleInitialReader(const std::string& path)
: path_(path) {}

std::vector<DataTypes::ParticleInitial> ParticleInitialReader::readRootAll()
{
    std::vector<DataTypes::ParticleInitial> particlesInitial;
    readRootAll(particlesInitial);
    return particlesInitial;
}

void ParticleInitialReader::readRootAll(std::vector<DataTypes::ParticleInitial>& particlesInitial)
{
    LOG_DEBUG("Reading particles initial from file: " + path_);

    TFile file(path_.c_str());
    if (!file.IsOpen())
    {
        LOG_ERROR("Failed to read particles initial from file: " + path_ + ", file not found");
        return;
    }

    TTreeReader reader("particles", &file);
    if (!reader.GetTree())
    {
        LOG_ERROR("Failed to read particles initial from file: " + path_ + ", no particles tree found");
        return;
    }

    TTreeReaderValue<unsigned> eventIdReader(reader, "event_id");
    TTreeReaderArray<unsigned long> particleIdReader(reader, "particle_id");
    TTreeReaderArray<int> particleTypeReader(reader, "particle_type");
    TTreeReaderArray<unsigned> processReader(reader, "process");
    TTreeReaderArray<float> vxReader(reader, "vx");
    TTreeReaderArray<float> vyReader(reader, "vy");
    TTreeReaderArray<float> vzReader(reader, "vz");
    TTreeReaderArray<float> vtReader(reader, "vt");
    TTreeReaderArray<float> pxReader(reader, "px");
    TTreeReaderArray<float> pyReader(reader, "py");
    TTreeReaderArray<float> pzReader(reader, "pz");
    TTreeReaderArray<float> mReader(reader, "m");
    TTreeReaderArray<float> qReader(reader, "q");
    TTreeReaderArray<float> etaReader(reader, "eta");
    TTreeReaderArray<float> phiReader(reader, "phi");
    TTreeReaderArray<float> ptReader(reader, "pt");
    TTreeReaderArray<float> pReader(reader, "p");
    TTreeReaderArray<unsigned> vertexPrimaryReader(reader, "vertex_primary");
    TTreeReaderArray<unsigned> vertexSecondaryReader(reader, "vertex_secondary");
    TTreeReaderArray<unsigned> particleReader(reader, "particle");
    TTreeReaderArray<unsigned> generationReader(reader, "generation");
    TTreeReaderArray<unsigned> subParticleReader(reader, "sub_particle");

    std::vector<uint32_t> eventIds;
    std::vector<uint64_t> particleIds;
    std::vector<int> particleTypes;
    std::vector<uint32_t> processes;
    std::vector<float> vxs;
    std::vector<float> vys;
    std::vector<float> vzs;
    std::vector<float> vts;
    std::vector<float> pxs;
    std::vector<float> pys;
    std::vector<float> pzs;
    std::vector<float> ms;
    std::vector<float> qs;
    std::vector<float> etas;
    std::vector<float> phis;
    std::vector<float> pts;
    std::vector<float> ps;
    std::vector<uint32_t> vertexPrimaryIds;
    std::vector<uint32_t> vertexSecondaryIds;
    std::vector<uint32_t> particles;
    std::vector<uint32_t> generations;
    std::vector<uint32_t> subParticleIds;   

    while (reader.Next())
    {
        uint32_t eventId = *eventIdReader;
        for (int i = 0; i < particleIdReader.GetSize(); i++)
        {
            eventIds.push_back(eventId);
            particleIds.push_back(particleIdReader[i]);
            particleTypes.push_back(particleTypeReader[i]);
            processes.push_back(processReader[i]);
            vxs.push_back(vxReader[i]);
            vys.push_back(vyReader[i]);
            vzs.push_back(vzReader[i]);
            vts.push_back(vtReader[i]);
            pxs.push_back(pxReader[i]);
            pys.push_back(pyReader[i]);
            pzs.push_back(pzReader[i]);
            ms.push_back(mReader[i]);
            qs.push_back(qReader[i]);
            etas.push_back(etaReader[i]);
            phis.push_back(phiReader[i]);
            pts.push_back(ptReader[i]);
            ps.push_back(pReader[i]);
            vertexPrimaryIds.push_back(vertexPrimaryReader[i]);
            vertexSecondaryIds.push_back(vertexSecondaryReader[i]);
            particles.push_back(particleReader[i]);
            generations.push_back(generationReader[i]);
            subParticleIds.push_back(subParticleReader[i]);
        }
    }

    particlesInitial.reserve(eventIds.size());
    for (size_t i = 0; i < eventIds.size(); ++i)
    {
        particlesInitial.emplace_back(eventIds[i], particleIds[i], particleTypes[i], processes[i], vxs[i], vys[i], vzs[i], vts[i], pxs[i], pys[i], pzs[i], ms[i], qs[i], etas[i], phis[i], pts[i], ps[i], vertexPrimaryIds[i], vertexSecondaryIds[i], particles[i], generations[i], subParticleIds[i]);
    }

    LOG_DEBUG("Read " + std::to_string(particlesInitial.size()) + " particles initial");
}
}   // namespace Readers