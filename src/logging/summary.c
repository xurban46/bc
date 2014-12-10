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
#include <assert.h>
#include <string.h>

#include "summary.h"
#include "../algo.h"
#include "../utils.h"
#include "../config.h"
#include "../fitness.h"


struct logger_summary {
    struct logger_base base;  // must be first!
    char *target_dir;
    bool summary_to_files;
    bool summary_to_stdout;
};
typedef struct logger_summary *logger_summary_t;


#define _USERTIME_STR \
    char _usertime_str[100]; \
    logger_snprintf_usertime(logger, _usertime_str, 100);


#define _WALLCLOCK_STR \
    char _wallclock_str[100]; \
    logger_snprintf_wallclock(logger, _wallclock_str, 100);


#define _BUFFER \
    int _buffer_size = strlen(slogger->target_dir) + 100; \
    char _buffer[_buffer_size];


#define SPRINTF_FILENAME(FILENAME) snprintf(_buffer, _buffer_size, "%s/" FILENAME, slogger->target_dir);


/* event handlers */
static void handle_started(logger_t logger, history_entry_t *state);
static void handle_finished(logger_t logger, finish_reason_t reason, history_entry_t *state, struct algo_data *work_data);

/* "destructor" */
static void logger_summary_destruct(logger_t logger);


/**
 * Create summarizing logger
 * @param logger
 */
logger_t logger_summary_create(config_t *config, char *target_dir,
    bool summary_to_stdout)
{
    logger_summary_t logger = (logger_summary_t) malloc(sizeof(struct logger_summary));
    if (logger == NULL) return NULL;

    // strdup includes malloc
    logger->target_dir = strdup(target_dir);
    if (logger->target_dir == NULL) {
        free(logger);
        return NULL;
    }

    logger->summary_to_files = strlen(target_dir) > 0;
    logger->summary_to_stdout = summary_to_stdout;

    // this is the same as &logger->base
    logger_t base = (logger_t) logger;

    logger_init_base(base, config);
    base->handler_started = handle_started;
    base->handler_finished = handle_finished;
    base->destructor = logger_summary_destruct;

    return base;
}


/**
 * Frees any resources allocated by summary logger
 */
static void logger_summary_destruct(logger_t logger)
{
    logger_summary_t slogger = (logger_summary_t) logger;
    free(slogger->target_dir);
    free(logger);
}


static void handle_started(logger_t logger, history_entry_t *state)
{
    logger_summary_t slogger = (logger_summary_t) logger;

    if (slogger->summary_to_files) {
        _BUFFER;

        SPRINTF_FILENAME("config.log");
        FILE *fp = fopen(_buffer, "wt");
        if (fp) {
            config_save_file(fp, logger->config);
            fclose(fp);
        }
    }
}


static void handle_finished(logger_t logger, finish_reason_t reason, history_entry_t *state,
    struct algo_data *work_data)
{
    logger_summary_t slogger = (logger_summary_t) logger;

    ga_chr_t circuit;
    FILE *fp;
    _USERTIME_STR;
    _WALLCLOCK_STR;
    _BUFFER;

    if (logger->config->algorithm == simple_cgp) {
        circuit = work_data->cgp_population->best_chromosome;

    } else {
        circuit = work_data->cgp_archive->best_chromosome_ever;
    }

    if (slogger->summary_to_files) {
        SPRINTF_FILENAME("best_circuit.txt");
        fp = fopen(_buffer, "wt");
        if (fp) {
            fprintf(fp, "Generation: %d\n", state->generation);
            fprintf(fp, "Fitness: " FITNESS_FMT "\n\n", circuit->fitness);
            fprintf(fp, "CGP Viewer format:\n");
            cgp_dump_chr(circuit, fp, compat);
            fprintf(fp, "\nASCII Art:\n");
            cgp_dump_chr(circuit, fp, asciiart);
            fprintf(fp, "\nASCII Art without inactive blocks:\n");
            cgp_dump_chr(circuit, fp, asciiart_active);
            fclose(fp);
        }

        SPRINTF_FILENAME("best_circuit.chr");
        fp = fopen(_buffer, "wt");
        if (fp) {
            cgp_dump_chr(circuit, fp, compat);
            fclose(fp);
        }

        SPRINTF_FILENAME("summary.log");
        fp = fopen(_buffer, "wt");
        if (fp) {
            fprintf(fp, "Final summary:\n\n");
            fprintf(fp, "Generation: %d\n", state->generation);
            fprintf(fp, "Best fitness: " FITNESS_FMT "\n", circuit->fitness);
            fprintf(fp, "PSNR: %.2f\n", fitness_to_psnr(circuit->fitness));
            fprintf(fp, "CGP evaluations: %ld\n\n", state->cgp_evals);
            fprintf(fp, "Time in user mode: %s\n", _usertime_str);
            fprintf(fp, "Wall clock: %s\n", _wallclock_str);
            fclose(fp);
        }

        SPRINTF_FILENAME("img_orignal.png");
        img_save_png(work_data->img_original, _buffer);

        SPRINTF_FILENAME("img_noisy.png");
        img_save_png(work_data->img_noisy, _buffer);

        SPRINTF_FILENAME("img_best.png");
        img_image_t img_best = fitness_filter_image(circuit);
        img_save_png(img_best, _buffer);
        img_destroy(img_best);
    }

    if (slogger->summary_to_stdout) {
        printf("Final summary:\n\n");
        printf("Generation: %d\n", state->generation);
        printf("Best fitness: " FITNESS_FMT "\n", circuit->fitness);
        printf("PSNR: %.2f\n", fitness_to_psnr(circuit->fitness));
        printf("CGP evaluations: %ld\n\n", state->cgp_evals);
        printf("Time in user mode: %s\n", _usertime_str);
        printf("Wall clock: %s\n", _wallclock_str);
    }
}

