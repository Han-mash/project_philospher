#include "codexion.h"

static int	read_numbers(char **argv, t_config *config)
{
    if (parse_unit(argv[1], &config->nb_coders) != 0
        || parse_unit(argv[2], &config->burnout) != 0
        || parse_unit(argv[3], &config->time_to_compile) != 0
        || parse_unit(argv[4], &config->time_to_debug) != 0
        || parse_unit(argv[5], &config->time_to_refactor) != 0
        || parse_unit(argv[6], &config->number_of_compiles_required) != 0
        || parse_unit(argv[7], &config->dongle_cooldown) != 0)
        return (1);
    return (0);
}

static int	check_bounds(t_config *config)
{
    if (config->nb_coders < 1
        || config->burnout < 1
        || config->time_to_compile < 1
        || config->time_to_debug < 1
        || config->time_to_refactor < 1
        || config->number_of_compiles_required < 1)
        return (1);
    return (0);
}

int parse_unit(const char *str, int *out)
{
    int digit;

    if (!str || !out || *str == '\0')
        return (1);
    *out = 0;
    while (*str >= '0' && *str <= '9')
    {
        digit = *str - '0';
        if (*out > (INT_MAX - digit) / 10)
            return (1);
        *out = *out * 10 + digit;
        str++;
    }
    if (*str != '\0')
        return (1);
    return (0);
}

int parse_scheduler(const char *str, int *out)
{
    if (!str || !out)
        return (1);
    if (strcmp(str, "fifo") == 0)
    {
        *out = 0;
        return (0);
    }
    else if (strcmp(str, "edf") == 0)
    {
        *out = 1;
        return (0);
    }
    return (1);
}


int	parse_args(int argc, char **argv, t_config *config)
{
	if (argc != 9)
		return (1);
	if (read_numbers(argv, config) != 0)
		return (1);
	if (check_bounds(config) != 0)
		return (1);
	return (parse_scheduler(argv[8], &config->scheduler));
}