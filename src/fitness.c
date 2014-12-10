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


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "cpu.h"
#include "random.h"
#include "fitness.h"

static img_image_t _original_image;
static img_window_array_t _noisy_image_windows;
static img_pixel_t *_noisy_image_simd[WINDOW_SIZE];
static archive_t _cgp_archive;
static archive_t _pred_archive;
static double _psnr_coeficient;

static long _cgp_evals;


static inline double fitness_psnr_coeficient(int pixels_count)
{
    return 255 * 255 * (double)pixels_count;
}


/**
 * For testing purposes only
 */
void fitness_test_init(img_image_t original_image,
    img_window_array_t noisy_image_windows,
    img_pixel_t *noisy_image_simd[WINDOW_SIZE])
{
    _original_image = original_image;
    _noisy_image_windows = noisy_image_windows;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        _noisy_image_simd[i] = noisy_image_simd[i];
    }
}


/**
 * Initializes fitness module - prepares test image
 * @param original
 * @param noisy
 * @param cgp_archive
 * @param pred_archive
 */
void fitness_init(img_image_t original, img_image_t noisy,
    archive_t cgp_archive, archive_t pred_archive)
{
    assert(original->width == noisy->width);
    assert(original->height == noisy->height);
    assert(original->comp == noisy->comp);

    _original_image = original;
    _noisy_image_windows = img_split_windows(noisy);
    _cgp_archive = cgp_archive;
    _pred_archive = pred_archive;
    _psnr_coeficient = fitness_psnr_coeficient(_noisy_image_windows->size);
    _cgp_evals = 0;

    if (can_use_simd()) {
        img_split_windows_simd(noisy, _noisy_image_simd);
    }
}


/**
 * Deinitialize fitness module internals
 */
void fitness_deinit()
{
    img_windows_destroy(_noisy_image_windows);

    for (int i = 0; i < WINDOW_SIZE; i++) {
        free(_noisy_image_simd[i]);
    }
}


/**
 * Returns number of performed CGP evaluations
 */
long fitness_get_cgp_evals()
{
    return _cgp_evals;
}


/**
 * Filters image using given filter. Caller is responsible for freeing
 * the filtered image
 *
 * @param  chr
 * @return fitness value
 */
img_image_t fitness_filter_image(ga_chr_t chr)
{
    img_image_t filtered = img_create(_original_image->width, _original_image->height,
        _original_image->comp);

    for (int i = 0; i < _noisy_image_windows->size; i++) {
        img_window_t *w = &_noisy_image_windows->windows[i];

        cgp_value_t *inputs = w->pixels;
        cgp_value_t output_pixel;
        cgp_get_output(chr, inputs, &output_pixel);

        img_set_pixel(filtered, w->pos_x, w->pos_y, output_pixel);
    }

    return filtered;
}


/**
 * Calculates difference between original and filtered pixel
 *
 * @param  chr
 * @param  w
 * @return
 */
int _fitness_get_diff(ga_chr_t chr, img_window_t *w)
{
    cgp_value_t *inputs = w->pixels;
    cgp_value_t output_pixel;
    cgp_get_output(chr, inputs, &output_pixel);
    return output_pixel - img_get_pixel(_original_image, w->pos_x, w->pos_y);
}


double _fitness_get_sqdiffsum_scalar(ga_chr_t chr)
{
    double sum = 0;
    for (int i = 0; i < _noisy_image_windows->size; i++) {
        img_window_t *w = &_noisy_image_windows->windows[i];
        double diff = _fitness_get_diff(chr, w);
        sum += diff * diff;
    }
    #pragma omp atomic
        _cgp_evals += _noisy_image_windows->size;
    return sum;
}


double _fitness_get_sqdiffsum_simd(ga_chr_t chr, img_pixel_t *original, img_pixel_t *noisy[WINDOW_SIZE], int data_length)
{
    fitness_simd_func_t func = NULL;
    int block_size = 0;
    double sum = 0;

    #ifdef AVX2
        if(can_use_intel_core_4th_gen_features()) {
            func = _fitness_get_sqdiffsum_avx;
            block_size = FITNESS_AVX2_STEP;
        }
    #endif

    #ifdef SSE2
        if(can_use_sse2()) {
            func = _fitness_get_sqdiffsum_sse;
            block_size = FITNESS_SSE2_STEP;
        }
    #endif

    assert(func != NULL);

    int offset = 0;
    int unaligned_bytes = data_length % block_size;
    data_length -= unaligned_bytes;

    for (; offset < data_length; offset += block_size) {
        sum += func(original, noisy, chr, offset, block_size);
        #pragma omp atomic
            _cgp_evals += block_size;
    }

    // fix image data not fitting into register
    // offset is set correctly here - it points to first pixel after
    // aligned data (we subtracted no. unaligned bytes before)
    if (unaligned_bytes > 0) {
        sum += func(original, noisy, chr, offset, unaligned_bytes);
        #pragma omp atomic
            _cgp_evals += unaligned_bytes;
    }

    return sum;
}


/**
 * Evaluates CGP circuit fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_cgp(ga_chr_t chr)
{
    double sum = 0;

    if(can_use_simd()) {
        sum = _fitness_get_sqdiffsum_simd(chr, _original_image->data,
            _noisy_image_simd, _noisy_image_windows->size);

    } else {
        sum = _fitness_get_sqdiffsum_scalar(chr);
    }

    return _psnr_coeficient / sum;
}


/**
 * Evaluates CGP circuit fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_or_predict_cgp(ga_chr_t chr)
{
    if (_pred_archive && _pred_archive->stored > 0) {
        return fitness_predict_cgp(chr, arc_get(_pred_archive, 0));
    } else {
        return fitness_eval_cgp(chr);
    }
}


double _fitness_predict_cgp_scalar(ga_chr_t cgp_chr, pred_genome_t predictor)
{
    double sum = 0;

    for (int i = 0; i < predictor->used_pixels; i++) {
        // fetch window specified by predictor
        pred_gene_t index = predictor->pixels[i];
        assert(index < _noisy_image_windows->size);
        img_window_t *w = &_noisy_image_windows->windows[index];

        int diff = _fitness_get_diff(cgp_chr, w);
        sum += diff * diff;
    }

    #pragma omp atomic
        _cgp_evals += predictor->used_pixels;

    return sum;
}


/**
 * Predictes CGP circuit fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_predict_cgp_by_genome(ga_chr_t cgp_chr, pred_genome_t predictor)
{
    // PSNR coefficcient is different here (less pixels are used)
    double coef = fitness_psnr_coeficient(predictor->used_pixels);
    double sum = 0;

    if (can_use_simd()) {
        sum = _fitness_get_sqdiffsum_simd(cgp_chr, predictor->original_simd,
            predictor->pixels_simd, predictor->used_pixels);

    } else {
        sum = _fitness_predict_cgp_scalar(cgp_chr, predictor);
    }

    return coef / sum;
}


/**
 * Predictes CGP circuit fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_predict_cgp(ga_chr_t cgp_chr, ga_chr_t pred_chr)
{
    pred_genome_t predictor = (pred_genome_t) pred_chr->genome;
    return fitness_predict_cgp_by_genome(cgp_chr, predictor);
}


/**
 * Evaluates predictor fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_predictor_genome(pred_genome_t predictor)
{
    double sum = 0;
    for (int i = 0; i < _cgp_archive->stored; i++) {
        ga_chr_t cgp_chr = arc_get(_cgp_archive, i);
        double predicted = fitness_predict_cgp_by_genome(cgp_chr, predictor);
        sum += fabs(cgp_chr->fitness - predicted);
    }
    return sum / _cgp_archive->stored;
}


/**
 * Evaluates predictor fitness
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_predictor(ga_chr_t pred_chr)
{
    pred_genome_t predictor = (pred_genome_t) pred_chr->genome;
    return fitness_eval_predictor_genome(predictor);
}


/**
 * Evaluates circular predictor fitness, using PRED_CIRCULAR_TRIES to
 * determine best offset
 *
 * @param  chr
 * @return fitness value
 */
ga_fitness_t fitness_eval_circular_predictor(ga_chr_t pred_chr)
{
    pred_genome_t predictor = (pred_genome_t) pred_chr->genome;
    int best_offset = predictor->_circular_offset;
    ga_fitness_t best_fitness = fitness_eval_predictor_genome(predictor);

    for (int i = 0; i < PRED_CIRCULAR_TRIES; i++) {
        // generate new phenotype
        int offset = rand_urange(0, pred_get_max_length() - 1);
        predictor->_circular_offset = offset;
        pred_calculate_phenotype(predictor);

        // calculate predictor fitness
        ga_fitness_t fit = fitness_eval_predictor_genome(predictor);

        // if it is better, store it
        if (ga_is_better(PRED_PROBLEM_TYPE, fit, best_fitness)) {
            best_offset = offset;
            best_fitness = fit;
        }
    }

    // set predictor to best found phenotype
    if (predictor->_circular_offset != best_offset) {
        predictor->_circular_offset = best_offset;
        pred_calculate_phenotype(predictor);
    }
    return best_fitness;
}


        /*
        if (_genome_repeated_subtype == circular) {
            int best_offset = 0;
            ga_fitness_t best_fitness = 0;

            for (int i = 0; i < PRED_CIRCULAR_TRIES; i++) {
                genome->_circular_offset = rand_urange(0, _max_genome_length - 1);
                _pred_calculate_repeated_phenotype(genome);
                ga_fitness_t fit = fitness_eval_predictor_genome(genome);
                if (i == 0 || ga_is_better(PRED_PROBLEM_TYPE, fit, best_fitness)) {
                    best_offset = genome->_circular_offset;
                    best_fitness = fit;
                }
                printf("%p #%d: %u; %.10g\n", genome, i, genome->_circular_offset, fit);
            }
            genome->_circular_offset = best_offset;

        } else {
            genome->_circular_offset = 0;
        }
        */


/**
 * Fills simd-friendly predictor arrays with correct image data
 * @param  genome
 */
void fitness_prepare_predictor_for_simd(pred_genome_t predictor)
{
    for (int i = 0; i < predictor->used_pixels; i++) {
        pred_gene_t index = predictor->pixels[i];
        assert(index < _noisy_image_windows->size);

        predictor->original_simd[i] = _original_image->data[index];
        for (int w = 0; w < WINDOW_SIZE; w++) {
            predictor->pixels_simd[w][i] = _noisy_image_simd[w][index];
        }
    }
}
