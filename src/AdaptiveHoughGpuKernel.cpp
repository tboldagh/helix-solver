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
        std::cout << "&ADAPTIVE_KERNEL_INITIAL_DIVISIONS" << ACC_X_SIZE <<std::endl;

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
        CDEBUG(DISPLAY_BOX_POSITION, section.xBegin<<","<<section.yBegin<<","<<section.xBegin + section.xSize<<","<<section.yBegin + section.ySize<<","<<section.divisionLevel<<":BoxPosition");

        // neccesary when applying pt_precision as exit-loop condition
        //if (std::fabs(1./section.yBegin) > MAX_PT || std::fabs(1./section.yBegin + section.ySize) > MAX_PT)
        //    return;

        // if we are sufficently far in division algorithm, cells can be rejected based also on the condition
        // none of the lines intersects within the cell boundaries,
        // countHits_checkOrder checks that condition
        uint16_t count{};
        if (section.divisionLevel < THRESHOLD_DIVISION_LEVEL_COUNT_HITS_CHECK_ORDER){
            count = countHits(section);
        } else count = countHits_checkOrder(section);

        CDEBUG(DISPLAY_BASIC, "count of lines in region x:" << section.xBegin
            << " xsz: " << section.xSize << " y: " << section.yBegin << " ysz: " << section.ySize << " divLevel: " << section.divisionLevel << " count: " << count);
        if ( count < THRESHOLD )
            return;

        // can be used to determine exit-loop condition based on pt_precision ->
        // solves the problem of strongly quantized pt for higher values of pt
        double section_pt_precision{};
        /*
        if (section.yBegin == 0){
            //section_pt_precision = 2 * (1./(std::fabs(section.yBegin) + 0.5 * section.ySize) - 1./(section.yBegin + section.ySize));
            section_pt_precision = 100;
        } else if ((section.yBegin + section.ySize) == 0){
            //section_pt_precision = 2 * (1./std::fabs(section.yBegin) - 1./(std::fabs(section.yBegin) + 0.5 * section.ySize));
            section_pt_precision = 100;
        } else {
            section_pt_precision = 1./std::fabs(section.yBegin) - 1./(std::fabs(section.yBegin) + section.ySize);
        }
        */

       if (!TO_DISPLAY_PRECISION_PAIR_ONCE){     // so that these values are displayed only once
            DEBUG("AdaptiveHoughGpuKernel.cpp: ACC_X_PRECISION = " << ACC_X_PRECISION << ", ACC_PT_PRECISION = " << ACC_X_PRECISION);
            DEBUG("AdaptiveHoughGpuKernel.cpp: &ACC_X_PRECISION = " << &ACC_X_PRECISION << ", &ACC_PT_PRECISION = " << &ACC_PT_PRECISION);
       }
       ++TO_DISPLAY_PRECISION_PAIR_ONCE;

        section_pt_precision = section.xSize;
        if ( section.xSize > ACC_X_PRECISION && section_pt_precision > ACC_PT_PRECISION) {
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
        } else if ( section_pt_precision > ACC_PT_PRECISION ) {
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

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        const uint32_t maxIndex = rs.size();
        for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index)
        {
            const float r = rs[index];
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            double yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            double yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);

            const double y_for_x_min_acc = inverse_r * INVERSE_A * (PHI_BEGIN - phi);
            const double y_for_x_max_acc = inverse_r * INVERSE_A * (PHI_END - phi);

            if (y_for_x_min_acc <= Q_OVER_PT_BEGIN && y_for_x_max_acc >= Q_OVER_PT_END){

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }
            } else if (y_for_x_min_acc > Q_OVER_PT_BEGIN && y_for_x_min_acc < Q_OVER_PT_END){

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }

                yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi) - 2 * M_PI * inverse_r * INVERSE_A;
                yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi) - 2 * M_PI * inverse_r * INVERSE_A;

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }
            } else if (y_for_x_max_acc < Q_OVER_PT_END && y_for_x_max_acc > Q_OVER_PT_BEGIN) {

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }

                yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi) + 2 * M_PI * inverse_r * INVERSE_A;
                yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi) + 2 * M_PI * inverse_r * INVERSE_A;

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }
            }
        }
        section.counts =  counter + 1 == section.OUT_OF_RANGE_COUNTS ? 0 : counter; // setting this counter to 0 == indices are invalid
        return counter;
    }


        uint8_t AdaptiveHoughGpuKernel::countHits_checkOrder(AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;
        uint16_t counter=0;
        CDEBUG(DISPLAY_BOX_POSITION, section.xBegin<<","<<section.yBegin<<","<<xEnd<<","<<yEnd<<","<<section.divisionLevel<<":BoxPosition");

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        const uint32_t maxIndex = rs.size();
        // 1st value will be coordinate of intersection point, the other is r of a given line
        std::vector<std::pair<float, uint16_t>> cell_intersection_before;
        std::vector<std::pair<float, uint16_t>> cell_intersection_after;

        for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index)
        {
            const float r = rs[index];
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            const double yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            const double yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);

            if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
            {
                section.indices[counter] = index;
                counter++;

                if (yLineAtBegin <= yEnd && yLineAtBegin >= section.yBegin){

                cell_intersection_before.push_back(std::make_pair(1 + (yLineAtBegin - section.yBegin)/section.ySize, counter));
                    } else {

                        float xLineAtBegin = phi + section.yBegin / (inverse_r * INVERSE_A);
                        cell_intersection_before.push_back(std::make_pair((section.xBegin + section.xSize - xLineAtBegin)/section.xSize, counter));
                }

                if (yLineAtEnd <= yEnd && yLineAtBegin >= section.yBegin){

                    cell_intersection_after.push_back(std::make_pair((yLineAtBegin - section.yBegin)/section.ySize, counter));
                } else {

                    float xLineAtEnd = phi + (section.yBegin + section.ySize) / (inverse_r * INVERSE_A);
                    cell_intersection_after.push_back(std::make_pair(1 + (section.xBegin + section.xSize - xLineAtEnd)/section.xSize, counter));
                }
            };
        }

        if (counter < THRESHOLD){
            return 0;
        }

        // the lines of code below check whether order of lines at the left and bottom boundary of cells is the same
        // as for right and upper boundary of the cells, if they are not, the cell is rejected
        sort(cell_intersection_before.begin(), cell_intersection_before.end());
        sort(cell_intersection_after.begin(), cell_intersection_after.end());
        //std::cout<<"here it also worker"<<std::endl;
        //std::cout<<"\n"<<std::endl;
        int8_t count_changes{};
        for(int8_t intersection_index; intersection_index < counter; ++intersection_index){
            if ((cell_intersection_before.at(intersection_index)).second != (cell_intersection_after.at(intersection_index)).second){
            //    std::cout<< "Additional condition worked" << std::endl;

                ++count_changes;

                if (count_changes > 5){
                    section.counts = counter;
                    return counter;
                }

            }
            //std::cout << int(intersection_index) << " " << (cell_intersection_before.at(intersection_index)).first << " " << (cell_intersection_before.at(intersection_index)).second << " " << (cell_intersection_after.at(intersection_index)).first << " " << (cell_intersection_after.at(intersection_index)).second << std::endl;
        }

        return 0;

        //if (counter + 1 == MAX_COUNT_PER_SECTION){
        //    std::runtime_error("Hits count in a cell is greater than size of an array!");
        //}

        section.counts =  counter + 1 == MAX_COUNT_PER_SECTION ? section.OUT_OF_RANGE_COUNTS : counter; // setting this counter to 0 == indices are invalid

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
        ASSURE_THAT(false, "Could not find place for solution!!");
    }

    void  AdaptiveHoughGpuKernel::fillPreciseSolution(const AccumulatorSection& section, SolutionCircle& s) const {
        // TODO complete it
    }



} // namespace HelixSolver