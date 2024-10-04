#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

void print2(char *str)
{
	 while (*str)
		 write(2, str++, 1);
}

int len_dbl(char **strs)
{
	int i;

	i = 0;
	while (strs[i])
		i++;
	return (i);
}

void _cd(char **path)
{
	if (len_dbl(path) != 1)
	{
		print2("error: cd: bad arguments\n");
		return ;
	}
	if (chdir(path[0]) == -1)
	{
		print2("error: cd: cannot change directory to ");
		print2(path[0]);
		print2("\n");
	}
}

int	peek(char **av, char *sep)
{	
	int i;

	i = 0;
	while (av[i])
	{
		if (!strcmp(av[i], sep))
			return (i);
		i++;
	}
	return (-1); 
}


void execute(char **av, char **envp)
{
	int pid;
	
	if (!strcmp(av[0], "cd"))
	{
		_cd(&av[1]);
		return ;
	}
	pid = fork();
	if (pid == -1)
	{
		print2("error: fatal\n");
		exit(1);
	}
	if (!pid)
	{
		execve(av[0], av, envp);
		print2("error: cannot execute ");
		print2(av[0]);
		print2("\n");
		exit(1);
	}
	waitpid(pid, NULL, 0);
}

void lookfor_pipe(char **av, char **envp)
{
	int index;
	int pid[2];
	int pipefd[2];
	
	if (av == NULL || av[0] == NULL)
		return ;
	index = peek(av, "|");
	if (index != -1)
	{
		av[index] = NULL;
		if (pipe(pipefd) == -1)
		{
			print2("error: fatal\n");
			exit(1);
		}
		pid[0] = fork();
		if (pid[0] == -1)
		{
			print2("error: fatal\n");
			exit(1);
		}
		if(!pid[0])
		{
			close(pipefd[0]);
			dup2(pipefd[1], 1);
			close(pipefd[1]);
			execute(av, envp);
			exit(0);
		}
		pid[1] = fork();
		if (pid[1] == -1)
		{
			print2("error: fatal\n");
			exit(1);
		}
		if (!pid[1])
		{
			close(pipefd[1]);
			dup2(pipefd[0], 0);
			close(pipefd[0]);
			lookfor_pipe(&av[index + 1], envp);
			exit(0);
		}
		close(pipefd[0]);
		close(pipefd[1]);
		waitpid(pid[0], NULL, 0);
		waitpid(pid[1], NULL, 0);
	}
	else
		execute(av, envp);
}	

void	lookfor_cmd_seperator(char **av, char **envp)
{
	int index;
   
	if (av == NULL || av[0] == NULL)
		return ;
	index = peek(av, ";");
	if (index != -1)
	{
		av[index] = NULL;
		lookfor_pipe(av, envp);
		lookfor_cmd_seperator(&av[index + 1], envp);
	}
	else
		lookfor_pipe(av, envp);
}

int	main(int ac, char **av, char **envp)
{
	(void)ac;
	lookfor_cmd_seperator(&av[1], envp);
	return (0);
}
