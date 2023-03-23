#include <cmath>
#include <iostream>
#include "HelixSolver/AdaptiveHoughGpuKernel.h"
#include "HelixSolver/Debug.h"

namespace HelixSolver
{
    AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel(FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, SolutionsWriteAccessor solutions) : rs(rs), phis(phis), solutions(solutions)
    {
        DEBUG(".. AdaptiveHoughKernel instantiated with " << rs.size() << " measurements ");
    }

    void AdaptiveHoughGpuKernel::operator()(Index2D idx) const
    {
        DEBUG(" .. AdaptiveHoughKernel initiated for subregion " << idx[0] << " " << idx[1]);

        constexpr float INITIAL_SECTION_WIDTH = ACC_WIDTH / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        constexpr float INITIAL_SECTION_HEIGHT = ACC_HEIGHT / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        const double xBegin = Q_OVER_PT_BEGIN + INITIAL_SECTION_WIDTH * idx[0];
        const double yBegin = PHI_BEGIN + INITIAL_SECTION_HEIGHT * idx[1];

        uint32_t stubListSizes[MAX_STUB_LISTS_NUM];
        for (uint32_t i = 0; i < MAX_STUB_LISTS_NUM; ++i)
            stubListSizes[i] = 0;

        uint32_t stubLists[MAX_STUB_LIST_ELEMENTS_NUM];
        const uint32_t stubsNum = rs.size();
        stubListSizes[0] = stubsNum;
        for (uint32_t i = 0; i < stubsNum; ++i)
            stubLists[i] = i;

        constexpr uint8_t MAX_SECTIONS_HEIGHT = (MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL) * 4;
        AccumulatorSection sections[MAX_SECTIONS_HEIGHT];
        uint8_t sectionsBufferSize = 1;
        uint8_t divisionLevel_smh = 0;
        sections[0] = AccumulatorSection(INITIAL_SECTION_WIDTH, INITIAL_SECTION_HEIGHT, xBegin, yBegin);

        uint32_t divisionLevelIterator[MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL + 4];
        for (uint8_t i = 0; i<MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL; ++i)
            divisionLevelIterator[i] = 0;
        divisionLevelIterator[0] = 1;

        while (sectionsBufferSize)
            fillAccumulatorSection(sections, sectionsBufferSize, stubLists, stubListSizes, divisionLevel_smh, divisionLevelIterator);
    }

    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection *sections, uint8_t &sectionsBufferSize, uint32_t *stubLists, uint32_t *stubListSizes, uint8_t &divisionLevel_smh, uint32_t *divisionLevelIterator) const
    {
        DEBUG("Regions buffer depth " << static_cast<int>(sectionsBufferSize));
        sectionsBufferSize--;
        const AccumulatorSection section = sections[sectionsBufferSize];

        while (divisionLevelIterator[divisionLevel_smh] == 0)
            divisionLevel_smh--;
        divisionLevelIterator[divisionLevel_smh]--;

        fillHits(stubLists, stubListSizes, divisionLevel_smh, section);

        if (stubListSizes[divisionLevel_smh + 1] - stubListSizes[divisionLevel_smh] < THRESHOLD)
            return;

        if (section.width > ACC_WIDTH_PRECISION)
        {
            DEBUG("Splitting in 2D");
            const double newWidth = section.width / 2;
            if (section.height > ACC_HEIGHT_PRECISION)
            {
                const double newHeight = section.height / 2.0;
                divisionLevel_smh++;
                divisionLevelIterator[divisionLevel_smh] += 4;
                sections[sectionsBufferSize] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin);
                sections[sectionsBufferSize + 1] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin + newHeight);
                sections[sectionsBufferSize + 2] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin);
                sections[sectionsBufferSize + 3] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin + newHeight );
                sectionsBufferSize += 4;
            }
            else
            {
                divisionLevel_smh++;
                divisionLevelIterator[divisionLevel_smh] += 2;
                sections[sectionsBufferSize] = AccumulatorSection(newWidth, section.height, section.xBegin, section.yBegin);
                sections[sectionsBufferSize + 1] = AccumulatorSection(newWidth, section.height, section.xBegin + newWidth, section.yBegin);
                sectionsBufferSize += 2;
            }
        }
        else
        {
            DEBUG("Splitting in 1D");
            if (section.height > ACC_HEIGHT_PRECISION)
            {
                divisionLevel_smh++;
                divisionLevelIterator[divisionLevel_smh+1] += 2;
                const double newHeight = section.height / 2;
                sections[sectionsBufferSize] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin);
                sections[sectionsBufferSize + 1] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin + newHeight);
                sectionsBufferSize += 2;
            }
            else
            {
                addSolution(section.width, section.height, section.xBegin, section.yBegin);
            }
        }

    }

    void AdaptiveHoughGpuKernel::fillHits(uint32_t *stubLists, uint32_t *stubListSizes, uint8_t divisionLevel, const AccumulatorSection &section) const
    {
        const double xLeft = section.xBegin;
        const double xRight = xLeft + section.width;
        const double yBottom = section.yBegin;
        const double yTop = yBottom + section.height;

        std::cout<<xLeft<<","<<M_PI_2+yBottom<<","<<xRight<<","<<M_PI_2+yTop<<":BoxPosition"<<std::endl;

        stubListSizes[divisionLevel + 1] = stubListSizes[divisionLevel];
        const uint32_t startStubIndexInList = divisionLevel ? stubListSizes[divisionLevel - 1] : 0;

        for (uint32_t stubIndexInList = startStubIndexInList; stubIndexInList < stubListSizes[divisionLevel]; ++stubIndexInList)
        {
            const uint32_t stubIndex = stubLists[stubIndexInList];
            const double yLeft = -rs[stubIndex] * xLeft + phis[stubIndex];
            const double yRight = -rs[stubIndex] * xRight + phis[stubIndex];

            std::cout<<yLeft<<","<<xLeft<<","<<yRight<<","<<xRight<<":LinePosition"<<std::endl;

            if (yLeft >= yBottom && yRight <= yTop)
            {
                stubLists[stubListSizes[divisionLevel + 1]] = stubIndex;
                stubListSizes[divisionLevel + 1]++;
            }
        }

/*
        int32_t maxStubIndex = stubListSizes[divisionLevel];
        for (uint32_t stubIndexInList = startStubIndexInList; stubIndexInList < maxStubIndex; ++stubIndexInList)
        {
            for(uint32_t stubSubIndexInList = stubIndexInList + 1; stubSubIndexInList < maxStubIndex; ++stubSubIndexInList)
            {
                const uint32_t stubIndex  = stubLists[stubIndexInList];
                const uint32_t stubSubIndex = stubLists[stubSubIndexInList];

                float x_cross_point = (phis[stubIndex] - phis[stubSubIndex])/(rs[stubIndex] - rs[stubSubIndex]);
                float y_cross_point = - rs[stubIndex] * x_cross_point + phis[stubIndex];

                if((x_cross_point > xLeft && x_cross_point < xRight) && (y_cross_point > yBottom && y_cross_point < yTop))
                {
                    stubLists[stubListSizes[divisionLevel + 1]] = stubIndex;
                    stubListSizes[divisionLevel + 1]++;
                }
            }
        }
*/
    }

    void AdaptiveHoughGpuKernel::addSolution(double qOverPt_width, double phi_height, double qOverPtIndex, double phiIndex) const
    {
        const double qOverPt = qOverPtIndex + 0.5 * qOverPt_width;
        const double phi_0 = phiIndex + 0.5 * phi_height;

        const uint32_t index = phiIndex / ACC_HEIGHT_PRECISION  + qOverPtIndex / ACC_WIDTH_PRECISION;
        solutions[index].isValid = true;
        solutions[index].r = 1000 / (qOverPt * MagneticInduction);
        solutions[index].phi = phi_0 + M_PI_2;
        DEBUG(" .. AdaptiveHoughKernel solution q/pt:" << qOverPt << " phi: " << solutions[index].phi);
        std::cout<<qOverPt<<","<<phi_0 + M_PI_2<<":SolutionPair"<<std::endl;
    }

    AdaptiveHoughGpuKernel::AccumulatorSection::AccumulatorSection(double width, double height, double xBegin, double yBegin)
        : width(width), height(height), xBegin(xBegin), yBegin(yBegin) {}

} // namespace HelixSolver
