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

#include "cgp_core.h"
#include "../random.h"


typedef struct {
    int size;
    int *values;
} int_array;


static int_array _allowed_gene_vals[CGP_COLS];
static int _mutation_rate;
static ga_fitness_func_t _fitness_func;

#ifdef CGP_LIMIT_FUNCS
    static int _allowed_functions_list[] = {
        c255,
        identity,
        add,
        add_sat,
        inversion,
        max,
        min
    };

    static int_array _allowed_functions = {
        .size = 7,
        .values = _allowed_functions_list
    };
#endif


#ifdef TEST_RANDOMIZE
    #define TEST_RANDOMIZE_PRINTF(...) printf(__VA_ARGS__)
#else
    #define TEST_RANDOMIZE_PRINTF(...) /* printf(__VA_ARGS__) */
#endif



/**
 * Initialize CGP internals
 */
void cgp_init(int mutation_rate, ga_fitness_func_t fitness_func)
{
    _mutation_rate = mutation_rate;
    _fitness_func = fitness_func;

    // calculate allowed values of node inputs in each column
    for (int x = 0; x < CGP_COLS; x++) {
        // range of outputs which can be connected to node in i-th column
        // maximum is actually by 1 larger than maximum allowed value
        // (so there is `val < maximum` in the for loop below instead of `<=`)
        int minimum = CGP_ROWS * (x - CGP_LBACK) + CGP_INPUTS;
        if (minimum < CGP_INPUTS) minimum = CGP_INPUTS;
        int maximum = CGP_ROWS * x + CGP_INPUTS;

        int size = CGP_INPUTS + maximum - minimum;
        _allowed_gene_vals[x].size = size;
        _allowed_gene_vals[x].values = (int*) malloc(sizeof(int) * size);

        int key = 0;
        // primary inputs
        for (int val = 0; val < CGP_INPUTS; val++, key++) {
            _allowed_gene_vals[x].values[key] = val;
        }

        // nodes to the left
        for (int val = minimum; val < maximum; val++, key++) {
            _allowed_gene_vals[x].values[key] = val;
        }
    }


#ifdef TEST_INIT
    for (int x = 0; x < CGP_COLS; x++) {
        printf ("x = %d: ", x);
        for (int y = 0; y < _allowed_gene_vals[x].size; y++) {
            if (y > 0) printf(", ");
            printf ("%d", _allowed_gene_vals[x].values[y]);
        }
        printf("\n");
    }
#endif
}


/**
 * Deinitialize CGP internals
 */
void cgp_deinit()
{
    for (int x = 0; x < CGP_COLS; x++) {
        free(_allowed_gene_vals[x].values);
    }
}


/**
 * Create a new cgp population with given size
 * @param  mutation rate (in number of genes)
 * @param  population size
 * @return
 */
ga_pop_t cgp_init_pop(int pop_size)
{
    /* prepare methods vector */
    ga_func_vect_t methods = {
        .alloc_genome = cgp_alloc_genome,
        .free_genome = cgp_free_genome,
        .init_genome = cgp_randomize_genome,

        .fitness = _fitness_func,
        .offspring = cgp_offspring,
    };

    /* initialize GA */
    ga_pop_t pop = ga_create_pop(pop_size, CGP_PROBLEM_TYPE, methods);
    return pop;
}


/* chromosome *****************************************************************/


/**
 * Allocates memory for new CGP genome
 * @param chromosome
 */
void* cgp_alloc_genome()
{
    return malloc(sizeof(struct cgp_genome));
}


/**
 * Initializes CGP genome to random values
 * @param chromosome
 */
int cgp_randomize_genome(ga_chr_t chromosome)
{
    cgp_genome_t genome = (cgp_genome_t) chromosome->genome;

    for (int i = 0; i < CGP_CHR_LENGTH; i++) {
        cgp_randomize_gene(genome, i);
    }

    cgp_find_active_blocks(chromosome);
    chromosome->has_fitness = false;

    return 0;
}


/**
 * Deinitialize CGP genome
 * @param  chromosome
 * @return
 */
void cgp_free_genome(void *genome)
{
    free(genome);
}


/* mutation *******************************************************************/


/**
 * Replace gene on given locus with random alele
 * @param chr
 * @param gene
 * @return whether active node was changed or not (phenotype has changed)
 */
bool cgp_randomize_gene(cgp_genome_t genome, int gene)
{
    if (gene >= CGP_CHR_LENGTH)
        return false;

    if (gene < CGP_CHR_OUTPUTS_INDEX) {
        // mutating node input or function
        int node_index = gene / 3;
        int gene_index = gene % 3;
        int col = cgp_node_col(node_index);

        TEST_RANDOMIZE_PRINTF("gene %u, node %u, gene %u, col %u\n", gene, node_index, gene_index, col);

        if (gene_index == CGP_FUNC_INPUTS) {
            // mutating function
            #ifdef CGP_LIMIT_FUNCS
                genome->nodes[node_index].function = (cgp_func_t) rand_schoice(_allowed_functions.size, _allowed_functions.values);
            #else
                genome->nodes[node_index].function = (cgp_func_t) rand_range(0, CGP_FUNC_COUNT - 1);
            #endif
            TEST_RANDOMIZE_PRINTF("func 0 - %u\n", CGP_FUNC_COUNT - 1);
            return genome->nodes[node_index].is_active;

        } else {
            // mutating input
            genome->nodes[node_index].inputs[gene_index] = rand_schoice(_allowed_gene_vals[col].size, _allowed_gene_vals[col].values);
            TEST_RANDOMIZE_PRINTF("input choice from %u\n", _allowed_gene_vals[col].size);
            return genome->nodes[node_index].is_active;
        }

    } else {
        // mutating primary output connection
        int index = gene - CGP_CHR_OUTPUTS_INDEX;
        genome->outputs[index] = rand_range(CGP_INPUTS, CGP_INPUTS + CGP_NODES - 1);
        TEST_RANDOMIZE_PRINTF("out %u - %u\n", CGP_INPUTS, CGP_INPUTS + CGP_NODES - 1);
        return true;
    }
}


/**
 * Mutate given chromosome
 * @param chr
 */
void cgp_mutate_chr(ga_chr_t chromosome)
{
    assert(_mutation_rate <= CGP_CHR_LENGTH);
    cgp_genome_t genome = (cgp_genome_t) chromosome->genome;

    int genes_to_change = rand_range(0, _mutation_rate);
    for (int i = 0; i < genes_to_change; i++) {
        int gene = rand_range(0, CGP_CHR_LENGTH - 1);
        cgp_randomize_gene(genome, gene);
    }

    cgp_find_active_blocks(chromosome);
    chromosome->has_fitness = false;
}


/**
 * Replace chromosome genes with genes from other chromomose
 * @param  chr
 * @param  replacement
 * @return
 */
void cgp_copy_genome(void *_dst, void *_src)
{
    cgp_genome_t dst = (cgp_genome_t) _dst;
    cgp_genome_t src = (cgp_genome_t) _src;

    memcpy(dst->nodes, src->nodes, sizeof(cgp_node_t) * CGP_NODES);
    memcpy(dst->outputs, src->outputs, sizeof(int) * CGP_OUTPUTS);
}


#define SWAP(A, B) (((A & 0x0F) << 4) | ((B & 0x0F)))
#define ADD_SAT(A, B) ((A > 0xFF - B) ? 0xFF : A + B)
#define MAX(A, B) ((A > B) ? A : B)
#define MIN(A, B) ((A < B) ? A : B)


/* evaluation *****************************************************************/


/**
 * Calculate output of given chromosome and inputs
 * @param chr
 */
void cgp_get_output(ga_chr_t chromosome, cgp_value_t *inputs, cgp_value_t *outputs)
{
    cgp_genome_t genome = (cgp_genome_t) chromosome->genome;
    cgp_value_t inner_outputs[CGP_INPUTS + CGP_NODES];

    // copy primary inputs to working array
    memcpy(inner_outputs, inputs, sizeof(cgp_value_t) * CGP_INPUTS);

    for (int i = 0; i < CGP_NODES; i++) {
        cgp_node_t *n = &(genome->nodes[i]);

        // skip inactive blocks
        if (!n->is_active) continue;

        cgp_value_t A = inner_outputs[n->inputs[0]];
        cgp_value_t B = inner_outputs[n->inputs[1]];
        cgp_value_t Y;

        switch (n->function) {
            case c255:          Y = 255;            break;
            case identity:      Y = A;              break;
            case inversion:     Y = 255 - A;        break;
            case b_or:          Y = A | B;          break;
            case b_not1or2:     Y = ~A | B;         break;
            case b_and:         Y = A & B;          break;
            case b_nand:        Y = ~(A & B);       break;
            case b_xor:         Y = A ^ B;          break;
            case rshift1:       Y = A >> 1;         break;
            case rshift2:       Y = A >> 2;         break;
            case swap:          Y = SWAP(A, B);     break;
            case add:           Y = A + B;          break;
            case add_sat:       Y = ADD_SAT(A, B);  break;
            case avg:           Y = (A + B) >> 1;   break;
            case max:           Y = MAX(A, B);      break;
            case min:           Y = MIN(A, B);      break;
            default:            abort();
        }

        inner_outputs[CGP_INPUTS + i] = Y;
    }

    for (int i = 0; i < CGP_OUTPUTS; i++) {
        outputs[i] = inner_outputs[genome->outputs[i]];
    }

#ifdef TEST_EVAL
    for (int i = 0; i < CGP_INPUTS + CGP_NODES; i++) {
        printf("%c: %u = %u\n", (i < CGP_INPUTS? 'I' : 'N'), i, inner_outputs[i]);
    }
    for (int i = 0; i < CGP_OUTPUTS; i++) {
        printf("O: %u = %u\n", i, outputs[i]);
    }
#endif
}


/**
 * Finds which blocks are active.
 * @param chromosome
 * @param active
 */
void cgp_find_active_blocks(ga_chr_t chromosome)
{
    cgp_genome_t genome = (cgp_genome_t) chromosome->genome;

    // first mark all nodes as inactive
    for (int i = CGP_NODES - 1; i >= 0; i--) {
        genome->nodes[i].is_active = false;
    }

    // mark inputs of primary outputs as active
    for (int i = 0; i < CGP_OUTPUTS; i++) {
        int index = genome->outputs[i] - CGP_INPUTS;
        // index may be negative (primary input), so do the check...
        if (index >= 0) {
            genome->nodes[index].is_active = true;
        }
    }

    // then walk nodes backwards and mark inputs of active nodes
    // as active nodes
    for (int i = CGP_NODES - 1; i >= 0; i--) {
        if (!genome->nodes[i].is_active) continue;
        cgp_node_t *n = &(genome->nodes[i]);

        for (int k = 0; k < CGP_FUNC_INPUTS; k++) {
            int index = n->inputs[k] - CGP_INPUTS;
            // index may be negative (primary input), so do the check...
            if (index >= 0) {
                genome->nodes[index].is_active = true;
            }
        }
    }
}



/* population *****************************************************************/


/**
 * Create new generation
 * @param pop
 * @param mutation_rate
 */
void cgp_offspring(ga_pop_t pop)
{
    ga_chr_t parent = pop->best_chromosome;

    #pragma omp parallel for
    for (int i = 0; i < pop->size; i++) {
        ga_chr_t chr = pop->chromosomes[i];
        if (chr == parent) continue;
        ga_copy_chr(chr, parent, cgp_copy_genome);
        cgp_mutate_chr(chr);
    }
}
