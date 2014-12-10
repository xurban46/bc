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

#include <math.h>
#include <assert.h>
#include <string.h>

#include "baldwin.h"
#include "predictors.h"


static inline int bw_relative_to_absolute(double coef, int base) {
    /*
        Simple and works every time:
        1.00 -> (1.00 - 1) * base = 0
        1.10 -> (1.10 - 1) * base = 0.10 * base
        0.90 -> (0.90 - 1) * base = -0.10 * base
     */
    return (coef - 1.0) * base;
}


/**
 * Gets current velocity according to algorithm
 * @param  alg
 * @param  history
 * @return
 */
double history_get_velocity(bw_algorithm_t alg, history_t *history)
{
    if (alg == bwalg_last) {
        return history_get(history, -1)->velocity;

    } else if (alg == bwalg_avg3) {
        // doesn't matter if there are less than 3 items, we get duplicates
        double a = history_get(history, -1)->velocity;
        double b = history_get(history, -2)->velocity;
        double c = history_get(history, -3)->velocity;
        return (a + b + c) / 3;

    } else if (alg == bwalg_avg7w) {
        double sum = 0;
        double divider = 0;
        for (int i = 1; i <= history->stored; i++) {
            double velo = history_get(history, -i)->velocity;
            int weight = 8 - i;
            sum += velo * weight;
            divider += weight;
            printf("%lf * %d\n", velo, weight);
        }
        printf("%lf / %lf = %lf\n", sum, divider, sum / divider);
        return sum / divider;

    } else if (alg == bwalg_median3) {
        // doesn't matter if there are less than 3 items, we get duplicates
        double a = history_get(history, -1)->velocity;
        double b = history_get(history, -2)->velocity;
        double c = history_get(history, -3)->velocity;

        if (a >= b && a >= c) {
            // a is greatest
            return b > c? b : c;
        } else if (b >= a && b >= c) {
            // b is greatest
            return a > c? a : c;
        } else {
            // c is greatest
            return a > b? a : b;
        }

    } else {
        fprintf(stderr, "Unimplemented baldwin algorithm.\n");
        assert(false);
    }
}


/**
 * Gets coefficient according to some symreg function
 */
double history_get_coef(bw_algorithm_t alg, history_t *history)
{
    double a = history_get(history, -1)->velocity;
    double b = history_get(history, -2)->velocity;
    double c = history_get(history, -3)->velocity;
    double d = history_get(history, -4)->velocity;
    double e = history_get(history, -5)->velocity;
    double f = history_get(history, -6)->velocity;
    double g = history_get(history, -7)->velocity;

    return (0.984805307321727
        + 2.92388275504055*e
        + 55.5973782292397*b*g
        + 11.5809571875034*b*d
        + 1.97691040282476*d*f
        - 0.144536309148617*a
        - 2.76098000498705*c*e
        - 1.97691040282476*d*d);
}

#define CONCAT(prefix, postfix) prefix ## postfix
#define BW_UPDATE_SIZE(coef_prefix) { \
    if (config->use_absolute_increments) { \
        new_length = old_length + config->CONCAT(coef_prefix, _increment); \
    } else { \
        new_length = old_length * config->CONCAT(coef_prefix, _coef); \
    } \
}


/**
 * Returns new predictor length
 * @param  config
 * @param  history
 * @return New length or zero if no change should happen
 */
int bw_get_new_predictor_length(bw_config_t *config, history_t *history)
{
    history_entry_t *last = history_get(history, -1);

    int old_length = pred_get_length();
    int new_length = old_length;

    // if inaccuracy raises over threshold, do big increment,
    // set as percentage from maximal size
    if (last->fitness_inaccuracy > config->inaccuracy_tolerance) {
        new_length = round(old_length * config->inaccuracy_coef);
        /*
        int increment = (100.0 / config->inaccuracy_coef) * pred_get_max_length();
        new_length = old_length + increment;
        */
        //printf("Inaccuracy over threshold (%.3g > %.3g), new length %d\n", inaccuracy, config->inaccuracy_tolerance, new_length);

    } else {
        if (config->algorithm == bwalg_symreg) {
            double coefficcient = history_get_coef(config->algorithm, history);
            new_length = round(old_length * coefficcient);

        } else {
            double velocity = history_get_velocity(config->algorithm, history);

            if (fabs(velocity) <= config->zero_epsilon) {
                // no change
                BW_UPDATE_SIZE(zero);

            } else if (velocity < 0) {
                // decrease
                BW_UPDATE_SIZE(decrease);

            } else if (velocity > 0 && velocity <= config->slow_threshold) {
                // slow increase
                BW_UPDATE_SIZE(increase_slow);

            } else if (velocity > config->slow_threshold) {
                // fast increase
                BW_UPDATE_SIZE(increase_fast);

            } else {
                fprintf(stderr, "Baldwin if-else fail. Velocity = %.10g\n", velocity);
                assert(false);
            }
        }
    }

    if (config->min_length >= 0 && new_length < config->min_length) {
        new_length = config->min_length;
    }
    if (config->max_length && new_length > config->max_length) {
        new_length = config->max_length;
    }

    if (new_length != old_length) {
        return new_length;
    } else {
        return 0;
    }
}
