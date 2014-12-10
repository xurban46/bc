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


#include <stdio.h>


#define MAX_FILENAME_LENGTH 1000
#define FITNESS_FMT "%.10g"


/**
 * Install signal handlers
 */
void init_signals();


/**
 * Checks for SIGXCPU and SIGINT signals
 * @return Received signal code
 */
int check_signals(int current_generation);


/**
 * Creates directory, if it does not exist
 * @param  dir
 */
int create_dir(const char *dir);


/**
 * Open specified file for writing. Caller is responsible for closing.
 * @param  dir
 * @param  file
 * @return
 */
FILE *open_file(const char *dir, const char *file);


/**
 * Prints compilation and OS configuration.
 */
void print_sysinfo();
