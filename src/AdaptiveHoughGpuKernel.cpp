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

        constexpr float INITIAL_X_SIZE = ACC_X_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        constexpr float INITIAL_Y_SIZE = ACC_Y_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        const double xBegin = PHI_BEGIN + INITIAL_X_SIZE * idx[0];
        const double yBegin = Q_OVER_PT_BEGIN + INITIAL_Y_SIZE * idx[1];
        DEBUG(" .. AdaptiveHoughKernel region, x: " << xBegin << " xsz: " << INITIAL_X_SIZE 
                                          << " y: " << yBegin << " ysz: " << INITIAL_Y_SIZE);

        // we need here a limited set of stubs

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

        AccumulatorSection sections[MAX_SECTIONS_BUFFER_SIZE]; // in here sections of image will be recorded
        uint8_t sectionsBufferSize = 1;
        const uint8_t initialDivisionLevel = 0;
        sections[0] = AccumulatorSection(INITIAL_X_SIZE, INITIAL_Y_SIZE, xBegin, yBegin, initialDivisionLevel);

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
        DEBUG("count of lines in region x:" << section.xBegin
            << " xsz: " << section.xSize << " y: " << section.yBegin << " ysz: " << section.ySize << " divLevel: " << section.divisionLevel << " count: " << count);
        if ( count < THRESHOLD )
            return;
        // if (stubListSizes[divisionLevel_smh + 1] - stubListSizes[divisionLevel_smh] < THRESHOLD)
        //     return;
        if ( section.xSize > ACC_X_PRECISION && section.ySize > ACC_Y_PRECISION) {
            DEBUG("Splitting region into 4");
            // by the order here we steer the direction of the search of image space
            // it may be relevant depending on the data ordering
            sections[sectionsBufferSize]     = section.bottomLeft();
            sections[sectionsBufferSize + 1] = section.topLeft();
            sections[sectionsBufferSize + 2] = section.topRight();
            sections[sectionsBufferSize + 3] = section.bottomRight();
            ASSURE_THAT( sectionsBufferSize + 3 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in 4 subregions split)");
            sectionsBufferSize += 4;            
        } else if ( section.xSize > ACC_X_PRECISION ) {
            DEBUG("Splitting region into 2 in x direction");
            sections[sectionsBufferSize]     = section.left();
            sections[sectionsBufferSize + 1] = section.right();
            ASSURE_THAT( sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in x split)");
            sectionsBufferSize += 2;
        } else if ( section.ySize > ACC_Y_PRECISION ) {
            DEBUG("Splitting region into 2 in y direction");
            sections[sectionsBufferSize]     = section.bottom();
            sections[sectionsBufferSize + 1] = section.top();
            ASSURE_THAT( sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in y split)");
            sectionsBufferSize += 2;
        } else { // no more splitting
            addSolution(section);
        }
    
    }

    uint8_t AdaptiveHoughGpuKernel::countHits(const uint8_t max, const AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;
        uint16_t counter=0;
        DEBUG(section.xBegin<<","<<section.yBegin<<","<<xEnd<<","<<yEnd<<":BoxPosition");

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
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            const double yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            const double yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);
            DEBUG(r << ", " << inverse_r << ", " << phi << ":RInvRPhi" );

            DEBUG(section.xBegin<<","<<yLineAtBegin<<","<<xEnd<<","<<yLineAtEnd<<":LinePosition");

            if (yLineAtBegin <= section.yBegin && yEnd <= yLineAtEnd)
            {
                counter++;
            }
        }
        return counter;
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
        // or have the first pass over to calculate number of solutions
//        const uint32_t index = phiIndex / ACC_HEIGHT_PRECISION  + qOverPtIndex / ACC_WIDTH_PRECISION;
        const uint32_t solutionsBufferSize = solutions.size();
        for ( uint32_t index = 0; index < solutionsBufferSize; ++index ) {
            if ( solutions[index].phi == SolutionCircle::INVALID_PHI ) {
                solutions[index].pt  = 1./qOverPt;
                solutions[index].phi = phi_0;
                DEBUG("AdaptiveHoughKernel solution q/pt:" << qOverPt << " phi: " << phi_0);
                DEBUG(qOverPt<<","<<phi_0<<":SolutionPair");
                return;
            }
        }
        INFO("Could not find place for solution!!");
    }




} // namespace HelixSolver
