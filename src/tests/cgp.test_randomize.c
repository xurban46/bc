/**
 * Tests CGP initialization.
 * Compile with -DTEST_RANDOMIZE
 */


#include <stdlib.h>

#include "../cgp.h"


int main(int argc, char const *argv[])
{
    cgp_init(0, NULL);

    cgp_genome_t genome = (cgp_genome_t) malloc(sizeof(struct cgp_genome));
    struct ga_chr chr = {
        .genome = genome
    };

    cgp_randomize_genome(&chr);

    free(genome);
    cgp_deinit();
}
