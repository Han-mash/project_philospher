#include "codexion.h"

int	is_stopped(t_sim *sim)
{
	int	value;

	pthread_mutex_lock(&sim->print_mutex);
	value = sim->stop;
	pthread_mutex_unlock(&sim->print_mutex);
	return (value);
}

void	set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->print_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->print_mutex);
}

void	precise_sleep(t_sim *sim, long ms)
{
	long	end;

	end = get_time_ms() + ms;
	while (get_time_ms() < end && !is_stopped(sim))
		usleep(200);
}