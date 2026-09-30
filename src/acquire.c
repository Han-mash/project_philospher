#include "codexion.h"

static int	hold_alone(t_coder *coder)
{
	while (!is_stopped(coder->sim))
		usleep(200);
	release_dongle(coder, coder->first);
	return (1);
}

int	acquire_dongles(t_coder *coder)
{
	if (take_dongle(coder, coder->first) != 0)
		return (1);
	log_state(coder->sim, coder->id, "has taken a dongle");
	if (coder->second == NULL)
		return (hold_alone(coder));
	if (take_dongle(coder, coder->second) != 0)
	{
		release_dongle(coder, coder->first);
		return (1);
	}
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (0);
}

void	release_dongles(t_coder *coder)
{
	release_dongle(coder, coder->first);
	if (coder->second != NULL)
		release_dongle(coder, coder->second);
}