#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_PROCESOS 7

/*
	Camilo Laiton y Cristian Vergel
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	Sistemas Operativos
	2019 - 1
*/

int validar_numero(int);//Funcion para validar numero ingresado
int enviar_info();//Funcion a ser usada solo por el padre
void leer_info(int);//Funcion que servira para leer la tuberia
int reenviar(int);//Funcion usada por los hijos para reenviar la informacion
void error(char *);//Funcion de error
int conocer_proceso(pid_t [], int);//funcion para conocer el proceso actual
void esperar_padre(int);//funcion que va a esperar la cantidad de hijos que se hayan creado

int **fd;

int main(int argc, char const *argv[])
{
	int sigo = 1;//Variable que servira como bandera
	pid_t *hijos;//Variable para guardar los pids -> sera de utilidad luego
	pid_t father = getpid();//guardo el pid del padre
	int cantidad = 0, n_tuberias = 0;//Variables para guardar cantidades

	printf("INFO: [Para salir de la tuberia escriba -> [salir]]\n");
	printf("Ingrese la cantidad de procesos: \n");//Pido la cantidad de procesos
	fflush(stdin);	//Limpio el buffer
	scanf("%d\n", &cantidad);//Guardo en la variable la cantidad de procesos
	
	if(validar_numero(cantidad))//Valido que el numero sea correcto
	{
		n_tuberias = cantidad + 1;	//La cantidad de tuberias sera mayor en uno que la cantidad

		fd = (int**)malloc(n_tuberias*sizeof(int*));	//Creo los fd dinamicamente

		for (int i = 0; i < n_tuberias; ++i)//Creo los dos espacios para la matriz
		{
			fd[i] = (int*)malloc(2*sizeof(int));
		}

		hijos = (pid_t*)malloc(cantidad*sizeof(pid_t));//Creo el espacio para los pid de los hijos

		for (int i = 0; i < cantidad; ++i)	//Inicializo los espacios en 0
		{
			hijos[i] = (pid_t)(0);
		}

		for (int i = 0; i < n_tuberias; ++i)//creo las tuberias
		{
			if(pipe(fd[i]) < 0)
				error("Error al crear el proceso");
		}

		for (int i = 0; i < cantidad; ++i)//Creo la cantidad de hijso para fork
		{
			hijos[i] = fork();

			if(hijos[i] == -1)	//Valido que se pudo crear
			{
				error("Ha ocurrido un error al crear el proceso");
			}
			else if(!hijos[i])	//Si es un hijo...
			{
				for (int w = 0; w < n_tuberias; ++w)	//Cierro los fd que no necsitamos
				{
					if(w == i)
					{
						close(fd[w][1]);	//dejo lectura abierta
					}
					else if(w == i+1)
					{
						close(fd[w][0]);	//Dejo escritura abierta
					}
					else	//Cierro los fd que no necesitamos
					{
						close(fd[w][0]);
						close(fd[w][1]);
					}
				}

				break;	//Salgo si es un hijo
			}
		}

		if(father == getpid())//Si soy el padre
		{
			for (int i = 0; i < n_tuberias; ++i)	//Cierro los fd que no necesite
			{
				if(i == 0)
				{
					close(fd[i][0]);	//Cierro la lectura para la primer tuberia
				}
				else if(i == (n_tuberias - 1) )
				{
					close(fd[i][1]);	//Cierro escritura pra la ultima
				}
				else
				{
					close(fd[i][0]);	//Cierro los otros que no necesite
					close(fd[i][1]);
				}
			}

			while(sigo)	//Mientras que lea
			{
				if(enviar_info())	//Envio info
				{
					close(fd[0][1]);	//Cuando ya no necesite enviar mas cierro los fd
					close(fd[n_tuberias-1][0]);
					printf("PID: [%d] PADRE -> Cierro decriptores por 'SALIR'\n", getpid());
					sigo = 0;
				}

				if(sigo)	//Si sigo leyendo leo de la tub
					leer_info(n_tuberias);
			}
		}
		else
		{
			/*Soy un hijo*/

			int proceso = 0;	//Veo el proceso

			while(sigo)
			{
				proceso = conocer_proceso(hijos, cantidad);	//Conozco proceso actual
				sigo = reenviar(proceso);	//reenvio la info
			}
			
			free(fd);	//Libero memoria
			free(hijos);
			exit(EXIT_SUCCESS);//Salgo exitoso hijo
		}
	}
	else
	{
		error("Error en la cantidad de procesos");
	}

	esperar_padre(cantidad);	///espero los nhijos

	free(fd);	//Libero memoria
	free(hijos);
	return 0;
}

void esperar_padre(int procesos)
{
	if(procesos > 0)
	{
		int ver = 0;

		for (int i = 0; i < procesos; ++i)
		{
			wait(&ver);

			if(WIFEXITED(ver))
			{
				printf("El proceso termino de manera correcta! Valor: [%d]\n", WIFEXITED(ver));
			}/*
			else
			{
				printf("El proceso termino de manera abrupta! Valor: [%d]\n", WIFEXITED(ver));
			}*/
		}
	}
	else
	{
		error("No se puede esperar una cantidad de procesos inexistente");
	}
}

int conocer_proceso(pid_t hijos[], int procesos)
{
	int acu = 0;

	if(procesos > 0)
	{
		for (int i = 0; i < procesos; ++i)
		{
			if(hijos[i] != 0)
				acu++;
		}
	}
	else
	{
		error("Error al momento de encontrar el proceso");
	}

	return acu;
}

int reenviar(int n_tub)
{
	int n = 0, bytes = 0;
	char buffer[50] = {'\0'};

	n = read(fd[n_tub][0], &bytes, sizeof(bytes));
	n = read(fd[n_tub][0], buffer, bytes);

	if(n != 0)
	{
		printf("PID: [%d] -> Mensaje recibido: [%s]\n", getpid(), buffer);

		if( (write(fd[n_tub+1][1], &bytes, sizeof(bytes))) < 0)
			error("Error al mandar por la tuberia");

		if( (write(fd[n_tub+1][1], buffer, bytes)) < 0)
			error("Error al mandar por la tuberia");

		return 1;
	}
	else
	{
		close(fd[n_tub][0]);
		close(fd[n_tub+1][1]);
		printf("PID: [%d] -> Cierro decriptores por 'SALIR'\n", getpid());
	}

	return 0;
}

int enviar_info()
{
	char buffer[50] = {'\0'};
	int bytes = 0;

	do
	{
		printf("\nPID: [%d] SOY PADRE -> Ingrese el mensaje: \n", getpid());
		fflush(stdin);
		scanf("%[^\n]", buffer);
		bytes = strlen(buffer);
		getchar();
	
	}while(bytes == 0);

	if(buffer[bytes -1] == '\n')
		bytes--;

	if( (write(fd[0][1], &bytes, sizeof(bytes))) < 0)
		error("No se pudo enviar mensaje por la tuberia [0]");

	if( (write(fd[0][1], buffer, bytes)) < 0)
		error("No se pudo enviar mensaje por la tuberia [0]");

	printf("PID: [%d] SOY PADRE -> Mensaje enviado: [%s]\n", getpid(), buffer);

	if(strcmp(buffer, "salir") == 0)
		return 1;
	return 0;
}

void leer_info(int n_tuberias)
{
	int bytes = 0, n = 0;
	char buffer[50] = {'\0'};

	n = read(fd[n_tuberias-1][0], &bytes, sizeof(bytes));
	
	if(n != 0)
	{
		n = read(fd[n_tuberias-1][0], buffer, bytes);
		printf("PID: [%d] -> Info recibida: [%s]\n", getpid(), buffer);	
	}
}

void error(char *str)
{
	perror(str);
	exit(EXIT_FAILURE);
}

int validar_numero(int cantidad)
{
	if(cantidad > 0 && cantidad <= MAX_PROCESOS)
	{
		return 1;
	}

	return 0;
}
