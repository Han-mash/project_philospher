#include "codexion.h"

static void destroy_dongles(t_sim *sim)
{
    int i;

    i = 0;
    while (i < sim->config.nb_coders)
    {
        heap_destroy(&sim->dongles[i].queue);
        pthread_cond_destroy(&sim->dongles[i].cond);
        pthread_mutex_destroy(&sim->dongles[i].mutex);
        i++;
    }
}

static void destroy_coders(t_sim *sim)
{
    int i;

    i = 0;
    while (i < sim->config.nb_coders)
    {
        pthread_mutex_destroy(&sim->coders[i].mutex);
        i++;
    }
}

void cleanup_sim(t_sim *sim)
{
    if (sim->coders)
        destroy_coders(sim);
    if (sim->dongles)
        destroy_dongles(sim);
        
    pthread_mutex_destroy(&sim->print_mutex);
    free(sim->coders);
    free(sim->dongles);
}
