#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct parametros
{
    int hilo;
    char *buffer;
};

pthread_mutex_t mutex;
void *fnc_leer(void *);
void *fnc_escribir(void *);
int validar_num(int);
void error(char*);
void crear_cerrojo(pthread_mutex_t *);
void destruir_cerrojo(pthread_mutex_t *);
void crear_condicional(pthread_cond_t *);
void destruir_condicional(pthread_cond_t *);

pthread_mutex_t mutex_lector, mutex_escritor;
pthread_cond_t cond_lector, cond_escritor;

int n_hilos = 0;

int main(int argc, char const *argv[])
{

    char buffer[200] = {'\0'};
    pthread_t *threads = NULL;
    struct parametros *par = NULL;

    printf("Ingrese la cantidad de hilos a crear: \n");
    scanf("%d", &n_hilos);
    getchar();

    if(validar_num(n_hilos))
    {

        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(threads)
        {

            crear_cerrojo(&mutex_lector);
            crear_cerrojo(&mutex_escritor);
            crear_condicional(&cond_escritor);
            crear_condicional(&cond_lector);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));
                par->hilo = i;
                par->buffer = buffer;

                if(i == 0)
                {
                    if(pthread_create(&threads[i], NULL, &fnc_leer, par))
                        error("No se pudo crear el hilo");
                }
                else
                {
                    if(pthread_create(&threads[i], NULL, &fnc_escribir, par))
                        error("No se pudo crear el hilo");
                }
                
            }

            for (int i = 0; i < n_hilos; i++)
            {
                par = NULL;

                if(!pthread_join(threads[i], (void*)&par))
                {
                    printf("Hilo [%d] terminado!\n", par->hilo);
                    free(par);
                }
            }

            destruir_condicional(&cond_escritor);
            destruir_condicional(&cond_lector);
            destruir_cerrojo(&mutex_lector);
            destruir_cerrojo(&mutex_escritor);

            free(threads);
        }
        else
        {
            error("No se pudo crear el espacio de los hilos");
        }
        
    }
    else
    {
        error("Numero de hilos invalido");
    }

    return 0;
}

void crear_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_init(cond, NULL))
        error("No se pudo crear condicional");
}
void destruir_condicional(pthread_cond_t *cond)
{
    if(pthread_cond_destroy(cond))
        error("No se pudo eliminar condicional");
}

void destruir_cerrojo(pthread_mutex_t *mt)
{
	if(pthread_mutex_destroy(mt))
		error("No se ha podido destruir cerrojo");
}

void crear_cerrojo(pthread_mutex_t *mt)
{
	if(pthread_mutex_init(mt, NULL))
		error("No se pudo crear mutex");
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int validar_num(int n_hilos)
{
    if(n_hilos > 0)
        return 1;
    return 0;
}

void *fnc_leer(void *param)
{
    struct parametros *par = (struct parametros *)param;

    //printf("Hilo [%d] -> funciona\n", par->hilo);

    int sigo = 1;

	while(sigo)
	{
		if(pthread_mutex_lock(&mutex_lector))
			error("No se pudo crear cerrojo");
		
		while(strlen(par->buffer) && strcmp(par->buffer, "salir") != 0)
		{
			if(pthread_cond_wait(&cond_lector, &mutex_lector))
				error("No se espero cond leer [R]");
		}

		do
		{
			printf("Hilo [%d] -> Ingrese un mensaje: \n", par->hilo);
			scanf("%[^\n]", par->buffer);
			getchar();

			if(strlen(par->buffer) == 0)
			{
				printf("Hilo [%d] -> ERROR, cadena incorrecta!\n", par->hilo);
			}

		}while(strlen(par->buffer) == 0);

		//printf("Activo sgnal write\n");
		if(pthread_cond_signal(&cond_escritor))
			error("No se activo cond escribir [R]");

		if(strcmp(par->buffer, "salir") == 0)
			sigo = 0;

		if(pthread_mutex_unlock(&mutex_lector))
			error("No se pudo quitar cerrojo");
	}

    pthread_exit(param);
}

void *fnc_escribir(void *param)
{
    struct parametros *par = (struct parametros *)param;

    //printf("Hilo [%d] -> funciona\n", par->hilo);
    

	int sigo = 1;

	while(sigo)
	{
		if(pthread_mutex_lock(&mutex_escritor))
			error("No se pudo hacer cerrojo");

		while(!strlen(par->buffer))
		{
			if(pthread_cond_wait(&cond_escritor, &mutex_escritor))
				error("No se espero cond escribir [W]");
		}

		printf("Hilo [%d] -> Imprime: [%s]\n", par->hilo, par->buffer);
		
        if(strcmp(par->buffer, "salir") == 0)
        {
            sigo = 0;
        }

        if(par->hilo == n_hilos - 1)
        {
            memset(par->buffer, '\0', 200);

            if(pthread_cond_signal(&cond_lector))
                error("No se ha podido activar cond lector");
        }
        else
        {

            if(pthread_cond_signal(&cond_escritor))
                error("No se ha podido activar cond escritor");

            if(sigo)
            {
                if(pthread_cond_wait(&cond_escritor, &mutex_escritor))
			    	error("No se espero cond escribir [W]");
            }
        }

		//printf("Activo sgnal read\n");

		if(pthread_mutex_unlock(&mutex_escritor))
			error("No se pudo quitar cerrojo");
	}

    
    pthread_exit(param);
}