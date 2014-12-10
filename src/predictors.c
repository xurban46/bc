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


#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "random.h"
#include "fitness.h"
#include "predictors.h"


static pred_metadata_t *_metadata;


#ifdef PRED_DEBUG
    #define VERBOSELOG(s, ...) fprintf(stderr, s "\n", __VA_ARGS__)
#else
    #define VERBOSELOG(s, ...)
#endif


enum _offspring_op {
    random_mutant,
    crossover_product,
    keep_intact,
};


/* initialization *************************************************************/


/**
 * Initialize predictor internals
 */
void pred_init(pred_metadata_t *metadata)
{
    _metadata = metadata;
    assert(metadata->genotype_used_length <= metadata->genotype_length);
}


/**
 * Create a new predictors population with given size
 * @param  size
 * @param  problem-specific methods
 * @return
 */
ga_pop_t pred_init_pop(int pop_size)
{
    ga_fitness_func_t fitfunc = fitness_eval_predictor;
    if (_metadata->genome_type == circular) {
        fitfunc = fitness_eval_circular_predictor;
    }

    /* prepare methods vector */
    ga_func_vect_t methods = {
        .alloc_genome = pred_alloc_genome,
        .free_genome = pred_free_genome,
        .init_genome = pred_randomize_genome,

        .fitness = fitfunc,
        .offspring = pred_offspring,
    };

    /* initialize GA */
    ga_pop_t pop = ga_create_pop(pop_size, PRED_PROBLEM_TYPE, methods);
    return pop;
}


/**
 * Allocates memory for new predictor genome
 * @return pointer to newly allocated genome
 */
void* pred_alloc_genome()
{
    pred_genome_t genome = (pred_genome_t) malloc(sizeof(struct pred_genome));
    if (genome == NULL) {
        return NULL;
    }

    genome->_genes = (pred_gene_t*) malloc(sizeof(pred_gene_t) * _metadata->genotype_length);
    if (genome->_genes == NULL) {
        free(genome);
        return NULL;
    }

    /*
        For permuted genotype: holds which values has been used.
        For repeated genotype: used for calculating genotype (also holds
            which values has been used in phenotype)
     */
    genome->_used_values = (bool*) malloc(sizeof(bool) * (_metadata->max_gene_value + 1));
    if (genome->_used_values == NULL) {
        free(genome->_genes);
        free(genome);
        return NULL;
    }

    if (_metadata->genome_type == permuted) {

        // one-to-one mapping
        // field genome->used_pixels must be updated manually after every change!
        genome->pixels = genome->_genes;

    } else {
        // phenotype is different
        genome->pixels = (unsigned int*) malloc(sizeof(unsigned int) * _metadata->genotype_length);
        if (genome->pixels == NULL) {
            free(genome->_genes);
            free(genome);
            return NULL;
        }
    }

    // allocate space for simd-friendly data using calloc, since we
    // want initialized padding bits
    if (can_use_simd()) {
        int size = _metadata->genotype_length;
        int padding = SIMD_PADDING_BYTES - (size % SIMD_PADDING_BYTES);
        genome->original_simd = (img_pixel_t *) calloc(size + padding, sizeof(img_pixel_t));
        if (genome->original_simd == NULL) {
            // TODO: implement proper deallocation on failure
            // Whatever, if it fails, everything fails, OS cleans it, so...
            return NULL;
        }

        for (int i = 0; i < WINDOW_SIZE; i++) {
            genome->pixels_simd[i] = (img_pixel_t *) calloc(size + padding, sizeof(img_pixel_t));
            if (genome->pixels_simd[i] == NULL) {
                return NULL;
            }
        }
    }

    return genome;
}


/**
 * Deinitialize predictor genome
 * @param  genome
 * @return
 */
void pred_free_genome(void *_genome)
{
    pred_genome_t genome = (pred_genome_t) _genome;
    free(genome->_used_values);
    free(genome->_genes);
    if (_metadata->genome_type != permuted) free(genome->pixels);
    if (can_use_simd()) {
        free(genome->original_simd);
        for (int i = 0; i < WINDOW_SIZE; i++) {
            free(genome->pixels_simd[i]);
        }
    }
    free(genome);
}


/**
 * Calculates real index of given gene in circular genome
 */
int _pred_get_circular_index(pred_genome_t genome, int index)
{
    int real = (genome->_circular_offset + index) % _metadata->genotype_length;
    if (real < 0) real += _metadata->genotype_length;
    return real;
}


void _pred_calculate_repeated_phenotype(pred_genome_t genome)
{
    // clear used values helper
    memset(genome->_used_values, 0, sizeof(bool) * (_metadata->max_gene_value + 1));

    int pheno_index = 0;
    for (int geno_index = 0; geno_index < _metadata->genotype_used_length; geno_index++) {
        int locus = _pred_get_circular_index(genome, geno_index);
        pred_gene_t value = genome->_genes[locus];
        if (genome->_used_values[value]) {
            continue;

        } else {
            genome->_used_values[value] = true;
            genome->pixels[pheno_index] = value;
            pheno_index++;
        }
    }
    genome->used_pixels = pheno_index;

    if (can_use_simd()) {
        fitness_prepare_predictor_for_simd(genome);
    }
}


/**
 * Recalculates phenotype for repeated genotype
 */
void pred_calculate_phenotype(pred_genome_t genome)
{
    if (_metadata->genome_type == permuted) {
        genome->used_pixels = _metadata->genotype_used_length;

    } else {
        _pred_calculate_repeated_phenotype(genome);
    }

    if (can_use_simd()) {
        fitness_prepare_predictor_for_simd(genome);
    }
}


/**
 * Recalculates phenotype for repeated genotype in whole population
 */
void pred_pop_calculate_phenotype(ga_pop_t pop)
{
    for (int i = 0; i < pop->size; i++) {
        pred_genome_t genome = (pred_genome_t) pop->chromosomes[i]->genome;
        pred_calculate_phenotype(genome);
    }
}


/**
 * Initializes predictor genome to random values
 * @param chromosome
 */
int pred_randomize_genome(ga_chr_t chromosome)
{
    pred_genome_t genome = (pred_genome_t) chromosome->genome;

    if (_metadata->genome_type == permuted) {
        memset(genome->_used_values, 0, sizeof(bool) * (_metadata->max_gene_value + 1));
    }

    for (int i = 0; i < _metadata->genotype_length; i++) {
        pred_gene_t value = rand_urange(0, _metadata->max_gene_value);
        if (_metadata->genome_type == permuted) {
            // only unused is valid, so make corrections
            while(genome->_used_values[value]) {
                value = (value + 1) % (_metadata->max_gene_value + 1);
            };
            genome->_used_values[value] = true;
        }

        genome->_genes[i] = value;
    }

    genome->_circular_offset = 0;
    pred_calculate_phenotype(genome);
    return 0;
}


/**
 * Replace chromosome genes with genes from other chromomose
 * @param  chr
 * @param  replacement
 * @return
 */
void pred_copy_genome(void *_dst, void *_src)
{
    pred_genome_t dst = (pred_genome_t) _dst;
    pred_genome_t src = (pred_genome_t) _src;

    memcpy(dst->_genes, src->_genes, sizeof(pred_gene_t) * _metadata->genotype_length);
    memcpy(dst->_used_values, src->_used_values, sizeof(bool) * _metadata->max_gene_value);

    if (_metadata->genome_type == repeated || _metadata->genome_type == circular) {
        memcpy(dst->pixels, src->pixels, sizeof(pred_gene_t) * _metadata->genotype_length);
    }

    if (can_use_simd()) {
        memcpy(dst->original_simd, src->original_simd, sizeof(img_pixel_t) * src->used_pixels);
        for (int w = 0; w < WINDOW_SIZE; w++) {
            memcpy(dst->pixels_simd[w], src->pixels_simd[w], sizeof(img_pixel_t) * src->used_pixels);
        }
    }

    dst->used_pixels = src->used_pixels;
    dst->_circular_offset = src->_circular_offset;
}



/**
 * Genome mutation function
 *
 * Recalculates phenotype if necessary
 *
 * @param  genes
 * @return
 */
void pred_mutate(pred_genome_t genome)
{
    int max_changed_genes = _metadata->mutation_rate * _metadata->genotype_length;
    int genes_to_change = rand_range(0, max_changed_genes);

    for (int i = 0; i < genes_to_change; i++) {
        // choose mutated gene
        int gene = rand_range(0, _metadata->genotype_length - 1);
        pred_gene_t old_value = genome->_genes[gene];

        // generate new value
        pred_gene_t value = rand_urange(0, _metadata->max_gene_value);
        if (_metadata->genome_type == permuted) {
            // either unused or same value is valid, so make corrections
            while(genome->_used_values[value] && old_value != value) {
                value = (value + 1) % (_metadata->max_gene_value + 1);
            };
        }

        // rewrite gene
        genome->_genes[gene] = value;
        genome->_used_values[value] = true;
    }

    pred_calculate_phenotype(genome);
}


void _find_elites(ga_pop_t pop, int count, enum _offspring_op ops[])
{
    for (; count > 0; count--) {
        ga_fitness_t best_fitness = ga_worst_fitness(pop->problem_type);
        int best_index = -1;

        // find best non-selected individual
        for (int i = 0; i < pop->size; i++) {
            if (ops[i] != keep_intact) {
                ga_chr_t chr = pop->chromosomes[i];
                if (ga_is_better(pop->problem_type, chr->fitness, best_fitness)) {
                    best_fitness = chr->fitness;
                    best_index = i;
                }
            }
        }

        // mark best found as elite
        if (best_index >= 0) {
            ops[best_index] = keep_intact;
        }
    }
}


void _tournament(ga_pop_t pop, ga_chr_t *winner, ga_chr_t red, ga_chr_t blue)
{
    if (ga_is_better_or_same(pop->problem_type, red->fitness, blue->fitness)) {
        *winner = red;
    } else {
        *winner = blue;
    }
}


void _crossover1p_repeated(pred_genome_t baby, pred_genome_t mom, pred_genome_t dad)
{
    const int split_point = rand_range(0, _metadata->genotype_length - 1);

    // copy from mom
    memcpy(baby->_genes, mom->_genes, sizeof(pred_gene_t) * split_point);

    // copy from dad
    memcpy(baby->_genes + split_point, dad->_genes + split_point,
        sizeof(pred_gene_t) * (_metadata->genotype_length - split_point));

    baby->_circular_offset = mom->_circular_offset;
}



void _crossover1p_permuted(pred_genome_t baby, pred_genome_t mom, pred_genome_t dad)
{
    const int split_point = rand_range(0, _metadata->genotype_length - 1);

    // first clear usage flags
    memset(baby->_used_values, 0, sizeof(bool) * (_metadata->max_gene_value + 1));

    // second copy everything we can from mom
    int geneIndex = 0;
    pred_gene_array_t parent_genes = mom->_genes;

    VERBOSELOG("Copying.");
    for (int i = 0; i < _metadata->genotype_length; i++) {
        pred_gene_t value = parent_genes[i];
        if (!baby->_used_values[value]) {
            baby->_genes[geneIndex] = value;
            baby->_used_values[value] = true;
            geneIndex++;
        }

        if (i == split_point) {
            VERBOSELOG("Switching to dad.");
            parent_genes = dad->_genes;
        }
    }

    VERBOSELOG("Finish with random values. Index: %d", geneIndex);
    // now create random values in place of duplicates
    for (; geneIndex < _metadata->genotype_length; geneIndex++) {
        pred_gene_t value = rand_urange(0, _metadata->max_gene_value);
        while(baby->_used_values[value]) {
            value = (value + 1) % (_metadata->max_gene_value + 1);
        };
        baby->_genes[geneIndex] = value;
        baby->_used_values[value] = true;
    }
}


void _create_combined(ga_pop_t pop, pred_genome_t children)
{
    ga_chr_t mom;
    ga_chr_t dad;
    int red, blue;

    red = rand_range(0, pop->size - 1);
    blue = rand_range(0, pop->size - 1);
    _tournament(pop, &mom, pop->chromosomes[red], pop->chromosomes[blue]);

    red = rand_range(0, pop->size - 1);
    blue = rand_range(0, pop->size - 1);
    _tournament(pop, &dad, pop->chromosomes[red], pop->chromosomes[blue]);

    VERBOSELOG("Making love.");
    pred_genome_t mom_genome = (pred_genome_t)mom->genome;
    pred_genome_t dad_genome = (pred_genome_t)dad->genome;

    if (_metadata->genome_type == permuted) {
        _crossover1p_permuted(children, mom_genome, dad_genome);

    } else {
        _crossover1p_repeated(children, mom_genome, dad_genome);
    }

    /*
    for (int i = 0; i < _metadata->genotype_length; i++) {
        printf("%4x ", mom_genome->_genes[i]);
    }
    printf("\n");
    for (int i = 0; i < _metadata->genotype_length; i++) {
        printf("%4x ", dad_genome->_genes[i]);
    }
    printf("\n");
    for (int i = 0; i < _metadata->genotype_length; i++) {
        printf("%4x ", children[i]);
    }
    printf("\n");
    */

    VERBOSELOG("Mutating newly created child.");
    pred_mutate(children);

    /*
    for (int i = 0; i < _metadata->genotype_length; i++) {
        printf("%4x ", children[i]);
    }
    printf("\n\n");
    */
}


/**
 * Create new generation
 * @param pop
 * @param mutation_rate
 */
void pred_offspring(ga_pop_t pop)
{
    // number of children copied and crossovered from parents
    const int elite_count = ceil(pop->size * _metadata->offspring_elite);
    const int crossover_count = ceil(pop->size * _metadata->offspring_combine);
    assert(elite_count + crossover_count <= pop->size);

    // this array describes how to create each individual
    enum _offspring_op child_type[pop->size];
    memset(child_type, random_mutant, sizeof(enum _offspring_op) * pop->size);

    // find which individuals will be kept intact
    _find_elites(pop, elite_count, child_type);

    // find which individuals will be replaced from parents
    // `i < pop->size` is already guarded by assert above
    int crossover_set = 0;
    for (int i = 0; crossover_set < crossover_count; i++) {
        if (child_type[i] != keep_intact) {
            child_type[i] = crossover_product;
            crossover_set++;
        }
    }

    // create new population
    #pragma omp parallel for
    for (int i = 0; i < pop->size; i++) {
        VERBOSELOG("Processing child %d.", i);

        // copy elites
        if (child_type[i] == keep_intact) {
            VERBOSELOG("Child %d is elite.", i);
            ga_copy_chr(pop->children[i], pop->chromosomes[i], pred_copy_genome);

        // if there are any combined children to make, do it
        } else if (child_type[i] == crossover_product) {

            VERBOSELOG("Child %d is crossover.", i);

            pred_genome_t target_genome = (pred_genome_t) pop->children[i]->genome;
            _create_combined(pop, target_genome);
            pop->children[i]->has_fitness = false;

        // otherwise create random mutant
        } else {

            VERBOSELOG("Child %d is random.", i);
            pred_randomize_genome(pop->children[i]);
            pop->children[i]->has_fitness = false;
        }
    }

    // switch new and old population
    ga_chr_t *tmp = pop->chromosomes;
    pop->chromosomes = pop->children;
    pop->children = tmp;
}


/**
 * Sets current genome length.
 * @param new_length
 */
void pred_set_length(int new_length)
{
    if (new_length > _metadata->genotype_length) {
        _metadata->genotype_used_length = _metadata->genotype_length;

    } else if (new_length > 0) {
        _metadata->genotype_used_length = new_length;
    }
}


/**
 * Returns current genome length.
 */
int pred_get_length()
{
    return _metadata->genotype_used_length;
}


/**
 * Returns maximal genome length.
 */
int pred_get_max_length()
{
    return _metadata->genotype_length;
}
