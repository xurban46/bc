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
#include "cgp/cgp_avx.h"


/**
 * Calculates difference between original and filtered pixel using AVX2
 * instructions.
 *
 * One call equals 32 CGP evaluations.
 *
 * @param  chr
 * @param  w
 * @return
 */
double _fitness_get_sqdiffsum_avx(
    img_image_t _original_image,
    img_pixel_t *_noisy_image_simd[WINDOW_SIZE],
    ga_chr_t chr,
    int offset)
{
    __m256i_aligned avx_inputs[CGP_INPUTS];
    __m256i_aligned avx_outputs[CGP_OUTPUTS];
    unsigned char *outputs_ptr = (unsigned char*) &avx_outputs;

    for (int i = 0; i < CGP_INPUTS; i++) {
        avx_inputs[i] = _mm256_load_si256((__m256i*)(&_noisy_image_simd[i][offset]));
    }

    cgp_get_output_avx(chr, avx_inputs, avx_outputs);

    // store
    double sum = 0;
    for (int i = 0; i < 32; i++) {
        int diff = outputs_ptr[i] - _original_image->data[offset + i];
        sum += diff * diff;
    }
    return sum;
}
