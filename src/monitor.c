#include "codexion.h"

static void	wake_all(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static void	announce_burnout(t_sim *sim, int id)
{
	pthread_mutex_lock(&sim->print_mutex);
	if (!sim->stop)
	{
		sim->stop = 1;
		printf("%ld %d burned out\n", get_time_ms() - sim->start_time, id);
	}
	pthread_mutex_unlock(&sim->print_mutex);
}

static int	coder_status(t_sim *sim, t_coder *coder)
{
	int		count;
	long	last;

	pthread_mutex_lock(&coder->mutex);
	count = coder->compile_count;
	last = coder->last_compile_start;
	pthread_mutex_unlock(&coder->mutex);
	if (count >= sim->config.number_of_compiles_required)
		return (0);
	if (get_time_ms() - last > sim->config.burnout)
		return (2);
	return (1);
}

static int	scan_coders(t_sim *sim)
{
	int	i;
	int	alive;
	int	state;

	i = 0;
	alive = 0;
	while (i < sim->config.nb_coders)
	{
		state = coder_status(sim, &sim->coders[i]);
		if (state == 2)
		{
			announce_burnout(sim, sim->coders[i].id);
			wake_all(sim);
			return (1);
		}
		if (state == 1)
			alive++;
		i++;
	}
	if (alive == 0)
	{
		set_stop(sim);
		wake_all(sim);
	}
	return (alive == 0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (!scan_coders(sim))
		usleep(500);
	return (NULL);
}
