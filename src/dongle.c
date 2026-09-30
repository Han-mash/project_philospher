#include "codexion.h"

static void	make_request(t_request *req, t_coder *coder, t_dongle *dongle)
{
	req->coder_id = coder->id;
	if (coder->sim->config.scheduler == MODE_FIFO)
	{
		req->key = dongle->ticket;
		dongle->ticket++;
	}
	else
		req->key = get_last_compile(coder) + coder->sim->config.burnout;
}

static int	my_turn(t_dongle *dongle, t_request *req)
{
	t_request	top;

	if (heap_peek(&dongle->queue, &top) != 0)
		return (0);
	if (top.coder_id != req->coder_id)
		return (0);
	if (dongle->taken)
		return (0);
	return (get_time_ms() >= dongle->available_at);
}

static void	wait_turn(t_dongle *dongle)
{
	long			now;
	long			limit;
	struct timespec	ts;

	now = get_time_ms();
	limit = now + 5;
	if (!dongle->taken && dongle->available_at > now
		&& dongle->available_at < limit)
		limit = dongle->available_at;
	ts.tv_sec = limit / 1000;
	ts.tv_nsec = (limit % 1000) * 1000000;
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
}

int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	req;

	pthread_mutex_lock(&dongle->mutex);
	make_request(&req, coder, dongle);
	if (heap_push(&dongle->queue, &req) != 0)
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (1);
	}
	while (!my_turn(dongle, &req))
	{
		if (is_stopped(coder->sim))
		{
			pthread_mutex_unlock(&dongle->mutex);
			return (1);
		}
		wait_turn(dongle);
	}
	heap_pop(&dongle->queue);
	dongle->taken = 1;
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->taken = 0;
	dongle->available_at = get_time_ms() + coder->sim->config.dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}