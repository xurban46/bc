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

#include "csv.h"


struct logger_csv {
    struct logger_base base;  // must be first!
    FILE *log_file;
    history_entry_t last_entry;
};
typedef struct logger_csv *logger_csv_t;


/* event handlers */
static void handle_started(logger_t logger, history_entry_t *state);
static void handle_finished(logger_t logger, finish_reason_t reason, history_entry_t *state, struct algo_data *work_data);
static void handle_baldwin_triggered(logger_t logger, history_entry_t *state);
static void handle_log_tick(logger_t logger, history_entry_t *state);
static void handle_signal(logger_t logger, int signal, history_entry_t *state);
static void handle_better_pred(logger_t logger, ga_fitness_t old_fitness, ga_fitness_t new_fitness);
static void handle_pred_length_change_applied(logger_t logger, int cgp_generation, unsigned int old_length, unsigned int new_length, unsigned int old_used_length, unsigned int new_used_length);

/* "destructor" */
static void logger_csv_destruct(logger_t logger);


/**
 * Create text logger
 * @param logger
 */
logger_t logger_csv_create(config_t *config, FILE *target)
{
    assert(target != NULL);

    logger_csv_t logger = (logger_csv_t) malloc(sizeof(struct logger_csv));
    if (logger == NULL) return NULL;

    logger->log_file = target;

    // this is the same as &logger->base
    logger_t base = (logger_t) logger;

    logger_init_base(base, config);
    base->handler_started = handle_started;
    base->handler_finished = handle_finished;
    base->handler_better_cgp = handle_log_tick;
    base->handler_baldwin_triggered = handle_baldwin_triggered;
    base->handler_log_tick = handle_log_tick;
    base->handler_better_pred = handle_better_pred;
    base->handler_pred_length_change_applied = handle_pred_length_change_applied;
    base->handler_signal = handle_signal;
    base->destructor = logger_csv_destruct;

    return base;
}


/**
 * Frees any resources allocated by text logger
 */
static void logger_csv_destruct(logger_t logger)
{
    free(logger);
}


/**
 * Extracts file pointer from logger "object"
 * @param  logger
 * @return
 */
static inline FILE *_get_fp(logger_t logger) {
    return ((logger_csv_t) logger)->log_file;
}


/**
 * Extracts last history entry from logger "object"
 * @param  logger
 * @return
 */
static inline history_entry_t *_get_last_entry(logger_t logger) {
    return &((logger_csv_t) logger)->last_entry;
}


/**
 * Extracts last history entry from logger "object"
 * @param  logger
 * @return
 */
static inline void _copy_last_entry(logger_t logger, history_entry_t *new_entry) {
    memcpy(&((logger_csv_t) logger)->last_entry, new_entry, sizeof(history_entry_t));
}


/**
 * Extracts last history entry from logger "object"
 * @param  logger
 * @return
 */
static inline void _print_line(logger_t logger, history_entry_t *entry) {
    FILE *fp = _get_fp(logger);
    fprintf(fp,
        "%d,"       // entry->generation,
        "%.10g,"    // entry->predicted_fitness,
        "%.10g,"     // entry->real_fitness,
        "%.10g,"     // entry->fitness_inaccuracy,
        "%.10g,"    // entry->best_real_fitness_ever,
        "%.10g,"     // entry->active_predictor_fitness,
        "%d,"        // entry->pred_length,
        "%d,"       // entry->pred_used_length,
        "%ld,"      // entry->cgp_evals,
        "%.10g,"    // entry->velocity,
        "%d,"        // entry->delta_generation,
        "%.10g,"     // entry->delta_real_fitness,
        "%.10g,"    // entry->delta_velocity,
        "%.10g,"     // logger_get_wallclock(logger).tv_sec / 60.0,
        "%.10g\n",  // logger_get_usertime(logger).tv_sec / 60.0

        entry->generation,
        entry->predicted_fitness,
        entry->real_fitness,
        entry->fitness_inaccuracy,
        entry->best_real_fitness_ever,
        entry->active_predictor_fitness,
        entry->pred_length,
        entry->pred_used_length,
        entry->cgp_evals,
        entry->velocity,
        entry->delta_generation,
        entry->delta_real_fitness,
        entry->delta_velocity,
        logger_get_wallclock(logger).tv_sec / 60.0,
        logger_get_usertime(logger).tv_sec / 60.0
    );
    fflush(fp);
}



static void handle_started(logger_t logger, history_entry_t *state)
{
    _copy_last_entry(logger, state);
    fprintf(_get_fp(logger),
        "generation,"               // entry->generation,
        "predicted_fitness,"         // entry->predicted_fitness,
        "real_fitness,"              // entry->real_fitness,
        "inaccuracy (pred/real),"    // entry->fitness_inaccuracy,
        "best_fitness_ever,"        // entry->best_real_fitness_ever,
        "active_predictor_fitness,"  // entry->active_predictor_fitness,
        "pred_length,"               // entry->pred_length,
        "pred_used_length,"         // entry->pred_used_length,
        "cgp_evals,"                // entry->cgp_evals,
        "velocity,"                 // entry->velocity,
        "delta_generation,"          // entry->delta_generation,
        "delta_fitness,"             // entry->delta_real_fitness,
        "delta_velocity,"           // entry->delta_velocity,
        "wallclock,"                 // logger_get_wallclock(logger).tv_sec / 60.0,
        "usertime\n"                // logger_get_usertime(logger).tv_sec / 60.0
    );
}


static void handle_finished(logger_t logger, finish_reason_t reason, history_entry_t *state,
    struct algo_data *work_data)
{
    _print_line(logger, state);
}


static void handle_baldwin_triggered(logger_t logger, history_entry_t *state)
{
    _copy_last_entry(logger, state);
}


static void handle_log_tick(logger_t logger, history_entry_t *state)
{
    _print_line(logger, state);
    _copy_last_entry(logger, state);
}


static void handle_signal(logger_t logger, int signal, history_entry_t *state)
{
    _copy_last_entry(logger, state);
}


static void handle_better_pred(logger_t logger, ga_fitness_t old_fitness, ga_fitness_t new_fitness)
{
}


static void handle_pred_length_change_applied(logger_t logger, int cgp_generation,
    unsigned int old_length, unsigned int new_length,
    unsigned int old_used_length, unsigned int new_used_length)
{
    history_entry_t *last = _get_last_entry(logger);
    last->pred_length = new_length;
    last->pred_used_length = new_used_length;
    _print_line(logger, last);
}
