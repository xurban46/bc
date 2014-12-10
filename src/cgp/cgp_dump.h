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

#include "cgp_core.h"
#include "cgp_config.h"


/**
 * Dump formats
 */
typedef enum {
    asciiart,
    asciiart_active,
    compat,
    readable,
} cgp_dump_format;



/**
 * Dumps chromosome to given file pointer in given format
 * @param fp
 */
void cgp_dump_chr(ga_chr_t chr, FILE *fp, cgp_dump_format fmt);


/**
 * Dumps chromosome to given file in CGP-viewer compatible format
 * @param chr
 * @param fp
 */
void cgp_dump_chr_compat(ga_chr_t chr, FILE *fp);


/**
 * Dumps chromosome to given file in more human-friendly fashion
 * @param chr
 * @param fp
 */
void cgp_dump_chr_readable(ga_chr_t chr, FILE *fp);


/**
 * Dumps chromosome to given file as an ASCII-art, which looks like this:
 *
 *      .------------------------------------.
 *      |      .----.            .----.      |
 * [ 0]>| [ 1]>|    |>[ 4]  [ 0]>|    |>[ 6] |>[ 6]
 * [ 1]>| [ 3]>|  a |       [ 4]>| xor|      |
 * [ 2]>|      '----'            '----'      |
 * [ 3]>|      .----.            .----.      |
 *      | [ 3]>|    |>[ 5]  [ 4]>|    |>[ 7] |
 *      | [ 0]>|a>>1|       [ 5]>|nand|      |
 *      |      '----'            '----'      |
 *      '------------------------------------'
 *
 * @param chr
 * @param fp
 * @param only_active_blocks Do not render inactive blocks at all
*/
void cgp_dump_chr_asciiart(ga_chr_t chr, FILE *fp, bool only_active_blocks);


static inline int cgp_dump_chr_asciiart_width() {
    return 5 + CGP_COLS * 18 + 5;
}


static inline int cgp_dump_chr_asciiart_height() {
    // header + border + nodes + border
    return 5 + 1 + (CGP_ROWS * 4) + 1;
}


/**
 * Dumps whole population to given file pointer with chromosomes in
 * CGP-viewer compatible format
 * @param pop
 * @param fp
 */
void cgp_dump_pop_compat(ga_pop_t pop, FILE *fp);
