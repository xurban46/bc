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

#include "text.h"
#include "../utils.h"


struct logger_text {
    struct logger_base base;  // must be first!
    FILE *log_file;
};
typedef struct logger_text *logger_text_t;


/**
 * Extracts file pointer from logger "object"
 * @param  logger
 * @return
 */
static inline FILE *_get_fp(logger_t logger) {
    return ((logger_text_t) logger)->log_file;
}

#define _USERTIME_STR \
    char _usertime_str[100]; \
    logger_snprintf_usertime(logger, _usertime_str, 100);


/* event handlers */
static void handle_started(logger_t logger, history_entry_t *state);
static void handle_finished(logger_t logger, finish_reason_t reason, history_entry_t *state, struct algo_data *work_data);
static void handle_better_cgp(logger_t logger, history_entry_t *state);
static void handle_baldwin_triggered(logger_t logger, history_entry_t *state);
static void handle_log_tick(logger_t logger, history_entry_t *state);
static void handle_signal(logger_t logger, int signal, history_entry_t *state);
static void handle_better_pred(logger_t logger, ga_fitness_t old_fitness, ga_fitness_t new_fitness);
static void handle_pred_length_change_scheduled(logger_t logger, int new_predictor_length, history_entry_t *state);
static void handle_pred_length_change_applied(logger_t logger, int cgp_generation, unsigned int old_length, unsigned int new_length,
    unsigned int old_used_length, unsigned int new_used_length);

/* "destructor" */
static void logger_text_destruct(logger_t logger);


/**
 * Create text logger
 * @param logger
 */
logger_t logger_text_create(config_t *config, FILE *target)
{
    assert(target != NULL);

    logger_text_t logger = (logger_text_t) malloc(sizeof(struct logger_text));
    if (logger == NULL) return NULL;

    logger->log_file = target;

    // this is the same as &logger->base
    logger_t base = (logger_t) logger;

    logger_init_base(base, config);
    base->handler_started = handle_started;
    base->handler_finished = handle_finished;
    base->handler_better_cgp = handle_better_cgp;
    base->handler_baldwin_triggered = handle_baldwin_triggered;
    base->handler_log_tick = handle_log_tick;
    base->handler_better_pred = handle_better_pred;
    base->handler_pred_length_change_scheduled = handle_pred_length_change_scheduled;
    base->handler_pred_length_change_applied = handle_pred_length_change_applied;
    base->handler_signal = handle_signal;
    base->destructor = logger_text_destruct;

    return base;
}


/**
 * Frees any resources allocated by text logger
 */
static void logger_text_destruct(logger_t logger)
{
    free(logger);
}


static void handle_started(logger_t logger, history_entry_t *state)
{
    fprintf(_get_fp(logger),
        "Evolution starts now.\n"
        "Generation %d: Fitness predicted / real: " FITNESS_FMT " / " FITNESS_FMT "\n",
        state->generation, state->predicted_fitness, state->real_fitness);
}


static void handle_finished(logger_t logger, finish_reason_t reason, history_entry_t *state,
    struct algo_data *work_data)
{
    fprintf(_get_fp(logger),
        "Generation %d: Evolution stopped. %s\n",
        state->generation,
        reason == generation_limit? "Generation limit reached."
        : reason == target_fitness? "Target fitness achieved."
        : reason == signal_received? "Signal received."
        : "");
}


static void handle_better_cgp(logger_t logger, history_entry_t *state)
{
    handle_log_tick(logger, state);
}


static void handle_baldwin_triggered(logger_t logger, history_entry_t *state)
{
    fprintf(_get_fp(logger),
        "Generation %d: Baldwin triggered. Inaccuracy: %5g\n",
        state->generation, state->fitness_inaccuracy);
}


static void handle_log_tick(logger_t logger, history_entry_t *state)
{
    _USERTIME_STR;
    fprintf(_get_fp(logger),
        "Generation %d: Fitness predicted / real: " FITNESS_FMT " / " FITNESS_FMT ". Usertime %s\n",
        state->generation, state->predicted_fitness, state->real_fitness, _usertime_str);
}


static void handle_signal(logger_t logger, int signal, history_entry_t *state)
{
    fprintf(_get_fp(logger),
        "Generation %d: Signal %d (%s) received\n",
        state->generation, signal, strsignal(signal));
}


static void handle_better_pred(logger_t logger, ga_fitness_t old_fitness, ga_fitness_t new_fitness)
{
    fprintf(_get_fp(logger),
        "Predictor's fitness changed " FITNESS_FMT " --> " FITNESS_FMT "\n",
        old_fitness, new_fitness);
}


static void handle_pred_length_change_scheduled(logger_t logger, int new_predictor_length, history_entry_t *state)
{
    fprintf(_get_fp(logger),
        "Generation %d: Predictor's length change scheduled %d --> %d\n",
        state->generation, state->pred_length, new_predictor_length);
}


static void handle_pred_length_change_applied(logger_t logger, int cgp_generation,
    unsigned int old_length, unsigned int new_length,
    unsigned int old_used_length, unsigned int new_used_length)
{
    fprintf(_get_fp(logger),
        "Generation %d: Predictor's length change applied   %d --> %d\n",
        cgp_generation, old_length, new_length);
}
