#include "HelixSolver/HoughTransformKernel.h"
#include "HelixSolver/AccumulatorHelper.h"

#include <algorithm>

namespace HelixSolver
{
    HoughTransformKernel::AccumulatorSection::AccumulatorSection(uint8_t qOverPtGridDivisionLevel, uint8_t phiGridDivisionLevel, uint16_t qOverPtBeginIndex, uint16_t phiBeginIndex)
    : qOverPtGridDivisionLevel(qOverPtGridDivisionLevel)
    , phiGridDivisionLevel(phiGridDivisionLevel)
    , qOverPtBeginIndex(qOverPtBeginIndex)
    , phiBeginIndex(phiBeginIndex) {}



    #ifdef DEBUG
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer,
                                            sycl::buffer<uint8_t, 1>& accumulatorSumBuf) 
    : solutions(mapBuffer, h, sycl::write_only)
    , rs(rBuffer, h, sycl::read_only)
    , phis(phiBuffer, h, sycl::read_only)
    , accumulator(accumulatorSumBuf, h, sycl::write_only) {}
    #else
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer) 
    : solutions(mapBuffer, h, sycl::write_only)
    , rs(rBuffer, h, sycl::read_only)
    , phis(phiBuffer, h, sycl::read_only) {}
    #endif

    void HoughTransformKernel::fillAccumulator(float* rs, float* phis, uint8_t* accumulator) const
    {
        float cellWidth = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, 1) - linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, 0);
        uint32_t stubsNum = this->rs.size();

        for(uint32_t stubIndex = 0; stubIndex < stubsNum; stubIndex++)
        {
            // TODO: Check if x is q over pt. Rename x prefix in variable names to qOverPt if so.
            // TODO: Change iteration range so as to cut calculation not affecting cells in the accumulator (when phi not in [PHI_BEGIN, PHI_END])
            for(uint32_t xIndex = 0; xIndex < ACC_WIDTH; xIndex++)
            {
                float x = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, xIndex);
                float xLeft = x - cellWidth * 0.5;
                float xRight = x + cellWidth * 0.5;

                float phiTop = -rs[stubIndex] * xLeft + phis[stubIndex];
                float phiBottom = -rs[stubIndex] * xRight + phis[stubIndex];

                // * The if statement probably causes votes at the ends of phi dimension not to be counted. Commented lines are intended to fix the bug
                // TODO: Check if the comment above is true and switch to new solution
                uint32_t phiTopIndex = mapToBeanIndex(phiTop);
                uint32_t phiBottomIndex = mapToBeanIndex(phiBottom);
                if (phiBottomIndex < 0 || phiTopIndex >= ACC_HEIGHT) continue;
                // uint32_t phiTopIndex = phiTop < PHI_END ? mapToBeanIndex(phiTop) : ACC_HEIGHT - 1;
                // uint32_t phiBottomIndex = phiBottm > PHI_BEGIN ? mapToBeanIndex(phiBottom) : 0;

                for(uint32_t phiIndex = phiBottomIndex; phiIndex <= phiTopIndex; phiIndex++) accumulator[phiIndex * ACC_WIDTH + xIndex]++;
            }
        }
    }

    void HoughTransformKernel::fillAccumulatorAdaptive() const
    {
        // * ACC_WIDTH and ACC_HEIGHT must be a power of 2

        uint32_t stubIndexes[MAX_STUB_NUM * (NUM_OF_LAYERS + 2)];
        uint32_t stubCounts[NUM_OF_LAYERS + 2];
        stubCounts[0] = 0;
        uint32_t stubsNum = rs.size();
        for(; stubCounts[0] < stubsNum; ++stubCounts[0])
        {
            stubIndexes[stubCounts[0]] = stubCounts[0];
        }

        // Cannot use std::stack
        AccumulatorSection sectionsStack[ACCUMULATOR_SECTION_STACK_MAX_HEIGHT];
        uint8_t sectionsStackHeight = 0;
        sectionsStack[sectionsStackHeight] = AccumulatorSection(0, 0, 0, 0);
        sectionsStackHeight++;
        while (sectionsStackHeight)
        {
            fillAccumulatorSectionAdaptive(sectionsStack, sectionsStackHeight, stubIndexes, stubCounts);
        }
    }

    void HoughTransformKernel::fillAccumulatorSectionAdaptive(AccumulatorSection* sectionsStack, uint8_t& sectionsStackHeight, uint32_t* stubIndexes, uint32_t* stubCounts) const
    {
        // * ACC_WIDTH and ACC_HEIGHT must be a power of 2

        sectionsStackHeight--;
        AccumulatorSection section = sectionsStack[sectionsStackHeight];

        uint8_t divisionLevel = section.qOverPtGridDivisionLevel > section.phiGridDivisionLevel ? section.qOverPtGridDivisionLevel : section.phiGridDivisionLevel;
        fillHits(stubIndexes, stubCounts, divisionLevel, section);
        if (stubCounts[divisionLevel + 1] - stubCounts[divisionLevel] < THRESHOLD) return;
        

        bool divideQOverPt = section.qOverPtGridDivisionLevel < Q_OVER_PT_MAX_GRID_DIVISION_LEVEL;
        bool dividePhi = section.phiGridDivisionLevel < PHI_MAX_GRID_DIVISION_LEVEL;
        if (divideQOverPt)
        {
            if(dividePhi)
            {
                sectionsStack[sectionsStackHeight] = AccumulatorSection(section.qOverPtGridDivisionLevel + 1, section.phiGridDivisionLevel + 1, section.qOverPtBeginIndex, section.phiBeginIndex);
                sectionsStack[sectionsStackHeight + 1] = AccumulatorSection(section.qOverPtGridDivisionLevel + 1, section.phiGridDivisionLevel + 1, section.qOverPtBeginIndex, section.phiBeginIndex + pow(2, PHI_MAX_GRID_DIVISION_LEVEL - section.phiGridDivisionLevel - 1));
                sectionsStack[sectionsStackHeight + 2] = AccumulatorSection(section.qOverPtGridDivisionLevel + 1, section.phiGridDivisionLevel + 1, section.qOverPtBeginIndex + pow(2, Q_OVER_PT_MAX_GRID_DIVISION_LEVEL - section.qOverPtGridDivisionLevel - 1), section.phiBeginIndex);
                sectionsStack[sectionsStackHeight + 3] = AccumulatorSection(section.qOverPtGridDivisionLevel + 1, section.phiGridDivisionLevel + 1, section.qOverPtBeginIndex + pow(2, Q_OVER_PT_MAX_GRID_DIVISION_LEVEL - section.qOverPtGridDivisionLevel - 1), section.phiBeginIndex + pow(2, PHI_MAX_GRID_DIVISION_LEVEL - section.phiGridDivisionLevel - 1));
                sectionsStackHeight += 4;
            }
            else
            {
                sectionsStack[sectionsStackHeight] = AccumulatorSection(section.qOverPtGridDivisionLevel + 1, section.phiGridDivisionLevel, section.qOverPtBeginIndex, section.phiBeginIndex);
                sectionsStack[sectionsStackHeight + 1] = AccumulatorSection(section.qOverPtGridDivisionLevel + 1, section.phiGridDivisionLevel, section.qOverPtBeginIndex + pow(2, Q_OVER_PT_MAX_GRID_DIVISION_LEVEL - section.qOverPtGridDivisionLevel - 1), section.phiBeginIndex);
                sectionsStackHeight += 2;
            }
        }
        else
        {
            if(dividePhi)
            {
                sectionsStack[sectionsStackHeight] = AccumulatorSection(section.qOverPtGridDivisionLevel, section.phiGridDivisionLevel + 1, section.qOverPtBeginIndex, section.phiBeginIndex);
                sectionsStack[sectionsStackHeight + 1] = AccumulatorSection(section.qOverPtGridDivisionLevel, section.phiGridDivisionLevel + 1, section.qOverPtBeginIndex, section.phiBeginIndex + pow(2, PHI_MAX_GRID_DIVISION_LEVEL - section.phiGridDivisionLevel - 1));
                sectionsStackHeight += 2;   
            }
            else
            {
                accumulator[section.phiBeginIndex * ACC_WIDTH + section.qOverPtBeginIndex] = stubCounts[divisionLevel + 1] - stubCounts[divisionLevel];
            }
        }
    }

    void HoughTransformKernel::fillHits(uint32_t* stubIndexes, uint32_t* stubCounts, uint8_t divisionLevel, const AccumulatorSection& section) const
    {
        float qOverPt = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, section.qOverPtBeginIndex);
        float phi = linspaceElement(PHI_BEGIN, PHI_END, ACC_HEIGHT, section.phiBeginIndex);

        // TODO: Rename
        float numCellsQOverPt = pow(2, Q_OVER_PT_MAX_GRID_DIVISION_LEVEL - section.qOverPtGridDivisionLevel);
        float numCellsPhi = pow(2, PHI_MAX_GRID_DIVISION_LEVEL - section.phiGridDivisionLevel);

        float qOverPtLeft = qOverPt - ACC_CELL_WIDTH * 0.5;
        float qOverPtRight = qOverPt + ACC_CELL_WIDTH * (numCellsQOverPt - 0.5);
        float phiBottom = phi - ACC_CELL_HEIGHT * 0.5;
        float phiTop = phi + ACC_CELL_HEIGHT * (numCellsPhi - 0.5);

        stubCounts[divisionLevel + 1] = stubCounts[divisionLevel];
        uint32_t startStubIndex = divisionLevel ? stubCounts[divisionLevel - 1] : 0;
        for(uint32_t stubIndex = startStubIndex; stubIndex < stubCounts[divisionLevel]; ++stubIndex)
        {
            float phiLeft = -rs[stubIndexes[stubIndex]] * qOverPtLeft + phis[stubIndexes[stubIndex]];
            float phiRight = -rs[stubIndexes[stubIndex]] * qOverPtRight + phis[stubIndexes[stubIndex]];

            if (phiLeft >= phiBottom && phiRight <= phiTop)
            {
                stubIndexes[stubCounts[divisionLevel + 1]] = stubIndexes[stubIndex];
                stubCounts[divisionLevel + 1]++;
            }            
        }
    }

    void HoughTransformKernel::transferSolutionToHostDevice() const
    {
        #pragma unroll 16
        // [[intel::ivdep]]
        for (uint32_t i = 0; i < ACC_SIZE; ++i)
        {
            if (accumulator[i] > THRESHOLD)
            {
                uint32_t qOverPtIndex = i % ACC_WIDTH;
                uint32_t phiIndex = i / ACC_WIDTH;
                addSolutionCircle(qOverPtIndex, phiIndex);
            }
        }
    }

    void HoughTransformKernel::addSolutionCircle(uint32_t qOverPtIndex, uint32_t phiIndex) const
    {
        float qOverPt = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, qOverPtIndex);
        float phi_0 = linspaceElement(PHI_BEGIN, Q_OVER_P_END, ACC_HEIGHT, phiIndex);
        
        uint32_t index = phiIndex * ACC_WIDTH + qOverPtIndex;
        solutions[index].isValid = true;
        solutions[index].r = ((1 / qOverPt) / B) * 1000;
        solutions[index].phi = phi_0 + M_PI_2;
    }


    // TODO: Rename
    uint32_t HoughTransformKernel::mapToBeanIndex(float y)
    {
        constexpr float temp = (ACC_HEIGHT - 1) / (PHI_END - PHI_BEGIN);
        float x = temp * (y - PHI_BEGIN);
        return static_cast<uint32_t>(x + 0.5);
    }

    void HoughTransformKernel::operator()() const
    {
        // [[intel::numbanks(4)]]
        // uint8_t accumulator[ACC_SIZE] = {0};
        for (uint32_t i = 0; i < ACC_SIZE; i++)
        {
            accumulator[i] = 0;
        }

        // ? All the data is alredy in sycl buffers. What is the reason behind duplicating the data in transferDataToBoardMemory?
        // TODO: Check if data duplication is necessary. Skip if possible
        // fillAccumulator(rs, phis, accumulator);
        fillAccumulatorAdaptive();
        // This is not transfering solution. It's the last step in calculating it. Refactor
        transferSolutionToHostDevice();
    }

} // namespace HelixSolver
