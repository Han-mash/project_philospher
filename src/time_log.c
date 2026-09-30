#include "codexion.h"

long get_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void log_state(t_sim *sim, int id, const char *msg)
{
    long timestamp;

    pthread_mutex_lock(&sim->print_mutex);
    if (!sim->stop)
    {
        timestamp = get_time_ms() - sim->start_time;
        printf("%ld %d %s\n", timestamp, id, msg);
    }
    pthread_mutex_unlock(&sim->print_mutex);
}

long get_last_compile(t_coder *coder)
{
    long value;

    pthread_mutex_lock(&coder->mutex);
    value = coder->last_compile_start;
    pthread_mutex_unlock(&coder->mutex);
    return (value);
}
