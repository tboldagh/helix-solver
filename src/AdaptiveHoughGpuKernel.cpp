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

        // uint32_t stubListSizes[MAX_STUB_LISTS_NUM];
        // for (uint32_t i = 0; i < MAX_STUB_LISTS_NUM; ++i)
        //     stubListSizes[i] = 0;

        // uint32_t stubLists[MAX_STUB_LIST_ELEMENTS_NUM];
        // const uint32_t stubsNum = rs.size();
        // stubListSizes[0] = stubsNum;
        // for (uint32_t i = 0; i < stubsNum; ++i)
        //     stubLists[i] = i;

//        constexpr uint8_t MAX_SECTIONS_SIZE = (MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL) * 4;
        // the size os somewhat arbitrary, for regular algorithm dividing into 4 sub-sections it defined by the depth allowed
        // but for more flexible algorithms that is less predictable
        // for now it is an arbitrary constant + checks that we stay within this limit
        constexpr uint8_t MAX_SECTIONS_SIZE = 100;
  
        AccumulatorSection sections[MAX_SECTIONS_SIZE]; // in here sections of image will be recorded
        uint8_t sectionsBufferSize = 1;
        const uint8_t initialDivisionLevel = 0;
        sections[0] = AccumulatorSection(INITIAL_SECTION_WIDTH, INITIAL_SECTION_HEIGHT, xBegin, yBegin, initialDivisionLevel);

        // uint32_t divisionLevelIterator[MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL + 4];
        // for (uint8_t i = 0; i<MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL; ++i)
        //     divisionLevelIterator[i] = 0;
        // divisionLevelIterator[0] = 1;

        // scan this region until there is no section to process (i.e. size, initially 1, becomes 0)
        while (sectionsBufferSize) {
            fillAccumulatorSection(sections, sectionsBufferSize);
        }
//            fillAccumulatorSection(sections, sectionsBufferSize, stubLists, stubListSizes, divisionLevel_smh, divisionLevelIterator);

    }

//    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection *sections, uint8_t &sectionsBufferSize, uint32_t *stubLists, uint32_t *stubListSizes, uint8_t &divisionLevel_smh, uint32_t *divisionLevelIterator) const
    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection *sections, uint8_t &sectionsBufferSize) const
    {
        DEBUG("Regions buffer depth " << static_cast<int>(sectionsBufferSize));
        // pop the region from the top of sections buffer
        sectionsBufferSize--;
        const AccumulatorSection section = sections[sectionsBufferSize];


        // while (divisionLevelIterator[divisionLevel_smh] == 0)
        //     divisionLevel_smh--;
        // divisionLevelIterator[divisionLevel_smh]--;

        // fillHits(stubLists, stubListSizes, divisionLevel_smh, section);
        const uint16_t count = countHits(THRESHOLD, section);
        if ( count < THRESHOLD )
            return;
        // if (stubListSizes[divisionLevel_smh + 1] - stubListSizes[divisionLevel_smh] < THRESHOLD)
        //     return;
        if ( section.xSize > ACC_WIDTH_PRECISION && section.ySize > ACC_HEIGHT_PRECISION) {
            // by the order here we steer the direction of the search of image space
            // it may be relevant depending on the data ordering
            sections[sectionsBufferSize]     = section.bottomLeft();
            sections[sectionsBufferSize + 1] = section.topLeft();
            sections[sectionsBufferSize + 2] = section.topRight();
            sections[sectionsBufferSize + 3] = section.bottomRight();
            sectionsBufferSize += 3;
        } else if ( section.xSize > ACC_WIDTH_PRECISION ) {
            sections[sectionsBufferSize]     = section.left();
            sections[sectionsBufferSize + 1] = section.right();
            sectionsBufferSize += 1;
        } else if ( section.ySize > ACC_WIDTH_PRECISION ) {
            sections[sectionsBufferSize]     = section.bottom();
            sections[sectionsBufferSize + 1] = section.top();
            sectionsBufferSize += 1;
        } else { // no more splitting
            addSolution(section);
        }
    
    }

    uint8_t AdaptiveHoughGpuKernel::countHits(const uint8_t max, const AccumulatorSection &section) const
    {
        const double xLeft = section.xBegin;
        const double xRight = xLeft + section.xSize;
        const double yBottom = section.yBegin;
        const double yTop = yBottom + section.ySize;
        uint16_t counter=0;
        DEBUG(xLeft<<","<<M_PI_2+yBottom<<","<<xRight<<","<<M_PI_2+yTop<<":BoxPosition");

        // need to understand this
        // stubListSizes[divisionLevel + 1] = stubListSizes[divisionLevel];
        // const uint32_t startStubIndexInList = divisionLevel ? stubListSizes[divisionLevel - 1] : 0;
        //        for (uint32_t stubIndexInList = startStubIndexInList; stubIndexInList < stubListSizes[divisionLevel]; ++stubIndexInList)
        // for (uint32_t stubIndexInList = startStubIndexInList; stubIndexInList < stubListSizes[divisionLevel] && count < max; ++stubIndexInList)

        // here we can improve by knowing over which stubs to iterate, this is related to geometry
        // this can be stored in section object, 
        // the index range smaller to be within the the range before division is simply not going to give much gain because stubs are in 2D

        const uint32_t maxIndex = rs.size();
        for (uint32_t index = 0; index < maxIndex && counter <= max; ++index)        
        {
            // const uint32_t stubIndex = stubLists[index];
            const float r = rs[index];
            const float phi = phis[index];
            const double yLeft = -r * xLeft + phi;
            const double yRight = -r * xRight + phi;

            DEBUG(yLeft<<","<<xLeft<<","<<yRight<<","<<xRight<<":LinePosition");

            if (yLeft >= yBottom && yRight <= yTop)
            {
                counter++;
                // stubLists[stubListSizes[divisionLevel + 1]] = stubIndex;
                // stubListSizes[divisionLevel + 1]++;
            }
        }
        return counter;
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

    void AdaptiveHoughGpuKernel::addSolution(const AccumulatorSection& section) const
    {
        const double qOverPt = section.yBegin + 0.5 * section.ySize;
        const double phi_0 = section.xBegin + 0.5 * section.xSize;
        // for truly adaptive algorithm the index can not be calculated in obvious way
        // therefore the kernel needs to find first available slot to store the solution
        // for now a stupid solution is to iterate over to the first free slot
        // this won't be that easy for truly parallel execution, 
        // we will likely need to deffer to sycl::atomic_ref compare_exchange_weak/strong
//        const uint32_t index = phiIndex / ACC_HEIGHT_PRECISION  + qOverPtIndex / ACC_WIDTH_PRECISION;
        const uint32_t solutionsBufferSize = solutions.size();
        for ( uint32_t index = 0; index < solutionsBufferSize; ++index ) {
            if ( solutions[index].phi == SolutionCircle::INVALID_PHI ) {
                solutions[index].pt = 1000 / (qOverPt * MagneticInduction);
                solutions[index].phi = phi_0;
                DEBUG("AdaptiveHoughKernel solution q/pt:" << qOverPt << " phi: " << phi);
                DEBUG(qOverPt<<","<<phi_0<<":SolutionPair");
                return;
            }
        }
        INFO("Could not find place for solution!!");
    }




} // namespace HelixSolver
