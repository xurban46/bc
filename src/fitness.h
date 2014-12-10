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


#include "image.h"
#include "cgp/cgp.h"
#include "archive.h"
#include "predictors.h"


static const int FITNESS_SSE2_STEP = 16;
static const int FITNESS_AVX2_STEP = 32;

static const int PRED_CIRCULAR_TRIES = 3;


/**
 * For testing purposes only
 */
void fitness_test_init(img_image_t original_image,
    img_window_array_t noisy_image_windows,
    img_pixel_t *noisy_image_simd[WINDOW_SIZE]);


/**
 * Initializes fitness module - prepares test image
 * @param original
 * @param noisy
 * @param cgp_archive
 * @param pred_archive
 */
void fitness_init(img_image_t original, img_image_t noisy,
    archive_t cgp_archive, archive_t pred_archive);


/**
 * Deinitialize fitness module internals
 */
void fitness_deinit();


/**
 * Returns number of performed CGP evaluations
 */
long fitness_get_cgp_evals();


/**
 * Filters image using given filter. Caller is responsible for freeing
 * the filtered image
 *
 * Works in single thread
 *
 * @param  chr
 * @return fitness value
 */
img_image_t fitness_filter_image(ga_chr_t chr);


/**
 * Evaluates CGP circuit fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_cgp(ga_chr_t chr);


/**
 * If predictors archive is empty, returns `fitness_eval_cgp` result.
 * If there is at least one predictor in archive
 * returns `fitness_predict_cgp` result using first predictor in archive.
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_or_predict_cgp(ga_chr_t chr);


/**
 * Predictes CGP circuit fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_predict_cgp(ga_chr_t cgp_chr, ga_chr_t pred_chr);


/**
 * Evaluates predictor fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_predictor(ga_chr_t chr);


/**
 * Evaluates circular predictor fitness, using PRED_CIRCULAR_TRIES to
 * determine best offset
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_circular_predictor(ga_chr_t pred_chr);


/**
 * Evaluates predictor fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_predictor_genome(pred_genome_t predictor);


/**
 * Calculates fitness using the PSNR (peak signal-to-noise ratio) function.
 * The higher the value, the better the filter.
 *
 * @param  original image
 * @param  filtered image
 * @return fitness value (PSNR)
 */
ga_fitness_t fitness_psnr(img_image_t original, img_image_t filtered);


/**
 * Computes real PSNR value from fitness value
 */
static inline double fitness_to_psnr(ga_fitness_t f) {
    return 10 * log10(f);
}


/**
 * SIMD fitness evaluator prototype
 */
typedef double (*fitness_simd_func_t)(
    img_pixel_t *original,
    img_pixel_t *noisy[WINDOW_SIZE],
    ga_chr_t chr,
    int offset,
    int block_size);


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
    int block_size);


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
    int offset);


/**
 * Fills simd-friendly predictor arrays with correct image data
 * @param  genome
 */
void fitness_prepare_predictor_for_simd(pred_genome_t predictor);
