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
        const AccumulatorSection section = sections[sectionsBufferSize];

        const uint16_t count = countHits(THRESHOLD, section);
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

    uint8_t AdaptiveHoughGpuKernel::countHits(const uint8_t max, const AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;
        uint16_t counter=0;
        CDEBUG(DISPLAY_BOX_POSITION, section.xBegin<<","<<section.yBegin<<","<<xEnd<<","<<yEnd<<","<<section.divisionLevel<<":BoxPosition");

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        const uint32_t maxIndex = rs.size();
        for (uint32_t index = 0; index < maxIndex && counter <= max; ++index)
        {
            const float r = rs[index];
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            const double yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            const double yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);

            if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
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

        // the coordinates of the solution can be much improved too
        // e.g. using exact formula (i.e. no sin x = x approx), d0 fit & reevaluation,
        // additional hits from pixels inner layers,
        // TODO future work

        const uint32_t solutionsBufferSize = solutions.size();
        for ( uint32_t index = 0; index < solutionsBufferSize; ++index ) {
            if ( solutions[index].phi == SolutionCircle::INVALID_PHI ) {
                solutions[index].pt  = 1./qOverPt;
                solutions[index].phi = phi_0;
                CDEBUG(DISPLAY_BASIC, "AdaptiveHoughKernel solution q/pt:" << qOverPt << " phi: " << phi_0);
                CDEBUG(DISPLAY_SOLUTION_PAIR, qOverPt<<","<<phi_0<<","<<section.xBegin<<","<<section.yBegin<<","<<section.xBegin + section.xSize<<","<<section.yBegin + section.ySize<<","<<section.divisionLevel<<":SolutionPair");
                return;
            }
        }
        INFO("Could not find place for solution!!");
    }

} // namespace HelixSolver