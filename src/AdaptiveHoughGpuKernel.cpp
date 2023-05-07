#include <cmath>
#ifndef USE_SYCL
#include <iostream>
#endif
#include "HelixSolver/Debug.h"

#include "HelixSolver/AdaptiveHoughGpuKernel.h"

namespace HelixSolver
{
    AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel(FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, FloatBufferReadAccessor /*z*/, SolutionsWriteAccessor solutions) : rs(rs), phis(phis), solutions(solutions)
    {
        CDEBUG(DISPLAY_BASIC, ".. AdaptiveHoughKernel instantiated with " << rs.size() << " measurements ");
    }

    void AdaptiveHoughGpuKernel::operator()(Index2D idx) const
    {
        CDEBUG(DISPLAY_BASIC, " .. AdaptiveHoughKernel initiated for subregion " << idx[0] << " " << idx[1]);

        constexpr float INITIAL_X_SIZE = ACC_X_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        constexpr float INITIAL_Y_SIZE = ACC_Y_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        const double xBegin = PHI_BEGIN + INITIAL_X_SIZE * idx[0];
        const double yBegin = Q_OVER_PT_BEGIN + INITIAL_Y_SIZE * idx[1];
        CDEBUG(DISPLAY_BASIC, " .. AdaptiveHoughKernel region, x: " << xBegin << " xsz: " << INITIAL_X_SIZE
                                          << " y: " << yBegin << " ysz: " << INITIAL_Y_SIZE);

        // we need here a limited set of Points

        // the size os somewhat arbitrary, for regular algorithm dividing into 4 sub-sections it defined by the depth allowed
        // but for more flexible algorithms that is less predictable
        // for now it is an arbitrary constant + checks that we stay within this limit

        AccumulatorSection sections[MAX_SECTIONS_BUFFER_SIZE]; // in here sections of image will be recorded
        uint8_t sectionsBufferSize = 1;
        const uint8_t initialDivisionLevel = 0;
        sections[0] = AccumulatorSection(INITIAL_X_SIZE, INITIAL_Y_SIZE, xBegin, yBegin, initialDivisionLevel);

        // scan this region until there is no section to process (i.e. size, initially 1, becomes 0)
        while (sectionsBufferSize) {
            fillAccumulatorSection(sections, sectionsBufferSize);
        }

    }

    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection *sections, uint8_t &sectionsBufferSize) const
    {
        CDEBUG(DISPLAY_BASIC, "Regions buffer depth " << static_cast<int>(sectionsBufferSize));
        // pop the region from the top of sections buffer
        sectionsBufferSize--;
        AccumulatorSection section = sections[sectionsBufferSize]; // copy section, it will be modified, TODO consider not to copy

        const uint16_t count = countHits(section);
        CDEBUG(DISPLAY_BASIC, "count of lines in region x:" << section.xBegin
            << " xsz: " << section.xSize << " y: " << section.yBegin << " ysz: " << section.ySize << " divLevel: " << section.divisionLevel << " count: " << count);
        if ( count < THRESHOLD )
            return;

        if ( section.xSize > ACC_X_PRECISION && section.ySize > ACC_Y_PRECISION) {
            CDEBUG(DISPLAY_BASIC, "Splitting region into 4");
            // by the order here we steer the direction of the search of image space
            // it may be relevant depending on the data ordering??? to be testes
            sections[sectionsBufferSize]     = section.bottomLeft();
            sections[sectionsBufferSize + 1] = section.topLeft();
            sections[sectionsBufferSize + 2] = section.topRight();
            sections[sectionsBufferSize + 3] = section.bottomRight();
            ASSURE_THAT( sectionsBufferSize + 3 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in 4 subregions split)");
            sectionsBufferSize += 4;
        } else if ( section.xSize > ACC_X_PRECISION ) {
            CDEBUG(DISPLAY_BASIC, "Splitting region into 2 in x direction");
            sections[sectionsBufferSize]     = section.left();
            sections[sectionsBufferSize + 1] = section.right();
            ASSURE_THAT( sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in x split)");
            sectionsBufferSize += 2;
        } else if ( section.ySize > ACC_Y_PRECISION ) {
            CDEBUG(DISPLAY_BASIC, "Splitting region into 2 in y direction");
            sections[sectionsBufferSize]     = section.bottom();
            sections[sectionsBufferSize + 1] = section.top();
            ASSURE_THAT( sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in y split)");
            sectionsBufferSize += 2;
        } else { // no more splitting, we have a solution
            addSolution(section);
        }

    }

    uint8_t AdaptiveHoughGpuKernel::countHits(AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;

        uint16_t counter=0;
        CDEBUG(DISPLAY_BOX_POSITION, section.xBegin<<","<<section.yBegin<<","<<xEnd<<","<<yEnd<<","<<section.divisionLevel<<":BoxPosition");

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        // logic that will quickly check if there are any solutions in this section is as follows
        // we order the solutions at the left and right, i.e. if the order is the same it means no two lines cross
        // and the solution is not there        
        struct Sol { 
            uint32_t index;
            double value;   // TODO see is float is not enough
            bool operator<( const Sol& rhs) const { return value < rhs.value; }
        };
        Sol solAtBegin[MAX_COUNT_PER_SECTION];
        Sol solAtEnd[MAX_COUNT_PER_SECTION];

        const uint32_t maxIndex = rs.size();
        for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index)
        {
            const float r = rs[index];
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            // TODO see if float is not enough
            const double yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            const double yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);
            if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
            {
                section.indices[counter] = index;
                solAtBegin[counter] = {index, yLineAtBegin};
                solAtEnd[counter] = {index, yLineAtEnd};
                counter++;
            }
        }
        section.counts =  counter >= MAX_COUNT_PER_SECTION ? 0 : counter; // setting this counter to 0 == indices are invalid
        if ( section.counts == 0 )
            return counter;
        if ( DO_LINE_CROSS_FILTERING ) {
            // sort the solutions 
            // TODO verify if SYCL needs different code (likely so) 
            std::sort(std::begin(solAtBegin), std::end(solAtBegin));
            std::sort(std::begin(solAtEnd), std::end(solAtEnd));
            for ( uint16_t i =0 ; i < counter; ++i ) {
                if ( solAtBegin[i].index != solAtEnd[i].index ) {
                    DEBUG("AdaptiveHoughKernel Reordering at the ends, there is a solution in this section");
                    return counter;
                }
            }        
            DEBUG("AdaptiveHoughKernel Ordering at ends is the same, no solution in this section");
            return 0;
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

        // the coordinates of the solution can be much improved too
        // e.g. using exact formula (i.e. no sin x = x approx), d0 fit & reevaluation,
        // additional hits from pixels inner layers,
        // TODO future work

        const uint32_t solutionsBufferSize = solutions.size();
        for ( uint32_t index = 0; index < solutionsBufferSize; ++index ) {
            if ( solutions[index].phi == SolutionCircle::INVALID_PHI ) {      
                if ( section.canUseIndices()) {
                    fillPreciseSolution(section, solutions[index]);
                    CDEBUG(DISPLAY_BASIC, "AdaptiveHoughKernel solution count: " << int(section.counts));
                } // but for now it is always the simple one
                solutions[index].pt  = 1./qOverPt;
                solutions[index].phi = phi_0;

                CDEBUG(DISPLAY_BASIC, "AdaptiveHoughKernel solution q/pt:" << qOverPt << " phi: " << phi_0);
                CDEBUG(DISPLAY_SOLUTION_PAIR, qOverPt<<","<<phi_0<<","<<section.xBegin<<","<<section.yBegin<<","<<section.xBegin + section.xSize<<","<<section.yBegin + section.ySize<<","<<section.divisionLevel<<":SolutionPair");
                // TODO calculate remaining parameters, eta, z, d0                
                return;
            }
        }
        INFO("Could not find place for solution!!");
    }

    void  AdaptiveHoughGpuKernel::fillPreciseSolution(const AccumulatorSection& section, SolutionCircle& s) const {
        // TODO complete it
    }



} // namespace HelixSolver