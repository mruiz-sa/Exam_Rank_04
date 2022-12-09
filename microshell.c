#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

int g_fd;

static int print(char *str)
{
	int i;

	i = 0;
	while (str[i])
		i++;
	write(2, str, i);
	return (1);
}

static int exec_cd(char **av)
{
	if (av[2] && strcmp(av[2], "|") && strcmp(av[2], ";"))
		return (0);
	if (chdir(av[1]) == -1)
	{
		print("error: cd: cannot change directory to ");
		print(av[1]);
		print("\n");
		return (0);
	}
	return (0);
}

static int executor(char **av, int i, char **env)
{
	int pid;
	int status;
	int next;
	int fd[2];

	next = 0;
	if (av[i] && !strcmp(av[i], "|"))
		next = 1;
	if (av[i] == *av)
		return (0);
	if (pipe(fd) == -1)
	{
		print("error: fatal\n");
		exit(0);
	}
	pid = fork();
	if (pid == -1)
	{
		print("error: fatal\n");
		exit(0);
	}
	else if (pid == 0)
	{
		close(fd[0]);
		dup2(g_fd, 0);
		av[i] = 0;
		if (next)
			dup2(fd[1], 1);
		if (g_fd)
			close(g_fd);
		close(fd[1]);
		if (execve(*av, av, env) == -1)
		{
			print("error: cannot execute ");
			print(*av);
			print("\n");
			exit(0);
		}
	}
	else
	{
		close(fd[1]);
		waitpid(pid, &status, 0);
		if (g_fd)
			close(g_fd);
		if (next)
			g_fd = dup(fd[0]);
		close(fd[0]);
	}
	return (0);
}

int main(int ac, char **av, char **env)
{
	int i;

	i = 1;
	if (ac == 1)
		return (0);
	av[ac] = 0;
	while (av[i] && av[i - 1])
	{
		av = av + i;
		i = 0;
		while (av[i] && strcmp(av[i], "|") && strcmp(av[i], "|"))
			i++;
		if (!strcmp(*av, "cd"))
			exec_cd(av);
		else
			executor(av, i, env);
		i++;
	}
}
