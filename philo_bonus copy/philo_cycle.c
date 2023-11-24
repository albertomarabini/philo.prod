/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 16:02:38 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/20 22:25:01 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

/**
 * We verify if the simulation has been stopped by anther thread.
 * If so we release the currenly held resources and we exit before
 * to log the event
 */
static void verify_simulation_status(t_philo *philo)
{
    sem_wait(&(philo->shared_resources->simulation_sem));
    if (!philo->shared_resources->simulation_active)
    {
        sem_post(&(philo->shared_resources->simulation_sem));
        while (philo->holding_forks--)
			sem_post(&(philo->shared_resources->fork_pool));
		printf("%lld %d %s\n", get_timestamp(), philo->id, "Exit by global command");
        exit(0);
    }
    sem_post(&(philo->shared_resources->simulation_sem));
}

/**
 * We write a log on screen.
 * The reason for using a mutex is that at high speed the screen can become
 * jumbled.
 */
static void	log_activity(t_philo *philo, const char *activity)
{
	verify_simulation_status(philo);
	sem_wait(&(philo->shared_resources->log_sem));
	printf("%lld %d %s\n", get_timestamp(), philo->id, activity);
	sem_post(&(philo->shared_resources->log_sem));
}

/**
 * We verify if ((get_timestamp()-philo->last_meal_time) >= philo->time_to_die)
 * if so we update the state of the simulation and we release each resource
 * keept by the thread.
 * The reason for using a mutex is that the flag simulation_active in a multi
 * threaded environment can really be different for every thread. This is a way
 * of having a safe access to it.
 */
static void	verify_death(t_philo *philo)
{
	verify_simulation_status(philo);
	if ((get_timestamp() - philo->last_meal_time) >= philo->time_to_die)
	{
		sem_wait(&(philo->shared_resources->simulation_sem));
		philo->shared_resources->simulation_active = 0;
		sem_post(&(philo->shared_resources->simulation_sem));
		while (philo->holding_forks--)
			sem_post(&(philo->shared_resources->fork_pool));
		log_activity(philo, "died");
		exit(0);
	}
}

/**
 * Executes the eating cycle for a philo in the simulation (picking forks,
 * eating for a given time, release the forks)
 *
 * If a philosopher has eaten the number of times specified in the optional
 * input parameter, the simulation ends for that thread.
 * Since while a philo waits for a mutex to be released will stay idle, every
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
 * @param fork1 Pointer to the mutex for the first fork to be acquired.
 * @param fork2 Pointer to the mutex for the second fork to be acquired.
 *
 */
static void	eat(t_philo *philo)
{
	log_activity(philo, "is thinking");
	sem_wait(&(philo->shared_resources->fork_pool));
	philo->holding_forks++;
	verify_death(philo);
	log_activity(philo, "has taken a fork");
	sem_wait(&(philo->shared_resources->fork_pool));
	philo->holding_forks++;
	verify_death(philo);
	log_activity(philo, "has taken a fork");
	log_activity(philo, "is eating");
	usleep(philo->time_to_eat * 1000);
	sem_post(&(philo->shared_resources->fork_pool));
	sem_post(&(philo->shared_resources->fork_pool));
	philo->holding_forks = 0;
	philo->last_meal_time = get_timestamp();
	if (philo->num_of_eating_times != -1)
	{
		philo->times_eaten++;
		if(philo->times_eaten >= philo->num_of_eating_times)
		{
			sem_wait(&(philo->shared_resources->simulation_sem));
			philo->shared_resources->simulation_active = 0;
			sem_post(&(philo->shared_resources->simulation_sem));
			printf("%lld %d %s\n", get_timestamp(), philo->id, "N Eats reached");
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
static void erratic_sleep(t_philo *philo)
{
    long long check_interval;
    long long safety_margin;

	check_interval = philo->time_to_sleep;
	if (check_interval > philo->time_to_die)
		check_interval = philo->time_to_die;
	check_interval = check_interval / 10;
	log_activity(philo, "is sleeping");
	while (philo->time_slept < philo->time_to_sleep)
	{
		safety_margin = philo->time_to_die * 0.9;
		//Review this!
		if(philo->time_to_die < philo->time_to_sleep - philo->time_slept - check_interval)
			safety_margin = (philo->time_to_sleep - philo->time_slept - check_interval) * 0.9;
		if (get_timestamp() - philo->last_meal_time <= safety_margin)
		{
			eat(philo);
			log_activity(philo, "is sleeping");
		}
		usleep(check_interval);
		philo->time_slept += check_interval;
		if (philo->time_to_sleep - philo->time_slept < check_interval)
			check_interval = philo->time_to_sleep - philo->time_slept;
		verify_death(philo);
	}
	philo->time_slept = 0;
	log_activity(philo, "is thinking");
}


/**
 * This function represents the behavior of each philo in the simulation.
 *
 * In order to avoid deadlocks and conflicts we scrumble up the starting state:
 *  each philo with an even ID will start by sleeping, the others by eating.
 * The even's will also try to pick first the left fork while the odd the right
 *
 * The function will also check for the philo's death using `verify_death`.
 *
 * @param arg A pointer to the `t_philo` created in the main program.
 */
void	*philo_cycle(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
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
