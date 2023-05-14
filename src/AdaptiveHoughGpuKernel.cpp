#include <cmath>
#ifndef USE_SYCL
#include <iostream>
#endif
#include "HelixSolver/Debug.h"

#include "HelixSolver/ZPhiPartitioning.h"
#include "HelixSolver/AdaptiveHoughGpuKernel.h"


namespace HelixSolver
{
    AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel(FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, FloatBufferReadAccessor zs, SolutionsWriteAccessor solutions) 
        : rs(rs), phis(phis), zs(zs), solutions(solutions)
    {
        CDEBUG(DISPLAY_BASIC, ".. AdaptiveHoughKernel instantiated with " << rs.size() << " measurements ");
    }

    void AdaptiveHoughGpuKernel::operator()(Index2D idx) const
    {
        CDEBUG(DISPLAY_BASIC, " .. AdaptiveHoughKernel initiated for subregion " << idx[0] << " " << idx[1]);

        const uint16_t phiRegionIndex = idx[0]; 
        // TODO test if we need to widen phi a bit
        const Reg phiRegion = region(-3.1415, 3.1415, phiRegionIndex, REGIONS_IN_PHI);
        ASSURE_THAT(phiRegionIndex < REGIONS_IN_PHI, "overrun in phi index, likely confused dimensions");
        DEBUG("... phi Region " << phiRegion.center << " +- " << phiRegion.width);

        const uint16_t etaRegionIndex = idx[1];        
        const Reg etaRegion = region(-3, 3, etaRegionIndex, REGIONS_IN_ETA);
        ASSURE_THAT(etaRegionIndex < REGIONS_IN_ETA, "overrun in eta index, likely confused dimensions");
        DEBUG("... eta Region " << etaRegion.center << " +- " << etaRegion.width);
        Wedge region( phiRegion, Reg({-20, 20}  ), etaRegion); // +- 20 mm along z        
        RegionData regionData;
        {
            uint16_t count = 0;
            for ( uint16_t i = 0; i < rs.size(); ++i ) {
                ASSURE_THAT(count < MAX_SP_PER_REGION, "Region memory not sufficient for SPs");
                if ( region.in_wedge_r_phi_z( rs[i], phis[i], zs[i]) ) {
                    regionData.one_over_RA[count] = INVERSE_A/(rs[i]);
                    regionData.phi[count] = phis[i];
                    count++;     
                }
            }
            regionData.spInRegion = count;
        }
        DEBUG("Count of SP per region: " << regionData.spInRegion << " all SP " << rs.size() 
                << " fraction:" << static_cast<double>(regionData.spInRegion)/rs.size());

        // TODO rework splitting into pt (likely eliminate further splitting in phi)                
        // constexpr float INITIAL_X_SIZE = ACC_X_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        // constexpr float INITIAL_Y_SIZE = ACC_Y_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        constexpr float INITIAL_X_SIZE = ACC_X_SIZE;
        constexpr float INITIAL_Y_SIZE = ACC_Y_SIZE;

        const double xBegin = PHI_BEGIN + INITIAL_X_SIZE * idx[0];
        const double yBegin = Q_OVER_PT_BEGIN + INITIAL_Y_SIZE * idx[1];
        CDEBUG(DISPLAY_BASIC, " .. AdaptiveHoughKernel region, x: " << xBegin << " xsz: " << INITIAL_X_SIZE
                                          << " y: " << yBegin << " ysz: " << INITIAL_Y_SIZE);


        // the size os somewhat arbitrary, for regular algorithm dividing into 4 sub-sections it defined by the depth allowed
        // but for more flexible algorithms that is less predictable
        // for now it is an arbitrary constant + checks that we stay within this limit

        AccumulatorSection sections[MAX_SECTIONS_BUFFER_SIZE]; // in here sections of image will be recorded
        uint8_t sectionsBufferSize = 1;
        const uint8_t initialDivisionLevel = 0;
        sections[0] = AccumulatorSection(INITIAL_X_SIZE, INITIAL_Y_SIZE, xBegin, yBegin, initialDivisionLevel);

        // scan this region until there is no section to process (i.e. size, initially 1, becomes 0)
        while (sectionsBufferSize) {
            fillAccumulatorSection(regionData, sections, sectionsBufferSize);
        }

    }

    void AdaptiveHoughGpuKernel::fillAccumulatorSection( const RegionData & data, AccumulatorSection *sections, uint8_t &sectionsBufferSize) const
    {
        CDEBUG(DISPLAY_BASIC, "Regions buffer depth " << static_cast<int>(sectionsBufferSize));
        // pop the region from the top of sections buffer
        sectionsBufferSize--;
        AccumulatorSection section = sections[sectionsBufferSize]; // copy section, it will be modified, TODO consider not to copy

        const uint16_t count = countHits(data, section);
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
            addSolution(data, section);
        }

    }

    uint8_t AdaptiveHoughGpuKernel::countHits(const RegionData& data, AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;
        uint16_t counter=0;
        CDEBUG(DISPLAY_BOX_POSITION, section.xBegin<<","<<section.yBegin<<","<<xEnd<<","<<yEnd<<","<<section.divisionLevel<<":BoxPosition");

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        const uint16_t maxIndex = data.spInRegion;
        for (uint16_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index)
        {
            const float inverse_r = data.one_over_RA[index];
            const float phi = data.phi[index];
            const double yLineAtBegin  = inverse_r * (section.xBegin - phi);
            const double yLineAtEnd    = inverse_r * (xEnd - phi);

            if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
            {
                section.indices[counter] = index;
                counter++;
            }
        }
        section.counts =  counter + 1 == MAX_COUNT_PER_SECTION ? 0 : counter; // setting this counter to 0 == indices are invalid
        return counter;
    }

    void AdaptiveHoughGpuKernel::addSolution(const RegionData& data, const AccumulatorSection& section) const
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
                    fillPreciseSolution(data, section, solutions[index]);
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

    void  AdaptiveHoughGpuKernel::fillPreciseSolution(const RegionData& data, const AccumulatorSection& section, SolutionCircle& s) const {
        // TODO complete it
    }



} // namespace HelixSolver