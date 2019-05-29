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
void subir_matriz(char **, int, int, char*);
void conocer_matriz(char *, int *, int *);
void mostrar_matriz(char **, int, int);
char **espacio_matriz(int, int); //Funcion para crear espacio de memoria
int buscar_fila(char **, int, int, int);
int buscar_columna(char **, int, int, int);

void error(char *); //Funcion de error
void *fnc_hilo(void *);

pthread_mutex_t mutex_1, mutex_2;
pthread_barrier_t barrier_1, barrier_2;

FILE *archivo = NULL;

char *palabra1 = "EXAMEN";
char *palabra2 = "GANAR";
char *palabra3 = "SUERTE";

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;   //Numero del hilo creado
    char **matriz;
    int soy_fila;
    int soy_columna;
    int filas;
    int columnas;
};

int main(int argc, char const *argv[])
{
    int filas = 0, columnas = 0, n_hilos = 2;
    char **matriz = NULL;
    char *filename = "archivo1.txt";
    pthread_t *threads = NULL;
    struct parametros *par = NULL;

    conocer_matriz(filename, &filas, &columnas);

    printf("Filas encontradas [%d]\n", filas);
    printf("Columnas encontradas [%d]\n", columnas);

    if(filas > 0 && columnas > 0)
    {
        matriz = espacio_matriz(filas, columnas);
        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(matriz && threads)
        {

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
                    
                    if(i%2 == 0)
                    {
                        par->soy_fila = 0;
                        par->soy_columna = 1;
                    }
                    else
                    {
                        par->soy_fila = 1;
                        par->soy_columna = 0;
                    }

                    if(pthread_create(&threads[i], NULL, &fnc_hilo, par))
                        error("No se pudo crear hilo");
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

            free(threads);
            free(matriz);
        }
        else
        {
            error("No se pudo crear el espacio de memoria de la matriz");
        }
        
        
    }
    else
    {
        error("No se puede crear proceso");
    }
    

    return 0;
}

int buscar_fila(char **matriz, int n_cols, int fila_fija, int sentido)
{
    int indice1 = 0, vista1 = -1;
    int indice2 = 0, vista2 = -1;
    int indice3 = 0, vista3 = -1;
    int encontro = 0;

    if(sentido)
    {
        for (int i = 0; i < n_cols; i++)
        {
            if(matriz[fila_fija][i] == palabra1[indice1])
            {
                if(vista1 == -1)
                {
                    vista1 = i;
                    encontro = 1;
                }

                indice1++;
            }

            if(matriz[fila_fija][i] == palabra2[indice2])
            {
                if(vista2 == -1)
                {
                    vista2 = i;
                    encontro = 2;
                }

                indice2++;
            }

            if(matriz[fila_fija][i] == palabra3[indice3])
            {
                if(vista3 == -1)
                {
                    vista3 = i;
                    encontro = 3;
                }

                indice3++;
            }
        }
    }
    else
    {
        for (int i = n_cols-1; i >= 0; i--)
        {
            if(matriz[fila_fija][i] == palabra1[indice1])
            {
                if(vista1 == -1)
                {
                    vista1 = i;
                    encontro = 1;
                }

                indice1++;
            }

            if(matriz[fila_fija][i] == palabra2[indice2])
            {
                if(vista2 == -1)
                {
                    vista2 = i;
                    encontro = 2;
                }

                indice2++;
            }

            if(matriz[fila_fija][i] == palabra3[indice3])
            {
                if(vista3 == -1)
                {
                    vista3 = i;
                    encontro = 3;
                }

                indice3++;
            }
        }
    }

    int mostrar = 0;

    if(strlen(palabra1) == indice1)
    {
        indice1--;

        if(!sentido)
            mostrar = vista1-indice1;
        else
            mostrar = vista1+indice1;
        
        printf("[%s]: en posicion inicial [%d] y posicion final [%d] (Hilo fila)\n", palabra1, vista1, mostrar);
    }

    if(strlen(palabra2) == indice2)
    {
        indice2--;

        if(!sentido)
            mostrar = vista2-indice2;
        else
            mostrar = vista2+indice2;

        printf("[%s]: en posicion inicial [%d] y posicion final [%d] (Hilo fila)\n", palabra2, vista2, mostrar);
    }

    if(strlen(palabra3) == indice3)
    {
        indice3--;

        if(!sentido)
            mostrar = vista3-indice3;
        else
            mostrar = vista3+indice3;

        printf("[%s]: en posicion inicial [%d] y posicion final [%d] (Hilo fila)\n", palabra3, vista3, mostrar);
    }

    return encontro;
}

int buscar_columna(char **matriz, int n_filas, int col_fija, int sentido)
{
    int indice1 = 0, vista1 = -1;
    int indice2 = 0, vista2 = -1;
    int indice3 = 0, vista3 = -1;
    int encontro = 0;

    if(sentido)
    {
        for (int i = 0; i < n_filas; i++)
        {
            if(matriz[i][col_fija] == palabra1[indice1])
            {
                if(vista1 == -1)
                {
                    vista1 = i;
                    encontro = 1;
                }

                indice1++;
            }

            if(matriz[i][col_fija] == palabra2[indice2])
            {
                if(vista2 == -1)
                {
                    vista2 = i;
                    encontro = 2;
                }

                indice2++;
            }

            if(matriz[i][col_fija] == palabra3[indice3])
            {
                if(vista3 == -1)
                {
                    vista3 = i;
                    encontro = 3;
                }

                indice3++;
            }
        }
    }
    else
    {
        for (int i = n_filas-1; i >= 0; i--)
        {
            if(matriz[i][col_fija] == palabra1[indice1])
            {
                if(vista1 == -1)
                {
                    vista1 = i;
                    encontro = 1;
                }

                indice1++;
            }

            if(matriz[i][col_fija] == palabra2[indice2])
            {
                if(vista2 == -1)
                {
                    vista2 = i;
                    encontro = 2;
                }

                indice2++;
            }

            if(matriz[i][col_fija] == palabra3[indice3])
            {
                if(vista3 == -1)
                {
                    vista3 = i;
                    encontro = 3;
                }

                indice3++;
            }
        }
    }

    int mostrar = 0;

    if(strlen(palabra1) == indice1)
    {
        indice1--;

        if(!sentido)
            mostrar = vista1-indice1;
        else
            mostrar = vista1+indice1;
        
        printf("[%s]: en posicion inicial [%d] y posicion final [%d] (Hilo fila)\n", palabra1, vista1, mostrar);
    }

    if(strlen(palabra2) == indice2)
    {
        indice2--;

        if(!sentido)
            mostrar = vista2-indice2;
        else
            mostrar = vista2+indice2;

        printf("[%s]: en posicion inicial [%d] y posicion final [%d] (Hilo fila)\n", palabra2, vista2, mostrar);
    }

    if(strlen(palabra3) == indice3)
    {
        indice3--;

        if(!sentido)
            mostrar = vista3-indice3;
        else
            mostrar = vista3+indice3;

        printf("[%s]: en posicion inicial [%d] y posicion final [%d] (Hilo fila)\n", palabra3, vista3, mostrar);
    }

    return encontro;
}

char **espacio_matriz(int filas, int columnas)
{
    char **matriz = NULL;

    matriz = (char**)malloc(filas*sizeof(char*));

    for (int i = 0; i < filas; i++)
    {
        matriz[i] = (char*)malloc(columnas*sizeof(char));
    }

    return matriz;
}

void mostrar_matriz(char **matriz, int rows, int cols)
{

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			printf("%c", matriz[r][c]);
		}

		printf("\n");
	}

	printf("\n");
}

void conocer_matriz(char *filename, int *rows, int *cols)
{
    if(archivo = fopen(filename, "r"))
    {
        *rows = *cols = 0;
        int filas = 0, columnas = 0;
        char letra = '\0';
        int salto = 0;

        while (!feof(archivo))
        {
            if(fscanf(archivo, "%c", &letra))
            {
                if(!salto && letra != '\n')
                {
                    columnas++;
                }
                
                if(letra == '\n')
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

        filas++;
        *rows = filas;
        *cols = columnas;
    }
    else
    {
        error("No se ha podido abrir el archivo");
    }
    
}

void subir_matriz(char **matriz, int rows, int cols, char *filename)
{
	if(archivo = fopen(filename, "r"))
	{
		int iRows = 0, iCols = 0;
		rows--;
        char valor = '\0';

		while(!feof(archivo))
		{

			if(fscanf(archivo, "%c", &valor) == 1)
			{
                if(valor != '\n')
                {
                    matriz[iRows][iCols] = valor;
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

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;
    
    int encontro = 0;

    if(par->soy_fila && par->soy_columna == 0)
    {
        printf("Hilo [%d] -> Busco por filas\n", par->hilo);

        for (int i = 0; i < par->filas; i++)
        {

            //printf("Hilo [%d] -> Bucar Fila[%d]\n", par->hilo, i);

            encontro = buscar_fila(par->matriz, par->columnas, i, 1);
            
            if(!encontro)
            {
                encontro = buscar_fila(par->matriz, par->columnas, i, 0);   
            }

            //printf("HILO [%d] -> PASO\n", par->hilo);
        }
    
    }
    else
    {
        printf("Hilo [%d] -> Busco por columnas\n", par->hilo);

        for (int i = 0; i < par->columnas; i++)
        {
            encontro = buscar_columna(par->matriz, par->filas, i, 1);
            
            if(!encontro)
            {
                encontro = buscar_columna(par->matriz, par->filas, i, 0);   
            }
        }
    }

    pthread_exit(param);
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
