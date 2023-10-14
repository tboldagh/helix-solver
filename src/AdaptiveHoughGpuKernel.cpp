#include <cmath>
#ifndef USE_SYCL
#include <iostream>
#endif
#include "HelixSolver/AdaptiveHoughGpuKernel.h"
#include "HelixSolver/Debug.h"
#include "HelixSolver/Sorting.h"

namespace HelixSolver {
AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel(OptionsAccessor o,
                                               FloatBufferReadAccessor rs,
                                               FloatBufferReadAccessor phis,
                                               FloatBufferReadAccessor z,
                                               SolutionsWriteAccessor solutions)
    : opts(o), rs(rs), phis(phis), zs(z), solutions(solutions) {
  CDEBUG(DISPLAY_BASIC, ".. AdaptiveHoughKernel instantiated with "
                            << rs.size() << " measurements ");
}

void AdaptiveHoughGpuKernel::operator()(Index2D idx) const {
  HelixSolver::Options opt = opts[0];
  
  // 'pure" width of wedge which can be used to determine wedge center,
  // obtaine dy division of the full range of variable by number of regions,
  // after addition of excess_wedge_*_width it informs about true wedge width
  const float wedge_phi_width = (PHI_END - PHI_BEGIN) / opt.N_PHI_WEDGE;
  const float wedge_eta_width =
      (ETA_WEDGE_MAX - ETA_WEDGE_MIN) / opt.N_ETA_WEDGE;

  for (uint16_t wedge_index_phi = 0; wedge_index_phi < opt.N_PHI_WEDGE;
        ++wedge_index_phi) {
    for (uint16_t wedge_index_eta = 0; wedge_index_eta < opt.N_ETA_WEDGE;
          ++wedge_index_eta) {
      float rs_wedge[MAX_SPACEPOINTS];
      float phis_wedge[MAX_SPACEPOINTS];
      float zs_wedge[MAX_SPACEPOINTS];
      uint32_t wedge_spacepoints_count{};

      const float wedge_phi_center = PHI_BEGIN +
                                      wedge_phi_width * wedge_index_phi +
                                      wedge_phi_width / 2.0;
      const float wedge_eta_center = ETA_WEDGE_MIN + wedge_eta_width / 2 +
                                      wedge_eta_width * wedge_index_eta;

      Reg phi_reg = Reg(wedge_phi_center,
                        wedge_phi_width / 2.0 + excess_wedge_phi_width);
      Reg z_reg = Reg(wedge_z_center, wedge_z_width);
      Reg eta_reg = Reg(wedge_eta_center,
                        wedge_eta_width / 2.0 + excess_wedge_eta_width);

      Wedge wedge = Wedge(phi_reg, z_reg, eta_reg);

      const uint32_t maxIndex = rs.size();
      for (uint32_t index = 0; index < maxIndex; ++index) {
        if (wedge.in_wedge_r_phi_z(rs[index], phis[index], zs[index])) {
          rs_wedge[wedge_spacepoints_count] = rs[index];
          phis_wedge[wedge_spacepoints_count] = phis[index];

          // take care about phi wrapping around +-PI
          // this is done bye moving the points by 2 PI
          if (wedge.phi_min() < -M_PI &&
              phis_wedge[wedge_spacepoints_count] > wedge.phi_max()) {
            phis_wedge[wedge_spacepoints_count] -= 2.0 * M_PI;
          }

          if (wedge.phi_max() > M_PI &&
              phis_wedge[wedge_spacepoints_count] < wedge.phi_min()) {
            phis_wedge[wedge_spacepoints_count] += 2.0 * M_PI;
          }

          zs_wedge[wedge_spacepoints_count] = zs[index];
          ++wedge_spacepoints_count;
        }
      }

      CDEBUG(DISPLAY_N_WEDGE, wedge_index_phi << "," << wedge_index_eta << ","
                                              << wedge_spacepoints_count
                                              << ":WedgeCounts");
      // do not conduct alogorithm calculations for empty region
      if (wedge_spacepoints_count == 0)
        continue;

      CDEBUG(DISPLAY_BASIC, " .. AdaptiveHoughKernel initiated for subregion "
                                << idx[0] << " " << idx[1]);

      // const float INITIAL_X_SIZE = (2*phi_reg.width) /
      // ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
      const float INITIAL_X_SIZE =
          ACC_X_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
      const float INITIAL_Y_SIZE =
          ACC_Y_SIZE / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

      // const double xBegin = (phi_reg.center - phi_reg.width) +
      // INITIAL_X_SIZE * idx[0];
      const double xBegin = PHI_BEGIN + INITIAL_X_SIZE * idx[0];
      const double yBegin = Q_OVER_PT_BEGIN + INITIAL_Y_SIZE * idx[1];

      CDEBUG(DISPLAY_BASIC, " .. AdaptiveHoughKernel region, x: "
                                << xBegin << " xsz: " << INITIAL_X_SIZE
                                << " y: " << yBegin
                                << " ysz: " << INITIAL_Y_SIZE);

      // we need here a limited set of Points

      // the size os somewhat arbitrary, for regular algorithm dividing into 4
      // sub-sections it defined by the depth allowed but for more flexible
      // algorithms that is less predictable for now it is an arbitrary
      // constant + checks that we stay within this limit

      AccumulatorSection
          sections[MAX_SECTIONS_BUFFER_SIZE]; // in here sections of image
                                              // will be recorded
                                              
      uint32_t sectionsBufferSize = 1;
      const uint32_t initialDivisionLevel = 0;
      sections[0] = AccumulatorSection(INITIAL_X_SIZE, INITIAL_Y_SIZE, xBegin,
                                        yBegin, initialDivisionLevel);

      // scan this region until there is no section to process (i.e. size,
      // initially 1, becomes 0)
      while (sectionsBufferSize) {
        fillAccumulatorSection(sections, sectionsBufferSize, rs_wedge,
                                phis_wedge, zs_wedge, wedge_eta_center,
                                wedge_spacepoints_count);

      }
    }
  }
}

void AdaptiveHoughGpuKernel::fillAccumulatorSection(
    AccumulatorSection *sections, uint32_t &sectionsBufferSize, float *rs_wedge,
    float *phis_wedge, float *zs_wedge, float wedge_eta_center,
    uint32_t wedge_spacepoints_count) const {
  HelixSolver::Options opt = opts[0];

  CDEBUG(DISPLAY_BASIC,
         "Regions buffer depth " << static_cast<int>(sectionsBufferSize));
  // pop the region from the top of sections buffer
  sectionsBufferSize--;
  AccumulatorSection section =
      sections[sectionsBufferSize]; // copy section, it will be modified, TODO
                                    // consider not to copy
  CDEBUG(DISPLAY_BOX_POSITION, section.xBegin
                                   << "," << section.yBegin << ","
                                   << section.xBegin + section.xSize << ","
                                   << section.yBegin + section.ySize << ","
                                   << section.divisionLevel << ":BoxPosition");

  // if we are sufficently far in division algorithm, cells can be rejected
  // based also on the condition none of the lines intersects within the cell
  // boundaries, countHits_checkOrder checks that condition
  uint16_t count{};
  if (section.divisionLevel < THRESHOLD_DIVISION_LEVEL_COUNT_HITS_CHECK_ORDER) {
    count = countHits(section, rs_wedge, phis_wedge, zs_wedge,
                      wedge_spacepoints_count);
  } else
    count = countHits_checkOrder(section, rs_wedge, phis_wedge, zs_wedge,
                                 wedge_spacepoints_count);

  CDEBUG(DISPLAY_BASIC,
         "count of lines in region x:"
             << section.xBegin << " xsz: " << section.xSize
             << " y: " << section.yBegin << " ysz: " << section.ySize
             << " divLevel: " << section.divisionLevel << " count: " << count);
  if (count < THRESHOLD)
    return;

  //    if (!TO_DISPLAY_PRECISION_PAIR_ONCE){     // so that these values are
  //    displayed only once
  //         DEBUG("AdaptiveHoughGpuKernel.cpp: ACC_X_PRECISION = " <<
  //         opt.ACC_X_PRECISION << ", ACC_PT_PRECISION = " <<
  //         opt.ACC_PT_PRECISION);
  //    }
  //    ++TO_DISPLAY_PRECISION_PAIR_ONCE;
  if (section.xSize > opt.ACC_X_PRECISION &&
      section.ySize > opt.ACC_PT_PRECISION) {
    CDEBUG(DISPLAY_BASIC, "Splitting region into 4");
    // by the order here we steer the direction of the search of image space
    // it may be relevant depending on the data ordering??? to be testes
    ASSURE_THAT(sectionsBufferSize + 3 < MAX_SECTIONS_BUFFER_SIZE,
                "Sections buffer depth to small (in 4 subregions split)");
    sections[sectionsBufferSize] = section.bottomLeft();
    sections[sectionsBufferSize + 1] = section.topLeft();
    sections[sectionsBufferSize + 2] = section.topRight();
    sections[sectionsBufferSize + 3] = section.bottomRight();
    sectionsBufferSize += 4;
  } else if (section.xSize > opt.ACC_X_PRECISION) {
    CDEBUG(DISPLAY_BASIC, "Splitting region into 2 in x direction");
    ASSURE_THAT(sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE,
                "Sections buffer depth to small (in x split)");
    sections[sectionsBufferSize] = section.left();
    sections[sectionsBufferSize + 1] = section.right();
    sectionsBufferSize += 2;
  } else if (section.ySize > opt.ACC_PT_PRECISION) {
    CDEBUG(DISPLAY_BASIC, "Splitting region into 2 in y direction");
    ASSURE_THAT(sectionsBufferSize + 1 < MAX_SECTIONS_BUFFER_SIZE,
                "Sections buffer depth to small (in x split)");
    sections[sectionsBufferSize] = section.bottom();
    sections[sectionsBufferSize + 1] = section.top();
    sectionsBufferSize += 2;
  } else { // no more splitting, we have a solution

    if (USE_GAUSS_FILTERING) {
      // Next step of solutions number minimization - checking phi of each line
      // Each helix is almost a straight line so we expect very small deviation
      // from the truth value. Outliner in terms of phi indicates that this
      // point does not belong to analyzed track We want to reduce number of
      // counter by number of lines with phi differing from mean by more than n
      // standard deviations
      if (isPeakWithinCell(section)) {
        addSolution(section, wedge_eta_center);
      }
    } else {

      addSolution(section, wedge_eta_center);
    }
  }
}

uint16_t
AdaptiveHoughGpuKernel::countHits(AccumulatorSection &section, float *rs_wedge,
                                  float *phis_wedge, float *zs_wedge,
                                  uint32_t wedge_spacepoints_count) const {
  uint16_t counter = 0;
  // here we can improve by knowing over which Points to iterate (i.e. indices
  // of measurements), this is related to geometry this can be stored in section
  // object maybe???

  const uint32_t maxIndex = wedge_spacepoints_count;
  for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION;
       ++index) {
    const float r = rs_wedge[index];
    const float inverse_r = 1.0 / r;
    const float phi = phis_wedge[index];

    if (lineInsideCell(section, inverse_r, phi)) {

      section.indices[counter] = index;
      counter++;
    }
  }

  section.counts =
      (counter + 1) == MAX_COUNT_PER_SECTION
          ? section.OUT_OF_RANGE_COUNTS
          : counter; // setting this counter to 0 == indices are invalid
  return counter;
}

uint16_t AdaptiveHoughGpuKernel::countHits_checkOrder(
    AccumulatorSection &section, float *rs_wedge, float *phis_wedge,
    float *zs_wedge, uint32_t wedge_spacepoints_count) const {
  const double xEnd = section.xBegin + section.xSize;
  const double yEnd = section.yBegin + section.ySize;
  uint16_t counter = 0;

  // here we can improve by knowing over which Points to iterate (i.e. indices
  // of measurements), this is related to geometry this can be stored in section
  // object maybe???

  const uint32_t maxIndex = wedge_spacepoints_count;

  uint32_t cell_intersection_acc_id[wedge_spacepoints_count];
  float cell_intersection_acc_distance[wedge_spacepoints_count];
  uint32_t cell_intersection_cc_id[wedge_spacepoints_count];
  float cell_intersection_cc_distance[wedge_spacepoints_count];

  // candidate for parallel_for
  for (uint32_t index = 0; index < maxIndex && counter < MAX_COUNT_PER_SECTION; ++index) {
    const float r = rs_wedge[index];
    const float inverse_r = 1.0 / r;
    const float phi = phis_wedge[index];
    const float a =  inverse_r * INVERSE_A;
    const float b = -inverse_r * INVERSE_A * phi;
    if (section.isLineInside(a, b)) {
      section.indices[counter] = index;
      cell_intersection_acc_id[counter] = counter;
      cell_intersection_acc_distance[counter] =
          section.distACC(a, b); // anti clockwise

      cell_intersection_cc_id[counter] = counter;
      cell_intersection_cc_distance[counter] =
          section.distCC(a, b); // clockwise
      counter++;
    }
  }
  if (counter < THRESHOLD) {
    return 0;
  }
  // sort indices according to distance
  CrossingsSorter::sort(cell_intersection_acc_distance,
                        cell_intersection_acc_id, counter);
  CrossingsSorter::sort(cell_intersection_cc_distance, cell_intersection_cc_id,
                        counter);
  const uint32_t count_changes = CrossingsSorter::count(
      cell_intersection_acc_id, cell_intersection_cc_id, counter);

  // we want to reject a cell if not enough intersections were found -> lines
  // only/almost only pararell -> fake solutions
  if (count_changes < MIN_COUNT_CHANGES) {

    section.counts = 0;
    return 0;
  }
  section.counts = counter + 1 == MAX_COUNT_PER_SECTION
                       ? section.OUT_OF_RANGE_COUNTS
                       : counter;
  return counter;
}

void AdaptiveHoughGpuKernel::addSolution(const AccumulatorSection &section,
                                         float wedge_eta_center) const {
  const double qOverPt = section.yBegin + 0.5 * section.ySize;
  const double phi_0 = section.xBegin + 0.5 * section.xSize;

  if (fabs(qOverPt) < 1. / MAX_PT)
    return;

  // for truly adaptive algorithm the index can not be calculated in obvious way
  // therefore the kernel needs to find first available slot to store the
  // solution for now a stupid solution is to iterate over to the first free
  // slot this won't be that easy for truly parallel execution, we will likely
  // need to deffer to sycl::atomic_ref compare_exchange_weak/strong or have the
  // first pass over to calculate number of solutions

  // the coordinates of the solution can be much improved too
  // e.g. using exact formula (i.e. no sin x = x approx), d0 fit & reevaluation,
  // additional hits from pixels inner layers,
  // TODO future work

  const uint32_t solutionsBufferSize = solutions.size();
  for (uint32_t index = 0; index < solutionsBufferSize; ++index) {
    if (solutions[index].phi == SolutionCircle::INVALID_PHI) {
      if (section.canUseIndices()) {
        fillPreciseSolution(section, solutions[index]);
        CDEBUG(DISPLAY_BASIC,
               "AdaptiveHoughKernel solution count: " << int(section.counts));
      } // but for now it is always the simple one
      solutions[index].pt = 1. / qOverPt;
      solutions[index].phi = phi_0;
      // temporary solution - eta of a particle is equal to ea of the region
      solutions[index].eta = wedge_eta_center;

      CDEBUG(DISPLAY_BASIC, "AdaptiveHoughKernel solution q/pt:"
                                << qOverPt << " phi: " << phi_0);
      CDEBUG(DISPLAY_SOLUTION_PAIR,
             qOverPt << "," << phi_0 << "," << section.xBegin << ","
                     << section.yBegin << "," << section.xBegin + section.xSize
                     << "," << section.yBegin + section.ySize << ","
                     << section.divisionLevel << ":SolutionPair");
      // TODO calculate remaining parameters, eta, z, d0
      return;
    }
  }
  ASSURE_THAT(false, "Could not find place for solution!!");
}

void AdaptiveHoughGpuKernel::fillPreciseSolution(
    const AccumulatorSection &section, SolutionCircle &s) const {
  // TODO complete it
}

bool AdaptiveHoughGpuKernel::lineInsideAccumulator(const float radius_inverse,
                                                   const float phi) const {

  const double qOverPt_for_PHI_BEGIN =
      radius_inverse * INVERSE_A * (PHI_BEGIN - phi);
  const double qOverPt_for_PHI_END =
      radius_inverse * INVERSE_A * (PHI_END - phi);

  if (qOverPt_for_PHI_BEGIN <= Q_OVER_PT_BEGIN &&
      qOverPt_for_PHI_END >= Q_OVER_PT_END) {
    return true;
  } else
    return false;
}

bool AdaptiveHoughGpuKernel::isPeakWithinCell(
    const AccumulatorSection &section) const {
  const uint32_t max_counts = section.counts == section.OUT_OF_RANGE_COUNTS
                           ? MAX_COUNT_PER_SECTION
                           : section.counts;
  uint32_t max_n_solutions = max_counts * (max_counts - 1) / 2;

  // array to store solutions, array *_update is used to save solutions which
  // pass criterion of +-N*sigma
  float solutions_phi[max_n_solutions];
  float solutions_phi_update[max_n_solutions];
  float solutions_qOverPt[max_n_solutions];
  float solutions_qOverPt_update[max_n_solutions];

  const float xEnd = section.xBegin + section.xSize;
  const float yEnd = section.yBegin + section.ySize;

  // calculating solutions (defined as intersection point of two lines) for each
  // pair of lines
  uint32_t index_array{};
  for (uint32_t index_main = 0; index_main < max_counts; ++index_main) {
    for (uint32_t index_secondary = 0; index_secondary < index_main;
         ++index_secondary) {

      uint32_t line_index_main = section.indices[index_main];
      uint32_t line_index_secondary = section.indices[index_secondary];

      float r_main = rs[line_index_main];
      float r_secondary = rs[line_index_secondary];

      float phi_main = phis[line_index_main];
      float phi_secondary = phis[line_index_secondary];

      float phi_solution{};
      float qOverPt_solution{};

      // For both lines within accumulator (none of them crosses left or right
      // limit) nothing interesting happens. If calculated intersection point is
      // within accumulator, it is accepted for further analysis. For at least
      // one line crossing left or right limit: If intersection point is to be
      // within accumulator, lines associated with this points must lie on one
      // side of the accumulator -> both phi > 0 or both phi < 0, which is
      // equivalent to phi_main * phi_secondary > 0. Then solution point is
      // calculated for "raw" line parameters (r and phi) and for altered values
      // (phi -> phi +- 2*pi depending on limit of accumulator which is
      // crossed.) If phi_main * phi_secondary < 0 lines lie on opposite sides
      // of the accumulator. Then solution is calculated for either phi
      // corrected by 2 * pi, and then for another phi corrected by this value.
      // This approach solves the problem of lower efficiency for phi close to
      // +- pi.
      if (lineInsideAccumulator(1. / r_main, phi_main) &&
          lineInsideAccumulator(1. / r_secondary, phi_secondary)) {

        phi_solution = (phi_main * r_secondary - phi_secondary * r_main) /
                       (r_secondary - r_main);
        qOverPt_solution = (phi_solution - phi_main) / (r_main)*INVERSE_A;

        if (phi_solution > PHI_BEGIN && phi_solution < PHI_END &&
            qOverPt_solution > Q_OVER_PT_BEGIN &&
            qOverPt_solution < Q_OVER_PT_END) {
          solutions_phi[index_array] = phi_solution;
          solutions_qOverPt[index_array] = qOverPt_solution;
          ++index_array;
        }
      } else if (phi_main * phi_secondary > 0) {

        phi_solution = (phi_main * r_secondary - phi_secondary * r_main) /
                       (r_secondary - r_main);
        qOverPt_solution = (phi_solution - phi_main) / (r_main)*INVERSE_A;

        if (phi_solution > PHI_BEGIN && phi_solution < PHI_END &&
            qOverPt_solution > Q_OVER_PT_BEGIN &&
            qOverPt_solution < Q_OVER_PT_END) {

          solutions_phi[index_array] = phi_solution;
          solutions_qOverPt[index_array] = qOverPt_solution;
          ++index_array;
        }

        if (phi_main > 0) {

          phi_main -= 2 * M_PI;
          phi_secondary -= 2 * M_PI;
        } else {

          phi_main += 2 * M_PI;
          phi_secondary += 2 * M_PI;
        }

        phi_solution = (phi_main * r_secondary - phi_secondary * r_main) /
                       (r_secondary - r_main);
        qOverPt_solution = (phi_solution - phi_main) / (r_main)*INVERSE_A;

        if (phi_solution > PHI_BEGIN && phi_solution < PHI_END &&
            qOverPt_solution > Q_OVER_PT_BEGIN &&
            qOverPt_solution < Q_OVER_PT_END) {

          solutions_phi[index_array] = phi_solution;
          solutions_qOverPt[index_array] = qOverPt_solution;
          ++index_array;
        }
      } else {
        if (phi_main > 0) {
          phi_main -= 2 * M_PI;
        } else {

          phi_secondary -= 2 * M_PI;
        }

        phi_solution = (phi_main * r_secondary - phi_secondary * r_main) /
                       (r_secondary - r_main);
        qOverPt_solution = (phi_solution - phi_main) / (r_main)*INVERSE_A;

        if (phi_solution > PHI_BEGIN && phi_solution < PHI_END &&
            qOverPt_solution > Q_OVER_PT_BEGIN &&
            qOverPt_solution < Q_OVER_PT_END) {

          solutions_phi[index_array] = phi_solution;
          solutions_qOverPt[index_array] = qOverPt_solution;
          ++index_array;
        }

        phi_main += 2 * M_PI;
        phi_secondary += 2 * M_PI;

        phi_solution = (phi_main * r_secondary - phi_secondary * r_main) /
                       (r_secondary - r_main);
        qOverPt_solution = (phi_solution - phi_main) / (r_main)*INVERSE_A;

        if (phi_solution > PHI_BEGIN && phi_solution < PHI_END &&
            qOverPt_solution > Q_OVER_PT_BEGIN &&
            qOverPt_solution < Q_OVER_PT_END) {

          solutions_phi[index_array] = phi_solution;
          solutions_qOverPt[index_array] = qOverPt_solution;
          ++index_array;
        }
      }
    }
  }

  max_n_solutions = index_array;
  if (!max_n_solutions) {

    return false;
  }

  double mean_phi{};
  double stdev_phi{};
  double mean_qOverPt{};
  double stdev_qOverPt{};
  uint32_t count_rejected{};

  // As long as solutions are rejected continue calculating mean and stdev and
  // eliminating values outside the +-n*sigma range
  do {
    max_n_solutions -= count_rejected;
    count_rejected = 0;

    mean_phi = 0;
    mean_qOverPt = 0;
    stdev_phi = 0;
    stdev_qOverPt = 0;

    for (uint32_t index = 0; index < max_n_solutions; ++index) {

      mean_phi += solutions_phi[index];
      mean_qOverPt += solutions_qOverPt[index];
    }

    mean_phi /= max_n_solutions;
    mean_qOverPt /= max_n_solutions;

    for (uint32_t index = 0; index < max_n_solutions; ++index) {

      stdev_phi +=
          (solutions_phi[index] - mean_phi) * (solutions_phi[index] - mean_phi);
      stdev_qOverPt += (solutions_qOverPt[index] - mean_qOverPt) *
                       (solutions_qOverPt[index] - mean_qOverPt);
    }

    stdev_phi = std::sqrt(stdev_phi / max_n_solutions);
    stdev_qOverPt = std::sqrt(stdev_qOverPt / max_n_solutions);

    CDEBUG(DISPLAY_MEAN_STDEV, mean_phi << "," << stdev_phi << ","
                                        << mean_qOverPt << "," << stdev_qOverPt
                                        << ":MeanStdev");

    float phi_lower = mean_phi - N_SIGMA_GAUSS * stdev_phi;
    float phi_upper = mean_phi + N_SIGMA_GAUSS * stdev_phi;
    float qOverPt_lower = mean_qOverPt - N_SIGMA_GAUSS * stdev_qOverPt;
    float qOverPt_upper = mean_qOverPt + N_SIGMA_GAUSS * stdev_qOverPt;

    uint32_t index_updates = 0;
    for (uint32_t index = 0; index < max_n_solutions; ++index) {

      float phi_solution = solutions_phi[index];
      float qOverPt_solution = solutions_qOverPt[index];

      if ((phi_solution > phi_lower && phi_solution < phi_upper) &&
          (qOverPt_solution > qOverPt_lower &&
           qOverPt_solution < qOverPt_upper)) {

        // intersection point is within +- N sigma
        solutions_phi_update[index_updates] = phi_solution;
        solutions_qOverPt_update[index_updates] = qOverPt_solution;
        ++index_updates;
      } else {

        ++count_rejected;
      }
    }

    for (uint32_t index_2 = 0; index_2 < index_updates; ++index_2) {

      solutions_phi[index_2] = solutions_phi_update[index_2];
      solutions_qOverPt[index_2] = solutions_qOverPt_update[index_2];

      solutions_phi_update[index_2] = 0;
      solutions_qOverPt_update[index_2] = 0;
    }

  } while (count_rejected);

  // Return false if all intersection points were rejected.
  if (!max_n_solutions) {

    return false;
  }

  // Parameter STDEV_CORRECTION allows to include sections for which mean in
  // either coordinate is only slightly outside the cell limits. Setting value
  // equal 0.6 results in 100% efficiency.
  float mean_phi_lower = section.xBegin - STDEV_CORRECTION * stdev_phi;
  float mean_phi_upper = xEnd + STDEV_CORRECTION * stdev_phi;
  float mean_qOverPt_lower = section.yBegin - STDEV_CORRECTION * stdev_qOverPt;
  float mean_qOverPt_upper = yEnd + STDEV_CORRECTION * stdev_qOverPt;

  if ((mean_phi > mean_phi_lower && mean_phi < mean_phi_upper) &&
      (mean_qOverPt > mean_qOverPt_lower &&
       mean_qOverPt < mean_qOverPt_upper)) {

    return true;
  } else {

    return false;
  }
}

bool AdaptiveHoughGpuKernel::lineInsideCell(const AccumulatorSection section,
                                            const float radius_inverse,
                                            const float phi) const {

  const double xEnd = section.xBegin + section.xSize;
  const double yEnd = section.yBegin + section.ySize;

  float yLineAtBegin = radius_inverse * INVERSE_A * (section.xBegin - phi);
  float yLineAtEnd = radius_inverse * INVERSE_A * (xEnd - phi);

  if (lineInsideAccumulator(radius_inverse, phi)) {
    if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd) {
      return true;
    }
  } else {
    if (yLineAtBegin <= yEnd && section.yBegin <= yLineAtEnd) {
      return true;
    }
  }
  return false;
}
} // namespace HelixSolver