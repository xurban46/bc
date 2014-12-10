/**
 * Tests CGP chromosome replacement.
 */


#include <stdlib.h>
#include <stdio.h>

#include "../cgp.h"


int main(int argc, char const *argv[])
{
    cgp_init();

    cgp_genome_t genome1 = (cgp_genome_t) malloc(sizeof(struct cgp_genome));
    struct ga_chr chr1 = {
        .genome = genome1
    };

    cgp_genome_t genome2 = (cgp_genome_t) malloc(sizeof(struct cgp_genome));
    struct ga_chr chr2 = {
        .genome = genome2
    };

    cgp_init_chr(&chr1);
    cgp_init_chr(&chr2);

    cgp_dump_chr_compat(&chr1, stdout);
    printf("\n");
    cgp_dump_chr_compat(&chr2, stdout);
    printf("\n");

    cgp_replace_chr(&chr2, &chr1);
    cgp_dump_chr_compat(&chr2, stdout);

    free(genome1);
    free(genome2);
    cgp_deinit();
}
