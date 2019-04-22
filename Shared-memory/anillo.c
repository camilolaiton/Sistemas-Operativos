#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <wait.h>
#include <stdlib.h>
#include <time.h>

void error(char*);
void esperar_padre(int);
unsigned int sizeof_dm(int, int, size_t);
void create_index(void **, int, int, size_t);
int validar_num(int, int);
int encontrar_num_proc(int, int);

int main(int argc, char const *argv[])
{

	srand(time(NULL));
	pid_t hijo = (pid_t)(0), padre = getpid();
	int shm_id_matriz = 0, shm_id_suma = 0;
	int *acu = 0, **matriz = NULL;
	int rows = 3, cols = 4, n_procesos = 0, i = 0;	//Calcular numero de procesos dependiendo la matriz

	if(validar_num(rows, cols))
	{
		n_procesos = encontrar_num_proc(rows, cols);
		size_t sizeMatriz = sizeof_dm(rows, cols, sizeof(int));
		shm_id_matriz = shmget(IPC_PRIVATE, sizeMatriz, IPC_CREAT | 0600);
		shm_id_suma = shmget(IPC_PRIVATE, n_procesos*sizeof(int), IPC_CREAT | 0600);

		if(shm_id_suma != -1 && shm_id_matriz != -1)
		{

			matriz = (int**)shmat(shm_id_matriz, NULL, 0);
			acu = (int*)shmat(shm_id_suma, NULL, 0);

			if(acu && matriz)
			{
				create_index((void*)matriz, rows, cols, sizeof(int));
				
				for (int p = 0; p < n_procesos; ++p)
				{
					acu[p] = 0;
				}

				printf("Numero de procesos: [%d]\n", n_procesos);

				for ( i = 0; i < n_procesos; ++i)
				{
					hijo = fork();

					if(hijo == -1)
					{
						error("No se pudo crear el proceso");
					}
					else if(!hijo)
					{
						break;
					}
				}

				if(padre == getpid())
				{
					/* Soy el padre */
					for (int r = 0; r < rows; ++r)
					{
						for (int c = 0; c < cols; ++c)
						{
							matriz[r][c] = 1; //1 + rand() % 9;
							printf("%d ", matriz[r][c]);
						}

						printf("\n");
					}

					char instruccion[400] = {'\0'};
					sprintf(instruccion, "pstree -lp %d", getpid());	//Mostrar el arbol de procesos
					system(instruccion);

					esperar_padre(n_procesos);
				
					for (int p = 0; p < n_procesos; ++p)
					{
						printf("PID: [%d] Suma del hijo [%d] es: [%d]\n", getpid(), p, acu[p]);
					}

					shmdt(matriz);
					shmdt(acu);
					shmctl(shm_id_matriz, IPC_RMID, 0);
					shmctl(shm_id_suma, IPC_RMID, 0);
				}
				else
				{
					//sleep(1);
					/*Soy alguno de los hijos*/

					acu[i] = 0;
					int temp = 0;

					int row_a_sumar = i;
					int row2_a_sumar = rows -1 -i;

					int col_a_sumar = i;
					int col2_a_sumar = cols - 1 -i;
					//Sumamos fila fija, columna se mueve

					for (int c = i; c < cols-i; ++c)
					{
						temp += matriz[row_a_sumar][c];	
						temp += matriz[row2_a_sumar][c];
						printf("\nHijo [%d] calcula posicion: [%d][%d]\nHijo [%d] calcula posicion: [%d][%d]", getpid(), c, row_a_sumar, getpid(), c, row2_a_sumar);
					}

					if((row_a_sumar+1) != row2_a_sumar)
					{	
						//printf("PID[%d] INICIO [%d] FINAL [%d]\n", getpid(), i+1, rows-1-i);

						for (int r = i+1; r < rows-1-i; ++r)
						{
							temp += matriz[r][col_a_sumar];
							temp += matriz[r][col2_a_sumar];

							printf("\nHijo [%d] calcula posicion: [%d][%d]\nHijo [%d] calcula posicion: [%d][%d]", getpid(), r, col_a_sumar, getpid(), r, col2_a_sumar);
						}

					}

					printf("\nPID: [%d]-> Resultado:[%d]\n", getpid(), temp);
					acu[i] = temp;

					shmdt(matriz);
					shmdt(acu);
					exit(EXIT_SUCCESS);
				}

			}
			else
			{
				error("No se pudo aadir espacio de memoria compartida");
			}

		}
		else
		{
			error("No se pudo crear espacios de memoria compartida");
		}
	}
	else
	{
		error("No se puede realizar proceso");
	}


	return 0;
}

int encontrar_num_proc(int rows, int cols)
{
	int menor = 1;

	if(rows != cols)
	{
		if(rows < cols)
		{
			menor = rows;
		}
		else
		{
			menor = cols;
		}

		if(menor % 2 == 0)
		{
			return(menor/2);
		}
		else
		{
			return (menor - 1);
		}
	}
	else
	{

		menor = rows / 2;

		if(rows%2 != 0)
		{
			menor += 1;
		}

		return menor;
	}
}

int validar_num(int rows, int cols)
{
	if(rows > 0 && cols > 0)
		return 1;
	return 0;
}

unsigned int sizeof_dm(int rows, int cols, size_t sizeElement)
{
	size_t size = rows * sizeof(void*);	//Index size
	size += (cols * rows * sizeElement);	//Data size
	return size;	//return the final size
}

void create_index(void **matrix, int rows, int cols, size_t sizeElement)
{
	size_t sizeRow = cols * sizeElement;	//cols times sizeElement --> sizeElement could be double, int, char...
	matrix[0] = matrix + rows;	//

	for (int i = 1; i < rows; ++i)
	{
		matrix[i] = (matrix[i-1] + sizeRow);
	}
}

void esperar_padre(int n_procesos)
{
	int ver = 0, pid = 0;

	for (int i = 0; i < n_procesos; ++i)
	{

		pid = wait(&ver);

		if(WIFEXITED(ver))
		{
			//printf("El proceso [%d] termino de manera correcta! Valor: [%d]\n", pid, WIFEXITED(ver));
		}
		else
		{
			printf("El proceso [%d] termino de manera abrupta! Valor: [%d]\n", pid, WIFEXITED(ver));
		}
	}
}

void error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}