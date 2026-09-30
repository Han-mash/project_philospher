#include "codexion.h"

static int	do_compile(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (acquire_dongles(coder) != 0)
		return (1);
	pthread_mutex_lock(&coder->mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->mutex);
	log_state(sim, coder->id, "is compiling");
	precise_sleep(sim, sim->config.time_to_compile);
	pthread_mutex_lock(&coder->mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->mutex);
	release_dongles(coder);
	return (0);
}

static void	do_debug(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is debugging");
	precise_sleep(coder->sim, coder->sim->config.time_to_debug);
}

static void	do_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is refactoring");
	precise_sleep(coder->sim, coder->sim->config.time_to_refactor);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (!is_stopped(sim))
	{
		if (do_compile(coder) != 0)
			break ;
		if (coder->compile_count >= sim->config.number_of_compiles_required)
			break ;
		do_debug(coder);
		do_refactor(coder);
	}
	return (NULL);
}