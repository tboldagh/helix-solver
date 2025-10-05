#include "Readers/SpacepointsReader.h"
#include "Readers/ParticleInitialReader.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"
#include "DataTypes/Spacepoint.h"
#include "DataTypes/ParticleInitial.h"

#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TList.h>
#include <TKey.h>
#include <TClass.h>
#include <TROOT.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>


void spacepointsToCsv(const std::string& inputFilePath, const std::string& outputFilePath)
{
    Readers::SpacepointsReader spacepointsReader(inputFilePath);
    std::vector<DataTypes::Spacepoint> spacepoints = spacepointsReader.readRootAll();

    // overwrite csv file
    std::ofstream csvFile(outputFilePath, std::ios::trunc);
    csvFile << "eventId,measurementId,geometryId,x,y,z,varR,varZ\n";
    for (const auto& spacepoint : spacepoints)
    {
        csvFile << spacepoint.eventId_
            << "," << spacepoint.measurementId_
            << "," << spacepoint.geometryId_
            << "," << spacepoint.x_
            << "," << spacepoint.y_
            << "," << spacepoint.z_
            << "," << spacepoint.varR_
            << "," << spacepoint.varZ_
            << "\n";
    }
    csvFile.close();
}

void particlesInitialToCsv(const std::string& inputFilePath, const std::string& outputFilePath)
{
    Readers::ParticleInitialReader particleInitialReader(inputFilePath);
    std::vector<DataTypes::ParticleInitial> particlesInitial = particleInitialReader.readRootAll();

    // overwrite csv file
    std::ofstream csvFile(outputFilePath, std::ios::trunc);
    csvFile << "eventId,particleId,particleType,process,vx,vy,vz,vt,px,py,pz,m,q,eta,phi,pt,p,vertexPrimaryId,vertexSecondaryId,particle,generation,subParticleId\n";
    for (const auto& particleInitial : particlesInitial)
    {
        csvFile << particleInitial.eventId_ 
            << "," << particleInitial.particleId_ 
            << "," << particleInitial.particleType_ 
            << "," << particleInitial.process_ 
            << "," << particleInitial.vx_ 
            << "," << particleInitial.vy_ 
            << "," << particleInitial.vz_ 
            << "," << particleInitial.vt_ 
            << "," << particleInitial.px_ 
            << "," << particleInitial.py_ 
            << "," << particleInitial.pz_ 
            << "," << particleInitial.m_ 
            << "," << particleInitial.q_ 
            << "," << particleInitial.eta_ 
            << "," << particleInitial.phi_ 
            << "," << particleInitial.pt_ 
            << "," << particleInitial.p_ 
            << "," << particleInitial.vertexPrimaryId_ 
            << "," << particleInitial.vertexSecondaryId_ 
            << "," << particleInitial.particles_ 
            << "," << particleInitial.generation_ 
            << "," << particleInitial.subParticleId_
            << "\n";
    }
    csvFile.close();
}

int main()
{
    Logger::OstreamLogger logger(std::cout);
    logger.setMinSeverity(Logger::LogMessage::Severity::Debug);
    Logger::ILogger::setGlobalInstance(&logger);

    // std::string spacepointsRootFilePath = "/helix/repo/application/thesis/spacepoints.root";
    // std::string spacepointsCsvFilePath = "/helix/repo/application/thesis/spacepoints.csv";
    // spacepointsToCsv(spacepointsRootFilePath, spacepointsCsvFilePath);

    // std::cout<<std::endl;

    // std::string particlesInitialRootFilePath = "/helix/repo/application/thesis/particles_initial.root";
    // std::string particlesInitialCsvFilePath = "/helix/repo/application/thesis/particles_initial.csv";
    // particlesInitialToCsv(particlesInitialRootFilePath, particlesInitialCsvFilePath);

    std::string spacepointsRootFilePath = "/helix/repo/data/odd_output_ttbar_PU200_100/spacepoints.root";
    std::string spacepointsCsvFilePath = "/helix/repo/data/odd_output_ttbar_PU200_100/spacepoints.csv";
    spacepointsToCsv(spacepointsRootFilePath, spacepointsCsvFilePath);

    std::cout<<std::endl;

    std::string particlesInitialRootFilePath = "/helix/repo/data/odd_output_ttbar_PU200_100/particles_initial.root";
    std::string particlesInitialCsvFilePath = "/helix/repo/data/odd_output_ttbar_PU200_100/particles_initial.csv";
    particlesInitialToCsv(particlesInitialRootFilePath, particlesInitialCsvFilePath);

    // std::string spacepointsRootFilePath = "/helix/repo/application/thesis/spacepoints_single.root";
    // std::string spacepointsCsvFilePath = "/helix/repo/application/thesis/spacepoints_single.csv";
    // spacepointsToCsv(spacepointsRootFilePath, spacepointsCsvFilePath);

    // std::cout<<std::endl;

    // std::string particlesInitialRootFilePath = "/helix/repo/application/thesis/particles_initial_single.root";
    // std::string particlesInitialCsvFilePath = "/helix/repo/application/thesis/particles_initial_single.csv";
    // particlesInitialToCsv(particlesInitialRootFilePath, particlesInitialCsvFilePath);

    return 0;
}