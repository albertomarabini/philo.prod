#include "philo_bonus.h"

int	ft_atoi(char *nptr)
{
	int			sign;
	long long	result;
	int			i;

	sign = 1;
	result = 0;
	i = 0;
	if (!nptr)
		return (0);
	while (nptr[i] < '0' || nptr[i] > '9')
		if (nptr[i++] == '-')
			sign *= -1;
	while (nptr[i] >= '0' && nptr[i] <= '9')
		result = (result * 10) + (nptr[i++] - '0');
	return ((int)(result * sign));
}

size_t	ft_strlen(const char *s)
{
	size_t	len;

	if (!s)
		return (0);
	len = 0;
	while (s[len])
		len++;
	return (len);
}

static char	*get_ulltoa_str(unsigned long long n, int l)
{
	int		l2;
	char	*str;

	str = (char *)malloc((l + 1) * sizeof(char));
	if (!str)
		return (NULL);
	l2 = l;
	while (0 < l--)
	{
		str[l] = (n % 10) + '0';
		n /= 10;
	}
	str[l2] = '\0';
	return (str);
}

char	*ft_ulltoa(unsigned long long n)
{
	int					l;
	unsigned long long	n_copy;

	l = 1;
	n_copy = n;
	if (n != 0)
	{
		l = 0;
		while (n_copy > 0)
		{
			n_copy /= 10;
			l++;
		}
	}
	return (get_ulltoa_str(n, l));
}

long long	get_timestamp(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (((tv.tv_sec) * 1000 + (tv.tv_usec)) / 1000);
}
