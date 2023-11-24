#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

# include <semaphore.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <limits.h>

typedef struct s_shared
{
	sem_t log_sem;
	sem_t simulation_sem;
	sem_t fork_pool;
	int simulation_active;
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
	int holding_forks;
	t_shared		*shared_resources;
}					t_philo;

int					ft_atoi(char *nptr);
long long			get_timestamp(void);
size_t				ft_strlen(const char *s);
char				*ft_ulltoa(unsigned long long n);
void				*philo_cycle(void *arg);

#endif
