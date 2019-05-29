#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/*
	Camilo Laiton
	Estudiante Ingenieria de Sistemas
	Universidad del Magdalena
	2019 - 1
	Sistema Operativo usado -> Linux 
*/

void crear_cerrojo(pthread_mutex_t *);  //funcion para crear cerrojo
void destruir_cerrojo(pthread_mutex_t *);   //Funcion para destrur cerrojo
void crear_barrera(pthread_barrier_t *, int);   //funcion para crear barrera
void destruir_barrera(pthread_barrier_t *); //Funcion para destruir barrera
void crear_condicional(pthread_cond_t *);   //Fucnion para crear condicional
void destruir_condicional(pthread_cond_t *);    //funcion para destruir condicional

void *fnc_hilo(void *);
void *fnc_hilo_resultado(void *);
void error(char*);
int **espacio_matriz(int, int);
void mostrar_matriz(int **, int, int);
void subir_matriz(int **, int, int, char*);
void conocer_matriz(char *, int *, int *);

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;   //Numero del hilo creado
    int **matriz;
    int filas;
    int columnas;
    int con;
};

FILE *archivo = NULL;
int fila_actual = 0, col_actual = 0, suma = 0, sigo = 1;
pthread_mutex_t mutex, mutex2;
pthread_cond_t cond, cond2;
pthread_barrier_t barrier;

int main(int argc, char const *argv[])
{
    int filas = 0, columnas = 0, n_hilos = 0, con = 0, **matriz = NULL;
    char *filename = "matriz_numerica.txt";
    pthread_t *threads = NULL;
    struct parametros *par = NULL;

    printf("Ingrese el numero de hilos: \n");
    scanf("%d", &n_hilos);

    printf("Ingrese el numero de casillas consecuentes: \n");
    scanf("%d", &con);

    conocer_matriz(filename, &filas, &columnas);

    //printf("[%d] [%d]\n", filas, columnas);

    if(filas > 0 && columnas > 0 && n_hilos > 0 && con > 0 && con*n_hilos <= filas*columnas)
    {   
        n_hilos++;
        matriz = espacio_matriz(filas, columnas);
        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(matriz && threads)
        {

            crear_cerrojo(&mutex);
            crear_cerrojo(&mutex2);
            crear_condicional(&cond);
            crear_condicional(&cond2);
            crear_barrera(&barrier, n_hilos);

            subir_matriz(matriz, filas, columnas, filename);
            mostrar_matriz(matriz, filas, columnas);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                
                if(par)
                {
                    par->hilo = i;
                    par->matriz = matriz;
                    par->filas = filas;
                    par->columnas = columnas;
                    par->con = con;

                    if(i == n_hilos-1)
                    {
                        if(pthread_create(&threads[i], NULL, &fnc_hilo_resultado, par))
                            error("No se pudo crear hilo");
                    }
                    else
                    {
                        if(pthread_create(&threads[i], NULL, &fnc_hilo, par))
                            error("No se pudo crear hilo");
                    }
                    
                }
                else
                {
                    error("No se pudo crear espacio para par");
                }
                
            }

            for (int i = 0; i < n_hilos; i++)   //Para que todos finalicen antes del main
            {
                par = NULL;

                if(!pthread_join(threads[i], (void*)&par))
                {
                    printf("Hilo [%d] -> Ha finalizado!\n", par->hilo);
                    free(par);
                }
                else
                {
                    printf("No se pudo esperar hilo\n");
                }
                
            }

            printf("Hilo principal -> Suma [%d]\n", suma);

            destruir_barrera(&barrier);
            destruir_cerrojo(&mutex2);
            destruir_cerrojo(&mutex);
            destruir_condicional(&cond2);
            destruir_condicional(&cond);

            free(threads);
            free(matriz);
        }
        else
        {
            error("Espacio de memoria no disponible!\n");
        }
        
    }
    else
    {
        error("Error en los parametros\n");
    }

    return 0;
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;

    if(pthread_mutex_lock(&mutex))
        error("No se pudo lock mutex");

    //printf("Hilo [%d] activo [W] condicional general\n", par->hilo);

    if(pthread_cond_wait(&cond, &mutex))    //Pongo a esperar todos los hilos para que esten creados todos
        error("No se pudo condicional");

    //printf("Hilo [%d] continuo actividad despues wait [1]\n", par->hilo);    

    sigo = fila_actual*col_actual < (par->filas-1)*(par->columnas-1);

    if(pthread_mutex_unlock(&mutex))
        error("No se pudo unlock mutex");

    while (sigo)
    {

        for (int i = 0; i < par->con; i++)
        {

            printf("Hilo [%d] -> Trabaja columna [%d] - fila [%d], sigo [%d], con [%d], i [%d] SUMA [%d]\n",
            par->hilo, col_actual, fila_actual, sigo, par->con, i, suma);

            suma += par->matriz[fila_actual][col_actual];

            if(col_actual == par->columnas-1 && fila_actual < par->filas-1)
            {
                col_actual = 0;
                fila_actual++;
            }
            else
            {
                col_actual++;
            }

            if(col_actual == par->columnas-1 && fila_actual == par->filas-1) 
            {
                printf("Hilo [%d] -> Trabaja columna [%d] - fila [%d], sigo [%d], con [%d], i [%d] SUMA [%d]\n",
                par->hilo, col_actual, fila_actual, sigo, par->con, i, suma);

                suma += par->matriz[fila_actual][col_actual];
                //printf("Hilo [%d] activo [S] condicional resultado\n", par->hilo);

                if(pthread_cond_signal(&cond2))   //Cuando termine de hacer el proceso llamo al proceso resultado
                    error("No se pudo condicional broadcast");

                break;
            }

        }

        if(pthread_mutex_lock(&mutex))
            error("No se pudo lock mutex");

        sigo = fila_actual*col_actual < (par->filas-1)*(par->columnas-1);

        if(sigo)
        {
            //printf("Hilo [%d] activo [S] condicional general [PROCESO]\n", par->hilo);

            if(pthread_cond_signal(&cond))  //Activo otro hilo para que continue
                error("No se pudo condicional");

            //printf("Hilo [%d] activo [W] condicional general [PROCESO]\n", par->hilo);
            
            if(pthread_cond_wait(&cond, &mutex))    //Y espero a que otro me llame
                error("No se pudo condicional");

        }

        if(pthread_mutex_unlock(&mutex))
            error("No se pudo unlock mutex");

    }

    pthread_exit(param);
}

void *fnc_hilo_resultado(void *param)
{
    struct parametros *par = (struct parametros*)param;
    
    if(pthread_mutex_lock(&mutex2))
        error("No se pudo lock mutex");

    //printf("Hilo [%d] activo [S] condicional general\n", par->hilo);

    if(pthread_cond_signal(&cond))  //Activo el primer proceso asegurando que ya todos se crearon
        error("No se pudo condicional");

    //printf("Hilo [%d] activo [W] condicional resultado\n", par->hilo);

    if(pthread_cond_wait(&cond2, &mutex2))  //Pongo a esperar hilo resultado
        error("No se pudo cond_result");

    //printf("Hilo [%d] activo [S] condicional [BROADCAST]\n", par->hilo);

    if(pthread_cond_broadcast(&cond))   //Si resultado siguio es porque termino proceso y desbloquea todos los hilos
        error("No se pudo activar cond_broad");

    if(pthread_mutex_unlock(&mutex2))
        error("No se pudo unlock mutex"); 

    printf("Hilo [%d] -> Resultado suma [%d]\n", par->hilo, suma);

    pthread_exit(param);
}


void conocer_matriz(char *filename, int *rows, int *cols)
{
    if(archivo = fopen(filename, "r"))
    {
        *rows = *cols = 0;
        int filas = 0, columnas = 0;
        char valor = '\0';
        int salto = 0;

        while (!feof(archivo))
        {
            if(fscanf(archivo, "%c", &valor))
            {
                if(!salto && valor != '\n')
                {
                    columnas++;
                }
                
                if(valor == '\n')
                {
                    salto = 1;
                    filas++;
                }
            }
        }

        if(fclose(archivo))
		{
			error("No se pudo cerrar el archivo de texto");
		}

        *rows = filas-1;
        *cols = columnas;
    }
    else
    {
        error("No se ha podido abrir el archivo");
    }
    
}

void subir_matriz(int **matriz, int rows, int cols, char *filename)
{
	if(archivo = fopen(filename, "r"))
	{
		int iRows = 0, iCols = 0;
        char valor = '\0';

		while(!feof(archivo))
		{
			if(fscanf(archivo, "%c", &valor))
			{
                if(valor != '\n')
                {
                    matriz[iRows][iCols] = valor-48;
                    //printf("[%d]", matriz[iRows][iCols]);
                }

				if(iCols == cols)
				{
					iCols = 0;
					iRows++;
				}
				else
				{
					iCols++;
				}
			}
		}

		if(fclose(archivo))
		{
			error("No se pudo cerrar el archivo de texto");
		}
	}
	else
	{
		error("No se pudo crear el archivo de texto");
	}	
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

int **espacio_matriz(int filas, int columnas)
{
    int **matriz = NULL;

    matriz = (int**)malloc(filas*sizeof(int*));

    for (int i = 0; i < filas; i++)
    {
        matriz[i] = (int*)malloc(columnas*sizeof(int));
    }

    return matriz;
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void crear_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_init(cond, NULL))
        error("No se pudo crear condicional\n");
}
void destruir_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_destroy(cond))
        error("No se pudo eliminar condicional\n");
}

void crear_barrera(pthread_barrier_t *br, int n_espera)
{
    if(pthread_barrier_init(br, NULL, n_espera))
        error("No se pudo crear barrera\n");
}

void destruir_barrera(pthread_barrier_t *br)
{
    if(pthread_barrier_destroy(br))
        error("No se pudo destruir barrera\n");
}

void destruir_cerrojo(pthread_mutex_t *mt)
{
    if(pthread_mutex_destroy(mt))
		error("No se ha podido destruir cerrojo\n");
}

void crear_cerrojo(pthread_mutex_t *mt)
{
    if(pthread_mutex_init(mt, NULL))
        error("No se ha podido crear cerrojo\n");
}

