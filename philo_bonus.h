/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/22 23:34:19 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/22 23:34:20 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

# include <fcntl.h>
# include <limits.h>
# include <semaphore.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>

typedef struct s_sem
{
	sem_t		*log_sem;
	sem_t		*simulation_sem;
	sem_t		*fork_pool;
}				t_sem;

typedef struct s_philo
{
	int			id;
	int			time_to_die;
	int			time_to_eat;
	int			time_to_sleep;
	int			num_of_eating_times;
	long long	last_meal_time;
	long long	time_slept;
	int			times_eaten;
	int			holding_forks;
	t_sem		semaphores;
}				t_philo;

int				ft_atoi(char *nptr);
long long		get_timestamp(void);
size_t			ft_strlen(const char *s);
char			*ft_ulltoa(unsigned long long n);
void			*philo_cycle(void *arg);

#endif
