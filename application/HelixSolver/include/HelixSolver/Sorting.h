#pragma once

#include <iostream>

#include <cstdint>

namespace HelixSolver {
struct CrossingsSorter {
    typedef uint32_t IndexType;
    static void sort(float *distances, IndexType* indices, uint32_t size) {
        for (uint32_t main_index = 0; main_index < size; ++main_index){

            uint32_t min_index = 0;
            constexpr float MAX= 1000000.0;
            float min_val = MAX;
            // std::cout << "before \n";
            // for ( uint32_t i = 0; i < size; ++i ) {
            //     std::cout << i  << " " << indices[i] <<  " " << distances[i] << std::endl;
            // }

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

            // std::cout << "after \n";
            // for ( uint32_t i = 0; i < size; ++i ) {
            //     std::cout << i  << " " << indices[i] <<  " " << distances[i] << std::endl;
            // }
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
};
}