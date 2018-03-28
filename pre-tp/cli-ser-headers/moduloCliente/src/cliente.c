/*
 * cliente.c
 *
 *  Created on: 25/3/2018
 *      Author: utnso
 */

#include "cliente.h"

void levantarCliente(){

    int sockfd, numbytes;
    char* buf = malloc(sizeof(char) * 25);
    struct sockaddr_in their_addr; // información de la dirección de destino

    //1° Creamos un socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
    their_addr.sin_port = htons(PORT);  // short, Ordenación de bytes de la red
    //their_addr.sin_addr = *((struct in_addr *)he->h_addr);//toma la ip del atributo de la consola
    their_addr.sin_addr.s_addr = inet_addr(IP);//toma la ip directo

    memset( &(their_addr.sin_zero) , 0 , 8);  // poner a cero el resto de la estructura

    if (connect(sockfd, (struct sockaddr *)&their_addr,
                                          sizeof(struct sockaddr)) == -1) {
        perror("connect");
        printf("No se pudo conectar\n");
        exit(1);
    }
    
    //Copia 25: 24 dato y ultimo es \0
    if ((numbytes = recv(sockfd, buf, 25, 0)) == -1) {
        perror("recv");
        printf("No se pudo recibir dato\n");
        exit(1);
    }
    
    
    printf("Recibi: %s\n",buf);
    free(buf);

    while(1){
        //scanf insertara todo en mensajeParaEnviar solo hasta  19 bytes
    	char* mensajeParaEnviar = malloc((sizeof(char) *20));
    	scanf("%s",mensajeParaEnviar);
        
		//Serializamos (deberia pasar q el strlen de diferente de 20 si solo escrib 5 por ejem)
		int tamanioMensaje = strlen(mensajeParaEnviar);
        void *bufferEnvio = malloc((sizeof(int32_t)) + tamanioMensaje);
		memcpy(bufferEnvio, &(tamanioMensaje), sizeof(int32_t));
		//Copio solo los caracteres con valor del mensajeParaEnviar sin el \0
		memcpy(bufferEnvio + sizeof(int32_t), mensajeParaEnviar, tamanioMensaje);

		if (strcmp(mensajeParaEnviar, "exit") == 0){
			if(send(sockfd, bufferEnvio,sizeof(int32_t) + tamanioMensaje, 0)== -1){
				puts("Error al enviar mensaje");
			}
			free(bufferEnvio);
			free(mensajeParaEnviar);
			break;
		}

		if(send(sockfd, bufferEnvio,sizeof(int32_t) + tamanioMensaje, 0)== -1){
			puts("Error al enviar mensaje");
		}

		free(bufferEnvio);
		free(mensajeParaEnviar);
	}

    close(sockfd);
}
