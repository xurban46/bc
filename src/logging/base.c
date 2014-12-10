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


#include <sys/resource.h>

#include "base.h"


static int _timeval_subtract(struct timeval *out, struct timeval *x, struct timeval *y);
int _snprintf_time(char *buffer, int buffer_size, struct timeval *time);


/**
 * Base logger initializer
 */
void logger_init_base(logger_t logger, config_t *config)
{
    logger->config = config;

    struct rusage resource_usage;
    getrusage(RUSAGE_SELF, &resource_usage);
    logger->usertime_start = resource_usage.ru_utime;
    gettimeofday(&logger->wallclock_start, NULL);

    logger->handler_started = NULL;
    logger->handler_finished = NULL;
    logger->handler_better_cgp = NULL;
    logger->handler_baldwin_triggered = NULL;
    logger->handler_log_tick = NULL;
    logger->handler_better_pred = NULL;
    logger->handler_pred_length_change_scheduled = NULL;
    logger->handler_pred_length_change_applied = NULL;
    logger->handler_signal = NULL;
}


/**
 * Returns elapsed usertime
 */
struct timeval logger_get_usertime(logger_t logger)
{
    struct rusage resource_usage;
    struct timeval diff;
    getrusage(RUSAGE_SELF, &resource_usage);
    _timeval_subtract(&diff, &resource_usage.ru_utime, &logger->usertime_start);
    return diff;
}


/**
 * Returns elapsed wall clock
 */
struct timeval logger_get_wallclock(logger_t logger)
{
    struct timeval end, diff;
    gettimeofday(&end, NULL);
    _timeval_subtract(&diff, &end, &logger->wallclock_start);
    return diff;
}



/**
 * Formats elapsed usertime as XXmYY.ZZs
 */
int logger_snprintf_usertime(logger_t logger, char *buffer, int buffer_size)
{
    struct timeval tmp = logger_get_usertime(logger);
    return _snprintf_time(buffer, buffer_size, &tmp);
}


/**
 * Formats elapsed wall clock as XXmYY.ZZs
 */
int logger_snprintf_wallclock(logger_t logger, char *buffer, int buffer_size)
{
    struct timeval tmp = logger_get_wallclock(logger);
    return _snprintf_time(buffer, buffer_size, &tmp);
}


/* "private" functions ********************************************************/


/**
 * Calculates difference of two `struct timeval` values
 *
 * http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 *
 * @param  result
 * @param  x
 * @param  y
 * @return 1 if the difference is negative, otherwise 0.
 */
static int _timeval_subtract(struct timeval *out, struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
    out->tv_sec = x->tv_sec - y->tv_sec;
    out->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}


/**
 * Formats elapsed time as XXmYY.ZZZs
 */
int _snprintf_time(char *buffer, int buffer_size, struct timeval *time)
{
    long minutes = time->tv_sec / 60;
    long seconds = time->tv_sec % 60;
    long microseconds = time->tv_usec;

    if (microseconds < 0) {
        microseconds = 1 - microseconds;
        seconds--;
    }

    return snprintf(buffer, buffer_size, "%ldm%ld.%06lds", minutes, seconds, microseconds);
}
