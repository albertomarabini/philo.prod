/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 16:02:38 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/22 23:30:04 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * Executes the eating cycle for a philo in the simulation (picking forks,
 * eating for a given time, release the forks)
 *
 * If a philosopher has eaten the number of times specified in the optional
 * input parameter, the simulation ends for that thread.
 * Since while a philo waits for a mutex to be released will stay idle, every
 * time we have a potential deathlock we check for the eventual philo death.
 *
 * @param philo Pointer to the t_philo structure representing the philo.
 * @param fork1 Pointer to the mutex for the first fork to be acquired.
 * @param fork2 Pointer to the mutex for the second fork to be acquired.
 *
 */
static void	eat(t_philo *philo, pthread_mutex_t *fork1, pthread_mutex_t *fork2)
{
	log_activity(philo, "is thinking", NULL, NULL);
	pthread_mutex_lock(fork1);
	verify_death(philo, fork1, NULL);
	log_activity(philo, "has taken a fork", fork1, NULL);
	pthread_mutex_lock(fork2);
	verify_death(philo, fork1, fork2);
	log_activity(philo, "has taken a fork", fork1, fork2);
	log_activity(philo, "is eating", fork1, fork2);
	usleep(philo->time_to_eat * 1000);
	pthread_mutex_unlock(fork2);
	pthread_mutex_unlock(fork1);
	philo->last_meal_time = get_timestamp();
	if (philo->num_of_eating_times != -1)
	{
		philo->times_eaten++;
		if (philo->times_eaten >= philo->num_of_eating_times)
		{
			pthread_mutex_lock(&(philo->shared_resources->simulation_mutex));
			philo->shared_resources->simulation_active = 0;
			pthread_mutex_unlock(&(philo->shared_resources->simulation_mutex));
			pthread_exit(NULL);
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
static void	erratic_sleep(t_philo *p, pthread_mutex_t *fork1,
		pthread_mutex_t *fork2)
{
	long long	chk_int;
	long long	sft_mrg;

	chk_int = p->time_to_sleep / 10;
	if (p->time_to_sleep > p->time_to_die)
		chk_int = p->time_to_die / 10;
	log_activity(p, "is sleeping", NULL, NULL);
	while (p->time_slept < p->time_to_sleep)
	{
		sft_mrg = p->time_to_die * 0.9;
		// Review this, also in the bonus proj!!
		if (p->time_to_die < p->time_to_sleep - p->time_slept - chk_int)
			sft_mrg = (p->time_to_sleep - p->time_slept - chk_int) * 0.9;
		if (get_timestamp() - p->last_meal_time <= sft_mrg)
		{
			eat(p, fork1, fork2);
			log_activity(p, "is sleeping", NULL, NULL);
		}
		usleep(chk_int);
		p->time_slept += chk_int;
		if (p->time_to_sleep - p->time_slept < chk_int)
			chk_int = p->time_to_sleep - p->time_slept;
		verify_death(p, NULL, NULL);
	}
	p->time_slept = 0;
	log_activity(p, "is thinking", NULL, NULL);
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
			erratic_sleep(philo, philo->left_fork, philo->right_fork);
			eat(philo, philo->left_fork, philo->right_fork);
		}
		if (philo->id % 2 != 0)
		{
			eat(philo, philo->right_fork, philo->left_fork);
			erratic_sleep(philo, philo->right_fork, philo->left_fork);
		}
		verify_death(philo, NULL, NULL);
	}
	return (NULL);
}
