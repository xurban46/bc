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


#include "history.h"

#include <string.h>


/**
 * Initializes history data structure
 * @param history
 */
void history_init(history_t *history)
{
    /*
    history->last_change.generation = 0;
    history->last_change.delta_generation = 0;

    history->last_change.predicted_fitness = 0;
    history->last_change.delta_predicted_fitness = 0;

    history->last_change.real_fitness = 0;
    history->last_change.delta_real_fitness = 0;

    history->last_change.fitness_inaccuracy = 0;

    history->last_change.best_fitness_ever = 0;
    history->last_change.active_predictor_fitness = 0;

    history->last_change.velocity = 0;
    history->last_change.delta_velocity = 0;

    history->last_change.cgp_evals = 0;
    history->last_change.pred_length = 0;
    history->last_change.pred_used_length = 0;

    memcpy(&history->entries[0], &history->last_change, sizeof(history_entry_t));
    */

    memset(&history->last_change, 0, sizeof(history_entry_t));
    memset(&history->entries[0], 0, sizeof(history_entry_t));

    history->stored = 1;
    history->pointer = 1;
}


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
) {
    entry->generation = generation;
    entry->delta_generation = generation - prev->generation;

    entry->predicted_fitness = predicted_fitness;
    entry->delta_predicted_fitness = entry->predicted_fitness - prev->predicted_fitness;

    entry->real_fitness = real_fitness;
    entry->delta_real_fitness = entry->real_fitness - prev->real_fitness;

    entry->fitness_inaccuracy = predicted_fitness / real_fitness;

    if (ga_is_better(CGP_PROBLEM_TYPE, real_fitness, prev->best_real_fitness_ever)) {
        entry->best_real_fitness_ever = real_fitness;
    } else {
        entry->best_real_fitness_ever = prev->best_real_fitness_ever;
    }

    entry->active_predictor_fitness = active_predictor_fitness;

    entry->velocity = entry->delta_real_fitness / entry->delta_generation;
    entry->delta_velocity = entry->velocity - prev->velocity;

    entry->cgp_evals = cgp_evals;

    entry->pred_length = pred_length;
    entry->pred_used_length = pred_used_length;
}


/**
 * Adds entry to history
 * @param  history
 * @param  entry
 * @return pointer to newly inserted entry
 */
history_entry_t *history_append_entry(history_t *history,
    history_entry_t *entry)
{
    history_entry_t *new = &history->entries[history->pointer];
    memcpy(new, entry, sizeof(history_entry_t));

    if (new->delta_real_fitness != 0) {
        memcpy(&history->last_change, new, sizeof(history_entry_t));
    }

    if (history->stored < HISTORY_LENGTH) {
        history->stored++;
    }
    history->pointer = (history->pointer + 1) % HISTORY_LENGTH;

    return new;
}


/**
 * Dumps history to file as ASCII art
 * @param fp
 * @param history
 */
void history_dump_asciiart(FILE *fp, history_t *history) {
    int len = history->stored;

    // boxes top
    fprintf(fp, "+--------+---------++");
    for (int i = 0; i < len; i++) {
        fprintf(fp, "---------+");
    }
    fprintf(fp, "\n");

    // generations
    fprintf(fp, "|      G |");
    fprintf(fp, " %7d ||", history->last_change.generation);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7d |", history_get(history, i)->generation);
    }
    fprintf(fp, "\n");

    // fitness
    fprintf(fp, "|     rf |");
    fprintf(fp, " %7.3lf ||", history->last_change.real_fitness);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7.3lf |", history_get(history, i)->real_fitness);
    }
    fprintf(fp, "\n");

    // predicted fitness
    fprintf(fp, "|     pf |");
    fprintf(fp, " %7.3lf ||", history->last_change.predicted_fitness);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7.3lf |", history_get(history, i)->predicted_fitness);
    }
    fprintf(fp, "\n");

    // predicted fitness
    fprintf(fp, "|  predf |");
    fprintf(fp, " %7.3lf ||", history->last_change.active_predictor_fitness);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7.3lf |", history_get(history, i)->active_predictor_fitness);
    }
    fprintf(fp, "\n");

    // divider
    fprintf(fp, "+--------+---------++");
    for (int i = 0; i < len; i++) {
        fprintf(fp, "---------+");
    }
    fprintf(fp, "\n");

    // delta generations
    fprintf(fp, "|     dG |");
    fprintf(fp, " %7d ||", history->last_change.delta_generation);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7d |", history_get(history, i)->delta_generation);
    }
    fprintf(fp, "\n");

    // delta fitness
    fprintf(fp, "|     df |");
    fprintf(fp, " %7.3lf ||", history->last_change.delta_real_fitness);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7.3lf |", history_get(history, i)->delta_real_fitness);
    }
    fprintf(fp, "\n");

    // divider
    fprintf(fp, "+--------+---------++");
    for (int i = 0; i < len; i++) {
        fprintf(fp, "---------+");
    }
    fprintf(fp, "\n");

    // velocity
    fprintf(fp, "|    f/G |");
    fprintf(fp, " %7.3lf ||", history->last_change.velocity);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7.3lf |", history_get(history, i)->velocity);
    }
    fprintf(fp, "\n");

    // delta velocity = acceleration
    fprintf(fp, "| d(f/G) |");
    fprintf(fp, " %7.3lf ||", history->last_change.delta_velocity);
    for (int i = 0; i < len; i++) {
        fprintf(fp, " %7.3lf |", history_get(history, i)->delta_velocity);
    }
    fprintf(fp, "\n");

    // boxes bottom
    fprintf(fp, "+--------+---------++");
    for (int i = 0; i < len; i++) {
        fprintf(fp, "---------+");
    }
    fprintf(fp, "\n");
}
