#pragma once

#include "CpuHelixSolver/HelixSolver.h"
#include "CpuHelixSolver/Task.h"
#include "CpuHelixSolver/Result.h"
#include "CpuHelixSolver/Event.h"
#include "SplitterUsm/Splitter.h"
#include "DataTypes/ParticleInitial.h"
#include "DataTypes/Spacepoint.h"

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>

class SolverTestParams
{
public:
    enum class TestType : uint8_t
    {
        Invalid,
        SpacepointsSingleEvent,
        ParticlesInitial,
        RunPerParticleInitial,
        SpacepointsSingleEventPerformance
    };
    
    static std::string testTypeToString(TestType testType);
    bool isOk() const;
    SolverTestParams readFromJson(const std::string& jsonPath) const;
    TestType testTypeFromString(const std::string& testTypeStr) const;
    std::string toString() const;

    // Test settings
    TestType testType_;
    uint32_t eventId_;
    std::vector<uint32_t> multipleEventIds_;
    uint32_t numRuns_;

    // IO
    std::string inputSpacepointsFile_;
    std::string inputParticlesInitialFile_;
    std::string inputSpacepointsGenerationParamsFile_;
    std::string outputResultsFile_;

    // Splitter settings
    float maxAbsXy_;
    float maxAbsZ_;
    float minZAngle_;
    float maxZAngle_;
    float minXAngle_;
    float maxXAngle_;
    float poleRegionAngle_;
    float interactionRegionMin_;
    float interactionRegionMax_;
    float zAngleMargin_;
    float xAngleMargin_;
    uint8_t numZRanges_;
    uint8_t numXRanges_;
    float filterOutCenterR_;
    float filterOutCenterZ_;
    std::vector<uint8_t> hitsThresholds_;
    std::vector<uint8_t> linesCrossingsThresholds_;

private:
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    std::string vectorToString(const std::vector<T>& vector) const
    {
        if (vector.empty())
        {
            return "[]";
        }

        std::stringstream ss;
        ss << "[" << static_cast<uint64_t>(vector[0]);
        for (size_t i = 1; i < vector.size(); ++i)
        {
            ss << ", " << static_cast<uint64_t>(vector[i]);
        }
        ss << "]";
        return ss.str();
    }
};

class SpacepointsGenerationParams
{
public:
    SpacepointsGenerationParams(float r, bool counterClockwise, uint8_t numPoints)
        : r_(r), counterClockwise_(counterClockwise), numPoints_(numPoints) {}

    float r_;
    bool counterClockwise_;
    uint8_t numPoints_;
};

class CpuHelixSolverTests
{
public:
    CpuHelixSolverTests(const SolverTestParams& params);
    void run();

private:
    bool createHelixSolver();
    bool createTasks();

    bool createTasksSpacepointsSingleEvent();
    bool createTasksParticlesInitial();
    bool createTasksRunPerParticleInitial();
    bool createTasksSpacepointsSingleEventPerformance();
    bool readParticlesInitial(std::vector<DataTypes::ParticleInitial>& particlesInitial);
    bool readSpacepointsGenerationParams(std::vector<SpacepointsGenerationParams>& spacepointsGenerationParams);

    bool createOutputFile(const std::string& filePath);
    void writeResultsHeader(std::ofstream& file);
    void writeResult(std::ofstream& file, const Task& task);
    void writeResultSpacepointsSingleEvent(std::ofstream& file, const Task& task);
    void writeResultParticlesInitial(std::ofstream& file, const Task& task);
    void writeResultRunPerParticleInitial(std::ofstream& file, const Task& task);
    void writeResultSpacepointsSingleEventPerformance(std::ofstream& file, const Task& task);
    
    static constexpr uint64_t NumThreads = 10;
    
    SolverTestParams params_;
    SplitterSettings splitterSettings_;
    Splitter splitter_;
    std::vector<std::unique_ptr<HelixSolver>> helixSolvers_;
    std::unique_ptr<Task> task_;
    std::unique_ptr<Result> result_;
    std::vector<std::unique_ptr<Event>> events_;
    std::vector<std::unique_ptr<Result>> results_;
    std::vector<std::unique_ptr<Task>> tasks_;
    std::unordered_map<uint32_t, uint64_t> eventIdParticleIdMap_;
    std::unordered_map<const Task*, uint32_t> taskToRunIdMap_;
};
