#pragma once

#include <iostream>
#include "AdaptiveHoughGpuKernel.h"
#include <cstdint>
#include "Debug.h"

namespace HelixSolver {
    struct CrossingsSorter {

        typedef uint32_t IndexType;

        static void sort(float *distances, IndexType* indices, uint32_t size) {
            for (uint32_t main_index = 0; main_index < size; ++main_index){

                uint32_t min_index = 0;
                constexpr float MAX= 1000000.0;
                float min_val = MAX;

                for (uint32_t index = 0; index < size; ++index){

                    if (distances[index] < min_val){

                        min_index = index;
                        min_val = distances[index];
                    }
                }
                uint32_t temp_index = indices[main_index];
                indices[main_index] = indices[min_index];
                indices[min_index] = temp_index;

                float temp_distance = distances[main_index];
                distances[min_index] = temp_distance;

                distances[main_index] = MAX;
            }
        }
        // counts how many times the two arrays have different values
        static uint32_t count(IndexType* a, IndexType* b, uint32_t size) {


            uint32_t count = 0;
            for(uint32_t i=0; i < size; ++i){
                if (a[i] != b[i]){
                    ++count;
                }
            }
            return count;
        }

        static bool isPhiOnTheRightSide(AccumulatorSection section, float phi){

            float phi_0 = section.xBegin;
            float q = section.yBegin > 0 ? 1. : -1.;

            if (q > 0){

                if (phi < phi_0){

                    return true;
                } else return false;
            } else {

                if (phi > phi_0){

                    return true;
                } else return false;
            }
        }
    
        static bool isCurvatureRight(AccumulatorSection section, float r, float phi, float r_previous, float phi_previous){

            float q = section.yBegin > 0 ? 1. : -1.;
            float correction_factor = 0.05;

            if (q > 0){

                if (r > r_previous){

                    if (phi < (phi_previous + correction_factor)){

                        return true;
                    } else return false;
                } if (phi > (phi_previous - correction_factor)){

                    return true;
                } else return false;
            } else {

                if (r > r_previous){

                    if (phi < (phi_previous + correction_factor)){

                        return true;
                    } else return false;
                } if (phi > (phi_previous - correction_factor)){

                    return true;
                } else return false;
            }
        }

        static float PhiIntersection(AccumulatorSection section, uint32_t index_main, uint32_t index_secondary, 
                                        float* rs_wedge, float* phis_wedge){

            float r_main = rs_wedge[section.indices[index_main]];
            float r_secondary = rs_wedge[section.indices[index_secondary]];

            float phi_main = phis_wedge[section.indices[index_main]];
            float phi_secondary = phis_wedge[section.indices[index_secondary]];

            return (phi_main * r_secondary - phi_secondary * r_main) / (r_secondary - r_main);
        }

        static float qOverPtIntersection(AccumulatorSection section, uint32_t index_main, uint32_t index_secondary, 
                                        float* rs_wedge, float* phis_wedge){

            float r_main = rs_wedge[section.indices[index_main]];
            float r_secondary = rs_wedge[section.indices[index_secondary]];

            float phi_main = phis_wedge[section.indices[index_main]];
            float phi_secondary = phis_wedge[section.indices[index_secondary]];

            float phi_solution = CrossingsSorter::PhiIntersection(section, index_main, index_secondary, 
                                                 rs_wedge, phis_wedge);

            return (phi_solution - phi_main) / (r_main)*INVERSE_A;
        }
    
        static bool isIntersectionCloseEnough(float phi_intersection, float qOverPt_intersection){


            if (phi_intersection > PHI_BEGIN && phi_intersection < PHI_END &&
            qOverPt_intersection > Q_OVER_PT_BEGIN && qOverPt_intersection < Q_OVER_PT_END){

                return true;
            } else return false;
        }

        static void rotateDetector(float* phis_wedge, uint32_t size, float angle){

            for (uint32_t index = 0; index < size; ++index){

                phis_wedge[index] += angle;
            }
        }

        static void phiWrap(float* phi, uint32_t size){

            for (uint32_t index = 0; index < size; ++index){

                while (phi[index] > M_PI) {
                    phi[index] -= 2.0 * M_PI;
                }
                
                while (phi[index] <= -M_PI) {
                    phi[index] += 2.0 * M_PI;
                }
            }
        }
    
        static float calculateMean(float* array, uint32_t size){

            float mean{};

            for (uint32_t index = 0; index < size; ++index) {

                mean += array[index];
            }

            return mean/size;
        }

        static float calculateStDev(float* array, uint32_t size){

            float stdev{};
            float mean = calculateMean(array, size);

            for (uint32_t index = 0; index < size; ++index) {

                stdev += std::pow(array[index] - mean, 2);
            }

            return std::sqrt(stdev/size);
        }

        static void zeroArray(float* array, uint64_t size){

            for (uint64_t index = 0; index < size; ++index){

                array[index] = 0;
            }
        }

        static bool areAllSectionLinesInsideAccumulator(AccumulatorSection section, float* rs_wedge, float* phis_wedge){

            uint32_t max_counts = section.returnCounter();

            for (uint32_t index = 0; index < max_counts; ++index){

                float r = rs_wedge[section.indices[index]];
                float phi = phis_wedge[section.indices[index]];

                if (lineInsideAccumulator(1. / r, phi) == 0) return false;
            }

            return true;
        }

        static bool lineInsideAccumulator(const float radius_inverse, const float phi) {

            const double qOverPt_for_PHI_BEGIN = radius_inverse * INVERSE_A * (PHI_BEGIN - phi);
            const double qOverPt_for_PHI_END   = radius_inverse * INVERSE_A * (PHI_END - phi);

            if (qOverPt_for_PHI_BEGIN <= Q_OVER_PT_BEGIN &&
                qOverPt_for_PHI_END >= Q_OVER_PT_END) {
                return true;
            } else return false;
        }
    
        static void copyArray(float* source_array, float* target_array, uint32_t size){

            for(uint32_t index = 0; index < size; ++index){

                target_array[index] = source_array[index];
            }
        }

        static void printArray(float* array, uint32_t size){

            for(uint32_t index = 0; index < size; ++index){

                std::cout << array[index] << " ";
            }

            std::cout << std::endl;
        }
    
        static float distanceSectionCenter(AccumulatorSection section, float inverse_r, float phi){

            const float x = section.xBegin + 0.5 * section.xSize;
            const float y = section.yBegin + 0.5 * section.ySize;

            // line defined as ax + by + c = 0
            // d = |a*x_1 + b*y_1 + c|/sqrt(a^2 + b^2)
            // phi/(rA) - q/pt - phi/(rA) = 0

            const float a = inverse_r * INVERSE_A;
            const float b = -1;
            const float c = - phi * inverse_r * INVERSE_A;

            return std::fabs(a*x + b*y + c)/std::sqrt(a*a + b*b);
        }
    
        static bool checkLinearity_R2(AccumulatorSection section, float* rs_wedge, float* phis_wedge, float* zs_wedge){

            float x_mean = 0;
            float y_mean = 0;

            // x-coordinate - z
            // y-coordinate - radius

            typedef uint32_t IndexType;

            IndexType max_counts = section.returnCounter();
            for (IndexType index = 0; index < max_counts; ++index){

                float r = rs_wedge[section.indices[index]];
                float z = zs_wedge[section.indices[index]];

                x_mean += z;
                y_mean += r;
            }

            x_mean /= max_counts;
            y_mean /= max_counts;

            // calculate b_1 and b_0
            float numerator{};
            float denominator{};

            for (IndexType index = 0; index < max_counts; ++index){

                float r = rs_wedge[section.indices[index]];
                float z = zs_wedge[section.indices[index]];

                numerator += (z - x_mean) * (r - y_mean);
                denominator += std::pow(z - x_mean, 2);
            }

            float b1 = numerator / denominator;
            float b0 = y_mean - b1 * x_mean;

            // calculate R^2 coefficient
            float ss_res{};
            float ss_tot{};

            for (IndexType index = 0; index < max_counts; ++index){

                float r = rs_wedge[section.indices[index]];
                float z = zs_wedge[section.indices[index]];

                ss_res += std::pow(r - std::fma(b1, z, b0), 2);
                ss_tot += std::pow(r - y_mean, 2);
            }

            float R2 = 1 - ss_res / ss_tot;
            //std::cout << R2 << std::endl;
            static IndexType solution_id = 0;

            for (IndexType index = 0; index < max_counts; ++index){

                float r = rs_wedge[section.indices[index]];
                float z = zs_wedge[section.indices[index]];

                if (r != 0 && z != 0){

                  CDEBUG(DISPLAY_R_Z, solution_id << "," << float(z) << "," << r << ":RZ");
                }
            }
            ++solution_id;

            CDEBUG(DISPLAY_R2, R2 << ":R2");

            if (R2 < MIN_R2){

                return false; 
            } else return true;
        }

        static bool checkLinearity_Simple(AccumulatorSection section, float* rs_wedge, float* phis_wedge, float* zs_wedge){

            typedef uint32_t IndexType;

            IndexType min_z_index = 0;
            IndexType max_z_index = 0;

            float min_z =  10000;
            float max_z = -10000;

            IndexType max_counts = section.returnCounter();
            for (IndexType index = 0; index < max_counts; ++index){

                float z = zs_wedge[section.indices[index]];
                if (z == 0) continue;

                if (z < min_z) {
                    min_z_index = index;
                    min_z = z;
                }
                if (z > max_z) {
                    max_z_index = index;
                    max_z = z;
                }
            }
            //std::cout << "min = " << min_z_index << ", max = " << max_z_index << "\n\n" << std::endl;

            const float a = (rs_wedge[section.indices[min_z_index]] - rs_wedge[section.indices[max_z_index]])/(zs_wedge[section.indices[min_z_index]] - zs_wedge[section.indices[max_z_index]]);
            const float b = rs_wedge[section.indices[min_z_index]] - a * zs_wedge[section.indices[min_z_index]];

            IndexType close_enough_points{};
            for (IndexType index = 0; index < max_counts; ++index){

                float z = zs_wedge[section.indices[index]];
                float r_dash = std::fma(a, z, b);
                float r = rs_wedge[section.indices[index]];

                float discrepancy = std::fabs((r_dash - r)/r_dash);

                if (discrepancy < MAX_LINEAR_DISCREPANCY) ++close_enough_points;
            }

            if (close_enough_points < LINEAR_THRESHOLD){

                return false;
            } else return true;
        }
    };
}