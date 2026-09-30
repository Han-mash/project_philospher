#ifndef CODEXION_H
# define CODEXION_H

# define MODE_FIFO 0
# define MODE_EDF 1

# include <limits.h>
# include <string.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>

typedef struct s_sim t_sim;
typedef struct s_coder t_coder;
typedef struct s_dongle t_dongle;
typedef struct s_request t_request;
typedef struct s_heap t_heap;

typedef struct s_config
{
    int nb_coders;
    int burnout;
    int time_to_compile;
    int time_to_debug;
    int time_to_refactor;
    int number_of_compiles_required;
    int dongle_cooldown;
    int scheduler;
} t_config;

struct s_request
{
    long key;
    int coder_id;
};

struct s_heap
{
    t_request *data;
    int size;
    int capacity;
};

struct s_dongle
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    t_heap queue;
    long ticket;
    long available_at;
    int taken;
};

struct s_sim
{
    long start_time;
    int stop;
    pthread_mutex_t print_mutex;
    t_config config;
    t_coder *coders;
    t_dongle *dongles;
    pthread_t monitor;
};

struct s_coder
{
    int id;
    pthread_t thread;
    t_dongle *first;
    t_dongle *second;
    long last_compile_start;
    int compile_count;
    pthread_mutex_t mutex;
    t_sim *sim;
};



int parse_unit(const char *str, int *out);
int parse_scheduler(const char *str, int *out);
int parse_args(int argc, char **argv, t_config *config);
long get_time_ms(void);
void log_state(t_sim *sim, int id, const char *msg);
int	init_sim(t_sim *sim);
void cleanup_sim(t_sim *sim);
int	start_threads(t_sim *sim);
void	join_threads(t_sim *sim, int count);
void    *coder_routine(void *arg);
long get_last_compile(t_coder *coder);
int is_stopped(t_sim *sim);
void set_stop(t_sim *sim);
void precise_sleep(t_sim *sim, long ms);
int heap_init(t_heap *heap, int capacity);
void heap_destroy(t_heap *heap);
int heap_push(t_heap *heap, t_request *request);
int heap_pop(t_heap *heap);
int heap_peek(t_heap *heap, t_request *out);
int heap_before(t_request *a, t_request *b);
void heap_swap(t_request *a, t_request *b);
int take_dongle(t_coder *coder, t_dongle *dongle);
void release_dongle(t_coder *coder, t_dongle *dongle);
int acquire_dongles(t_coder *coder);
void release_dongles(t_coder *coder);
void	*monitor_routine(void *arg);


#endif