/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 00:44:57 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/20 22:37:52 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

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
	//attention!!
	//somehow the process id 1 never gets visualized!
	f_tmpl->time_to_die = ft_atoi(argv[2]);
	f_tmpl->time_to_eat = ft_atoi(argv[3]);
	f_tmpl->time_to_sleep = ft_atoi(argv[4]);
	f_tmpl->num_of_eating_times = -1;
	if (argc == 6)
		f_tmpl->num_of_eating_times = ft_atoi(argv[5]);
	*number_of_philosophers = ft_atoi(argv[1]);
	if ((f_tmpl->num_of_eating_times < -1 || f_tmpl->num_of_eating_times == 0)
		|| f_tmpl->time_to_die <= 0 || f_tmpl->time_to_eat <= 0
		|| f_tmpl->time_to_sleep <= 0 || *number_of_philosophers <= 0)
	{
		printf("Invalid parameters.\n");
		return (0);
	}
	sem_init(&(shared->log_sem), 0, 1);
	sem_init(&(shared->simulation_sem), 0, 1);
	sem_init(&(shared->fork_pool), 0, *number_of_philosophers);
	shared->simulation_active = 1;
	return (1);
}
/**
 * Relevant Elements/Functions:
 * sem_init(&forks[i++], 0, 1);
 * Initialize each semaphore to 1 (unlocked).
 */
t_philo	*init_philos(int number_of_philosophers, sem_t *forks,
		t_philo *f_tmpl, t_shared shared_resources)
{
	int		i;
	t_philo	*philos;

	philos = malloc((number_of_philosophers) * sizeof(t_philo));
	if (!philos)
		return (NULL);
	i = 0;
	while (number_of_philosophers > i)
		sem_init(&forks[i++], 0, 1);
	i = 0;
	while (number_of_philosophers > i++)
	{
		philos[i - 1] = *f_tmpl;
		philos[i - 1].id = i;
        philos[i - 1].shared_resources = &shared_resources;
	}
	return (philos);
}

/**
 * Creates threads for each philosopher and then detach them.
 * Each philosopher's lifecycle is managed in the `philo_cycle` function.
 * Relevant Elements/Functions:
 * Fork(): creates (spawns) a new process
 * When you call fork(), it creates a new process that is an almost exact copy
 * of the current process. Both the parent and the child continue executing
 * from the point where fork() was called. The only difference is the value
 * returned by fork():
 * In the parent process, fork() returns the child's Process ID (PID),
 * which is a positive integer. In the child process, fork() returns 0.
 * So when you check if (child_pids[i] == 0), you're essentially asking,
 * "Am I the child process?" If the answer is yes, you proceed to run the
 * philo_cycle function and then exit(0) to terminate the child process cleanly
 * Conversely, else if (child_pids[i] < 0) checks if the fork() failed to
 * create a new process. In such cases, fork() returns a negative value.
 * Therefore, this condition is for error-handling.
 * To summarize:
 * if (child_pids[i] == 0): This block is executed by the child process.
 * else if (child_pids[i] < 0): This block is executed by the parent if fork()
 * fails.
 *
 * Relevant parts:
 * kill() sends a signal to a process. In this case is being used to make sure
 * that processes did actually terminate.
 * we are ensuring this already by calling sem_wait(&(shared->fork_pool));
 * on each of the forks in the pool that should be released by the graceful
 * termination of each child process.
 * waitpid() does the final check waiting for the process to terminate and
 * Both these functions will return -1 if was impossible to reach the process
 * specified in the pid
 * It's also worth noting that if the parent process itself terminates, all
 * its child processes are adopted by the "init" process which automatically
 * waits on its child processes, thereby preventing them from becoming zombies.
 *
 * @param number_of_philosophers The total number of philosophers.
 * @param philos The array of philosopher structs.
 * @param shared_resources contains a reference to the global simulation_mutes and to
 * the simulation_active flag
 * @return 1 on failure (e.g., thread or memory allocation issues), 0 otherwise.
 */
int	execute_phils(int number_of_philosophers, t_philo *philos, t_shared shared)
{
	int			i;
	pid_t		*child_pids;

	i = 0;
	child_pids = malloc(number_of_philosophers * sizeof(pid_t));
	if (!child_pids)
		return (1);
	while (number_of_philosophers > i++)
	{
		child_pids[i - 1] = fork();
		if (child_pids[i - 1] == 0)
			philo_cycle(&philos[i]);
		else if (child_pids[i] < 0)
			return (1);
	}
	while (1)
	{
		sem_wait(&(shared.simulation_sem));
		if (!shared.simulation_active) {
			sem_post(&(shared.simulation_sem));
			break;
		}
		sem_post(&(shared.simulation_sem));
		usleep(10000);
	}
	i = 0;
	while (number_of_philosophers > i++)
		sem_wait(&(shared.fork_pool));
	i = 0;
	while (number_of_philosophers > i++)
		sem_post(&(shared.fork_pool));
	i = 0;
	while (number_of_philosophers > i++)
	{
		kill(child_pids[i], SIGKILL);
		waitpid(child_pids[i], NULL, 0);
	}
	// while (number_of_philosophers > i++)
	// {
	// 	if(waitpid(child_pids[i], &status, WNOHANG) == 0) {
	// 		kill(child_pids[i], SIGTERM); // or SIGKILL, your choice
	// 	}
	// }
	free(child_pids);
	return (0);
}

int	main(int argc, char **argv)
{
	t_philo			f_tmpl;
	sem_t			*forks;
	t_philo			*philos;
	t_shared		shared;
	int				number_of_philosophers;

	f_tmpl.times_eaten = 0;
	f_tmpl.last_meal_time = 0;
	f_tmpl.holding_forks = 0;
	if (!validate_params(argc, argv, &f_tmpl, &number_of_philosophers, &shared))
		return (1);
	forks = malloc((number_of_philosophers) * sizeof(sem_t));
	philos = init_philos(number_of_philosophers, forks, &f_tmpl, shared);
	if (!philos || !forks)
		return (1);
	if (execute_phils(number_of_philosophers, philos, shared))
		return (1);
	while (0 <= number_of_philosophers--)
		sem_destroy(&forks[number_of_philosophers]);
	sem_destroy(&shared.log_sem);
	sem_destroy(&shared.simulation_sem);
	free(forks);
	free(philos);
	return (0);
}
