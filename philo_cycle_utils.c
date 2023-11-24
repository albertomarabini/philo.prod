/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_cycle_utils.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 16:02:38 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/22 23:29:29 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * We verify if the simulation has been stopped by anther thread.
 * If so we release the currenly held resources and we exit before
 * to log the event
 */
void	verify_simulation_status(t_philo *philo, pthread_mutex_t *fork1,
		pthread_mutex_t *fork2)
{
	pthread_mutex_lock(&philo->shared_resources->simulation_mutex);
	if (!philo->shared_resources->simulation_active)
	{
		pthread_mutex_unlock(&philo->shared_resources->simulation_mutex);
		if (fork1)
			pthread_mutex_unlock(fork1);
		if (fork2)
			pthread_mutex_unlock(fork2);
		pthread_exit(NULL);
	}
	pthread_mutex_unlock(&philo->shared_resources->simulation_mutex);
}

/**
 * We write a log on screen.
 * The reason for using a mutex is that at high speed the screen can become
 * jumbled.
 */
void	log_activity(t_philo *philo, const char *activity,
		pthread_mutex_t *fork1, pthread_mutex_t *fork2)
{
	verify_simulation_status(philo, fork1, fork2);
	pthread_mutex_lock(&(philo->shared_resources->log_mutex));
	printf("%lld %d %s\n", get_timestamp(), philo->id, activity);
	pthread_mutex_unlock(&(philo->shared_resources->log_mutex));
}

/**
 * We verify if ((get_timestamp()-philo->last_meal_time) >= philo->time_to_die)
 * if so we update the state of the simulation and we release each resource
 * keept by the thread.
 * The reason for using a mutex is that the flag simulation_active in a multi
 * threaded environment can really be different for every thread. This is a way
 * of having a safe access to it.
 */
void	verify_death(t_philo *philo, pthread_mutex_t *fork1,
		pthread_mutex_t *fork2)
{
	verify_simulation_status(philo, fork1, fork2);
	if ((get_timestamp() - philo->last_meal_time) >= philo->time_to_die)
	{
		pthread_mutex_lock(&(philo->shared_resources->simulation_mutex));
		philo->shared_resources->simulation_active = 0;
		pthread_mutex_unlock(&(philo->shared_resources->simulation_mutex));
		if (fork1)
			pthread_mutex_unlock(fork1);
		if (fork2)
			pthread_mutex_unlock(fork2);
		log_activity(philo, "died", NULL, NULL);
		pthread_exit(NULL);
	}
}
