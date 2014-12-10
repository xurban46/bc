/**
 * Tests CGP evaluation = calculation of the outputs.
 * Compile with -DTEST_EVAL
 * Source files cgp_core.c cgp_dump.c ga.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "../cgp.h"


int main(int argc, char const *argv[])
{
    cgp_value_t inputs[CGP_INPUTS] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    cgp_value_t outputs[CGP_OUTPUTS] = {};

    cgp_init(0, NULL);

    cgp_genome_t genome = (cgp_genome_t) cgp_alloc_genome();
    struct ga_chr chr = {
        .genome = genome
    };

    // define nodes
    for (int x = 0; x < CGP_COLS; x++) {
        for (int y = 0; y < CGP_ROWS; y++) {
            int i = cgp_node_index(x, y);
            cgp_node_t *n = &(genome->nodes[i]);
            n->inputs[0] = i + CGP_INPUTS - y - y - 1;
            n->inputs[1] = i + CGP_INPUTS - CGP_ROWS;
            n->function = (cgp_func_t) (i % CGP_FUNC_COUNT);
        }
    }

    // define outputs
    genome->outputs[0] = 13;

    cgp_dump_chr_asciiart(&chr, stdout, false);
    putchar('\n');
    cgp_get_output(&chr, inputs, outputs);

    free(genome);
    cgp_deinit();
}
