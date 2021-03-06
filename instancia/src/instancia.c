#include <stdio.h>
#include <stdlib.h>
#include "funcionalidad_instancia.h"
#include "cliente.h"

int main(int argc, char *argv[]) {
	if(argc < 2){
		puts("Falta el archivo de config");
		return EXIT_FAILURE;
	}

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	CANT_ENTRADA = 0;
	TAMANIO_ENTRADA = 0;
	get_parametros_config(argv[1]);
	int sockfd = conectar_coodinador();
	saludo_inicial_coordinador(sockfd);

	crearPuntoDeMontaje(PUNTO_MONTAJE);

	//REcibo datos de entrada
	recibo_datos_entrada(sockfd);

	//Inicializo mis estructuras
	inicializo_estructuras();

	//leer los archivos .txt creador a partir del dump para asi poder cargar mis estructuras administrativas
	reestablecer_datos();

	pthread_mutex_init(&MUTEX_INSTANCIA,NULL);
	pthread_t punteroHiloDump;

	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

	pthread_create(&punteroHiloDump, &tattr, (void*) realizar_dump, NULL);

	pthread_attr_destroy(&tattr);

	//Envio mis datos
	envio_datos(sockfd);

	//recibo mensaje de si me pudieron agregar a la lista  o no
	recibo_mensaje_aceptacion(sockfd);

	while(1){
		int resultado = recibo_sentencia(sockfd);
		if(resultado != COMPACTACION_LOCAL && resultado != OK_STATUS){
			envio_resultado_al_coordinador(sockfd,resultado);
		}
	}
	//Por ahora libero la memoria que me quedó (solo hasta agregar funcionalidad posta)
	free_algo_punt_nom();
	free_estruct_admin();
	return EXIT_SUCCESS;
}
