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

#include "../ga.h"
#include "cgp_config.h"


#define CGP_FUNC_INPUTS 2
#define CGP_NODES (CGP_COLS * CGP_ROWS)
#define CGP_CHR_OUTPUTS_INDEX ((CGP_FUNC_INPUTS + 1) * CGP_NODES)
#define CGP_CHR_LENGTH (CGP_CHR_OUTPUTS_INDEX + CGP_OUTPUTS)

static const ga_problem_type_t CGP_PROBLEM_TYPE = maximize;

typedef unsigned char cgp_value_t;

#define CGP_FUNC_COUNT 16
typedef enum
{
    c255 = 0,   // 255
    identity,   // a
    inversion,  // 255 - a
    b_or,       // a or b
    b_not1or2,  // (not a) or b
    b_and,      // a and b
    b_nand,     // not (a and b)
    b_xor,      // a xor b
    rshift1,    // a >> 1
    rshift2,    // a >> 2
    swap,       // a <-> b
    add,        // a + b
    add_sat,    // a +S b
    avg,        // (a + b) >> 1
    max,        // max(a, b)
    min,        // min(a, b)
} cgp_func_t;


/**
 * One CGP node (function block)
 */
typedef struct {
    int inputs[CGP_FUNC_INPUTS];
    cgp_func_t function;
    bool is_active;
} cgp_node_t;


/**
 * Chromosome
 */
struct cgp_genome {
    cgp_node_t nodes[CGP_COLS * CGP_ROWS];
    int outputs[CGP_OUTPUTS];
};
typedef struct cgp_genome* cgp_genome_t;


/**
 * Initialize CGP internals
 */
void cgp_init(int mutation_rate, ga_fitness_func_t fitness_func);


/**
 * Deinitialize CGP internals
 */
void cgp_deinit();


/**
 * Create a new CGP population with given size
 * @param  mutation rate (in number of genes)
 * @return
 */
ga_pop_t cgp_init_pop(int pop_size);


/**
 * Allocates memory for new CGP genome
 * @return pointer to newly allocated genome
 */
void* cgp_alloc_genome();


/**
 * Initializes CGP genome to random values
 * @param chromosome
 * @return 0 on success, other value on error
 */
int cgp_randomize_genome(ga_chr_t chromosome);


/**
 * Deinitialize CGP genome
 * @param  genome
 * @return
 */
void cgp_free_genome(void *genome);


/**
 * Replace chromosome genes with genes from other chromomose
 * @param  chr
 * @param  replacement
 * @return
 */
void cgp_copy_genome(void *_dst, void *_src);


/**
 * Replace gene on given locus with random alele
 * @param chr
 * @param gene
 * @return whether active node was changed or not (phenotype has changed)
 */
bool cgp_randomize_gene(cgp_genome_t genome, int gene);


/**
 * Mutate given chromosome
 * @param chr
 */
void cgp_mutate_chr(ga_chr_t chromosome);


/**
 * Replace chromosome genes with genes from other chromomose
 * @param  chr
 * @param  replacement
 * @return
 */
void cgp_replace_chr(ga_chr_t chromosome, ga_chr_t replacement);


/**
 * Calculate output of given chromosome and inputs
 * @param chr
 */
void cgp_get_output(ga_chr_t chromosome, cgp_value_t *inputs, cgp_value_t *outputs);


/**
 * Create new generation
 * @param pop
 * @param mutation_rate
 */
void cgp_offspring(ga_pop_t pop);


/**
 * Returns index of node in given column and row
 * @param  col
 * @param  row
 * @return
 */
static inline int cgp_node_index(int col, int row)
{
    return CGP_ROWS * col + row;
}


/**
 * Returns node column based on its index
 * @param  index
 * @return
 */
static inline int cgp_node_col(int index)
{
    return index / CGP_ROWS;
}


/**
 * Returns node row based on its index
 * @param  index
 * @return
 */
static inline int cgp_node_row(int index)
{
    return index % CGP_ROWS;
}


/**
 * Finds which blocks are active.
 * @param chromosome
 * @param active
 */
void cgp_find_active_blocks(ga_chr_t chromosome);
