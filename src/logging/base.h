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


#include <stdlib.h>
#include <sys/time.h>

#include "history.h"

#include "../ga.h"
#include "../config.h"


typedef enum {
    generation_limit,
    target_fitness,
    signal_received,
} finish_reason_t;


/* solves cyclic type dependency */
struct logger_base;
typedef struct logger_base *logger_t;
struct algo_data; // declared in algo.h

/* event handlers */
typedef void (*handler_started_t)(logger_t logger, history_entry_t *state);
typedef void (*handler_finished_t)(logger_t logger, finish_reason_t reason, history_entry_t *state, struct algo_data *work_data);
typedef void (*handler_better_cgp_t)(logger_t logger, history_entry_t *state);
typedef void (*handler_baldwin_triggered_t)(logger_t logger, history_entry_t *state);
typedef void (*handler_log_tick_t)(logger_t logger, history_entry_t *state);
typedef void (*handler_signal_t)(logger_t logger, int signal, history_entry_t *state);
typedef void (*handler_better_pred_t)(logger_t logger, ga_fitness_t old_fitness, ga_fitness_t new_fitness);
typedef void (*handler_pred_length_change_scheduled_t)(logger_t logger, int new_predictor_length, history_entry_t *state);
typedef void (*handler_pred_length_change_applied_t)(logger_t logger, int cgp_generation,
    unsigned int old_length, unsigned int new_length,
    unsigned int old_used_length, unsigned int new_used_length);

/* "destructor" */
typedef void (*logger_destructor_t)(logger_t logger);


struct logger_base {
    /* time */
    struct timeval usertime_start;
    struct timeval wallclock_start;

    /* program configuration */
    config_t *config;

    /* event handlers */
    handler_started_t handler_started;
    handler_finished_t handler_finished;
    handler_better_cgp_t handler_better_cgp;
    handler_baldwin_triggered_t handler_baldwin_triggered;
    handler_log_tick_t handler_log_tick;
    handler_better_pred_t handler_better_pred;
    handler_pred_length_change_scheduled_t handler_pred_length_change_scheduled;
    handler_pred_length_change_applied_t handler_pred_length_change_applied;
    handler_signal_t handler_signal;

    /* "destructor" */
    logger_destructor_t destructor;
};


/**
 * Base logger initializer
 */
void logger_init_base(logger_t logger, config_t *config);


/**
 * Destroy logger
 */
static inline void logger_destroy(logger_t logger)
{
    logger->destructor(logger);
}


/**
 * Returns elapsed usertime
 */
struct timeval logger_get_usertime(logger_t logger);


/**
 * Returns elapsed wall clock
 */
struct timeval logger_get_wallclock(logger_t logger);


/**
 * Formats elapsed usertime as XXmYY.ZZs
 */
int logger_snprintf_usertime(logger_t logger, char *buffer, int buffer_size);


/**
 * Formats elapsed wall clock as XXmYY.ZZs
 */
int logger_snprintf_wallclock(logger_t logger, char *buffer, int buffer_size);


/**
 * Create devnull logger (which does nothing)
 * @param logger
 */
static inline logger_t logger_devnull_create(config_t *config)
{
    logger_t logger = (logger_t) malloc(sizeof(struct logger_base));
    if (logger == NULL) return NULL;

    logger_init_base(logger, config);
    return logger;
}
