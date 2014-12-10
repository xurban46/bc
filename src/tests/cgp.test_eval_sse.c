/**
 * Tests CGP evaluation = calculation of the outputs.
 * Compile with -DTEST_EVAL_SSE2 -DSSE2
 * Source files cgp_core.c cgp_dump.c cgp_sse.c cpu.c ga.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>

#include "../cpu.h"
#include "../cgp.h"
#include "../cgp_sse.h"



#define in1 {0, 1, 2, 3, 4, 5, 6, 7, 8}
#define in4 in1, in1, in1, in1
#define in16 in4, in4, in4, in4
#define in32 in16, in16

#define rep4(x) (x), (x), (x), (x)
#define rep16(x) rep4((x)), rep4((x)), rep4((x)), rep4((x))


int main(int argc, char const *argv[])
{
    // pre-flight check
    if (!can_use_sse2()) {
        fprintf(stderr, "%s", "SSE2 is not supported.\n");
        exit(1);
    }

    unsigned char _inputs[CGP_INPUTS][16] = {
        {rep16(0)},
        {rep16(1)},
        {rep16(2)},
        {rep16(3)},
        {rep16(4)},
        {rep16(5)},
        {rep16(6)},
        {rep16(7)},
        {rep16(8)},
    };

    __m128i_aligned inputs[CGP_INPUTS];
    __m128i_aligned outputs[CGP_OUTPUTS];

    for (int i = 0; i < CGP_INPUTS; i++) {
        inputs[i] = _mm_load_si128((__m128i*)(&_inputs[i]));
    };

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
            n->is_active = true;
        }
    }

    // define outputs
    genome->outputs[0] = 13;

    cgp_dump_chr_asciiart(&chr, stdout, false);
    putchar('\n');
    cgp_get_output_sse(&chr, inputs, outputs);

    free(genome);
    cgp_deinit();
}
