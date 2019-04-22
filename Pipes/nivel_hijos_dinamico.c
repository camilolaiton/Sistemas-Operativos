#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

//https://gerardnico.com/os/linux/shared_memory

#define MAX_PROCESOS 1023

int validar_numero(int);
void error(char*);
void esperar_padre(int);
int enviar_info();
void leer_info(int);
int reenviar(int);

int **fd = NULL;

int main(int argc, char const *argv[])
{
	int cantidad = 0, n_tuberias = 0, n_proc = 0, p = 0, p_nivel = 0, sigo = 1;
	int n_ramas = 2;
	pid_t hijo = (pid_t)(0);
	pid_t padre = getpid();
	printf("Ingrese la cantidad de niveles: \n");
	fflush(stdin);
	scanf("%d\n", &cantidad);
	
	if(validar_numero(cantidad))
	{
		n_proc = cantidad*n_ramas;
		n_tuberias = (cantidad*n_ramas) + 1;

		fd = (int**)malloc(n_tuberias*sizeof(int*));

		for (int i = 0; i < n_tuberias; ++i)
		{
			fd[i] = (int*)malloc(2*sizeof(int));
			if(pipe(fd[i]) < 0)
				error("No se pudo crear la tuberia");

			printf("fd[%d] : [%d][%d]\n", i, fd[i][0], fd[i][1]);
		}

		for (p = 0; p < n_ramas; ++p)
		{
				
			hijo = fork();

			if(hijo == -1)
			{
				error("No se pudo crear la tuberia");
			}
			else if(!hijo)
			{
				repetir:

				if(cantidad == 1)
					break;

				hijo = fork();

				if(hijo == -1)
				{
					error("No se pudo crear tuberia interna");
				}
				else if(!hijo)
				{
					p_nivel++;

					if(p_nivel+1 < cantidad)
						goto repetir;
					
				}

				break;
			}
		}

		if(padre == getpid())
		{
			char instruccion[50] = {'\0'};
			sprintf(instruccion,"pstree -lp %d", getpid());

			for (int i = 0; i < n_tuberias; ++i)
			{
				if(i != n_tuberias-1)
				{
					close(fd[i][0]);
				}
				else
				{
					//printf("PID[%d] -> Padre deja abierta tuberia [%d] modo lectura\n", getpid(), i);
				}
				
				if(i != 0)
				{
					close(fd[i][1]);
				}
				else
				{
					//printf("PID[%d] -> Padre deja abierta tuberia [%d] modo escritura\n", getpid(), i);
				}

			}

			while(sigo)
			{
				system(instruccion);
				sigo = enviar_info();
				leer_info(n_tuberias-1);
				//printf("PID[%d] padre -> sigo[%d]\n", getpid(), sigo);
			}

			close(fd[n_tuberias-1][0]);
			close(fd[0][1]);
		}
		else
		{

			//printf("PID[%d] -> P: [%d] - P_NIVEL: [%d]\n",getpid(), p, p_nivel);


			int sobra = 0;

			if(p == 0)
			{
				sobra = cantidad - (p_nivel+1);

				if(sobra == 0)
				{
					sobra = cantidad;
					//printf("PID[%d] -> sobra igual: [%d]\n", getpid(), sobra);
				}
				else
				{
					sobra += cantidad;
					//printf("PID[%d] -> sobra: [%d]\n", getpid(), sobra);
				}
			}

			for (int i = 0; i < n_tuberias; ++i)
			{
				if(p == 1)
				{
					if(i != p_nivel)
					{
						close(fd[i][0]);
					}
					else
					{
						sobra = i;
						//printf("PID: [%d] -> Dejo abierta tuberia [%d] en modo lectura P:[%d]\n", getpid(), i, p);
					}

					if(i != p_nivel+1)
					{
						close(fd[i][1]);
					}
					else
					{
						//printf("PID: [%d] -> Dejo abierta tuberia [%d] en modo escritura P:[%d]\n", getpid() , i, p);
					}
				}
				else
				{
				
					if(sobra != i)
					{
						close(fd[i][0]);
					}
					else
					{
						//printf("PID: [%d] -> Dejo abierta tuberia [%d] en modo lectura P:[%d]\n", getpid(), i, p );
					}

					if(sobra+1 != i)
					{
						close(fd[i][1]);
					}
					else
					{
						//printf("PID: [%d] -> Dejo abierta tuberia [%d] en modo escritura P:[%d]\n", getpid(), i, p );
					}				
				}
			}

			//Hago proceso

			//printf("PID[%d] -> sobra: [%d]\n", getpid(), sobra);

			while(sigo)
			{
				sigo = reenviar(sobra);
				//printf("PID[%d] hijo -> sigo[%d]\n", getpid(), sigo);
			}

			//Cierro las que hagan falta
			close(fd[sobra][0]);
			close(fd[sobra+1][1]);

			free(fd);
			exit(EXIT_SUCCESS);

		}

		free(fd);
	}
	else
	{
		error("Numero de niveles invalido!");
	}

	return 0;
}

int validar_numero(int cantidad)
{	
	int n = (cantidad*2) + 1;

	if(cantidad > 0 && n <= MAX_PROCESOS)
	{
		return 1;
	}

	return 0;
}

int reenviar(int n_tub)
{
	char buffer[50] = {'\0'}, merr[50] = {'\0'};
	int bytes = 0, n = 0;

	sprintf(merr, "No se pudo leer - PID[%d] -> Tub [%d]", getpid(), n_tub);

	if( ( n = read(fd[n_tub][0], &bytes, sizeof(bytes)) ) < 0)
		error(merr);

	if( ( n = read(fd[n_tub][0], buffer, bytes)) < 0)
		error(merr);

	if( n != 0)
	{
		memset(merr, '\0', 50);

		sprintf(merr, "No se pudo enviar - PID[%d] -> Tub [%d]", getpid(), n_tub);

		if( write(fd[n_tub+1][1], &bytes, sizeof(bytes)) < 0)
		error(merr);

		if( write(fd[n_tub+1][1], buffer, bytes) < 0)
			error(merr);

		printf("PID: [%d] -> Info recibida: [%s] - Tub: [%d]\nPID: [%d] -> Info enviada: [%s] - Tub: [%d]\n", 
		getpid(), buffer, n_tub, getpid(), buffer, n_tub+1);

		return 1;
	}
	else
	{
		close(fd[n_tub][0]);
		close(fd[n_tub+1][1]);
	}

	return 0;
}

int enviar_info()
{

	char buffer[50] = {'\0'};
	int bytes = 0;

	do
	{
		printf("PID: [%d] PADRE -> Ingrese el mensaje a enviar: \n", getpid());
		fflush(stdin);
		scanf("%[^\n]", buffer);
		bytes = strlen(buffer);
		getchar();

	}while(bytes == 0);

	if( write(fd[0][1], &bytes, sizeof(bytes)) < 0)
		error("No se pudo enviar por la tuberia");

	if( write(fd[0][1], buffer, bytes) < 0)
		error("No se pudo enviar por la tuberia");

	printf("PID: [%d] SOY PADRE -> Mensaje enviado: [%s] - Tub [0]\n", getpid(), buffer);

	if(strcmp("salir", buffer) == 0)
		return 0;

	return 1;
}

void leer_info(int n_tub)
{
	char buffer[50] = {'\0'};
	int bytes = 0, n = 0;

	if( ( n = read(fd[n_tub][0], &bytes, sizeof(bytes)) ) < 0)
		error("No se pudo leer de tuberia");

	if(n != 0)
	{
		if( ( n = read(fd[n_tub][0], buffer, bytes)) < 0)
			error("No se pudo leer de tuberia");

		printf("PID: [%d] SOY PADRE -> Info recibida: [%s] - Tub: [%d]\n", getpid(), buffer, n_tub);
	}

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