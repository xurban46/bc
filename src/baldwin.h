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


#include "ga.h"
#include "logging/history.h"


#define BW_HISTORY_LENGTH 7

typedef enum {
    bwalg_last = 0,
    bwalg_median3,
    bwalg_avg3,
    bwalg_avg7w,
    bwalg_symreg,
} bw_algorithm_t;


// multiple const to avoid "unused variable" warnings
static const char * const bw_algorithm_names[] = {
    "last",
    "median3",
    "avg3",
    "avg7w",
    "symreg",
};


typedef struct {
    bw_algorithm_t algorithm;
    bool use_absolute_increments;

    double inaccuracy_tolerance;
    double inaccuracy_coef;

    double zero_epsilon;
    double slow_threshold;

    double zero_coef;
    double decrease_coef;
    double increase_slow_coef;
    double increase_fast_coef;

    int absolute_increment_base;
    int zero_increment;
    int decrease_increment;
    int increase_slow_increment;
    int increase_fast_increment;

    int min_length;
    int max_length;
} bw_config_t;


typedef struct {
    bool predictor_length_changed;
    int old_predictor_length;
    int new_predictor_length;
} bw_update_t;


typedef struct {
    int new_predictor_length;
    int last_applied_generation;
} bw_state_t;


/**
 * Calculate absolute predictor length increments values
 */
void bw_init_absolute_increments(bw_config_t *config, int base);


/**
 * Returns new predictor length
 * @param  config
 * @param  history
 * @return New length or zero if no change should happen
 */
int bw_get_new_predictor_length(bw_config_t *config, history_t *history);
