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


#include "../ga.h"
#include "../cgp/cgp.h"

#define HISTORY_LENGTH 7


typedef struct {
    int generation;
    int delta_generation;

    // last found CGP fitness
    ga_fitness_t predicted_fitness;
    ga_fitness_t delta_predicted_fitness;

    ga_fitness_t real_fitness;
    ga_fitness_t delta_real_fitness;

    // predicted / real
    double fitness_inaccuracy;

    // best CGP ever
    ga_fitness_t best_real_fitness_ever;

    // currently used predictor
    ga_fitness_t active_predictor_fitness;

    // delta_real_fitness / delta_generation
    double velocity;
    double delta_velocity;

    long cgp_evals;

    int pred_length;
    int pred_used_length;
} history_entry_t;


typedef struct {
    history_entry_t last_change;
    history_entry_t entries[HISTORY_LENGTH];

    /* number of stored items */
    int stored;

    /* pointer to beginning of ring buffer - where new item will be stored */
    int pointer;
} history_t;


/**
 * Initializes history data structure
 * @param history
 */
void history_init(history_t *history);


/**
 * Calculate history entry values
 * @param  target entry
 * @param  previous entry
 */
void history_calc_entry(
    history_entry_t *entry,
    history_entry_t *prev,
    int generation,
    ga_fitness_t real_fitness,
    ga_fitness_t predicted_fitness,
    ga_fitness_t active_predictor_fitness,
    long cgp_evals,
    int pred_length,
    int pred_used_length
);


/**
 * Adds entry to history
 * @param  history
 * @param  entry
 * @return pointer to newly inserted entry
 */
history_entry_t *history_append_entry(history_t *history,
    history_entry_t *entry);


/**
 * Returns real index of item in ring buffer
 */
static inline int history_real_index(history_t *history, int index)
{
    if (history->stored < HISTORY_LENGTH) {
        int real = index % history->stored;
        if (real < 0) real += history->stored;
        return real;

    } else {
        int real = (history->pointer + index) % HISTORY_LENGTH;
        if (real < 0) real += HISTORY_LENGTH;
        return real;
    }
}


/**
 * Returns item stored on given index
 */
static inline history_entry_t *history_get(history_t *history, int index)
{
    return &history->entries[history_real_index(history, index)];
}


/**
 * Returns last item
 */
static inline history_entry_t *history_last(history_t *history)
{
    return history_get(history, -1);
}


/**
 * Dumps history to file as ASCII art
 * @param fp
 * @param history
 */
void history_dump_asciiart(FILE *fp, history_t *history);
