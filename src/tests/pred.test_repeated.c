/**
 * Tests CGP initialization.
 */

#include "../predictors.h"


int main(int argc, char const *argv[])
{
    pred_init(10, 10, 10, 0, 0, 0, repeated);

    pred_gene_t genes[10] = {
        1, 2, 3, 4, 1, 1, 5, 6, 9, 10
    };

    bool used_values[10] = {};
    unsigned int pixels[10] = {};

    struct pred_genome genome = {
        ._genes = &genes[0],
        ._used_values = &used_values[0],
        .pixels = &pixels[0],
    };

    pred_calculate_phenotype(&genome);

    printf("Genotype: ");
    for (int i = 0; i < 10; i++) {
        if (i) printf(", ");
        printf("%d", genes[i]);
    }
    printf("\n");

    printf("Phenotype: ");
    for (int i = 0; i < genome.used_pixels; i++) {
        if (i) printf(", ");
        printf("%d", pixels[i]);
    }
    printf("\n");
}
