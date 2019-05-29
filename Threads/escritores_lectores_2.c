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
void usar_barrera(pthread_barrier_t*, int);

void crear_condicional(pthread_cond_t *);   //Fucnion para crear condicional
void destruir_condicional(pthread_cond_t *);    //funcion para destruir condicional
void usar_condicional_wait(pthread_cond_t*, pthread_mutex_t *, int);

void error(char *); //Funcion de error
void *fnc_hilo_escritor(void *);
void *fnc_hilo_lector(void *);

int n_impresiones = 0;

pthread_cond_t cond_lector, cond_escritor;
pthread_mutex_t mutex_lector, mutex_escritor;
pthread_barrier_t barrier;

struct parametros   //EStructura para pasar datos a hilos
{
    int hilo;   //Numero del hilo creado
    int n_hilos;
    char *buffer;
};

int main(int argc, char const *argv[])
{
    struct parametros *par = NULL;
    pthread_t *threads = NULL;  //Variable de hilos
    int n_hilos = 0;
    char buffer[100] = {'\0'};

    printf("Ingrese la cantidad de hilos: \n");
    scanf("%d", &n_hilos);
    getchar();

    if(n_hilos > 0)
    {

        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(threads)
        {
            crear_condicional(&cond_escritor);
            crear_condicional(&cond_lector);
            crear_cerrojo(&mutex_escritor);
            crear_cerrojo(&mutex_lector);
            crear_barrera(&barrier, n_hilos);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));    //Para saber el indice de hilo y pasar info
                
                if(par)
                {
                    par->hilo = i;
                    par->n_hilos = n_hilos-1;
                    par->buffer = buffer;

                    if(par->hilo == n_hilos-1)
                    {
                        if(pthread_create(&threads[i], NULL, &fnc_hilo_escritor, par))
                            error("No se pudo crear hilo");
                    }
                    else
                    {
                        if(pthread_create(&threads[i], NULL, &fnc_hilo_lector, par))
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

            destruir_condicional(&cond_escritor);
            destruir_condicional(&cond_lector);
            destruir_cerrojo(&mutex_escritor);
            destruir_cerrojo(&mutex_lector);
            destruir_barrera(&barrier);
        }
        else
        {
            error("No se ha podido crear espacio de memoria threads\n");
        }
        
    }
    else
    {
        error("Error en parametros1\n");
    }
    

    return 0;
}

void *fnc_hilo_escritor(void *param)
{
    struct parametros *par = (struct parametros*)param;

    usar_barrera(&barrier, par->hilo);

    while (strcmp(par->buffer, "salir") != 0)
    {
        do
        {
            memset(par->buffer, '\0', 100);
            printf("Hilo [%d] -> Ingrese el texto: \n", par->hilo);
            scanf("%[^\n]", par->buffer);

            getchar();

            if(strlen(par->buffer) == 0)
            {
                printf("Ingrese caracteres validos...");
            }

        } while (strlen(par->buffer) == 0);
        
        if(pthread_mutex_lock(&mutex_escritor))
            error("No se pudo unlock mutex escritor 1\n");

        n_impresiones = 0;

        //printf("Hilo [%d] -> Activo lector\n", par->hilo);

        if(pthread_cond_signal(&cond_lector))
            error("No se pudo activar cond_lector 2\n");

        usar_condicional_wait(&cond_escritor, &mutex_escritor, par->hilo);

        if(pthread_mutex_unlock(&mutex_escritor))
            error("No se pudo unlock mutex escritor 1\n");
    }
    
    //printf("Hilo [%d] -> Despierto a todos\n", par->hilo);

    n_impresiones = -1;

    if(pthread_cond_broadcast(&cond_lector))
        error("Error al despertar todos los hijos\n");

    pthread_exit(param);
}

void *fnc_hilo_lector(void *param)
{
    struct parametros *par = (struct parametros*)param;
    
    usar_barrera(&barrier, par->hilo);

    if(pthread_mutex_lock(&mutex_lector))
        error("No se pudo lock mutex lector 1\n");

    usar_condicional_wait(&cond_lector, &mutex_lector, par->hilo);

    if(pthread_mutex_unlock(&mutex_lector))
        error("No se pudo unlock mutex lector 1\n");

    do
    {
        if(pthread_mutex_lock(&mutex_lector))
            error("No se pudo lock mutex lector 2\n");

        //printf("Hilo [%d] -> Comienzo proc\n", par->hilo);

        if(n_impresiones < par->n_hilos-1)
        {
            printf("Hilo [%d] -> Contenido en buffer [%s]\n", par->hilo, par->buffer);
            n_impresiones++;

            //printf("Hilo [%d] -> Activo algun lector\n", par->hilo);

            if(pthread_cond_signal(&cond_lector))
                error("No se pudo activar otro lector\n");

            usar_condicional_wait(&cond_lector, &mutex_lector, par->hilo);
        }
        else
        {
            printf("Hilo [%d] -> Contenido en buffer [%s]\n", par->hilo, par->buffer);

            //printf("Hilo [%d] -> Activo escritor\n", par->hilo);

            if(pthread_cond_signal(&cond_escritor))
                error("No se pudo activar escritor desde lector\n");

            usar_condicional_wait(&cond_lector, &mutex_lector, par->hilo);

            //printf("Hilo [%d] -> Continuo, n_im [%d]\n", par->hilo, n_impresiones);
        }

        if(strcmp(par->buffer, "salir") == 0 && n_impresiones != -1)
        {
            //printf("Hilo [%d] -> Activo hilo escritor por salir\n", par->hilo);

            if(pthread_cond_signal(&cond_escritor))
                error("No se pudo activar escritor 2\n");
                
            usar_condicional_wait(&cond_lector, &mutex_lector, par->hilo);
        }

        if(pthread_mutex_unlock(&mutex_lector))
            error("No se pudo unlock mutex lector 1\n");
        
    } while (strcmp(par->buffer, "salir") != 0);

    pthread_exit(param);
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void crear_condicional(pthread_cond_t *cd)
{
    if(pthread_cond_init(cd, NULL))
        error("No se pudo crear condicional\n");
}
void destruir_condicional(pthread_cond_t *cd)
{
    if(pthread_cond_destroy(cd))
        error("No se pudo eliminar condicional\n");
}

void usar_condicional_wait(pthread_cond_t *cd, pthread_mutex_t *mt, int hilo)
{
    //printf("Hilo [%d] -> Espero...\n", hilo);

    if(pthread_cond_wait(cd, mt))
        error("No se pudo esperar cond\n");
        
    //printf("Hilo [%d] -> Continuo ejecucion\n", hilo);
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

void usar_barrera(pthread_barrier_t *br, int hilo)
{
    //printf("Hilo [%d] -> Espera por barrera\n", hilo);

    pthread_barrier_wait(br);

    //printf("Hilo [%d] -> Continua despues de barrera\n", hilo);
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
