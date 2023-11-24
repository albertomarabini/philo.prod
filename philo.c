/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 00:44:57 by amarabin          #+#    #+#             */
/*   Updated: 2023/11/21 05:13:20 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * Validates the command-line arguments for the philosopher simulation
 *
 * @argc: Number of command-line arguments.
 * @argv: Array of command-line arguments.
 * @f_tmpl: Pointer to a t_philo template used to populate other philo objects.
 * @number_of_philosophers: Will store the number of philosophers.
 * @shared: Holds shared mutex among philosophers.
 *
 * The function expects at least 5 and at most 6 command-line arguments.
 *  1st argument: Number of philosophers.
 *  2nd argument: Time for a philosopher to die.
 *  3rd argument: Time for a philosopher to eat.
 *  4th argument: Time for a philosopher to sleep.
 *  5th argument (optional): Number of times a philo has to eat to end the app.
 *
 * The function initializes the mutexes in the shared resource and sets the
 * simulation to active.
 * Return: 1 if parameters are valid, 0 otherwise.
 */
int	validate_params(int argc, char **argv, t_philo *f_tmpl,
		int *number_of_philosophers, t_shared *shared)
{
	if (argc < 5 || argc > 6)
	{
		printf("Invalid number of arguments.\n");
		return (0);
	}
	f_tmpl->time_to_die = ft_atoi(argv[2]);
	f_tmpl->time_to_eat = ft_atoi(argv[3]);
	f_tmpl->time_to_sleep = ft_atoi(argv[4]);
	f_tmpl->num_of_eating_times = -1;
	if (argc == 6)
		f_tmpl->num_of_eating_times = ft_atoi(argv[5]);
	*number_of_philosophers = ft_atoi(argv[1]);
	if ((f_tmpl->num_of_eating_times < -1 || f_tmpl->num_of_eating_times == 0)
		|| f_tmpl->time_to_die <= 0 || f_tmpl->time_to_eat <= 0
		|| f_tmpl->time_to_sleep <= 0 || *number_of_philosophers <= 1)
	{
		printf("Invalid parameters.\n");
		return (0);
	}
	pthread_mutex_init(&(shared->log_mutex), NULL);
	pthread_mutex_init(&(shared->simulation_mutex), NULL);
	shared->simulation_active = 1;
	return (1);
}

/**
 * Initialize each philo and assign them the relative couple of
 * fork mutex and a reference to the shared resources
*/
static t_philo	*init_philos(int number_of_philosophers, pthread_mutex_t *forks,
		t_philo *f_tmpl, t_shared shared_resources)
{
	int		i;
	t_philo	*philos;

	philos = malloc((number_of_philosophers) * sizeof(t_philo));
	if (!philos)
		return (NULL);
	i = 0;
	while (number_of_philosophers > i)
		pthread_mutex_init(&forks[i++], NULL);
	i = 0;
	while (number_of_philosophers > i++)
	{
		philos[i - 1] = *f_tmpl;
		philos[i - 1].id = i;
		philos[i - 1].left_fork = &forks[i - 1];
		philos[i - 1].right_fork = &forks[0];
		if (i != number_of_philosophers)
			philos[i - 1].right_fork = &forks[i];
		philos[i - 1].shared_resources = &shared_resources;
	}
	return (philos);
}

/**
 * Creates threads for each philosopher and then detach them.
 * Each philosopher's lifecycle is managed in the `philo_cycle` function.
 * Relevant Functions/Parts
 * pthread_create: create a new thread using a pthread_t variable as reference.
 * Parameters:
 *	thread: A pointer to a pthread_t variable that will hold the thread ID.
 *	attr: Thread attributes (usually set to NULL for default attributes).
 *	start_routine: A pointer to the function the thread will execute.
 *  arg: The argument passed to the start_routine as void *
 * pthread_detach: is used to detach a thread. When a thread is detached, its
 *	resources are released back to the system when the thread ends.
 *	Until then it lives it's own life, without the need of another thread
 *  to wait idle for its execution. This in contrast with pthread_join that
 *  waits for the thread specified to terminate.
 * The simulation_mutex is a way to communicate safely between all the threads
 * about the current state of the simulation. When one of the philos terminates,
 * it will set the flag shared_resources.simulation_active = 0.
 * Here we wait for that event in order to terminate the app
 *
 * @param number_of_philosophers The total number of philosophers.
 * @param philos The array of philosopher structs.
 * @param shared_resources contains a reference to the global simulation_mutes and to
 * the simulation_active flag
 * @return 1 on failure (e.g., thread or memory allocation issues), 0 otherwise.
 */
static int	execute_phils(int number_of_philosophers, t_philo *philos, t_shared shared_resources)
{
	int			i;
	int 		sim_state;
	pthread_t	*threads;

	i = 0;
	threads = malloc(number_of_philosophers * sizeof(pthread_t));
	if (!threads)
		return (1);
	while (number_of_philosophers > i++)
		if (pthread_create(&threads[i - 1], NULL, philo_cycle, &philos[i - 1]) || pthread_detach(threads[i - 1]))
			return (1);
	while (1)
	{
		usleep(10000);
		pthread_mutex_lock(&shared_resources.simulation_mutex);
		sim_state = shared_resources.simulation_active;
		pthread_mutex_unlock(&shared_resources.simulation_mutex);
		if(!sim_state)
			break ;
	}
	free(threads);
	return (0);
}

int	main(int argc, char **argv)
{
	t_philo			f_tmpl;
	pthread_mutex_t	*forks;
	t_philo			*philos;
	t_shared		shared;
	int				number_of_philosophers;

	f_tmpl.times_eaten = 0;
	f_tmpl.last_meal_time = 0;
	if (!validate_params(argc, argv, &f_tmpl, &number_of_philosophers, &shared))
		return (1);
	forks = malloc((number_of_philosophers + 1) * sizeof(pthread_mutex_t));
	philos = init_philos(number_of_philosophers, forks, &f_tmpl, shared);
	if (!philos || !forks)
		return (1);
	if (execute_phils(number_of_philosophers, philos, shared))
		return (1);
	while (0 <= number_of_philosophers--)
		pthread_mutex_destroy(&forks[number_of_philosophers]);
	pthread_mutex_destroy(&shared.log_mutex);
	pthread_mutex_destroy(&shared.simulation_mutex);
	free(forks);
	free(philos);
	return (0);
}
