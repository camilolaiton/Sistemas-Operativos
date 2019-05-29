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

void voltear_palabra();
void montar_matriz(char*, char**, int, int);
char **espacio_matriz(int, int);
void mostrar_matriz(char **, int, int);
void conocer_matriz(char *, int *, int *);
int contar_palabras(char *, int*);
void error(char *); //Funcion de error
void *fnc_hilo(void *);

FILE *archivo = NULL;
char palabra1[50] = {'\0'};
char palabra2[50] = {'\0'};
int fila_actual = 0;

pthread_mutex_t mutex;

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;   //Numero del hilo creado
    int division_trabajo;
    int columnas;
    int filas;
    char **matriz;
};

int main(int argc, char const *argv[])
{
    pthread_t *threads = NULL;
    char **matriz = NULL;
    int n_hilos = 0, n_palabras = 0, n_carac_palabra = 0, columnas = 0, division_trabajo = 0;
    char *filename = "archivo.txt";
    struct parametros *par = NULL;

    printf("Ingrese la palabra a buscar: \n");
    scanf("%[^\n]", palabra1);

    n_palabras = contar_palabras(filename, &columnas);
    n_carac_palabra = strlen(palabra1);

    voltear_palabra();
    printf("Ingrese la cantidad de hilos: \n");
    scanf("%d", &n_hilos);

    if(n_carac_palabra > 0 && n_carac_palabra < n_palabras && n_hilos > 0)
    {
        crear_cerrojo(&mutex);
        division_trabajo = (n_palabras / n_hilos)+1;

        matriz = espacio_matriz(n_palabras, columnas);
        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));
        
        printf("Cantidad de palabras del archivo [%d] - Division [%d]\n", n_palabras, division_trabajo);
        printf("Palabras a buscar: [%s][%s] - Division de palabras por hilo [%d]\n\n", palabra1, palabra2, division_trabajo);

        if(matriz && threads)
        {
            montar_matriz(filename, matriz, n_palabras, columnas);
            //mostrar_matriz(matriz, n_palabras, columnas);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                
                if(par)
                {
                    par->hilo = i;
                    par->division_trabajo = division_trabajo;
                    par->columnas = columnas;
                    par->filas = n_palabras;
                    par->matriz = matriz;

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
                    //printf("Hilo [%d] -> Ha finalizado!\n", par->hilo);
                    free(par);
                }
                else
                {
                    printf("No se pudo esperar hilo\n");
                }
                
            }
        }
        else
        {
            error("No se pudo crear espacio de memoria\n");
        }
        
        destruir_cerrojo(&mutex);
    }
    else
    {
        error("Error en los parametros\n");
    }

    return 0;
}

void voltear_palabra()
{
    int letras = strlen(palabra1);

    int c = 0;
    for (int i = letras-1; i >= 0; i--)
    {
        palabra2[c] = palabra1[i];
        c++;
    }
    
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;
    
    char *ret1 = NULL, *ret2 = NULL;

    for (int i = 0; i < par->division_trabajo; i++)
    {
        if(pthread_mutex_lock(&mutex))
            error("Error al lock del mutex");

        ret1 = strstr(par->matriz[fila_actual], palabra1);
        ret2 = strstr(par->matriz[fila_actual], palabra1);

        if(ret1)
        {
            printf("Hilo [%d] -> Encontro palabra [%s] en [%s]\n", par->hilo, palabra1, par->matriz[fila_actual]);
        }
        
        if(ret2)
        {
            printf("Hilo [%d] -> Encontro palabra [%s] en [%s]\n", par->hilo, palabra2, par->matriz[fila_actual]);
        }

        if(fila_actual < par->filas)
            fila_actual++;

        ret1 = ret2 = NULL;

        if(pthread_mutex_unlock(&mutex))
            error("Error al unlock del mutex");
    }
    
    pthread_exit(param);
}

void montar_matriz(char *filename, char **matriz, int filas, int columnas)
{
    char buffer[50] = {'\0'};
    int colum_inser = 0;
    int palabras = 0;

    for (int f = 0; f < filas; f++)
    {
        for (int c = 0; c < columnas; c++)
        {
            matriz[f][c] = '\0';
        }
        
    }

    if(archivo = fopen(filename, "r"))
    {
        while (!feof(archivo))
        {   
            memset(buffer, '\0', 50);

            if(fscanf(archivo, "%s", buffer))
            {
                for (int i = 0; i < strlen(buffer); i++)
                {
                    matriz[palabras][i] = buffer[i];
                }
                
                //printf("%s ", buffer);

                palabras++;
            }
        }

        if(fclose(archivo))
		{
			error("No se pudo cerrar el archivo de texto");
		}

    }
    else
    {
        error("No se ha podido abrir el archivo");
    }
}

void mostrar_matriz(char **matriz, int rows, int cols)
{
	//Muestro la matriz

	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			printf("%c", matriz[r][c]);
		}

		printf("\n");
	}

	printf("\n\n");
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

int contar_palabras(char *filename, int *columnas)
{
    int palabras = 0;
    int mayor_letras = 0;
    char buffer[50] = {'\0'};

    if(archivo = fopen(filename, "r"))
    {
        while (!feof(archivo))
        {
            if(fscanf(archivo, "%s", buffer))
            {
                palabras++;

                if(strlen(buffer) > mayor_letras)
                    mayor_letras = strlen(buffer);

                memset(buffer, '\0', 50);
            }
        }

        if(fclose(archivo))
		{
			error("No se pudo cerrar el archivo de texto");
		}

    }
    else
    {
        error("No se ha podido abrir el archivo");
    }

    *columnas = mayor_letras;

    return palabras;
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
