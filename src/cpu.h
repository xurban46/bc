/*
 * Colearning in Coevolutionary Algorithms
 * Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>
 *
 * Master Thesis
 * 2014/2015
 *
 * Supervisor: Ing. Michaela Šikulová <isikulova@fit.vutbr.cz>
 *
 * Faculty of Information Technologies
 * Brno University of Technology
 * http://www.fit.vutbr.cz/
 *
 * Started on 28/07/2014.
 *      _       _
 *   __(.)=   =(.)__
 *   \___)     (___/
 */


/*
    CPU new instruction set check.
    Source: https://software.intel.com/en-us/articles/how-to-detect-new-instruction-support-in-the-4th-generation-intel-core-processor-family
 */


#pragma once

#include <stdbool.h>
#include <cpuid.h>


#define SIMD_PADDING_BYTES 32


/**
 * Checks whether current CPU supports AVX2 and other New Haswell features
 */
bool can_use_intel_core_4th_gen_features();


/**
 * Checks whether current CPU supports SSE4.1 instruction set
 */
bool can_use_sse4_1();


/**
 * Checks whether current CPU supports SSE2 instruction set
 */
bool can_use_sse2();



static inline bool can_use_simd() {
    #ifdef AVX2
        if (can_use_intel_core_4th_gen_features()) {
            return true;
        }
    #endif

    #ifdef SSE2
        if (can_use_sse2()) {
            return true;
        }
    #endif

    return false;
}
