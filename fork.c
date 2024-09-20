#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

/*microshell*/

void	execute(char **av, char **envp)
{
	int	pid;

	pid = fork();
	// ls
	if (pid == 0)
	{
		execve(av[0], &av[1], envp);
		perror("execve");
		exit(0);
	}
}
int	dblptr_len(char **av)
{
	int	len = 0;

	while (av[len])
		len++;
	return (len);
}

void	lookfor_pipe(int ac, char **av, char **envp)
{
	int	idx;
	int pipefd[2];
	int pid;
	int pid2;
	int status;

	idx = 0;
	//[ls][|][ls][NULL] 
	while (idx < ac && av[idx])
	{
		if (!strcmp(av[idx], "|"))
			break;
		idx++;
	}
	// idx == 1 && ac = 3
	if (idx != ac)
	{
		av[idx] =  NULL;
		// [ls][NULL][ls][NULL]
		pipe(pipefd);
		//  1 ==> 0
		pid = fork();
		if (!pid)
		{
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
			execute(av, envp);
			exit(0);
		}
		pid2 = fork();
		if (!pid2)
		{
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			lookfor_pipe(dblptr_len(&av[idx + 1]), &av[idx + 1], envp);
			exit(0);
		}
		waitpid(pid, &status, 0);
		waitpid(pid2, &status, 0);
		close(pipefd[0]);
		close(pipefd[1]);
	}
	else
		execute(av, envp);		
}


void	lookfor_cmd_separator(int ac, char **av, char **envp)
{
	int i;

	i = 0;
	while (av[i])
	{
		if (!strcmp(av[i], ";"))
			break;
		i++;
	}
	// i = 3 && ac = 8;
	if (i != ac)
	{
		av[i] = NULL;
		//[ls][|][ls][NULL]                   [echo][salam][|]][ls][NULL]
		lookfor_pipe(dblptr_len(av), av, envp);
		lookfor_cmd_separator(dblptr_len(av + i + 1), &av[i + 1], envp);
	}
	else
		lookfor_pipe(ac, av, envp);
}


int main(int ac, char **av, char **envp)
{
	//bin/ls | /bin/ls ; /bin/echo salam | /bin/ls
	lookfor_cmd_separator(ac - 1, &av[1], envp);
	return (0);
}



































/* int main(void)
{
	int pid;
	int n;
	pid = fork();
	if (pid == 0)
		n = 1;
	else
		n = 6;
	if (pid != 0)
		wait(0);
	for (int i = n; i < n + 5 ;i++)
	{
		printf("%d", i);
		fflush(stdout);
	}
	if (pid != 0)
		printf("\n");
	return (0);
}
 */
/* 
int main(int ac, char **av, char **envp)
{
	char *arg[3] = {"/bin/ls","-la", NULL}; 
	int status = 0;
	int pid = fork();
	if (!pid)
	{
		execve(arg[0], arg, envp);
		perror("execve");
	}
	waitpid(pid, &status, 0);	
	return (status << 8);
}
 */