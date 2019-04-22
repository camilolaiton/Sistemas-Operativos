#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <wait.h>
#include <stdlib.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
*/

void error(char *);

int main(int argc, char const *argv[])
{
	void *ptr = NULL;
	int shm_id = 0;
	int shm_size = 1024;

	shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);

	if(shm_id != -1)
	{
		ptr = shmat(shm_id, 0, 0);

		if(ptr)
		{

			if(!fork())
			{
				sleep(2);
				printf("[%d]-> Valor: [%s]\n", getpid(), (char*)ptr);
				sprintf(ptr, "Bye! Enviado de: [%d]", getpid());
				shmdt(ptr);
			}
			else
			{
				sprintf(ptr, "hola mundo -> [%d]", getpid());
				printf("[%d] -> Valor: [%s]\n", getpid(), (char*)ptr);
				wait(NULL);
				printf("[%d] -> Valor: [%s]\n", getpid(), (char*)ptr);
				shmdt(ptr);
				shmctl(shm_id, IPC_RMID, 0);
			}

		}
		else
		{
			error("Error al momento de acomodar la memoria");
		}
	}
	else
	{
		error("Error al momento de crear la memoria compartida");
	}

	return 0;
}

void error(char *str)
{
	perror(str);
	exit(EXIT_FAILURE);
}