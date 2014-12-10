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
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>


/**
 * Generates random seed using gettimeofday()
 * @return generated seed
 */
static inline unsigned int rand_seed_from_time()
{
    // time() from time.h is not enough, because it can return same value in
    // all processes started within one second
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec);
}


/**
 * Initializes random seed using given value.
 * @return used random seed
 */
static inline unsigned int rand_init_seed(unsigned int seed)
{
    srand(seed);
    return seed;
}


/**
 * Initializes random seed using gettimeofday().
 * @return used random seed
 */
static inline unsigned int rand_init()
{
    return rand_init_seed(rand_seed_from_time());
}



/**
 * Generates random number between low and high, inclusive
 * @param  low
 * @param  high
 * @return
 */
static inline int rand_range(int low, int high)
{
    return rand() % (high - low + 1) + low;
}


/**
 * Generates random number between low and high, inclusive
 * @param  low
 * @param  high
 * @return
 */
static inline unsigned int rand_urange(unsigned int low, unsigned int high)
{
    return rand() % (high - low + 1) + low;
}


/**
 * Returns randomly chosen number from the list of signed integers
 * @param  length
 * @param  choices
 * @return
 */
static inline int rand_schoice(int length, int choices[])
{
    unsigned int index = rand_range(0, length - 1);
    return choices[index];
}



/**
 * Returns randomly chosen number from the list of unsigned integers
 * @param  length
 * @param  choices
 * @return
 */
static inline unsigned int rand_uchoice(unsigned int length, unsigned int choices[])
{
    unsigned int index = rand_range(0, length - 1);
    return choices[index];
}
