/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 00:44:57 by amarabin          #+#    #+#             */
/*   Updated: 2023/11/21 05:12:07 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/**
 * kill() sends a signal to a process. In this case is being used to make sure
 * that processes did actually terminate.
 * waitpid() does the final check waiting for the process to terminate and
 * Both these functions will return -1 if was impossible to reach the process
 * specified in the pid
 * this is fine since child processes are not allocating memory on their own.
 * It's also worth to mention that if the parent process itself terminates, all
 * its child processes are adopted by the "init" process which automatically
 * waits on its child processes, thereby preventing them from becoming zombies.
 */
static void	cleanup(int number_of_philosophers, t_philo *philos,
		t_sem semaphores, pid_t *child_pids)
{
	int	i;

	i = 0;
	while (number_of_philosophers > i++)
	{
		kill(child_pids[i - 1], SIGKILL);
		waitpid(child_pids[i - 1], NULL, 0);
	}
	free(child_pids);
	sem_close(semaphores.fork_pool);
	sem_close(semaphores.log_sem);
	sem_close(semaphores.simulation_sem);
	sem_unlink("log_sem");
	sem_unlink("sim_sem");
	sem_unlink("fork_pool");
	free(philos);
}

/**
 * Validates the command-line arguments for the philosopher simulation
 *
 * @argc: Number of command-line arguments.
 * @argv: Array of command-line arguments.
 * @f_tmpl: Pointer to a t_philo template used to populate other philo objects.
 * @number_of_philosophers: Will store the number of philosophers.
 *
 * The function expects at least 5 and at most 6 command-line arguments.
 *  1st argument: Number of philosophers.
 *  2nd argument: Time for a philosopher to die.
 *  3rd argument: Time for a philosopher to eat.
 *  4th argument: Time for a philosopher to sleep.
 *  5th argument (optional): Number of times a philo has to eat to end the app.
 *
 * Return: 1 if parameters are valid, 0 otherwise.
 */
static int	validate_params(int argc, char **argv, t_philo *f_tmpl,
		int *number_of_philosophers)
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
	return (1);
}

/**
 * Relevant Elements/Functions:
 *
 * Using Named semaphores in a multiprocess environment:
 * Differently from a a regular semaphore (sem_init(<&sem_id>, 0, <n>>);)
 * that would work intraproc, named semaphores works interproc, creating
 * an absolute reference at the OS level.
 * This is why we use
 * sem_unlink("<sem_name>");
 * to prevent the situation where the main process might have been
 * interrupted abruptly leaving the sems hanging uncontrolled.
 * sem_open("<sem_name>", O_CREAT, 0644, <n>);
 * Initialize a semaphore to a value n > 0, specifying that we are
 * creating it (O_CREAT, flag has no effect if the semaphore already exixts),
 * permissions (0644: following Unix permissions model, means that the owner
 * has read and write permissions (rw-r--r--), while children can sem_wait or
 * sem_post only.
 * This means that the child will be able to access the semaphore using
 * sem_t *sem = sem_open("<sem_name>", 0);
 * (0 is a flag that means "open an existing semaphore with no options")
 */
static t_philo	*init_philos(int n_of_philos, t_philo f_tmpl, t_sem *sems)
{
	int		i;
	t_philo	*philos;

	sem_unlink("log_sem");
	sem_unlink("sim_sem");
	sem_unlink("fork_pool");
	sems->log_sem = sem_open("log_sem", O_CREAT, 0644, 1);
	sems->simulation_sem = sem_open("sim_sem", O_CREAT, 0644, n_of_philos);
	sems->fork_pool = sem_open("fork_pool", O_CREAT, 0644, n_of_philos);
	philos = malloc(n_of_philos * sizeof(t_philo));
	if (!philos)
		return (NULL);
	i = 0;
	while (n_of_philos > i++)
	{
		philos[i - 1] = f_tmpl;
		philos[i - 1].id = i;
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
 * philo_cycle function and then exit(0) to terminate cleanly.
 * Conversely, else if (child_pids[i] < 0) checks if the fork() failed to
 * create a new process. In such cases, fork() returns a negative value.
 * Therefore, this condition is for error-handling (thus we try to kill
 * any process created up to that point)
 * To summarize:
 * if (child_pids[i] == 0): This block is executed by the child process.
 * else if (child_pids[i] < 0): This block is executed by the parent if fork()
 * fails.
 * Once spawn each child process gets its own copy of the process memory,
 * including the philos array (thus being able to access the value passed to
 * the fn philo_cycle
 *
 * Relevant parts:
 * usleep(10000); //(10 ms)
 * sem_wait(semaphores.simulation_sem);
 * each child process at start will reclame a spot of semaphores.simulation_sem
 * by giving each process a lille space to start, the parent process will
 * procede at cleanup once a spot is available (a philosopher died or so).
 *
 * @param number_of_philosophers The total number of philosophers.
 * @param philos The array of philosopher structs.
 * the simulation_active flag
 * @return 1 on failure (e.g., thread or memory allocation issues), 0 otherwise.
 */
static int	execute_phils(int number_of_philosophers, t_philo *philos,
		t_sem semaphores)
{
	int		i;
	pid_t	*child_pids;

	i = 0;
	child_pids = malloc(number_of_philosophers * sizeof(pid_t));
	if (!child_pids)
		return (1);
	while (number_of_philosophers > i++)
	{
		child_pids[i - 1] = fork();
		if (child_pids[i - 1] == 0)
			philo_cycle(&philos[i - 1]);
		else if (child_pids[i - 1] < 0)
		{
			while (0 < i--)
				kill(child_pids[i], SIGKILL);
			return (1);
		}
	}
	usleep(10000);
	sem_wait(semaphores.simulation_sem);
	cleanup(number_of_philosophers, philos, semaphores, child_pids);
	return (0);
}

int	main(int argc, char **argv)
{
	t_philo	f_tmpl;
	t_philo	*philos;
	t_sem	semaphores;
	int		number_of_philosophers;

	f_tmpl.times_eaten = 0;
	f_tmpl.last_meal_time = 0;
	f_tmpl.holding_forks = 0;
	if (!validate_params(argc, argv, &f_tmpl, &number_of_philosophers))
		return (1);
	philos = init_philos(number_of_philosophers, f_tmpl, &semaphores);
	if (!philos)
		return (1);
	if (execute_phils(number_of_philosophers, philos, semaphores))
		return (1);
	return (0);
}
