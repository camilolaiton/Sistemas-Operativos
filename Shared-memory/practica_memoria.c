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

void error(char *);	//Funcion basica de Error

int main(int argc, char const *argv[])
{
	void *ptr = NULL;	//Puntero para cast y direccion de espacio memoria
	int shm_id = 0;	//Variable for saving shared memory id
	int shm_size = 1024;	//Size of the space

	shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);	//Let's create the space memory
	//This function returns -1 if the space couldn't be created, and the id if it could

	if(shm_id != -1)
	{
		ptr = shmat(shm_id, 0, 0);//Let's attach the memory space into this process
		//This function returns NULL if it couldn't attach the memory space

		if(ptr)
		{

			if(!fork())//Here we create the new process
			{
				sleep(2);
				printf("[%d]-> Valor: [%s]\n", getpid(), (char*)ptr);
				sprintf(ptr, "Bye! Enviado de: [%d]", getpid());
				shmdt(ptr);
			}
			else//Change the memory space value
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