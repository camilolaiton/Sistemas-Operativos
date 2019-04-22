#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <wait.h>
#include <stdlib.h>
#include <time.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux
*/

/*
	El ejercicio consta de multiplicar las matrices y guardar el resultado en una matriz 
	resultante en otro espacio de memoria, la multiplicacion será:

	Si quiero el valor 0, 0 de la matriz resultante, debo multiplicar todos los valores 
	de la fila 0 de matriz A y columna 0 matriz B y hacerlo en forma acumulativa y luego 
	guardar ese valor en la matriz C.

	El padre hará de mostrar los resultados finales.
*/

void error(char *);	//Funcion de error
void esperar_padre(int);	//Funcion de esperar padre
unsigned int sizeof_dm(int, int, size_t);	//Obtener el tamaño de la matriz
void create_index(void **, int, int, size_t);	//Para usar el espacio de la memoria como matriz
int validar_num(int, int);	//Valido los numeros que ingresen
void inicializar_matriz(int **, int, int, int);	//Inicializo los valores de la matriz
void mostrar_matriz(int **, int, int);	//Muestro matriz
int multiplicar_matrices(int , int , int , int );	//Multiplico dos matrices para obtener una matriz C

int **matriz_a = NULL, **matriz_b = NULL, **matriz_c = NULL;	//Doble punteros para las matrices

int main(int argc, char const *argv[])
{
	srand(time(NULL));	//Inicializo la semilla para el random
	pid_t hijo = (pid_t)(0), padre = getpid();	//Creo las variables para validar
	int shm_id_matrices[3] = {0};	//Creo un vector para guardar los id de los espacios de memoria compartida
	int rows = 6, cols = 6, n_procesos = 3, p = 0, n_matrices = 3;
	//Variables que se van a utilizar en el algoritmo

	if(validar_num(rows, cols))
	{
		size_t sizeMatriz = sizeof_dm(rows, cols, sizeof(int));	//Obtengo el espacio de la memoria compartida

		for (int i = 0; i < n_matrices; ++i)	//For para crear los espacios de memoria
		{
			shm_id_matrices[i] = shmget(IPC_PRIVATE, sizeMatriz, IPC_CREAT | 0600);	//Obtengo los espacios

			if(shm_id_matrices[i] == -1)//Validar si se pudo crear la memoria
			{
				error("Error al crear espacio de memoria compartida");
				return -1;
			}
		}

		matriz_a = (int**)shmat(shm_id_matrices[0], NULL, 0);//Añado el espacio a el proceso
		matriz_b = (int**)shmat(shm_id_matrices[1], NULL, 0);
		matriz_c = (int**)shmat(shm_id_matrices[2], NULL, 0);

		if(matriz_a && matriz_b && matriz_c)//Valido que los tres espacios se añadieron
		{
			create_index((void*)matriz_a, rows, cols, sizeof(int));//Consigo que se pueda usar en forma matricial
			create_index((void*)matriz_b, rows, cols, sizeof(int));
			create_index((void*)matriz_c, rows, cols, sizeof(int));

			inicializar_matriz(matriz_a, rows, cols, 0);//Inicializo las matrices
			inicializar_matriz(matriz_b, rows, cols, 0);
			inicializar_matriz(matriz_c, rows, cols, 1);//1 inicializo en 0 -> 0 en random

			for ( p = 0; p < n_procesos; ++p)//For para crear los procesos
			{
				hijo = fork();//Creo proceso

				if(hijo == -1)	//Valido el proceso 
				{
					error("No se pudo crear el proceso");
				}
				else if(!hijo)//Si es hijo salgo del for
				{
					break;
				}
			}

			if(padre == getpid())//Valido que sea padre
			{

				char instruccion[50] = {'\0'};
				sprintf(instruccion, "pstree -lp %d", getpid());	//Mostrar el arbol de procesos
				system(instruccion);
				//Instruccion para mostrar el arbol de procesos
				
				printf("PID: [%d] PADRE -> ANTES\nPID: [%d] PADRE -> Matriz A\n", padre, padre);
				mostrar_matriz(matriz_a, rows, cols);
				printf("PID: [%d] PADRE -> Matriz B\n", padre);
				mostrar_matriz(matriz_b, rows, cols);
				printf("PID: [%d] PADRE -> Matriz C\n", padre);
				mostrar_matriz(matriz_c, rows, cols);

				//Muestro las matrices antes de la terminacion de ejecucion de los hijos

				esperar_padre(n_procesos);

				printf("PID: [%d] -> Matriz A\n", padre);
				mostrar_matriz(matriz_a, rows, cols);
				printf("PID: [%d] -> Matriz B\n", padre);
				mostrar_matriz(matriz_b, rows, cols);
				printf("PID: [%d] -> Matriz C\n", padre);
				mostrar_matriz(matriz_c, rows, cols);

				//Muestro los resultados
				
				shmdt(matriz_a);//Quito espacios de memoria compartida del proceso
				shmdt(matriz_b);
				shmdt(matriz_c);
				
				for (int i = 0; i < n_matrices; ++i)//Elimino los espacios de memoria compartida
				{
					shmctl(shm_id_matrices[i], IPC_RMID, 0);
				}
			}
			else
			{	
				sleep(3);//Me aseguro que se ejecute primero el padre

				if(p == 0)	//El proceso 0 hara la diagonal principal
				{
					for (int q = 0; q < rows; ++q)
					{
						multiplicar_matrices(q, q, rows-1, cols-1);	//Diagonal principal
					}

				}
				else if(p == 1)//EL proceso 1 hara el espacio izquierda de diagonal
				{

					for (int r = 0; r < rows; ++r)
					{
						for (int c = 0; c < r; ++c)	//Multiplico izquierda matriz
						{
							
							multiplicar_matrices(r, c, rows-1, cols-1);
							//Multiplico las matrices	
						}
					}
				}
				else
				{

					for (int c = 0; c < cols; ++c)//El proceso final hara la parte derecha de matriz
					{
						for (int r = 0; r < c; ++r)	//Matriz derecha
						{
							multiplicar_matrices(r, c, rows-1, cols-1);
							//Multiplico matriz derecha
						}
					}

					//printf("FINAL\n");
					//mostrar_matriz(matriz_c, rows, cols);
				}
						
				shmdt(matriz_a);//Quito los espacios de memoria de los hijos
				shmdt(matriz_b);
				shmdt(matriz_c);

				exit(EXIT_SUCCESS);//Salgo de los procesos de forma exitosa
			}
		}
		else
		{
			error("No se pudieron añadir espacios de memoria compartida");
		}

	}
	else
	{
		error("Matriz invalida");
	}

	return 0;
}

int multiplicar_matrices(int row_a, int col_b, int tam_col_a, int tam_row_b)
{
	//Funcion para multiplicar dos matrices y guardar en una tercera

	int temp = 0;
	int c_a = 0, r_b = 0;

	while(1)
	{
		
		temp += matriz_a[row_a][c_a] * matriz_b[r_b][col_b];

		if(c_a < tam_col_a)
			c_a++;

		if(r_b < tam_row_b)
			r_b++;

		if(c_a == tam_col_a && r_b == tam_row_b)
			break;
	}

	matriz_c[row_a][col_b] = temp;
	//mostrar_matriz(matriz_c, tam_row_b+1, tam_col_a+1);
}

void mostrar_matriz(int **matriz, int rows, int cols)
{
	//Muestro la matriz

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			printf("%d ", matriz[r][c]);
		}

		printf("\n");
	}

	printf("\n\n");
}

void inicializar_matriz(int **matriz, int rows, int cols, int cero)
{
	//Inicializar matriz

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			if(cero)
			{
				matriz[r][c] = 0;
			}
			else
			{
				matriz[r][c] = 1 + rand() % 9;
			}
		}
	}
}

int validar_num(int rows, int cols)
{
	//Validar para hacer proceso
	if(rows > 0 && cols == rows)
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

void esperar_padre(int procesos)
{
	//Funcion esperar padre, sirve para ver como termina el proceso

	if(procesos > 0)
	{
		int ver = 0, pid = 0;

		pid = wait(&ver);

		if(WIFEXITED(ver))
		{
			printf("El proceso [%d] termino de manera correcta! Valor: [%d]\n", pid, WIFEXITED(ver));
		}
		else
		{
			printf("El proceso [%d] termino de manera abrupta! Valor: [%d]\n", pid, WIFEXITED(ver));
		}
	}
	else
	{
		error("No se puede esperar una cantidad de procesos irreal");
	}
}

void error(char *msg)
{
	//Funcion de error
	perror(msg);
	exit(EXIT_FAILURE);
}