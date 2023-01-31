#include <cmath>
#include <iostream>
#include "HelixSolver/AdaptiveHoughGpuKernel.h"
#include "HelixSolver/Debug.h"

namespace HelixSolver
{
    AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel( FloatBufferReadAccessor rs,
                                                    FloatBufferReadAccessor phis,
                                                    SolutionsWriteAccessor solutions)
    : rs(rs)
    , phis(phis)
    , solutions(solutions) {
        DEBUG( ".. AdaptiveHoughKernel instantiated with " << rs.size() << " measurements " );
    }

    void AdaptiveHoughGpuKernel::operator()(Index2D idx) const
    {
        DEBUG( " .. AdaptiveHoughKernel initiated for subregion " << idx[0] << " " << idx[1] );
        constexpr uint8_t MAX_STUB_LISTS_NUM = MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL + 2;
        constexpr uint32_t MAX_STUB_LIST_ELEMENTS_NUM = MAX_STUB_NUM * MAX_STUB_LISTS_NUM;
        constexpr uint16_t INITIAL_SECTION_WIDTH = ACC_WIDTH / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        constexpr uint16_t INITIAL_SECTION_HEIGHT = ACC_HEIGHT / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        const uint16_t xBegin = ACC_WIDTH * idx[0] / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        const uint16_t yBegin = ACC_HEIGHT * idx[1] / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        uint32_t stubListSizes[MAX_STUB_LISTS_NUM];
        for(uint32_t i = 0; i < MAX_STUB_LISTS_NUM; ++i) stubListSizes[i] = 0;

        uint32_t stubLists[MAX_STUB_LIST_ELEMENTS_NUM];
        const uint32_t stubsNum = rs.size();
        stubListSizes[0] = stubsNum;
        for(uint32_t i = 0; i < stubsNum; ++i) stubLists[i] = i;

        constexpr uint8_t MAX_SECTIONS_HEIGHT = (MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL) * 4;
        AccumulatorSection sections[MAX_SECTIONS_HEIGHT];
        uint8_t sectionsBufferSize = 1;
        sections[0] = AccumulatorSection(INITIAL_SECTION_WIDTH, INITIAL_SECTION_HEIGHT, xBegin, yBegin);
        while (sectionsBufferSize) fillAccumulatorSection(sections, sectionsBufferSize, stubLists, stubListSizes);
    }

    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection* sections, uint8_t& sectionsBufferSize, uint32_t* stubLists, uint32_t* stubListSizes) const
    {
        DEBUG("Regions buffer depth " << static_cast<int>(sectionsBufferSize));
        sectionsBufferSize--;
        const AccumulatorSection section = sections[sectionsBufferSize];

        const uint8_t divisionLevel = MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL - static_cast<uint8_t>(round(std::log2(section.width > section.height ? section.width : section.height)));
        fillHits(stubLists, stubListSizes, divisionLevel, section);
        if (stubListSizes[divisionLevel + 1] - stubListSizes[divisionLevel] < THRESHOLD) return;
        
        if (section.width > 1)
        {
            DEBUG("Splitting in 2D");
            const uint16_t newWidth = section.width / 2;
            if(section.height > 1)
            {
                const uint16_t newHeight = section.height / 2;
                sections[sectionsBufferSize] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin);
                sections[sectionsBufferSize + 1] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin + newHeight);
                sections[sectionsBufferSize + 2] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin);
                sections[sectionsBufferSize + 3] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin + newHeight);
                sectionsBufferSize += 4;
            }
            else
            {
                sections[sectionsBufferSize] = AccumulatorSection(newWidth, section.height, section.xBegin, section.yBegin);
                sections[sectionsBufferSize + 1] = AccumulatorSection(newWidth, section.height, section.xBegin + newWidth, section.yBegin);
                sectionsBufferSize += 2;
            }
        }
        else
        {
            DEBUG("Splitting in 1D");
            if(section.height > 1)
            {
                const uint16_t newHeight = section.height / 2;
                sections[sectionsBufferSize] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin);
                sections[sectionsBufferSize + 1] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin + newHeight);
                sectionsBufferSize += 2;   
            }
            else
            {
                addSolution(section.xBegin, section.yBegin);
            }
        }
    }

    void AdaptiveHoughGpuKernel::fillHits(uint32_t* stubLists, uint32_t* stubListSizes, uint8_t divisionLevel, const AccumulatorSection& section) const
    {
        const float x = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, section.xBegin);
        const float y = linspaceElement(PHI_BEGIN, PHI_END, ACC_HEIGHT, section.yBegin);
        const float xLeft = x - ACC_CELL_WIDTH * 0.5;
        const float xRight = x + ACC_CELL_WIDTH * (section.width - 0.5);
        const float yBottom = y - ACC_CELL_HEIGHT * 0.5;
        const float yTop = y + ACC_CELL_HEIGHT * (section.height - 0.5);

        stubListSizes[divisionLevel + 1] = stubListSizes[divisionLevel];
        const uint32_t startStubIndexInList = divisionLevel ? stubListSizes[divisionLevel - 1] : 0;
        for(uint32_t stubIndexInList = startStubIndexInList; stubIndexInList < stubListSizes[divisionLevel]; ++stubIndexInList)
        {
            const uint32_t stubIndex = stubLists[stubIndexInList];
            const float yLeft = -rs[stubIndex] * xLeft + phis[stubIndex];
            const float yRight = -rs[stubIndex] * xRight + phis[stubIndex];

            if (yLeft >= yBottom && yRight <= yTop)
            {
                stubLists[stubListSizes[divisionLevel + 1]] = stubIndex;
                stubListSizes[divisionLevel + 1]++;
            }            
        }
    }

    void AdaptiveHoughGpuKernel::addSolution(uint32_t qOverPtIndex, uint32_t phiIndex) const
    {
        const float qOverPt = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, qOverPtIndex);
        const float phi_0 = linspaceElement(PHI_BEGIN, Q_OVER_P_END, ACC_HEIGHT, phiIndex);
        
        const uint32_t index = phiIndex * ACC_WIDTH + qOverPtIndex;
        solutions[index].isValid = true;
        solutions[index].r = 1000 / (qOverPt * B);
        solutions[index].phi = phi_0 + M_PI_2;
        DEBUG( " .. AdaptiveHoughKernel solution q/pt:" << qOverPt << " phi: " << solutions[index].phi );
    }

    AdaptiveHoughGpuKernel::AccumulatorSection::AccumulatorSection(uint16_t width, uint16_t height, uint16_t xBegin, uint16_t yBegin)
    : width(width)
    , height(height)
    , xBegin(xBegin)
    , yBegin(yBegin) {}
} // namespace HelixSolver
