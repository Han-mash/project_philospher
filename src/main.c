#include "codexion.h"

int	main(int argc, char **argv)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(t_sim));
	if (parse_args(argc, argv, &sim.config) != 0)
	{
		write(2, "Error: Invalid arguments\n", 25);
		return (1);
	}
	if (init_sim(&sim) != 0)
	{
		write(2, "Error: Failed to initialize simulation\n", 39);
		return (1);
	}
	if (start_threads(&sim) != 0)
	{
		write(2, "Error: Failed to start threads\n", 31);
		cleanup_sim(&sim);
		return (1);
	}
	join_threads(&sim, sim.config.nb_coders);
	pthread_join(sim.monitor, NULL);
	cleanup_sim(&sim);
	return (0);
}
