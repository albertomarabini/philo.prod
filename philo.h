/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/22 23:34:10 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/22 23:34:11 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_shared
{
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	simulation_mutex;
	int				simulation_active;
}					t_shared;

typedef struct s_philo
{
	int				id;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				num_of_eating_times;
	long long		last_meal_time;
	long long		time_slept;
	int				times_eaten;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_shared		*shared_resources;
}					t_philo;

int					ft_atoi(char *nptr);
long long			get_timestamp(void);
size_t				ft_strlen(const char *s);
char				*ft_ulltoa(unsigned long long n);
void				*philo_cycle(void *arg);
void				verify_death(t_philo *philo, pthread_mutex_t *fork1,
						pthread_mutex_t *fork2);
void				log_activity(t_philo *philo, const char *activity,
						pthread_mutex_t *fork1, pthread_mutex_t *fork2);
void				verify_simulation_status(t_philo *philo,
						pthread_mutex_t *fork1, pthread_mutex_t *fork2);

#endif
