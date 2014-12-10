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


#include "cgp/cgp.h"
#include "image.h"
#include "config.h"
#include "archive.h"
#include "baldwin.h"
#include "predictors.h"
#include "logging/logging.h"


typedef struct algo_data {
    // config
    config_t *config;

    // populations
    // if algo == simple_cgp, only cgp population is necessary
    ga_pop_t cgp_population;
    ga_pop_t pred_population;

    // archives
    // not used when algo == simple_cgp
    archive_t cgp_archive;
    archive_t pred_archive;

    // history
    history_t history;

    // baldwin (colearning state)
    bw_state_t baldwin_state;

    // log files
    FILE *log_file;
    FILE *history_file;

    // loggers
    logger_list_t loggers;

    // source images - in fact used only to provide them to loggers
    img_image_t img_original;
    img_image_t img_noisy;

    // indicates that the algorithm should terminate ASAP
    bool finished;
} algo_data_t;


/**
 * CGP main loop
 * @param  work_data
 * @return Program return value
 */
int cgp_main(algo_data_t *work_data);


/**
 * Coevolutionary predictors main loop
 * @param  work_data
 */
void pred_main(algo_data_t *work_data);
