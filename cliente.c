#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>
#define PUERTO 8000

int main(int argc, char *argv[])
{
  int cliente_socket;
  struct sockaddr_in direccion_socket;
  char pregunta[256];
  char respuesta[256];

  //Create socket
  // Exactamente igual a la creación del socket en el server
  cliente_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (cliente_socket == -1)
  {
    printf("No se puedo crear el socket");
  }
  puts("Socket creado");
  direccion_socket.sin_addr.s_addr = inet_addr("127.0.0.1");
  direccion_socket.sin_family = AF_INET;
  direccion_socket.sin_port = htons(PUERTO);

  //Connectar
  // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
  // Conecta el socket 'sockfd' a la dirección especificada por addr,
  // la dirección del server y el puerto estan especificados en addr
  if (connect(cliente_socket, (struct sockaddr *)&direccion_socket, sizeof(direccion_socket)) < 0)
  {
    perror("Server no esta corriendo.");
    return 1;
  }

  puts("Connectado\n");

  //keep communicating with server
  while (1)
  {
    if (read(cliente_socket, pregunta, sizeof(pregunta)) < 0)
    {
      printf("Server cerrado.");
      break;
    }
    printf("Pregunta: %s",pregunta);
    scanf("%s", respuesta);
    // Mandar respuesta al mensaje leido.
    if (send(cliente_socket, respuesta, sizeof(respuesta), 0) < 0)
    {
      puts("Fallo al mandar mensaje");
    }
    if (strcmp(respuesta, "-salir") == 0)
    {
      printf("Saliendo.");
      break;
    }
  }

  close(cliente_socket);
  return 0;
}