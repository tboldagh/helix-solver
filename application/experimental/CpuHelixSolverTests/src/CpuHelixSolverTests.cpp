#include "CpuHelixSolverTests/CpuHelixSolverTests.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"
#include "Readers/SpacepointsReader.h"
#include "Readers/ParticleInitialReader.h"
#include "SpacepointsGenerator/SpacepointsGenerator.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <thread>

std::string SolverTestParams::testTypeToString(TestType testType)
{
    switch (testType)
    {
        case TestType::SpacepointsSingleEvent:
            return "SpacepointsSingleEvent";
        case TestType::ParticlesInitial:
            return "ParticlesInitial";
        case TestType::RunPerParticleInitial:
            return "RunPerParticleInitial";
        case TestType::SpacepointsSingleEventPerformance:
            return "SpacepointsSingleEventPerformance";
        default:
            return "Invalid";
    }
}

bool SolverTestParams::isOk() const
{
    switch (testType_)
    {
        case TestType::Invalid:
            return false;
        case TestType::SpacepointsSingleEvent:
            return !inputSpacepointsFile_.empty() && !outputResultsFile_.empty();
        case TestType::ParticlesInitial:
            return !inputParticlesInitialFile_.empty()
                    && !inputSpacepointsGenerationParamsFile_.empty() && !outputResultsFile_.empty();
        case TestType::RunPerParticleInitial:
            return !inputParticlesInitialFile_.empty()
                    && !inputSpacepointsGenerationParamsFile_.empty() && !outputResultsFile_.empty();
        case TestType::SpacepointsSingleEventPerformance:
            return !inputSpacepointsFile_.empty() && !outputResultsFile_.empty();
        default:
            return false;
    }
}

SolverTestParams SolverTestParams::readFromJson(const std::string& jsonPath) const
{
    try
    {
        std::ifstream file(jsonPath);
        nlohmann::json json;
        file >> json;

        SolverTestParams params;
        params.testType_ = json.contains("testType") ? static_cast<TestType>(testTypeFromString(json["testType"])) : TestType::Invalid;
        params.eventId_ = json.contains("eventId") ? static_cast<uint32_t>(json["eventId"]) : 0;
        params.multipleEventIds_ = json.contains("multipleEventIds") ? json["multipleEventIds"].get<std::vector<uint32_t>>() : std::vector<uint32_t>();
        params.numRuns_ = json.contains("numRuns") ? static_cast<uint32_t>(json["numRuns"]) : 0;
        params.inputSpacepointsFile_ = json.contains("inputSpacepointsFile") ? static_cast<std::string>(json["inputSpacepointsFile"]) : "";
        params.inputParticlesInitialFile_ = json.contains("inputParticlesInitialFile") ? static_cast<std::string>(json["inputParticlesInitialFile"]) : "";
        params.inputSpacepointsGenerationParamsFile_ = json.contains("inputSpacepointsGenerationParamsFile") ? static_cast<std::string>(json["inputSpacepointsGenerationParamsFile"]) : "";
        params.outputResultsFile_ = json.contains("outputResultsFile") ? static_cast<std::string>(json["outputResultsFile"]) : "";
        params.maxAbsXy_ = json.contains("maxAbsXy") ? static_cast<float>(json["maxAbsXy"]) : 0.0f;
        params.maxAbsZ_ = json.contains("maxAbsZ") ? static_cast<float>(json["maxAbsZ"]) : 0.0f;
        params.minZAngle_ = json.contains("minZAngle") ? static_cast<float>(json["minZAngle"]) : 0.0f;
        params.maxZAngle_ = json.contains("maxZAngle") ? static_cast<float>(json["maxZAngle"]) : 0.0f;
        params.minXAngle_ = json.contains("minXAngle") ? static_cast<float>(json["minXAngle"]) : 0.0f;
        params.maxXAngle_ = json.contains("maxXAngle") ? static_cast<float>(json["maxXAngle"]) : 0.0f;
        params.poleRegionAngle_ = json.contains("poleRegionAngle") ? static_cast<float>(json["poleRegionAngle"]) : 0.0f;
        params.interactionRegionMin_ = json.contains("interactionRegionMin") ? static_cast<float>(json["interactionRegionMin"]) : 0.0f;
        params.interactionRegionMax_ = json.contains("interactionRegionMax") ? static_cast<float>(json["interactionRegionMax"]) : 0.0f;
        params.zAngleMargin_ = json.contains("zAngleMargin") ? static_cast<float>(json["zAngleMargin"]) : 0.0f;
        params.xAngleMargin_ = json.contains("xAngleMargin") ? static_cast<float>(json["xAngleMargin"]) : 0.0f;
        params.numZRanges_ = json.contains("numZRanges") ? static_cast<uint32_t>(json["numZRanges"]) : 0;
        params.numXRanges_ = json.contains("numXRanges") ? static_cast<uint32_t>(json["numXRanges"]) : 0;
        params.filterOutCenterR_ = json.contains("filterOutCenterR") ? static_cast<float>(json["filterOutCenterR"]) : 0.0f;
        params.filterOutCenterZ_ = json.contains("filterOutCenterZ") ? static_cast<float>(json["filterOutCenterZ"]) : 0.0f;
        params.hitsThresholds_ = json.contains("hitsThresholds") ? json["hitsThresholds"].get<std::vector<uint8_t>>() : std::vector<uint8_t>();
        params.linesCrossingsThresholds_ = json.contains("linesCrossingsThresholds") ? json["linesCrossingsThresholds"].get<std::vector<uint8_t>>() : std::vector<uint8_t>();
        return params;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Failed to read test settings from JSON file: " + std::string(e.what()));
        return SolverTestParams();
    }
}

SolverTestParams::TestType SolverTestParams::testTypeFromString(const std::string& testTypeStr) const
{
    if (testTypeStr == "SpacepointsSingleEvent")
    {
        return TestType::SpacepointsSingleEvent;
    }
    else if (testTypeStr == "ParticlesInitial")
    {
        return TestType::ParticlesInitial;
    }
    else if (testTypeStr == "RunPerParticleInitial")
    {
        return TestType::RunPerParticleInitial;
    }
    else if (testTypeStr == "SpacepointsSingleEventPerformance")
    {
        return TestType::SpacepointsSingleEventPerformance;
    }
    else return TestType::Invalid;
}

std::string SolverTestParams::toString() const
{
    std::stringstream ss;
    ss << "{\n"
        << "\t\"testType\": \"" << testTypeToString(testType_) << "\",\n"
        << "\t\"eventId\": " << eventId_ << ",\n"
        << "\t\"multipleEventIds\": " << vectorToString(multipleEventIds_) << ",\n"
        << "\t\"inputSpacepointsFile\": \"" << inputSpacepointsFile_ << "\",\n"
        << "\t\"inputParticlesInitialFile\": \"" << inputParticlesInitialFile_ << "\",\n"
        << "\t\"inputSpacepointsGenerationParamsFile\": \"" << inputSpacepointsGenerationParamsFile_ << "\",\n"
        << "\t\"outputResultsFile\": \"" << outputResultsFile_ << "\",\n"
        << "\t\"maxAbsXy\": " << maxAbsXy_ << ",\n"
        << "\t\"maxAbsZ\": " << maxAbsZ_ << ",\n"
        << "\t\"minZAngle\": " << minZAngle_ << ",\n"
        << "\t\"maxZAngle\": " << maxZAngle_ << ",\n"
        << "\t\"minXAngle\": " << minXAngle_ << ",\n"
        << "\t\"maxXAngle\": " << maxXAngle_ << ",\n"
        << "\t\"poleRegionAngle\": " << poleRegionAngle_ << ",\n"
        << "\t\"interactionRegionMin\": " << interactionRegionMin_ << ",\n"
        << "\t\"interactionRegionMax\": " << interactionRegionMax_ << ",\n"
        << "\t\"zAngleMargin\": " << zAngleMargin_ << ",\n"
        << "\t\"xAngleMargin\": " << xAngleMargin_ << ",\n"
        << "\t\"numZRanges\": " << static_cast<uint32_t>(numZRanges_) << ",\n"
        << "\t\"numXRanges\": " << static_cast<uint32_t>(numXRanges_) << ",\n"
        << "\t\"filterOutCenterR\": " << filterOutCenterR_ << ",\n"
        << "\t\"filterOutCenterZ\": " << filterOutCenterZ_ << ",\n"
        << "\t\"hitsThresholds\": " << vectorToString(hitsThresholds_) << ",\n"
        << "\t\"linesCrossingsThresholds\": " << vectorToString(linesCrossingsThresholds_) << "\n"
        << "}";
    return ss.str();
}

CpuHelixSolverTests::CpuHelixSolverTests(const SolverTestParams& params)
    : params_(params)
{
}

void CpuHelixSolverTests::run()
{
    LOG_INFO("Running test: " + params_.toString());

    if (!createHelixSolver())
    {
        LOG_ERROR("Failed to create helix solver");
        return;
    }

    if (!createTasks())
    {
        LOG_ERROR("Failed to create tasks");
        return;
    }
    LOG_INFO("Created " + std::to_string(tasks_.size()) + " tasks");

    if (!createOutputFile(params_.outputResultsFile_))
    {
        LOG_ERROR("Failed to create output file");
        return;
    }

    std::ofstream file(params_.outputResultsFile_, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open output file: " + params_.outputResultsFile_);
        return;
    }
    writeResultsHeader(file);

    // if (params_.testType_ == SolverTestParams::TestType::SpacepointsSingleEventPerformance)
    // {
    //     for (uint64_t i = 0; i < tasks_.size(); ++i)
    //     {
    //         helixSolvers_[0]->solve(*tasks_[i]);
    //         writeResult(file, *tasks_[i]);
    //     }
    // }
    // else
    // {
        for (uint64_t i = 0; i < tasks_.size(); i += NumThreads)
        {
            std::vector<std::thread> threads;
            const uint64_t lastIndex = std::min(i + NumThreads, tasks_.size());
            for (uint64_t j = 0; j < NumThreads; ++j)
            {
                if (i + j >= lastIndex)
                {
                    break;
                }

                threads.emplace_back([this, j, i]() {
                    helixSolvers_[j]->solve(*tasks_[i + j]);
                });
            }

            for (auto& thread : threads)
            {
                thread.join();
            }

            for (uint64_t j = i; j < lastIndex; ++j)
            {
                writeResult(file, *tasks_[j]);
            }
        }
    // }
    
    LOG_INFO("Test completed successfully. Results written to: " + params_.outputResultsFile_);
}

bool CpuHelixSolverTests::createHelixSolver()
{
    splitterSettings_ = SplitterSettings(
        params_.maxAbsXy_, params_.maxAbsZ_,
        params_.minZAngle_, params_.maxZAngle_,
        params_.minXAngle_, params_.maxXAngle_,
        params_.poleRegionAngle_,
        params_.interactionRegionMin_, params_.interactionRegionMax_,
        params_.zAngleMargin_, params_.xAngleMargin_,
        params_.numZRanges_, params_.numXRanges_,
        params_.filterOutCenterR_, params_.filterOutCenterZ_
    );

    if (!params_.hitsThresholds_.empty())
    {
        for (uint32_t i = 0; i < splitterSettings_.wedges_.getSize(); ++i)
        {
            splitterSettings_.wedges_[i].solutionHitsThreshold_ = params_.hitsThresholds_[i / splitterSettings_.numZRanges_];
        }
    }

    if (!params_.linesCrossingsThresholds_.empty())
    {
        for (uint32_t i = 0; i < splitterSettings_.wedges_.getSize(); ++i)
        {
            splitterSettings_.wedges_[i].linesCrossingsThreshold_ = params_.linesCrossingsThresholds_[i / splitterSettings_.numZRanges_];
        }
    }

    if (!splitterSettings_.isValid())
    {
        LOG_ERROR("Invalid splitter settings");
        return false;
    }

    splitter_ = Splitter(splitterSettings_);
    for (uint64_t i = 0; i < NumThreads; ++i)
    {
        helixSolvers_.emplace_back(std::make_unique<HelixSolver>(splitter_));
    }
    return true;
}

bool CpuHelixSolverTests::createTasks()
{
    switch (params_.testType_)
    {
        case SolverTestParams::TestType::SpacepointsSingleEvent:
            return createTasksSpacepointsSingleEvent();
        case SolverTestParams::TestType::ParticlesInitial:
            return createTasksParticlesInitial();
        case SolverTestParams::TestType::RunPerParticleInitial:
            return createTasksRunPerParticleInitial();
        case SolverTestParams::TestType::SpacepointsSingleEventPerformance:
            return createTasksSpacepointsSingleEventPerformance();
        default:
            LOG_ERROR("Invalid test type");
            return false;
    }
}

bool CpuHelixSolverTests::createTasksSpacepointsSingleEvent()
{
    std::vector<uint32_t> eventIds;
    if (params_.multipleEventIds_.empty())
    {
        eventIds.push_back(params_.eventId_);
    }
    else
    {
        eventIds = params_.multipleEventIds_;
    }

    Readers::SpacepointsReader spacepointsReader(params_.inputSpacepointsFile_);
    std::vector<DataTypes::Spacepoint> spacepoints = spacepointsReader.readRootAll();
    if (spacepoints.empty())
    {
        LOG_ERROR("No spacepoints found in " + params_.inputSpacepointsFile_);
        return false;
    }

    for (const auto& eventId : eventIds)
    {
        LOG_DEBUG("Creating task for spacepoints single event, id: " + std::to_string(eventId));
        
        Event& event = *events_.emplace_back(std::make_unique<Event>(eventId));
        uint32_t index = 0;
        for (const auto& spacepoint : spacepoints)
        {
            if (spacepoint.eventId_ != eventId)
            {
                continue;
            }
            
            event.xs_[index] = spacepoint.x_;
            event.ys_[index] = spacepoint.y_;
            event.zs_[index] = spacepoint.z_;
            ++index;
        }
        event.numPoints_ = index;

        Result& result = *results_.emplace_back(std::make_unique<Result>(eventId));
        tasks_.emplace_back(std::make_unique<Task>(event, result));
    }
        
    return true;
}

bool CpuHelixSolverTests::createTasksParticlesInitial()
{
    LOG_DEBUG("Creating tasks for particles initial, eventId: " + std::to_string(params_.eventId_));

    std::vector<DataTypes::ParticleInitial> particlesInitial;
    if (!readParticlesInitial(particlesInitial))
    {
        LOG_ERROR("Failed to read particles initial");
        return false;
    }

    std::vector<SpacepointsGenerationParams> spacepointsGenerationParams;
    if (!readSpacepointsGenerationParams(spacepointsGenerationParams))
    {
        LOG_ERROR("Failed to read spacepoints generation params");
        return false;
    }
    if (spacepointsGenerationParams.size() != particlesInitial.size())
    {
        LOG_ERROR("Number of spacepoints generation params (" + std::to_string(spacepointsGenerationParams.size()) + ") does not match number of particles initial (" + std::to_string(particlesInitial.size()) + ")");
        return false;
    }

    SpacepointsGenerator spacepointsGenerator(params_.maxAbsZ_, params_.maxAbsXy_);
    std::vector<DataTypes::Spacepoint> spacepoints;
    for (uint32_t i = 0; i < particlesInitial.size(); ++i)
    {
        spacepointsGenerator.generate(spacepoints, particlesInitial[i], spacepointsGenerationParams[i].r_, spacepointsGenerationParams[i].counterClockwise_, spacepointsGenerationParams[i].numPoints_);
    }
    LOG_DEBUG("Generated " + std::to_string(spacepoints.size()) + " spacepoints");
    
    const uint32_t eventId = 0;
    Event& event = *events_.emplace_back(std::make_unique<Event>(eventId));
    event.numPoints_ = spacepoints.size();
    for (uint32_t i = 0; i < spacepoints.size(); ++i)
    {
        event.xs_[i] = spacepoints[i].x_;
        event.ys_[i] = spacepoints[i].y_;
        event.zs_[i] = spacepoints[i].z_;
    }

    Result& result = *results_.emplace_back(std::make_unique<Result>(eventId));
    tasks_.emplace_back(std::make_unique<Task>(event, result));

    LOG_INFO("Tasks created for particles initial, eventId: " + std::to_string(eventId));
    return true;
}

bool CpuHelixSolverTests::createTasksRunPerParticleInitial()
{
    LOG_DEBUG("Creating tasks for run per particle initial, eventId: " + std::to_string(params_.eventId_));

    std::vector<DataTypes::ParticleInitial> particlesInitial;
    if (!readParticlesInitial(particlesInitial))
    {
        LOG_ERROR("Failed to read particles initial");
        return false;
    }

    std::vector<SpacepointsGenerationParams> spacepointsGenerationParams;
    if (!readSpacepointsGenerationParams(spacepointsGenerationParams))
    {
        LOG_ERROR("Failed to read spacepoints generation params");
        return false;
    }
    if (spacepointsGenerationParams.size() != particlesInitial.size())
    {
        LOG_ERROR("Number of spacepoints generation params (" + std::to_string(spacepointsGenerationParams.size()) + ") does not match number of particles initial (" + std::to_string(particlesInitial.size()) + ")");
        return false;
    }

    SpacepointsGenerator spacepointsGenerator(params_.maxAbsZ_, params_.maxAbsXy_);
    for (uint32_t i = 0; i < particlesInitial.size(); ++i)
    {
        // if (particlesInitial[i].particleId_ == 81064794869727232)
        // {
        //     LOG_DEBUG("The one");
        // }

        std::vector<DataTypes::Spacepoint> spacepoints;
        spacepointsGenerator.generate(spacepoints, particlesInitial[i], spacepointsGenerationParams[i].r_, spacepointsGenerationParams[i].counterClockwise_, spacepointsGenerationParams[i].numPoints_);

        const uint32_t eventId = i;
        eventIdParticleIdMap_[eventId] = particlesInitial[i].particleId_;
        Event& event = *events_.emplace_back(std::make_unique<Event>(eventId));
        event.numPoints_ = spacepoints.size();
        for (uint32_t j = 0; j < spacepoints.size(); ++j)
        {
            event.xs_[j] = spacepoints[j].x_;
            event.ys_[j] = spacepoints[j].y_;
            event.zs_[j] = spacepoints[j].z_;
        }

        Result& result = *results_.emplace_back(std::make_unique<Result>(eventId));
        tasks_.emplace_back(std::make_unique<Task>(event, result));
    }

    LOG_INFO("Tasks created for run per particle initial, eventId: " + std::to_string(params_.eventId_));
    return true;
}

bool CpuHelixSolverTests::createTasksSpacepointsSingleEventPerformance()
{
    std::vector<uint32_t> eventIds;
    if (params_.multipleEventIds_.empty())
    {
        eventIds.push_back(params_.eventId_);
    }
    else
    {
        eventIds = params_.multipleEventIds_;
    }

    Readers::SpacepointsReader spacepointsReader(params_.inputSpacepointsFile_);
    std::vector<DataTypes::Spacepoint> spacepoints = spacepointsReader.readRootAll();
    if (spacepoints.empty())
    {
        LOG_ERROR("No spacepoints found in " + params_.inputSpacepointsFile_);
        return false;
    }

    for (uint32_t runId = 0; runId < params_.numRuns_; ++runId)
    {
        for (const auto& eventId : eventIds)
        {
            LOG_DEBUG("Creating task for spacepoints single event performance, id: " + std::to_string(eventId));
            
            Event& event = *events_.emplace_back(std::make_unique<Event>(eventId));
            uint32_t index = 0;
            for (const auto& spacepoint : spacepoints)
            {
                if (spacepoint.eventId_ != eventId)
                {
                    continue;
                }
                
                event.xs_[index] = spacepoint.x_;
                event.ys_[index] = spacepoint.y_;
                event.zs_[index] = spacepoint.z_;
                ++index;
            }
            event.numPoints_ = index;
            
            Result& result = *results_.emplace_back(std::make_unique<Result>(eventId));
            tasks_.emplace_back(std::make_unique<Task>(event, result));
            taskToRunIdMap_[tasks_.back().get()] = runId;
        }
    }

    return true;
}

bool CpuHelixSolverTests::readParticlesInitial(std::vector<DataTypes::ParticleInitial>& particlesInitial)
{
    Readers::ParticleInitialReader particleInitialReader(params_.inputParticlesInitialFile_);
    particleInitialReader.readRootAll(particlesInitial);
    if (particlesInitial.empty())
    {
        LOG_ERROR("No particles initial found in " + params_.inputParticlesInitialFile_);
        return false;
    }
    
    particlesInitial.erase(std::remove_if(particlesInitial.begin(), particlesInitial.end(), [this](const DataTypes::ParticleInitial& particle) {
        return particle.eventId_ != params_.eventId_;
    }), particlesInitial.end());

    return true;
}

bool CpuHelixSolverTests::readSpacepointsGenerationParams(std::vector<SpacepointsGenerationParams>& spacepointsGenerationParams)
{
    std::ifstream file(params_.inputSpacepointsGenerationParamsFile_);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open " + params_.inputSpacepointsGenerationParamsFile_);
        return false;
    }

    // Skip header
    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string r, counterClockwise, numPoints;
        std::getline(ss, r, ',');
        std::getline(ss, counterClockwise, ',');
        std::getline(ss, numPoints, ',');
        spacepointsGenerationParams.emplace_back(std::stof(r), (counterClockwise == "true" || counterClockwise == "True"), std::stoi(numPoints));
    }

    return true;
}

bool CpuHelixSolverTests::createOutputFile(const std::string& filePath)
{
    try
    {
        std::filesystem::path path(filePath);
        std::filesystem::path parentDir = path.parent_path();
        
        if (!parentDir.empty() && !std::filesystem::exists(parentDir))
        {
            LOG_DEBUG("Creating parent directory: " + parentDir.string());
            std::filesystem::create_directories(parentDir);
        }
        
        std::ofstream file(filePath, std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
            LOG_ERROR("Failed to create/open output file: " + filePath);
            return false;
        }
        
        LOG_DEBUG("Successfully created/opened output file: " + filePath);
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception while creating output file: " + std::string(e.what()));
        return false;
    }
}

void CpuHelixSolverTests::writeResultsHeader(std::ofstream& file)
{
    if (params_.testType_ == SolverTestParams::TestType::SpacepointsSingleEventPerformance)
    {
        file << "eventId,\ttotal,\tsplitter,\tsolver\n";
    }
    else
    {
        file << "eventId,\trunId,\tindex,\tr,\tphi,\thitCount,\txAngleMin,\txAngleMax,\tparticleId\n";
    }
}

void CpuHelixSolverTests::writeResult(std::ofstream& file, const Task& task)
{
    switch (params_.testType_)
    {
        case SolverTestParams::TestType::SpacepointsSingleEvent:
            writeResultSpacepointsSingleEvent(file, task);
            break;
        case SolverTestParams::TestType::ParticlesInitial:
            writeResultParticlesInitial(file, task);
            break;
        case SolverTestParams::TestType::RunPerParticleInitial:
            writeResultRunPerParticleInitial(file, task);
            break;
        case SolverTestParams::TestType::SpacepointsSingleEventPerformance:
            writeResultSpacepointsSingleEventPerformance(file, task);
            break;
        default:
            LOG_ERROR("Invalid test type");
            break;
    }
}

void CpuHelixSolverTests::writeResultSpacepointsSingleEvent(std::ofstream& file, const Task& task)
{
    LOG_DEBUG("Writing result for spacepoints single event, id: " + std::to_string(task.getEvent().eventId_));

    const Result& result = task.getResult();
    const uint32_t eventId = task.getEvent().eventId_;
    for (uint32_t i = 0; i < result.numSolutions_; ++i)
    {
        const float r = result.solutionRs_[i];
        const float phi = result.solutionPhis_[i];
        const uint32_t hitCount = result.solutionHitCounts_[i];
        const float xAngleMin = result.xAngleMins_[i];
        const float xAngleMax = result.xAngleMaxs_[i];

        file << eventId << ",\t" << i << ",\t" << r << ",\t" << phi << ",\t" << hitCount << ",\t" << xAngleMin << ",\t" << xAngleMax << ",\tnull\n";
    }
}

void CpuHelixSolverTests::writeResultParticlesInitial(std::ofstream& file, const Task& task)
{
    const Result& result = task.getResult();
    const uint32_t eventId = params_.eventId_;
    for (uint32_t i = 0; i < result.numSolutions_; ++i)
    {
        const float r = result.solutionRs_[i];
        const float phi = result.solutionPhis_[i];
        const uint32_t hitCount = result.solutionHitCounts_[i];
        const float xAngleMin = result.xAngleMins_[i];
        const float xAngleMax = result.xAngleMaxs_[i];

        file << eventId << ",\t" << i << ",\t" << r << ",\t" << phi << ",\t" << hitCount << ",\t" << xAngleMin << ",\t" << xAngleMax << ",\tnull\n";
    }
}

void CpuHelixSolverTests::writeResultRunPerParticleInitial(std::ofstream& file, const Task& task)
{
    const Result& result = task.getResult();
    const uint32_t eventId = task.getEvent().eventId_;
    for (uint32_t i = 0; i < result.numSolutions_; ++i)
    {
        const float r = result.solutionRs_[i];
        const float phi = result.solutionPhis_[i];
        const uint32_t hitCount = result.solutionHitCounts_[i];
        const float xAngleMin = result.xAngleMins_[i];
        const float xAngleMax = result.xAngleMaxs_[i];
        const uint64_t particleId = eventIdParticleIdMap_[eventId];
    
        file << eventId << ",\t" << i << ",\t" << r << ",\t" << phi << ",\t" << hitCount << ",\t" << xAngleMin << ",\t" << xAngleMax << ",\t" << particleId << "\n";
    }
}

void CpuHelixSolverTests::writeResultSpacepointsSingleEventPerformance(std::ofstream& file, const Task& task)
{
    const uint32_t runId = taskToRunIdMap_[&task];
    LOG_DEBUG("Writing result for spacepoints single event performance, id: " + std::to_string(task.getEvent().eventId_) + ", runId: " + std::to_string(runId));

    const Result& result = task.getResult();
    file << task.getEvent().eventId_ << ",\t" << runId << ",\t" << result.totalExecutionTime_.count() << ",\t" << result.splitterExecutionTime_.count() << ",\t" << result.solverExecutionTime_.count() << "\n";
}

std::string getConfigPathFromArgs(int argc, char* argv[])
{
    for (int i = 1; i < argc - 1; ++i)
    {
        if (std::string(argv[i]) == "--config")
        {
            return std::string(argv[i + 1]);
        }
    }
    return "";
}

int main(int argc, char* argv[])
{
    Logger::OstreamLogger logger(std::cout);
    logger.setMinSeverity(Logger::LogMessage::Severity::Debug);
    Logger::ILogger::setGlobalInstance(&logger);

    const std::string configPath = getConfigPathFromArgs(argc, argv);
    if (configPath.empty())
    {
        LOG_ERROR("No config file provided (use --config <path> to specify config file)");
        return 1;
    }
    LOG_INFO("Using config file: " + configPath);

    SolverTestParams params;
    params = params.readFromJson(configPath);
    if (!params.isOk())
    {
        LOG_ERROR("Invalid test parameters");
        return 1;
    }

    CpuHelixSolverTests test(params);
    test.run();

    return 0;
}