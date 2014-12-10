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


#include <stdlib.h>

#include "archive.h"



/**
 * Allocate memory for and initialize new archive
 *
 * @param  size Archive size
 * @param  problem-specific genome function pointers
 * @param  problem type - used to compare new chromosomes with best found
 * @return pointer to created archive
 */
archive_t arc_create(int capacity, arc_func_vect_t methods, ga_problem_type_t problem_type)
{
    archive_t arc = (archive_t) malloc(sizeof(struct archive));
    if (arc == NULL) {
        return NULL;
    }

    ga_chr_t *items = (ga_chr_t*) malloc(sizeof(ga_chr_t) * capacity);
    if (items == NULL) {
        free(arc);
        return NULL;
    }

    ga_fitness_t *original_fitness = (ga_fitness_t*) malloc(sizeof(ga_fitness_t) * capacity);
    if (original_fitness == NULL) {
        free(items);
        free(arc);
        return NULL;
    }

    ga_chr_t best_ever = ga_alloc_chr(methods.alloc_genome);
    if (best_ever == NULL) {
        free(original_fitness);
        free(items);
        free(arc);
        return NULL;
    }
    best_ever->has_fitness = false;

    for (int i = 0; i < capacity; i++) {
        items[i] = ga_alloc_chr(methods.alloc_genome);
        if (items[i] == NULL) {
            for (int x = i - 1; x >= 0; i--) {
                ga_destroy_chr(items[x], methods.free_genome);
            }
            free(arc);
            return NULL;
        }
    }

    arc->chromosomes = items;
    arc->best_chromosome_ever = best_ever;
    arc->original_fitness = original_fitness;
    arc->capacity = capacity;
    arc->stored = 0;
    arc->pointer = 0;
    arc->methods = methods;
    arc->problem_type = problem_type;
    return arc;
}


/**
 * Release given archive from memory
 */
void arc_destroy(archive_t arc)
{
    if (!arc) return;

    for (int i = 0; i < arc->capacity; i++) {
        ga_destroy_chr(arc->chromosomes[i], arc->methods.free_genome);
    }
    free(arc->chromosomes);
    free(arc->original_fitness);
    ga_destroy_chr(arc->best_chromosome_ever, arc->methods.free_genome);
    free(arc);
}


/**
 * Insert chromosome into archive
 *
 * Chromosome is copied into place and pointer to it is returned.
 *
 * Chromosome is reevaluated using `arc->methods.fitness` (if set).
 *
 * @param  arc
 * @param  chr
 * @return pointer to stored chromosome in archive
 */
ga_chr_t arc_insert(archive_t arc, ga_chr_t chr)
{
    ga_chr_t dst = arc->chromosomes[arc->pointer];
    ga_copy_chr(dst, chr, arc->methods.copy_genome);
    arc->original_fitness[arc->pointer] = chr->has_fitness? chr->fitness : 0;

    if (arc->methods.fitness != NULL) {
        dst->fitness = arc->methods.fitness(dst);
        dst->has_fitness = true;
    }

    if (arc->stored == 0 || ga_is_better(arc->problem_type, dst->fitness, arc->best_chromosome_ever->fitness)) {
        ga_copy_chr(arc->best_chromosome_ever, dst, arc->methods.copy_genome);
    }

    if (arc->stored < arc->capacity) {
        arc->stored++;
    }
    arc->pointer = (arc->pointer + 1) % arc->capacity;
    return dst;
}
