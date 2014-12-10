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


#include <stdio.h>
#include <stdbool.h>

#include "utils.h"
#include "baldwin.h"
#include "predictors.h"


typedef enum
{
    cfg_ok = 0,
    cfg_err = 1,
    cfg_help = 2
} config_retval_t;


typedef enum
{
    simple_cgp = 0,
    predictors,
    baldwin,
} algorithm_t;


// multiple const to avoid "unused variable" warnings
static const char * const config_algorithm_names[] = {
    "cgp",
    "predictors",
    "baldwin"
};


typedef struct
{
    int max_generations;
    double target_fitness;
    algorithm_t algorithm;
    unsigned int random_seed;

    char input_image[MAX_FILENAME_LENGTH + 1];
    char noisy_image[MAX_FILENAME_LENGTH + 1];

    int cgp_mutate_genes;
    int cgp_population_size;
    int cgp_archive_size;

    float pred_size;
    float pred_initial_size;
    float pred_min_size;
    float pred_mutation_rate;
    float pred_offspring_elite;
    float pred_offspring_combine;
    int pred_population_size;
    pred_genome_type_t pred_genome_type;

    int bw_interval;
    bw_config_t bw_config;
    float bw_zero_increment_percent;
    float bw_decrease_increment_percent;
    float bw_increase_slow_increment_percent;
    float bw_increase_fast_increment_percent;

    int log_interval;
    char log_dir[MAX_FILENAME_LENGTH + 1];

} config_t;


static inline void print_help() {
    fputs(
        "Colearning in Coevolutionary Algorithms\n"
        "Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>\n"
        "\n"
        "Master Thesis\n"
        "2014/2015\n"
        "\n"
        "Supervisor: Ing. Michaela Sikulova <isikulova@fit.vutbr.cz>\n"
        "\n"
        "Faculty of Information Technologies\n"
        "Brno University of Technology\n"
        "http://www.fit.vutbr.cz/\n"
        "     _       _\n"
        "  __(.)=   =(.)__\n"
        "  \\___)     (___/\n"
        "\n"
        "To see system configuration just call:\n"
        "    ./coco\n"
        "\n"
        "\n"
        "To run evolution:\n"
        "    ./coco --original original.png --noisy noisy.png [options]\n"
        "\n"
        "Command line options:\n"
        "    --help, -h\n"
        "          Show this help and exit.\n"
        "\n"
        "Required:\n"
        "    --original FILE, -i FILE\n"
        "          Original image filename.\n"
        "    --noisy FILE, -n FILE\n"
        "          Noisy image filename.\n"
        "\n"
        "Optional:\n"
        "    --algorithm ALG, -a ALG\n"
        "          Evolution algorithm selection, one of {cgp|coev|baldwin},\n"
        "          default is \"predictors\".\n"
        "          - cgp: Simple CGP without any coevolution.\n"
        "          - coev: CGP coevoluting with fitness predictors of fixed size.\n"
        "          - baldwin: CGP coevoluting with fitness predictors of flexible size.\n"
        "\n"
        "    --random-seed NUM, -r ALG\n"
        "          PRNG seed value, default is obtained using gettimeofday() call.\n"
        "\n"
        "    --max-generations NUM, -g NUM\n"
        "          Stop after given number of CGP generations, default is 50000.\n"
        "\n"
        "    --target-psnr NUM, -g NUM\n"
        "          Stop after reaching given PSNR (0 to disable), default is 0.\n"
        "          If --target-fitness is specified, only one option is used.\n"
        "\n"
        "    --target-fitness NUM, -g NUM\n"
        "          Stop after reaching given fitness (0 to disable), default is 0.\n"
        "          Fitness can be obtained from PSNR as F = 10 ^ (PSNR / 10).\n"
        "          If --target-psnr is specified, only one option is used.\n"
        "\n"
        "    --log-dir DIR, -l DIR\n"
        "          Log (results) directory, default is \"cocolog\".\n"
        "\n"
        "    --log-interval NUM, -k NUM\n"
        "          Logging interval (in generations), default is 0.\n"
        "          If zero, only fitness changes are logged.\n"
        "\n"
        "    --cgp-mutate NUM, -m NUM\n"
        "          Number of (max) mutated genes in CGP, default is 5.\n"
        "\n"
        "    --cgp-population-size NUM, -p NUM\n"
        "          CGP population size, default is 8.\n"
        "\n"
        "    --cgp-archive-size NUM, -s NUM\n"
        "          CGP archive size, default is 10.\n"
        "\n"
        "    --pred-size NUM, -S NUM\n"
        "          Predictor size (in percent), default is 0.25.\n"
        "\n"
        "    --pred-mutate NUM, -M NUM\n"
        "          Predictor mutation rate (in percent), default is 0.05.\n"
        "\n"
        "    --pred-population-size NUM, -P NUM\n"
        "          Predictors population size, default is 10.\n"
        "\n"
        "    --pred-type TYPE, -T TYPE\n"
        "          Predictor genome type, one of {permuted|repeated}\n"
        "          Default is \"permuted\" for coevolution and \"repeated\" for baldwin.\n"
        "          - permuted: No value can be repeated in genotype, phenotype equals\n"
        "                      genotype. Cannot be used with \"baldwin\".\n"
        "          - repeated: No limitations on genotype, duplicities are eliminated\n"
        "                      during phenotype construction. Typically, phenotype is\n"
        "                      shorter than genotype.\n"
        "          - repeated-circular: Same as repeated, but phenotype construction\n"
        "                      starts from any locus (offset). It is determined as the\n"
        "                      locus with best fitness from 5 tries.\n"
        "\n"
        "    --baldwin-interval NUM, -b NUM\n"
        "          Minimal interval of evolution parameters update in \"baldwin\" mode\n"
        "          Default is \"0\" which means, that parameters are updated only if.\n"
        "          CGP fitness changes.\n"
        "\n"
        "Baldwin - predictor size settings:\n"
        "    Maximal predictor size is specified by --pred-size parameter.\n"
        "\n"
        "    --bw-pred-initial-size NUM, -I NUM\n"
        "          Predictor initial size (in percent), default is predictor size.\n"
        "\n"
        "    --bw-pred-min-size 0, -N 0\n"
        "          Predictor minimal size (in percent), default is no limit.\n"
        "\n"
        "Baldwin - inaccuracy settings\n"
        "    If predictor gets too short, the difference between predicted and real\n"
        "    fitness becomes too high. To avoid this, if inaccuracy exceedes given\n"
        "    threshold, special rule is applied:\n"
        "        (f_pred / f_real) > inac_tol   --->  len = len * inac_coef\n"
        "    Options and default values are:"
        "        --bw-inac-tol  1.2\n"
        "        --bw-inac-coef 2.0\n"
        "\n"
        "Baldwin - relative predictor size increment/decrement mode\n"
        "\n"
        "    Algorithm options (with those default values):\n"
        "        --bw-zero-eps  0.001\n"
        "        --bw-zero-coef 0.93\n"
        "\n"
        "        --bw-decr-coef 0.97\n"
        "\n"
        "        --bw-slow-thr  0.1\n"
        "        --bw-slow-coef 1.03\n"
        "        --bw-fast-coef 1\n"
        "\n"
        "    Rules are (processed in this order):\n"
        "        (f_pred / f_real) > inac_tol   --->  len = len * inac_coef\n"
        "        velocity <= zero_eps           --->  len = len * zero_coef\n"
        "        velocity < 0                   --->  len = len * decr_coef\n"
        "        velocity < slow_thr            --->  len = len * slow_coef\n"
        "        velocity > slow_thr            --->  len = len * fast_coef\n"
        "\n"
        "Baldwin - absolute predictor size increment/decrement mode\n"
        "\n"
        "    Use --bw-by-max-length to switch to this mode.\n"
        "\n"
        "    Algorithm options (with those default values):\n"
        "        --bw-zero-eps 0.001\n"
        "        --bw-zero-inc -0.07\n"
        "\n"
        "        --bw-decr-inc -0.03\n"
        "\n"
        "        --bw-slow-thr 0.1\n"
        "        --bw-slow-inc +0.03\n"
        "        --bw-fast-inc 0\n"
        "\n"
        "    Rules are (processed in this order):\n"
        "        (f_pred / f_real) > inac_tol   --->  len = len * inac_coef\n"
        "        velocity <= zero_eps           --->  len += zero_increment\n"
        "        velocity < 0                   --->  len += decr_increment\n"
        "        velocity < slow_thr            --->  len += slow_increment\n"
        "        velocity > slow_thr            --->  len += fast_increment\n"
    , stdout);  // this comma is ugly, I know
}


/**
 * Load configuration from command line
 */
config_retval_t config_load_args(int argc, char **argv, config_t *cfg);


/**
 * Save configuration to file
 */
void config_save_file(FILE *file, config_t *cfg);
