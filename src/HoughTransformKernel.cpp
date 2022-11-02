#include "HelixSolver/HoughTransformKernel.h"
#include "HelixSolver/AccumulatorHelper.h"

#include <algorithm>

namespace HelixSolver
{
    HoughTransformKernel::AccumulatorSection::AccumulatorSection(uint16_t width, uint16_t height, uint16_t xBegin, uint16_t yBegin)
    : width(width)
    , height(height)
    , xBegin(xBegin)
    , yBegin(yBegin) {}

    #ifdef DEBUG
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer,
                                            sycl::buffer<uint8_t, 1>& accumulatorBuffer) 
    : solutions(mapBuffer, h, sycl::write_only)
    , rs(rBuffer, h, sycl::read_only)
    , phis(phiBuffer, h, sycl::read_only)
    , accumulator(accumulatorBuffer, h, sycl::write_only) {}
    #else
    HoughTransformKernel::HoughTransformKernel(sycl::handler& h,
                                            sycl::buffer<SolutionCircle, 1>& mapBuffer,
                                            sycl::buffer<float, 1>& rBuffer,
                                            sycl::buffer<float, 1>& phiBuffer) 
    : solutions(mapBuffer, h, sycl::write_only)
    , rs(rBuffer, h, sycl::read_only)
    , phis(phiBuffer, h, sycl::read_only) {}
    #endif

    void HoughTransformKernel::fillAccumulator() const
    {
        constexpr uint8_t MAX_STUB_LIST_NUM = NUM_OF_LAYERS + 2;
        constexpr uint32_t MAX_STUB_LIST_ELEMENT_NUM = MAX_STUB_NUM * MAX_STUB_LIST_NUM;
        uint32_t stubListSizes[MAX_STUB_LIST_NUM];
        stubListSizes[0] = 0;
        uint32_t stubLists[MAX_STUB_LIST_ELEMENT_NUM];
        uint32_t stubsNum = rs.size();
        for(; stubListSizes[0] < stubsNum; ++stubListSizes[0])
        {
            stubLists[stubListSizes[0]] = stubListSizes[0];
        }

        // Cannot use std::stack
        constexpr uint8_t MAX_SECTIONS_HEIGHT = MAX_DIVISION_LEVEL * 4;
        AccumulatorSection sections[MAX_SECTIONS_HEIGHT];
        uint8_t sectionsHeight = 1;

        sections[0] = AccumulatorSection(ACC_WIDTH, ACC_HEIGHT, 0, 0);
        while (sectionsHeight)
        {
            fillAccumulatorSection(sections, sectionsHeight, stubLists, stubListSizes);
        }
    }

    void HoughTransformKernel::fillAccumulatorSection(AccumulatorSection* sections, uint8_t& sectionsHeight, uint32_t* stubLists, uint32_t* stubListSizes) const
    {
        sectionsHeight--;
        AccumulatorSection section = sections[sectionsHeight];

        uint8_t divisionLevel = MAX_DIVISION_LEVEL - static_cast<uint8_t>(round(log2(section.width > section.height ? section.width : section.height)));
        fillHits(stubLists, stubListSizes, divisionLevel, section);
        if (stubListSizes[divisionLevel + 1] - stubListSizes[divisionLevel] < THRESHOLD) return;
        
        if (section.width > 1)
        {
            uint32_t newWidth = section.width / 2;
            if(section.height > 1)
            {
                uint32_t newHeight = section.height / 2;
                sections[sectionsHeight] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin);
                sections[sectionsHeight + 1] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin + newHeight);
                sections[sectionsHeight + 2] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin);
                sections[sectionsHeight + 3] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin + newHeight);
                sectionsHeight += 4;
            }
            else
            {
                sections[sectionsHeight] = AccumulatorSection(newWidth, section.height, section.xBegin, section.yBegin);
                sections[sectionsHeight + 1] = AccumulatorSection(newWidth, section.height, section.xBegin + newWidth, section.yBegin);
                sectionsHeight += 2;
            }
        }
        else
        {
            if(section.height > 1)
            {
                uint32_t newHeight = section.height / 2;
                sections[sectionsHeight] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin);
                sections[sectionsHeight + 1] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin + newHeight);
                sectionsHeight += 2;   
            }
            else
            {
                accumulator[section.yBegin * ACC_WIDTH + section.xBegin] = stubListSizes[divisionLevel + 1] - stubListSizes[divisionLevel];
            }
        }
    }

    void HoughTransformKernel::fillHits(uint32_t* stubLists, uint32_t* stubListSizes, uint8_t divisionLevel, const AccumulatorSection& section) const
    {
        float x = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, section.xBegin);
        float y = linspaceElement(PHI_BEGIN, PHI_END, ACC_HEIGHT, section.yBegin);
        float xLeft = x - ACC_CELL_WIDTH * 0.5;
        float xRight = x + ACC_CELL_WIDTH * (section.width - 0.5);
        float yBottom = y - ACC_CELL_HEIGHT * 0.5;
        float yTop = y + ACC_CELL_HEIGHT * (section.height - 0.5);

        stubListSizes[divisionLevel + 1] = stubListSizes[divisionLevel];
        uint32_t startStubIndex = divisionLevel ? stubListSizes[divisionLevel - 1] : 0;
        for(uint32_t stubIndex = startStubIndex; stubIndex < stubListSizes[divisionLevel]; ++stubIndex)
        {
            float yLeft = -rs[stubLists[stubIndex]] * xLeft + phis[stubLists[stubIndex]];
            float yRight = -rs[stubLists[stubIndex]] * xRight + phis[stubLists[stubIndex]];

            if (yLeft >= yBottom && yRight <= yTop)
            {
                stubLists[stubListSizes[divisionLevel + 1]] = stubLists[stubIndex];
                stubListSizes[divisionLevel + 1]++;
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

    void HoughTransformKernel::operator()() const
    {
        for (uint32_t i = 0; i < ACC_SIZE; i++)
        {
            accumulator[i] = 0;
        }

        fillAccumulator();
        // This is not transfering solution. It's the last step in calculating it. Refactor
        transferSolutionToHostDevice();
    }

} // namespace HelixSolver
