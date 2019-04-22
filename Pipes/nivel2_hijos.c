#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define MAX_SPACE_TUB 1048576	//Espacio maximo de tuberia

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
*/

void error(char*);
void esperar_padre(int);
void enviar_info(int);
void leer_tuberia(int);

int fd[2][2] = {0};

int main(int argc, char const *argv[])
{
	int p1 = 0, p2 = 0;
	int n_procesos = 2, n_tubs = 2;
	pid_t hijo = (pid_t)(0), padre = getpid(), hijo2 = (pid_t)(0);

	char instruccion[50] = {'\0'};
	sprintf(instruccion,"pstree -lp %d", getpid());

	for (int i = 0; i < 2; ++i)
	{
		if(pipe(fd[i]) < 0)
			error("No se ha podido crear la tuberia");

		printf("fd[%d] : [%d][%d]\n", i, fd[i][0], fd[i][1]);
	}

	for (p1 = 0; p1 < n_procesos; ++p1)
	{
		hijo = fork();

		if(hijo == -1)
		{
			error("No se pudo crear nuevo proceso");
		}
		else if(!hijo)
		{

			hijo2 = fork();

			if(hijo2 == -1)
			{
				error("No se pudo crear espacio de memoria compartida");
			}
			else if(!hijo2)
			{
				p2++;
			}

			break;
		}
	}

	if(padre == getpid())
	{
		/* soy padre */

		printf("PID:[%d] -> Soy el padre\n", getpid());
		system(instruccion);

		for (int i = 0; i < n_tubs; ++i)
		{
			close(fd[i][0]);
			close(fd[i][1]);
		}

		esperar_padre(n_procesos*2);

		printf("PID: [%d] -> Finalizo proceso.\n", getpid());
	}
	else
	{

		sleep(2);

		if( (p1 == 0 || p1 == 1) && p2 == 0)	/* Soy hijo nivel 0 */
		{
			for (int i = 0; i < n_tubs; ++i)
			{
				if(i == p1)
				{
					close(fd[i][0]);
				}
				else
				{
					close(fd[i][0]);
					close(fd[i][1]);
				}
			}

			enviar_info(p1);
			close(fd[p1][1]);
		}
		else	/* Soy hijo interno */
		{
			for (int i = 0; i < n_tubs; ++i)
			{
				if(i == p1)
				{
					close(fd[i][0]);
					close(fd[i][1]);
				}
				else
				{
					close(fd[i][1]);
				}
			}

			if(p1 == 0)
			{
				leer_tuberia(p1+1);
			}
			else 
			{
				leer_tuberia(p1-1);
			}

			close(fd[p1][0]);
		}

		exit(EXIT_SUCCESS);
	}

	return 0;
}

void leer_tuberia(int n_tub)
{
	char buffer[50] = {'\0'};
	int bytes = 0, n = 0;

	n = read(fd[n_tub][0], &bytes, sizeof(bytes));
	
	n = read(fd[n_tub][0], buffer, bytes);

	if( n > 0)
	{	
		printf("PID: [%d] -> Lectura: [%s] - Tuberia: [%d]\n", getpid(), buffer, n_tub);
	}

}

void enviar_info(int n_tub)
{
	
	char buffer[50] = {'\0'};
	int bytes = 0;
	
	sprintf(buffer, " SOY PID: [%d]", getpid());

	bytes = strlen(buffer);

	if( (write(fd[n_tub][1], &bytes, sizeof(bytes))) < 0 )
		error("Error al enviar informacion en la tuberia");

	if( (write(fd[n_tub][1], buffer, bytes)) < 0 )
		error("Error al enviar informacion en la tuberia");

	printf("Mensaje [%s] enviado de [%d] a tuberia [%d]\n", buffer, getpid(), n_tub);

}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void esperar_padre(int procesos)
{
	if(procesos > 0)
	{
		int ver = 0, proceso = 0;

		for (int i = 0; i < procesos; ++i)
		{
			proceso = wait(&ver);

			if(WIFEXITED(ver))
			{
				//printf("El proceso [%d] termino de manera correcta! Valor: [%d]\n", proceso, WIFEXITED(ver));
			}
			else
			{
				//printf("El proceso [%d] termino de manera abrupta! Valor: [%d]\n", proceso, WIFEXITED(ver));
			}
		}
	}
	else
	{
		error("No se puede esperar una cantidad de procesos inexistente");
	}
}