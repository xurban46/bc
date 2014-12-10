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


#pragma once

#include <immintrin.h>

#include "cgp_core.h"


typedef __m128i __m128i_aligned __attribute__ ((aligned (16)));


/**
 * Calculate output of given chromosome and inputs using SSE instructions
 * @param chr
 * @param inputs
 * @param outputs
 */
void cgp_get_output_sse(ga_chr_t chromosome,
    __m128i_aligned inputs[CGP_INPUTS], __m128i_aligned outputs[CGP_OUTPUTS]);
