/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 16:02:38 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/22 23:23:33 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/**
 * We write a log on screen.
 * The reason for using a sem is that at high speed the screen can become
 * jumbled of messages.
 */
static void	log_activity(t_philo *philo, const char *activity)
{
	sem_wait(philo->semaphores.log_sem);
	printf("%lld %d %s\n", get_timestamp(), philo->id, activity);
	sem_post(philo->semaphores.log_sem);
}

/**
 * We verify if ((get_timestamp()-philo->last_meal_time) >= philo->time_to_die)
 * if so we update the state of the simulation and we release each resource
 * keept by the thread.
 * The reason for using a sem is to release the simulation_sem so the parent
 * process will be able to perceive the event and terminate all the processes
 */
static void	verify_death(t_philo *philo)
{
	if ((get_timestamp() - philo->last_meal_time) >= philo->time_to_die)
	{
		while (philo->holding_forks--)
			sem_post(philo->semaphores.fork_pool);
		log_activity(philo, "died");
		sem_post(philo->semaphores.simulation_sem);
		exit(0);
	}
}

/**
 * Executes the eating cycle for a philo in the simulation (picking forks,
 * eating for a given time, release the forks)
 *
 * If a philosopher has eaten the number of times specified in the optional
 * input parameter, the simulation ends for that thread.
 * Since while a philo waits for a sem to be released will stay idle, every
 * time we have a potential deathlock we check for the eventual philo death.
 * Relevant parts:
 * sem_wait(sem_t *sem): decrements (locks) the semaphore pointed to by sem.
 * If the semaphore's value is > 0 the decrement proceeds, and the function
 * returns immediately. If the semaphore currently has the value zero, then
 * the call blocks until it becomes possible to perform the decrement.
 * sem_post(sem_t *sem): This function increments (unlocks) the semaphore
 * pointed to by sem. If the semaphore's value consequently becomes greater
 * than zero, then another process blocked in a sem_wait call will be
 * awakened and proceed to lock the semaphore.
 *
 * @param philo Pointer to the t_philo structure representing the philo.
 *
 */
static void	eat(t_philo *p)
{
	log_activity(p, "is thinking");
	sem_wait(p->semaphores.fork_pool);
	p->holding_forks++;
	verify_death(p);
	log_activity(p, "has taken a fork");
	sem_wait(p->semaphores.fork_pool);
	p->holding_forks++;
	verify_death(p);
	log_activity(p, "has taken a fork");
	log_activity(p, "is eating");
	usleep(p->time_to_eat * 1000);
	sem_post(p->semaphores.fork_pool);
	sem_post(p->semaphores.fork_pool);
	p->holding_forks = 0;
	p->last_meal_time = get_timestamp();
	if (p->num_of_eating_times != -1)
	{
		p->times_eaten++;
		if (p->times_eaten >= p->num_of_eating_times)
		{
			sem_post(p->semaphores.simulation_sem);
			// printf("%lld %d %s%d\n", get_timestamp(), p->id,
			// 	p->times_eaten "n.Eats reached = ");
			exit(0);
		}
	}
}

/**
 * Makes the philosopher sleep for a given period, but while keep checking on
 * his life and trying to eat before the end the sleep cycle if needed.
 *
 * @param philo       the philo.
 * @param fork1       The first fork to pick up if in need to eat.
 * @param fork2       The second fork to pick up if in need to eat.
 *
 * The amount of accumulated time slept is keept in the var philo->time_slept.
 * We calculate a safety margin as the smaller between the time to die * 0.9
 * or the time left in the sleeping cycle.
 * If this margin becomes smaller than the time since the last meal we stop the
 * sleep cycle and send the philo to eat.
 * Once is done with eating the Philosopher will start thinking...this at least
 */
static void	erratic_sleep(t_philo *p)
{
	long long	chk_int;
	long long	sft_margn;

	chk_int = p->time_to_sleep / 10;
	if (p->time_to_sleep > p->time_to_die)
		chk_int = p->time_to_die / 10;
	log_activity(p, "is sleeping");
	while (p->time_slept < p->time_to_sleep)
	{
		sft_margn = p->time_to_die * 0.9;
		// Review this using pen and paper, in both projects!!!!!
		if (p->time_to_die < p->time_to_sleep - p->time_slept - chk_int)
			sft_margn = (p->time_to_sleep - p->time_slept - chk_int) * 0.9;
		if (get_timestamp() - p->last_meal_time <= sft_margn)
		{
			eat(p);
			log_activity(p, "is sleeping");
		}
		usleep(chk_int);
		p->time_slept += chk_int;
		if (p->time_to_sleep - p->time_slept < chk_int)
			chk_int = p->time_to_sleep - p->time_slept;
		verify_death(p);
	}
	p->time_slept = 0;
	log_activity(p, "is thinking");
}

/**
 * This function represents the behavior of each philo in the simulation.
 *
 * Even if here we have lesser problems of conflicts (being forks all piled)
 * in order to lower the risk of deadlocks and we scrumble up the starting:
 * each philo with an even ID will start by sleeping, the others by eating.
 *
 * The function will also check for the philo's death using `verify_death`.
 *
 * @param arg A pointer to the `t_philo` created in the main program.
 */
void	*philo_cycle(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	philo->semaphores.log_sem = sem_open("log_sem", 0);
	philo->semaphores.simulation_sem = sem_open("sim_sem", 0);
	philo->semaphores.fork_pool = sem_open("fork_pool", 0);
	sem_wait(philo->semaphores.simulation_sem);
	philo->last_meal_time = get_timestamp();
	philo->time_slept = 0;
	philo->times_eaten = 0;
	while (1)
	{
		if (philo->id % 2 == 0)
		{
			erratic_sleep(philo);
			eat(philo);
		}
		if (philo->id % 2 != 0)
		{
			eat(philo);
			erratic_sleep(philo);
		}
		verify_death(philo);
	}
	return (NULL);
}
