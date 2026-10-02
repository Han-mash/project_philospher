#include "codexion.h"

static int alloc_arrays(t_sim *sim)
{
    sim->coders = malloc(sizeof(t_coder) * sim->config.nb_coders);
    if (!sim->coders)
        return (1);
    memset(sim->coders, 0, sizeof(t_coder) * sim->config.nb_coders);

    sim->dongles = malloc(sizeof(t_dongle) * sim->config.nb_coders);
    if (!sim->dongles)
    {
        free(sim->coders);
        return (1);
    }
    memset(sim->dongles, 0, sizeof(t_dongle) * sim->config.nb_coders);
    return (0);
}

static void init_dongles(t_sim *sim)
{
    int i;

    i = 0;
    while (i < sim->config.nb_coders)
    {
        pthread_mutex_init(&sim->dongles[i].mutex, NULL);
        pthread_cond_init(&sim->dongles[i].cond, NULL);
        heap_init(&sim->dongles[i].queue, sim->config.nb_coders);
        
        sim->dongles[i].available_at = 0;
        sim->dongles[i].taken = 0;
        sim->dongles[i].ticket = 0;
        i++;
    }
}


static void assign_dongles(t_sim *sim, int i)
{
    int a;
    int b;
    t_coder *c;

    c = &sim->coders[i];
    a = i;
    b = (i + 1) % sim->config.nb_coders;
    if (sim->config.nb_coders == 1)
    {
        c->first = &sim->dongles[0];
        c->second = NULL;
    }
    else if (a < b)
    {
        c->first = &sim->dongles[a];
        c->second = &sim->dongles[b];
    }
    else
    {
        c->first = &sim->dongles[b];
        c->second = &sim->dongles[a];
    }
}

static void init_coders(t_sim *sim)
{
    int i;
    t_coder *c;

    i = 0;
    while (i < sim->config.nb_coders)
    {
        c = &sim->coders[i];
        c->id = i + 1;
        c->last_compile_start = sim->start_time;
        c->compile_count = 0;
        c->sim = sim;
        pthread_mutex_init(&c->mutex, NULL);
        c->sim = sim;
        assign_dongles(sim, i);
        i++;
    }
}

int init_sim(t_sim *sim)
{
    if (alloc_arrays(sim))
        return (1);
    sim->stop = 0;
    sim->start_time = get_time_ms();
    pthread_mutex_init(&sim->print_mutex, NULL);
    init_dongles(sim);
    init_coders(sim);
    return (0);
}
