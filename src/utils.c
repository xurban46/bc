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


#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#ifdef _OPENMP
    #include <omp.h>
#endif

#include "utils.h"
#include "cpu.h"


#define MAX_FILENAME_LENGTH 1000
#define FITNESS_FMT "%.10g"


// signal handlers
volatile sig_atomic_t terminated = 0;
volatile sig_atomic_t interrupted = 0;
volatile sig_atomic_t cpu_limit_reached = 0;

// if SIGINT is received twice within this gap, program exits
const int SIGINT_GENERATIONS_GAP = 1000;

// special `check_signals` return value indicating first catch of SIGINT
const int SIGINT_FIRST = -SIGINT;


/**
 * Handles SIGINT. Sets `interrupted` flag.
 */
static void sigint_handler(int _)
{
    signal(SIGINT, sigint_handler);
    interrupted = 1;
}


/**
 * Handles SIGTERM. Sets `terminated` flag.
 */
static void sigterm_handler(int _)
{
    signal(SIGTERM, sigterm_handler);
    terminated = 1;
}


/**
 * Handles SIGXCPU. Sets `cpu_limit_reached` flag.
 */
static void sigxcpu_handler(int _)
{
    signal(SIGXCPU, sigxcpu_handler);
    cpu_limit_reached = 1;
}


/**
 * Install signal handlers
 */
void init_signals()
{
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGXCPU, sigxcpu_handler);
}


/**
 * Checks for SIGXCPU and SIGINT signals
 * @return Received signal code
 */
int check_signals(int current_generation)
{
    // when SIGINT was received last time
    static long interrupted_generation = -1;

    // SIGXCPU
    if (cpu_limit_reached) {
        fprintf(stderr, "SIGXCPU received!\n");
        cpu_limit_reached = false;
        return SIGXCPU;
    }

    // SIGTERM
    if (terminated) {
        fprintf(stderr, "SIGTERM received!\n");
        terminated = false;
        return SIGTERM;
    }

    // SIGINT
    if (interrupted) {
        if (interrupted_generation >= 0
            &&  interrupted_generation > current_generation - SIGINT_GENERATIONS_GAP) {
            fprintf(stderr, "SIGINT received and terminating!\n");
            return SIGINT;
        }

        fprintf(stderr, "SIGINT received!\n");
        interrupted = 0;
        interrupted_generation = current_generation;
        return SIGINT_FIRST;
    }

    return 0;
}


 /**
  * Creates directory, if it does not exist
  * @param  dir
  */
 int create_dir(const char *dir)
 {
     int retval = mkdir(dir, S_IRWXU);
     if (retval != 0 && errno != EEXIST) {
         return retval;
     }
     return 0;
 }


/**
 * Open specified file for writing. Caller is responsible for closing.
 * @param  dir
 * @param  file
 * @return
 */
FILE *open_file(const char *dir, const char *file)
{
    char filename[MAX_FILENAME_LENGTH + 1];
    snprintf(filename, MAX_FILENAME_LENGTH + 1, "%s/%s", dir, file);
    FILE *fp = fopen(filename, "wt");
    return fp;
}


/**
 * Prints compilation and OS configuration.
 */
void print_sysinfo()
{
    #ifdef _OPENMP
        printf("OpenMP is compiled. CPUs: %d. Max threads: %d.\n",
            omp_get_num_procs(), omp_get_max_threads());
    #else
        printf("OpenMP is not compiled, coevolution is not available.\n");
    #endif

    #ifdef AVX2
        if (can_use_intel_core_4th_gen_features()) {
            printf("AVX2 is compiled.\n");
        } else {
            printf("AVX2 is compiled, but not supported by CPU.\n");
        }
    #else
        printf("AVX2 is not compiled. Recompile with -DAVX2 defined to enable.\n");
    #endif

    #ifdef SSE2
        if (can_use_sse2()) {
            printf("SSE2 is compiled.\n");
        } else {
            printf("SSE2 is compiled, but not supported by CPU.\n");
        }
    #else
        printf("SSE2 is not compiled. Recompile with -DSSE2 defined to enable.\n");
    #endif
}
