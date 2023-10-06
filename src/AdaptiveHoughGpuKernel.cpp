#include <cmath>
#ifndef USE_SYCL
#include <iostream>
#endif
#include "HelixSolver/Debug.h"

#include "HelixSolver/AdaptiveHoughGpuKernel.h"



namespace HelixSolver
{
    AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel(OptionsAccessor o, FloatBufferReadAccessor rs, FloatBufferReadAccessor phis, FloatBufferReadAccessor /*z*/, SolutionsWriteAccessor solutions) : opts(o), rs(rs), phis(phis), solutions(solutions)
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
        CDEBUG(DISPLAY_BOX_POSITION, section.xBegin<<","<<section.yBegin<<","<<section.xBegin + section.xSize<<","<<section.yBegin + section.ySize<<","<<section.divisionLevel<<":BoxPosition");

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

    //    if (!TO_DISPLAY_PRECISION_PAIR_ONCE){     // so that these values are displayed only once
    //         DEBUG("AdaptiveHoughGpuKernel.cpp: ACC_X_PRECISION = " << opt.ACC_X_PRECISION << ", ACC_PT_PRECISION = " << opt.ACC_PT_PRECISION);
    //    }
    //    ++TO_DISPLAY_PRECISION_PAIR_ONCE;
        HelixSolver::Options opt = opts[0];
        if ( section.xSize > opt.ACC_X_PRECISION && section.ySize > opt.ACC_PT_PRECISION) {
            CDEBUG(DISPLAY_BASIC, "Splitting region into 4");
            // by the order here we steer the direction of the search of image space
            // it may be relevant depending on the data ordering??? to be testes
            sections[sectionsBufferSize]     = section.bottomLeft();
            sections[sectionsBufferSize + 1] = section.topLeft();
            sections[sectionsBufferSize + 2] = section.topRight();
            sections[sectionsBufferSize + 3] = section.bottomRight();
            ASSURE_THAT( sectionsBufferSize + 3 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in 4 subregions split)");
            sectionsBufferSize += 4;
        } else if ( section.xSize > opt.ACC_X_PRECISION ) {
            CDEBUG(DISPLAY_BASIC, "Splitting region into 2 in x direction");
            sections[sectionsBufferSize]     = section.left();
            sections[sectionsBufferSize + 1] = section.right();
            ASSURE_THAT( sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in x split)");
            sectionsBufferSize += 2;
        } else if ( section.ySize > opt.ACC_PT_PRECISION ) {
            CDEBUG(DISPLAY_BASIC, "Splitting region into 2 in y direction");
            sections[sectionsBufferSize]     = section.bottom();
            sections[sectionsBufferSize + 1] = section.top();
            ASSURE_THAT( sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE, "Sections buffer depth to small (in y split)");
            sectionsBufferSize += 2;
        } else { // no more splitting, we have a solution
            //if (isSolutionWithinCell(section)){
                addSolution(section);
            //}
        }
    }

    uint8_t AdaptiveHoughGpuKernel::countHits(AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;
        uint16_t counter = 0;

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        const uint32_t maxIndex = rs.size();
        for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index)
        {
            const float r = rs[index];
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            float yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            float yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);

            // The code below allows to extend lines which intersect lew or right limit of the accumulator to the other side
            // This maneuver allows to significantly increase reconsruction efficiency for phi \approx +-pi

            if (lineInsideAccumulator(inverse_r, phi)){

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }
            } else {

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }

                yLineAtBegin = yLineAtBegin_modify(inverse_r, phi, section);
                yLineAtEnd   = yLineAtEnd_modify(inverse_r, phi, section);

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }
            }
        }

        section.counts =  counter + 1 > MAX_COUNT_PER_SECTION ? section.OUT_OF_RANGE_COUNTS : counter; // setting this counter to 0 == indices are invalid
        return counter;
    }

    uint8_t AdaptiveHoughGpuKernel::countHits_checkOrder(AccumulatorSection &section) const
    {
        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;
        uint16_t counter = 0;

        // here we can improve by knowing over which Points to iterate (i.e. indices of measurements), this is related to geometry
        // this can be stored in section object maybe???

        const uint32_t maxIndex = rs.size();
        // 1st value is coordinate of intersection point, the other is r of the given line
        std::vector<std::pair<float, uint16_t>> cell_intersection_before;
        std::vector<std::pair<float, uint16_t>> cell_intersection_after;

        for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index)
        {
            const float r = rs[index];
            const float inverse_r = 1.0/r;
            const float phi = phis[index];
            double yLineAtBegin  = inverse_r * INVERSE_A * (section.xBegin - phi);
            double yLineAtEnd    = inverse_r * INVERSE_A * (xEnd - phi);

            if (lineInsideAccumulator(inverse_r, phi)){

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }

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
            } else {

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }

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

                yLineAtBegin = yLineAtBegin_modify(inverse_r, phi, section);
                yLineAtEnd   = yLineAtEnd_modify(inverse_r, phi, section);

                if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd)
                {
                    section.indices[counter] = index;
                    counter++;
                }
            }
        }

        if (counter < THRESHOLD){
            return 0;
        }

        // the lines of code below check whether order of lines at the left and bottom boundary of cells is the same
        // as for right and upper boundary of the cells
        sort(cell_intersection_before.begin(), cell_intersection_before.end());
        sort(cell_intersection_after.begin(), cell_intersection_after.end());

        int8_t count_changes{};
        for(int8_t intersection_index; intersection_index < counter; ++intersection_index){
            if ((cell_intersection_before.at(intersection_index)).second != (cell_intersection_after.at(intersection_index)).second){

                ++count_changes;
            }
        }

        // we want to reject a cell if not enough intersections were found -> lines only/almost only pararell -> fake solutions
        if (count_changes < MIN_COUNT_CHANGES){

            section.counts = 0;
            return 0;
        }

        // Next step of solutions number minimization - checking phi of each line
        // Each helix is almost a straight line so we expect very small deviation from the truth value. Outliner in terms of
        // phi indicates that this point does not belong to analyzed track
        // We want to reduce number of counter by number of lines with phi differing from mean by more than n standard deviations

        // calculate mean and standard deviation of phi
        float mean_phi{};
        float stdev_phi{};
        for (uint8_t index = 0; index < counter; ++index){

            uint32_t line_index = section.indices[index];

            mean_phi += phis[line_index];
            stdev_phi += (phis[line_index]) * (phis[line_index]);
        }
        mean_phi /= counter;

        stdev_phi /= counter;
        stdev_phi -= mean_phi * mean_phi;
        stdev_phi = std::sqrt(stdev_phi);

        for (uint8_t index = 0; index < counter; ++index){

            uint32_t line_index = section.indices[index];
            float lower_phi_limit = mean_phi - N_SIGMA_PHI * stdev_phi;
            float upper_phi_limit = mean_phi + N_SIGMA_PHI * stdev_phi;

            if (phis[line_index] < lower_phi_limit || phis[line_index] > upper_phi_limit){
                //--counter;
            }
        }

        section.counts =  counter + 1 == MAX_COUNT_PER_SECTION ? section.OUT_OF_RANGE_COUNTS : counter;
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

    void  AdaptiveHoughGpuKernel::fillPreciseSolution(const AccumulatorSection& section, SolutionCircle& s) const
    {
        // TODO complete it
    }

    bool AdaptiveHoughGpuKernel::lineInsideAccumulator(const float radius_inverse, const float phi) const
    {

        const double qOverPt_for_PHI_BEGIN = radius_inverse * INVERSE_A * (PHI_BEGIN - phi);
        const double qOverPt_for_PHI_END   = radius_inverse * INVERSE_A * (PHI_END - phi);

        if (qOverPt_for_PHI_BEGIN <= Q_OVER_PT_BEGIN && qOverPt_for_PHI_END >= Q_OVER_PT_END){
            return true;
        } else return false;
    }

    float AdaptiveHoughGpuKernel::yLineAtBegin_modify(const float radius_inverse, const float phi, const AccumulatorSection& section) const
    {

        const double qOverPt_for_PHI_BEGIN = radius_inverse * INVERSE_A * (PHI_BEGIN - phi);
        const double qOverPt_for_PHI_END   = radius_inverse * INVERSE_A * (PHI_END - phi);

        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;

         if (qOverPt_for_PHI_BEGIN > Q_OVER_PT_BEGIN && qOverPt_for_PHI_BEGIN < Q_OVER_PT_END){

            return radius_inverse * INVERSE_A * (section.xBegin - phi) - 2 * M_PI * radius_inverse * INVERSE_A;
        } else if (qOverPt_for_PHI_END < Q_OVER_PT_END && qOverPt_for_PHI_END > Q_OVER_PT_BEGIN) {

            return radius_inverse * INVERSE_A * (section.xBegin - phi) + 2 * M_PI * radius_inverse * INVERSE_A;
        } else return 0;
    }

    float AdaptiveHoughGpuKernel::yLineAtEnd_modify(const float radius_inverse, const float phi, const AccumulatorSection& section) const
    {

        const double qOverPt_for_PHI_BEGIN = radius_inverse * INVERSE_A * (PHI_BEGIN - phi);
        const double qOverPt_for_PHI_END   = radius_inverse * INVERSE_A * (PHI_END - phi);

        const double xEnd = section.xBegin + section.xSize;
        const double yEnd = section.yBegin + section.ySize;

         if (qOverPt_for_PHI_BEGIN > Q_OVER_PT_BEGIN && qOverPt_for_PHI_BEGIN < Q_OVER_PT_END){

            return radius_inverse * INVERSE_A * (xEnd - phi) - 2 * M_PI * radius_inverse * INVERSE_A;

        } else if (qOverPt_for_PHI_END < Q_OVER_PT_END && qOverPt_for_PHI_END > Q_OVER_PT_BEGIN) {

            return radius_inverse * INVERSE_A * (xEnd - phi) + 2 * M_PI * radius_inverse * INVERSE_A;
        }
        return 0;
    }

    bool AdaptiveHoughGpuKernel::isSolutionWithinCell(const AccumulatorSection& section) const
    {
        if (section.counts > MAX_COUNT_PER_SECTION){
            return true;
        }

        uint32_t max_n_solutions = section.counts * (section.counts - 1) / 2;
        uint32_t count_rejected{};

        float solutions_phi[max_n_solutions];
        float solutions_qOverPt[max_n_solutions];

        float solutions_phi_update[max_n_solutions];
        float solutions_qOverPt_update[max_n_solutions];

        const float xEnd = section.xBegin + section.xSize;
        const float yEnd = section.yBegin + section.ySize;

        uint8_t index_array{};
        for (uint32_t index_main = 0; index_main < section.counts; ++index_main){
            for (uint8_t index_secondary = 0; index_secondary < index_main; ++index_secondary){

                uint32_t line_index_main      = section.indices[index_main];
                uint32_t line_index_secondary = section.indices[index_secondary];

                float r_main      = rs[line_index_main];
                float r_secondary = rs[line_index_secondary];

                float phi_main      = phis[line_index_main];
                float phi_secondary = phis[line_index_secondary];

                float phi_solution = (phi_main * r_secondary - phi_secondary * r_main) / (r_secondary - r_main);
                float qOverPt_solution = (phi_solution - phi_main) / (r_main) * INVERSE_A;

                if (phi_solution > PHI_BEGIN && phi_solution < PHI_END && qOverPt_solution > Q_OVER_PT_BEGIN && qOverPt_solution < Q_OVER_PT_END){

                    solutions_phi[index_array] = phi_solution;
                    solutions_qOverPt[index_array] = qOverPt_solution;
                    ++index_array;
                } else {

                    ++count_rejected;
                }
            }
        }

        max_n_solutions -= count_rejected;
        count_rejected = 0;

        float mean_phi{};
        float stdev_phi{};
        float mean_qOverPt{};
        float stdev_qOverPt{};

        /////////////////////////////////

        for (uint32_t index = 0; index < max_n_solutions; ++index){

            mean_phi += solutions_phi[index];
            stdev_phi += solutions_phi[index] * solutions_phi[index];

            mean_qOverPt += solutions_qOverPt[index];
            stdev_phi += solutions_qOverPt[index] * solutions_qOverPt[index];
        }

        mean_phi /= max_n_solutions;
        mean_qOverPt /= max_n_solutions;
        //std::cout << section.xBegin << " " << mean_phi << " " << xEnd << std::endl;

        stdev_phi     = (stdev_phi / max_n_solutions - mean_phi * mean_phi);
        stdev_qOverPt = std::sqrt(stdev_qOverPt / max_n_solutions - mean_qOverPt * mean_qOverPt);

        //        if ((mean_phi > section.xBegin && mean_phi < xEnd) && (mean_qOverPt > section.yBegin && mean_qOverPt < yEnd)){
        //            return true;
        //        } else {
        //            return false;
        //        }

        do {
            max_n_solutions -= count_rejected;
            count_rejected = 0;

            for (uint8_t index = 0; index < max_n_solutions; ++index){

                mean_phi += solutions_phi[index];
                stdev_phi += solutions_phi[index] * solutions_phi[index];

                mean_qOverPt += solutions_qOverPt[index];
                stdev_phi += solutions_qOverPt[index] * solutions_qOverPt[index];
            }

            mean_phi /= max_n_solutions;
            mean_qOverPt /= max_n_solutions;

            stdev_phi     = std::sqrt(stdev_phi / max_n_solutions - mean_phi * mean_phi);
            stdev_qOverPt = std::sqrt(stdev_qOverPt / max_n_solutions - mean_qOverPt * mean_qOverPt);

            uint8_t index_updates{};
            for (uint8_t index = 0; index < max_n_solutions; ++index){

                float phi_solution     = solutions_phi[index];
                float qOverPt_solution = solutions_qOverPt[index];

                float phi_lower = mean_phi - N_SIGMA_GAUSS * stdev_phi;
                float phi_upper = mean_phi + N_SIGMA_GAUSS * stdev_phi;
                float qOverPt_lower = mean_qOverPt - N_SIGMA_GAUSS * stdev_qOverPt;
                float qOverPt_upper = mean_qOverPt + N_SIGMA_GAUSS * stdev_qOverPt;

                if ((phi_solution > phi_lower && phi_solution < phi_upper) && (qOverPt_solution > qOverPt_lower && qOverPt_solution < qOverPt_upper)){

                    // intersection powint is within cell
                    solutions_phi_update[index_updates]     = phi_solution;
                    solutions_qOverPt_update[index_updates] = qOverPt_solution;
                    ++index_updates;
                } else {

                    ++ count_rejected;
                }
            }
        } while (count_rejected);

        if ((mean_phi > section.xBegin && mean_phi < xEnd) && (mean_qOverPt > section.yBegin && mean_qOverPt < yEnd)){
            return true;
        } else {
            return false;
        }

    }
} // namespace HelixSolver