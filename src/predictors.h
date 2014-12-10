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


#include "ga.h"
#include "image.h"


static const ga_problem_type_t PRED_PROBLEM_TYPE = minimize;


/* genome types ***************************************************************/

typedef unsigned int pred_gene_t;
typedef pred_gene_t* pred_gene_array_t;

struct pred_genome {
    /* genotype */
    pred_gene_array_t _genes;

    /*
        for permuted genotype: which gene values were already used?
        for repeated genotype: used to generate phenotype to avoid duplicities
    */
    bool *_used_values;

    /* how many pixels are in the phenotype */
    unsigned int used_pixels;

    /* for circular repeated genotype: phenotype starting locus */
    unsigned int _circular_offset;

    /* phenotype */
    unsigned int *pixels;

    /* simd-friendly prepared image data */
    img_pixel_t *original_simd;
    img_pixel_t *pixels_simd[WINDOW_SIZE];
};
typedef struct pred_genome* pred_genome_t;


/* metadata types *************************************************************/


typedef enum {
    permuted,
    repeated,
    circular,
} pred_genome_type_t;


typedef struct {
    /* genome type */
    pred_genome_type_t genome_type;

    /* maximal gene value (inclusive) */
    pred_gene_t max_gene_value;

    /* genotype length */
    unsigned int genotype_length;

    /* genotype used portion length */
    unsigned int genotype_used_length;

    /* relative mutation rate */
    float mutation_rate;

    /* relative number of elite and crossovered children */
    float offspring_elite;
    float offspring_combine;
} pred_metadata_t;


/* initialization *************************************************************/


/**
 * Initialize predictor internals
 */
void pred_init(pred_metadata_t *metadata);


/**
 * Create a new predictors population with given size
 * @param  size
 * @param  problem-specific methods
 * @return
 */
ga_pop_t pred_init_pop(int pop_size);


/**
 * Allocates memory for new predictor genome
 * @return pointer to newly allocated genome
 */
void* pred_alloc_genome();


/**
 * Deinitialize predictor genome
 * @param  genome
 * @return
 */
void pred_free_genome(void *genome);


/**
 * Recalculates phenotype for repeated genotype
 */
void pred_calculate_phenotype(pred_genome_t genome);


/**
 * Recalculates phenotype for repeated genotype in whole population
 */
void pred_pop_calculate_phenotype(ga_pop_t pop);


/**
 * Initializes predictor genome to random values
 * @param chromosome
 */
int pred_randomize_genome(ga_chr_t chromosome);


/**
 * Replace chromosome genes with genes from other chromomose
 * @param  chr
 * @param  replacement
 * @return
 */
void pred_copy_genome(void *_dst, void *_src);


/**
 * Genome mutation function
 *
 * Recalculates phenotype if necessary
 *
 * @param  genes
 * @return
 */
void pred_mutate(pred_genome_t genes);


/**
 * Create new generation
 * @param pop
 * @param mutation_rate
 */
void pred_offspring(ga_pop_t pop);


/**
 * Sets current genome length.
 * @param new_length
 */
void pred_set_length(int new_length);


/**
 * Returns current genome length.
 */
int pred_get_length();


/**
 * Returns maximal genome length.
 */
int pred_get_max_length();
