#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

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

int obtener_saltos(int, int);
void subir_vector(char *, int *, int);
void mostrar_vector(int*, int);
int cant_vector_archivo(char *);
void error(char *); //Funcion de error
void *fnc_hilo(void *);

FILE *archivo = NULL;
pthread_mutex_t mutex_1, mutex_2;
pthread_barrier_t barrier_1, barrier_2;

struct parametros	//Parametros enviado a los hilos
{
	int hilo;
	int inicio;
	int final;
    int buscar;
    int *vector;
};

int main(int argc, char const *argv[])
{
    struct parametros *par = NULL;
    pthread_t *threads = NULL;  //Variable de hilos
    char *filename = "paralelo.txt";
    int n_hilos = 0, posiciones = 0, *vector = NULL, saltos = 0, buscar = 0;

    printf("Ingrese el numero de hilos: ");
    scanf("%d", &n_hilos);

    printf("Ingrese el valor a buscar: \n");
    scanf("%d", &buscar);

    posiciones = cant_vector_archivo(filename);

    if(n_hilos > 0 && posiciones > 0 && posiciones >= n_hilos && buscar >= 0 && buscar <= 9)
    {
        vector = (int*)malloc(posiciones*sizeof(int));
        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(vector && threads)
        {
            subir_vector(filename, vector, posiciones);
            mostrar_vector(vector, posiciones);

            printf("Saltos [%d]\n", obtener_saltos(n_hilos, posiciones));

            
            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                
                if(par)
                {
                    par->hilo = i;
                    par->inicio = saltos;
                    par->buscar = buscar;
                    par->vector = vector;
                    saltos += obtener_saltos(n_hilos, posiciones);

                    if(i == n_hilos-1)
                        par->final = posiciones;
                    else
                        par->final = saltos;

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

            free(vector);
            free(threads);
        }   
        else
        {
            error("Espacio de memoria no pudo ser asignado 2\n");
        }  
    }
    else
    {
        error("Error en los parametros 1\n");
    }
    
    return 0;
}

int obtener_saltos(int hilos, int cantidad_vector)
{
	if(hilos > 0 && cantidad_vector > 0)
	{
		float validar = 0;

		validar = (float)cantidad_vector / (float)hilos;
        int ver = (int)validar;

        float resto = fmod( validar, 1  );

        if(resto != 0)
        {
            if(resto > 0.7)
            {
                ver++;
            }
        }
        
        //printf("Validar [%f] Ver [%d]\n", validar, ver);

		return ver;
	}
	else
	{
		error("Numero invalido, no se puede obtener saltos");
	}

	return 0;
}

void subir_vector(char *filename, int *vector, int pos)
{
    if(archivo = fopen(filename, "r"))
    {
        char cant = '\0';
        int pos_interna = 0;

        while (!feof(archivo))
        {   
            if(fscanf(archivo, "%c", &cant))
            {
                if(cant != '\n')
                {
                    if (pos_interna < pos)
                    {
                        vector[pos_interna] = cant - 48;
                        pos_interna++;
                    }
                    
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
        error("No se ha podido abrir el archivo");
    }
}

void mostrar_vector(int *vector, int pos)
{
    for (int i = 0; i < pos; i++)
    {
        printf("[%d]", vector[i]);
    }

    printf("\n");
    
}

int cant_vector_archivo(char *filename)
{
    int pos = 0;

    if(archivo = fopen(filename, "r"))
    {
        char cant = '\0';

        while (!feof(archivo))
        {   

            if(fscanf(archivo, "%c", &cant))
            {
                if(cant != '\n')
                    pos++;
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

    return pos;
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;
    int cant = 0;

    for (int i = par->inicio; i < par->final; i++)
    {
        if(par->vector[i] == par->buscar)
            cant++;
    }
    
    printf("Hilo [%d] -> Inicio [%d] Final [%d] - Encontro [%d] veces a [%d]\n", par->hilo, par->inicio, par->final-1, cant, par->buscar);

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
