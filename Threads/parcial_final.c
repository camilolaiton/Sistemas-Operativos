#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <wait.h>

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

void usar_condicional_wait(pthread_cond_t *, pthread_mutex_t *, int);
void crear_condicional(pthread_cond_t *);   //Fucnion para crear condicional
void destruir_condicional(pthread_cond_t *);    //funcion para destruir condicional

void error(char *); //Funcion de error
void *fnc_hilo(void *);

int sensores_pasados = 0;
int sensor_habilitado = -1;
pthread_mutex_t mutex_1, mutex_2;
pthread_barrier_t barrier_1, barrier_2;
pthread_cond_t cond_1, cond_2;

struct parametros   //EStructura para pasar datos a hilos
{
    int id_sensor;   //Numero del hilo creado
    int tiempo;
    char *orden;
    char *delimitador;
};

int main(int argc, char const *argv[])
{
    struct parametros *par = NULL;
    pthread_t *threads = NULL;  //Variable de hilos
    int n_hilos = 0;
    char orden[50] = {'\0'};
    char delimitador[2] = {'|', '\0'};

    printf("\n\t*********************************\n");
    printf("\t**     SISTEMA DE CONTROL      **\n");
    printf("\t*********************************\n");
    printf("\t**        CAMILO LAITON        **\n");
    printf("\t*********************************\n");

    printf("Ingrese la cantidad de sensores del edificio inteligente: \n");
    scanf("%d", &n_hilos);
    getchar();

    if(n_hilos > 0)
    {
        threads = (pthread_t*)malloc(n_hilos*sizeof(pthread_t));

        if(threads)
        {

            crear_condicional(&cond_1);
            crear_condicional(&cond_2);
            crear_cerrojo(&mutex_1);
            crear_cerrojo(&mutex_2);

            for (int i = 0; i < n_hilos; i++)
            {
                par = (struct parametros*)malloc(sizeof(struct parametros));

                if(par)
                {
                    par->id_sensor = i;
                    par->tiempo = 0;
                    par->orden = orden;
                    par->delimitador = delimitador;

                    if(pthread_create(&threads[i], NULL, &fnc_hilo, par))
                        error("No se ha podido crear hilo\n");
                }
                else
                {
                    error("No se ha podido crear espacio de memoria par\n");
                }
            }

            while (strcmp(orden, "salir") != 0)
            {
                do
                {
                    printf("SERVIDOR -> Ingrese la orden: \n");
                    scanf("%[^\n]", orden);
                    getchar();

                    if(strlen(orden) == 0)
                    {
                        error("SERVIDOR -> Ingrese una orden valida!\n");
                    }

                } while (strlen(orden) == 0);

                if(pthread_mutex_lock(&mutex_1))
                        error("No se ha podido hacer lock en mutex\n");

                sensor_habilitado = -1;

                if(pthread_mutex_unlock(&mutex_1))
                    error("No se ha podido hacer unlock en mutex\n");

                if(strcmp(orden, "salir") == 0)
                {
                    if(pthread_cond_broadcast(&cond_1))
                        error("No se ha podido despertar todos los sensores\n");
                }
                else
                {
                    if(pthread_cond_broadcast(&cond_1))
                        error("No se ha podido despertar todos los sensores\n");
                }
                
                /*
                if(pthread_mutex_lock(&mutex_2))
                    error("No se ha podido hacer lock en mutex\n");

                sensores_pasados = 0;

                if(pthread_cond_signal(&cond_1))
                    error("No se ha podido activar otro sensor\n");

                usar_condicional_wait(&cond_2, &mutex_2, -1);

                if(pthread_mutex_unlock(&mutex_2))
                    error("No se ha podido hacer unlock en mutex\n");
                    
                */
            }
            
            for (int i = 0; i < n_hilos; i++)
            {
                if(!pthread_join(threads[i], (void*)&par))
                {
                    printf("Hilo [%d] -> Ha terminado\n", i);
                }
                else
                {
                    error("No se ha podido esperar hilo\n");
                }
                
            }

            destruir_cerrojo(&mutex_2);
            destruir_cerrojo(&mutex_1);
            destruir_condicional(&cond_1);
            destruir_condicional(&cond_2);
            
        }
        else
        {
            error("No se pudo alocar espacio de memoria\n");
        }
        
    }
    else
    {
        error("Cantidad de sensores invalida\n");
    }
    

    return 0;
}

void *fnc_hilo(void *param)
{
    struct parametros *par = (struct parametros*)param;
    int tiempo = 0;
    int veces_pasadas = 0;
    char mensaje[40] = {'\0'};
    char *token = NULL;
    char buffer[50] = {'\0'};

    if(pthread_mutex_lock(&mutex_1))
        error("No se ha podido hacer lock en mutex\n");

    usar_condicional_wait(&cond_1, &mutex_1, par->id_sensor);

    if(pthread_mutex_unlock(&mutex_1))
        error("No se ha podido hacer unlock en mutex\n");

    while (strcmp(par->orden, "salir") != 0)
    {
        strcpy(buffer, par->orden);
        token = strtok(buffer, par->delimitador);
        //printf("Sensor [%d] -> Paso orden [%s] DEL[%s]\n", par->id_sensor, par->orden, par->delimitador);

        if(token != NULL)
        {
            veces_pasadas = 0;

            while (token != NULL)
            {
                veces_pasadas++;

                if(veces_pasadas == 1)
                {
                    if(pthread_mutex_lock(&mutex_1))
                        error("No se ha podido hacer lock en mutex\n");

                    sensor_habilitado = atoi(token);

                    if(pthread_mutex_unlock(&mutex_1))
                        error("No se ha podido hacer unlock en mutex\n");
                    //printf("SENSOR [%d] -> Vez pasada [%d] VALoR [%d]\n", par->id_sensor, veces_pasadas, sensor_habilitado);
                }
                else if(veces_pasadas == 2)
                {
                    tiempo = atoi(token);
                    //printf("SENSOR [%d] -> Vez pasada [%d] VALoR [%d]\n", par->id_sensor, veces_pasadas,tiempo);
                }
                else if(veces_pasadas == 3)
                {
                    //printf("SENSOR [%d] -> Vez pasada [%d] VALoR [%s]\n", par->id_sensor, veces_pasadas, token);

                    if(sensor_habilitado == par->id_sensor)
                    {
                        printf("SENSOR [%d] -> FUI HABILITADO TIEMPO [%d] MENSAJE [%s]\n", par->id_sensor, tiempo, token);
                        par->tiempo = tiempo;

                        usleep(tiempo);
                        printf("SENSOR [%d] -> TERMINO EL TIEMPO [%d]\n", par->id_sensor, par->tiempo);
                        memset(buffer, '\0', 50);
                    }
                }
                
                //printf("SENSOR [%d] -> Token: %s\n",par->id_sensor, token);
                token = strtok(NULL, par->delimitador);
            }

            //printf("SENSOR [%d] -> CADENA LUEGO [%s]\n", par->id_sensor, buffer);
        }
        
        token = NULL;

        if(pthread_mutex_lock(&mutex_1))
        error("No se ha podido hacer lock en mutex\n");

        usar_condicional_wait(&cond_1, &mutex_1, par->id_sensor);

        if(pthread_mutex_unlock(&mutex_1))
            error("No se ha podido hacer unlock en mutex\n");

    }
    
    pthread_exit(param);
}

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void usar_condicional_wait(pthread_cond_t *cd, pthread_mutex_t *mt, int hilo)
{
    //printf("Hilo [%d] -> Espero...\n", hilo);

    if(pthread_cond_wait(cd, mt))
        error("No se pudo esperar cond\n");
        
    //printf("Hilo [%d] -> Continuo ejecucion\n", hilo);
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
