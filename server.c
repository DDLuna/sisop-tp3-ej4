// Objetivos:
// *Comunicacion con los clientes
//  sumarle un punto si contestó bien, restarle 1 punto si contestó mal.
// *Determinar el ganador del juego.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define PUERTO 8000
#define MAX_CANTIDAD_CLIENTES 20

void *aceptar_clientes(void *puntero);
void *leer_clientes(void *numero_cliente);

int server_socket;
int socket_clientes[MAX_CANTIDAD_CLIENTES];
struct sockaddr_in direccion_socket;
int tamanio_direccion = sizeof(direccion_socket);
int cant_clientes = 0;
char respuesta_cliente[MAX_CANTIDAD_CLIENTES][256];
pthread_t hilos_lectores[MAX_CANTIDAD_CLIENTES];

int main(int arg, char **argv)
{

  FILE *arch;
  char linea[256];
  arch = fopen("preguntas.txt", "r");
  fgets(linea, sizeof(linea), arch);
  int cant_preguntas = atoi(linea);
  char preguntas[cant_preguntas][256];
  char respuestas[cant_preguntas][256];

  pthread_t hilo_acepta_clientes;
  setbuf(stdout, NULL);
  for (int i = 0; i < cant_preguntas; i++)
  {
    fgets(linea, sizeof(linea), arch);
    strcpy(preguntas[i], linea);
    fgets(linea, sizeof(linea), arch);
    strcpy(respuestas[i], linea);
  }
  // funcionan bien las lecturas
  // for(int i = 0; i < cantPreguntas; i++) {
  //   printf("%s\n", preguntas[i]);
  //   printf("%s\n", respuestas[i]);
  // }
  fclose(arch);
  //Creacion de socket:
  // int sockfd = socket(dominio, tipo, protocolo)
  // sockfd = descriptor del socket (como un manejador del socket)
  // dominio = entero que especifica dominio de comunicacion, AF_INET para IPv4
  // tipo = tipo de comunicacion, SOCK_STREAM para TCP, SOCK_DGRAM para UDP
  // protocolo = valor para el protocolo de internet (IP), que es 0
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1)
  {
    puts("No se pudo crear socket");
    exit(1);
  }
  printf("Socket creado...\n");
  //Completar la estructra de la direccion
  direccion_socket.sin_family = AF_INET;         // IPV4
  direccion_socket.sin_addr.s_addr = INADDR_ANY; //Local
  direccion_socket.sin_port = htons(PUERTO);     //Numero de puerto
  // Bind
  // int bind(int sockfd, cosnt struct sockaddr *addr, socklen_t addrlen)
  // Enlaza el socket 'sockfd' a la dirección y al puerto especificado
  // en addr
  if (bind(server_socket, (struct sockaddr *)&direccion_socket, tamanio_direccion) < 0)
  {
    puts("no se pudo bindear;");
    exit(1);
  }
  printf("Socket enlazado...\n");
  //Escuchar
  // int listen(int sockfd, int backlog)
  // Pone al socket en modo passivo, donde espera a que un cliente haga una conexion
  // backlog define la cantidad máxima de conexiónes pendientes para el socket
  // Si llega una conexion y la cola está llena se rechaza la conexión
  listen(server_socket, 10);
  printf("Socket escuchando...\n");
  // Creacion de thread
  // int pthread_create(pthread_t *hilo, const pthread_artr_t* attr, void *(*rutina), void *argumentos);
  // Crea un hilo y guarda el ID en "hilo",
  pthread_create(&hilo_acepta_clientes, NULL, &aceptar_clientes, NULL);
  for (int i = 10; i > 0; i--)
  {
    printf("Esperado clientes durante %d segundos...\n", i);
    sleep(1);
  }
  pthread_cancel(hilo_acepta_clientes); //Deja de aceptar clientes
  int puntajes[cant_clientes];
  for (int i = 0; i < cant_clientes; i++)
  {
    puntajes[i] = 0;
    strcpy(respuesta_cliente[i], "*");
    printf("contando\n");
  }

  for (int nro_pregunta = 0; nro_pregunta < cant_preguntas; nro_pregunta++)
  {
    for (int nro_cliente = 0; nro_cliente < cant_clientes; nro_cliente++)
      send(socket_clientes[nro_cliente], preguntas[nro_pregunta], sizeof(preguntas[nro_pregunta]), 0);
    // read(socket_clientes[0], mensaje_cliente, sizeof(mensaje_cliente));
    int alguien_respondio = 0;
    while (!alguien_respondio)
    {
      // printf("Respuesta: %s\n", respuesta_cliente[0]);
      for (int i = 0; i < cant_clientes; i++)
      {
        if (strcmp(respuesta_cliente[i], "*") != 0)
        {
          printf("%s\n%s",respuesta_cliente[i], respuestas[nro_pregunta]);
          if (strcmp(respuesta_cliente[i], respuestas[nro_pregunta]) == 0)
            puntajes[i]++;
          else
            puntajes[i]--;
          strcpy(respuesta_cliente[i], "*");
          alguien_respondio = 1;
        }
      }
    }
    printf("Puntajes:\n");
    for (int i = 0; i < cant_clientes; i++)
    {
      printf("Cliente: %d Puntaje: %d\n", (i + 1), puntajes[i]);
    }
  }
  int max_puntaje = puntajes[0];
  int max_cliente = 0;
  for (int i = 1; i < cant_clientes; i++)
  {
    if (puntajes[i] > max_puntaje)
    {
      max_puntaje = puntajes[i];
      max_cliente = i;
    }
  }
  printf("Gano el jugador %d, con %d puntos!", max_cliente + 1, max_puntaje);
  for (int i = 0; i < cant_clientes; i++)
  {
    close(socket_clientes[i]);
  }
  close(server_socket);
  return 0;
}

void *aceptar_clientes(void *puntero_a_void)
{
  printf("Thread creado exitosamente.\n");
  while (1)
  { //Aceptar peticiones
    // int listen(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    // Acepta una conexión de la cola de conexiones pendiendientes en el socket sockfd
    // Si no hay una conexión en la cola de espera, el proceso se bloquea esperando.
    // Crea un nuevo socket conectado, y devuelve un socket descriptor asociado al socket
    // Luego de este punto el server y el cliente pueden enviarse datos a traves de este socket.
    socket_clientes[cant_clientes] = accept(server_socket, (struct sockaddr *)&direccion_socket, (socklen_t *)&tamanio_direccion); // Aceptar la primer conexion para poder empezar a jugar.
    cant_clientes++;
    pthread_create(&hilos_lectores[cant_clientes], NULL, leer_clientes, (void *)&cant_clientes);
    printf("Aceptada la %d conexion.\n", cant_clientes);
  }
}

void *leer_clientes(void *puntero_nro_cliente)
{
  char respuesta[256];
  int numero_cliente = (*(int *)puntero_nro_cliente) - 1;
  printf("%d\n", numero_cliente);
  while (1)
  {
    read(socket_clientes[numero_cliente], respuesta, 256 * sizeof(char));
    strcpy(respuesta_cliente[numero_cliente], respuesta);
  }
}