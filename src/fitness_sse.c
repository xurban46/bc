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


#include "fitness.h"
#include "cgp/cgp_sse.h"


/**
 * Calculates difference between original and filtered pixel using SSE2
 * instructions.
 *
 * One call equals 16 CGP evaluations.
 *
 * @param  original_image
 * @param  noisy_image_simd
 * @param  chr
 * @param  offset Where to start in arrays
 * @param  block_size How many pixels to process
 * @return
 */
double _fitness_get_sqdiffsum_sse(
    img_pixel_t *original,
    img_pixel_t *noisy[WINDOW_SIZE],
    ga_chr_t chr,
    int offset,
    int block_size)
{
    __m128i_aligned sse_inputs[CGP_INPUTS];
    __m128i_aligned sse_outputs[CGP_OUTPUTS];
    unsigned char *outputs_ptr = (unsigned char*) &sse_outputs;

    for (int i = 0; i < CGP_INPUTS; i++) {
        sse_inputs[i] = _mm_load_si128((__m128i*)(&noisy[i][offset]));
    }

    cgp_get_output_sse(chr, sse_inputs, sse_outputs);

    double sum = 0;
    for (int i = 0; i < block_size; i++) {
        int diff = outputs_ptr[i] - original[offset + i];
        sum += diff * diff;
    }
    return sum;
}

