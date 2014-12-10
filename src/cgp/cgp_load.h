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

#include <stdbool.h>
#include <stdio.h>

#include "cgp.h"
#include "cgp_config.h"


/**
 * Loads chromosome from given file stored in CGP-viewer compatible format
 * @param chr
 * @param fp
 * @return 0 on success, -1 on file format error, -2 on incompatible CGP config
 */
int cgp_load_chr_compat(ga_chr_t chr, FILE *fp);
