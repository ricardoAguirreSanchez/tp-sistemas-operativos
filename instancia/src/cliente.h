#ifndef CLIENTEH
#define CLIENTEH

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include "funcionalidad_instancia.h"

#define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez


int conectar_coodinador();

void saludo_inicial_coordinador(int sockfd);

#endif


