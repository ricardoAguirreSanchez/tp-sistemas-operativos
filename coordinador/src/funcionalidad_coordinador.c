/*
 * funcionalidad_coordinador.c
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#include "funcionalidad_coordinador.h"



void get_parametros_config(char* path){

	//t_config* config = config_create("config.cfg");
	t_config* config = config_create(path);
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit(1);
	}

	IP_CONFIG_MIO = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_MIO,config_get_string_value(config, "IP_CONFIG_MIO"));
	PUERTO_ESCUCHA_CONEXION = config_get_int_value(config,"PUERTO_ESCUCHA_CONEXION");
	PUERTO_ESCUCHA_CONEXION_STATUS = config_get_int_value(config,"PUERTO_ESCUCHA_CONEXION_STATUS");
	ALGORITMO_DISTRIBUCION = malloc(sizeof(char) * 100);
	strcpy(ALGORITMO_DISTRIBUCION,config_get_string_value(config, "ALGORITMO_DISTRIBUCION"));

	CANTIDAD_ENTRADAS = config_get_int_value(config,"CANTIDAD_ENTRADAS");
	TAMANIO_ENTRADA = config_get_int_value(config,"TAMANIO_ENTRADA");
	RETARDO = config_get_int_value(config,"RETARDO");
	//RETARDO = RETARDO / 1000;
	config_destroy(config);
}

//libera todos los parametros que tenga
void free_parametros_config(){

	free(ALGORITMO_DISTRIBUCION);
}

void configure_logger() {
  LOGGER = log_create("log de operaciones.log","tp-redistinto",0,LOG_LEVEL_INFO);
  log_info(LOGGER, "Empezamos.....");

}

void inicializo_semaforos(){
	pthread_mutex_init(&MUTEX_INDEX,NULL);
	pthread_mutex_init(&MUTEX_RECV_INSTANCIA,NULL);
	pthread_mutex_init(&MUTEX_INSTANCIA,NULL);
	pthread_mutex_init(&MUTEX_REGISTRO_INSTANCIA,NULL);

	pthread_mutex_init(&MUTEX_RESPUESTA_STATUS,NULL);
	pthread_cond_init(&CONDICION_RESPUESTA_STATUS, NULL);

	pthread_cond_init(&CONDICION_REGISTRO_INSTANCIA, NULL);
	pthread_cond_init(&CONDICION_INSTANCIA, NULL);
	pthread_cond_init(&CONDICION_RECV_INSTANCIA, NULL);

	pthread_mutex_init(&MUTEX_RESPUESTA_PLANIFICADOR,NULL);
	pthread_cond_init(&CONDICION_RESPUESTA_PLANIFICADOR, NULL);


}

t_list* create_list(){
	t_list * Lready = list_create();
	return Lready;
}

void envio_datos_entrada(int fd_instancia){
	void* bufferEnvio = malloc(sizeof(int32_t)*2);
	memcpy(bufferEnvio,&TAMANIO_ENTRADA,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),&CANTIDAD_ENTRADAS,sizeof(int32_t));

	if (send(fd_instancia, bufferEnvio,sizeof(int32_t)*2, 0) == -1) {
		printf("No se pudo enviar datos de entrada a la instancia\n");
		//muere hilo
		pthread_exit(NULL);
	}
	printf("Datos de entrada enviado correctamente\n");
	free(bufferEnvio);
}

t_registro_instancia* creo_registro_instancia(char* nombre_instancia, char* clave){
	t_registro_instancia* registro_instancia = malloc(sizeof(t_registro_instancia));
	registro_instancia->nombre_instancia = malloc(sizeof(char)*100);
	registro_instancia->clave = malloc(sizeof(char)*40);
	strcpy(registro_instancia->nombre_instancia,nombre_instancia);
	strcpy(registro_instancia->clave,clave);
	return registro_instancia;
}

t_instancia_respuesta* creo_instancia_respuesta(char* nombre_instancia,int respuesta,int espacio_libre){
	t_instancia_respuesta* instancia_respuesta = malloc(sizeof(t_instancia_respuesta));
	instancia_respuesta->nombre_instancia = malloc(sizeof(char)*200);
	strcpy(instancia_respuesta->nombre_instancia,nombre_instancia);
	instancia_respuesta->respuesta = respuesta;
	instancia_respuesta->espacio_libre = espacio_libre;
	return instancia_respuesta;
}


t_Instancia* creo_instancia(int fd_instancia){
	t_Instancia* instancia_nueva = malloc(sizeof(t_Instancia));
	instancia_nueva->nombre_instancia = malloc(sizeof(char)*100);

	instancia_nueva->fd = fd_instancia;

	//Recibo int:longitud nombre y char*: nombre
	int32_t longitud = 0;
	int numbytes = 0;

	if ((numbytes = recv(fd_instancia, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el tamaño del nombre de la instancia\n");
		pthread_exit(NULL);
	}
	char* nombreInstancia = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(fd_instancia, nombreInstancia, longitud, 0)) == -1) {
		printf("No se pudo recibir mensaje saludo\n");
		pthread_exit(NULL);
	}
	strcpy(instancia_nueva->nombre_instancia,nombreInstancia);

	int32_t espacio_libre = 0;
	if ((numbytes = recv(fd_instancia, &espacio_libre, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el espacio libre de la instancia\n");
		free(nombreInstancia);
		pthread_exit(NULL);
	}

	instancia_nueva->nombre_instancia[strlen(instancia_nueva->nombre_instancia)]='\0';
	instancia_nueva->espacio_libre = espacio_libre;

	free(nombreInstancia);
	printf("Quiere ingresar la instancia de nombre:%s\n",instancia_nueva->nombre_instancia);

	return (instancia_nueva);
}

bool controlo_existencia(t_Instancia * instanciaNueva){
	bool _existInstancia(t_Instancia* una_instancia) { return strcmp(una_instancia->nombre_instancia,instanciaNueva->nombre_instancia)== 0;}
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	if(list_find(LIST_INSTANCIAS, (void*)_existInstancia) != NULL){
		pthread_mutex_unlock(&MUTEX_INSTANCIA);
		return true;
	}
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
	return false;
}

void send_mensaje_rechazo(t_Instancia * instancia_nueva){
	int32_t rechazo = 1;
	if (send(instancia_nueva->fd, &rechazo,sizeof(int32_t), 0) == -1) {
		printf("No se pudo enviar rechazo a la INSTANCIA\n");
	} else {
		printf("Se envio rechazo de NOMBRE existente para la INSTANCIA: %s\n",
				instancia_nueva->nombre_instancia);
	}
}

void agrego_instancia_lista(t_list* list,t_Instancia* instancia_nueva){

	//Aviso a la instancia q si lo pude agregar sin problema
	int32_t ok = 2; //instancia entiende q 1 es rechazo, cualquier otro valor es OK
	if (send(instancia_nueva->fd, &ok,sizeof(int32_t), 0) == -1) {
		printf("No se pudo enviar aceptacion a la INSTANCIA\n");
	} else {
		printf("Se envio aceptacion la INSTANCIA: %s\n",
				instancia_nueva->nombre_instancia);
	}
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	list_add(list,instancia_nueva);
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
	printf("Se agrego la instancia de nombre:%s y de cant. de entradas libres:%d a la lista\n",instancia_nueva->nombre_instancia,instancia_nueva->espacio_libre);

}

t_Instancia* busco_instancia_por_algortimo(char* clave,int flag_reestablecer){
	if (strstr(ALGORITMO_DISTRIBUCION, "EL") != NULL) {
		return equitativeLoad(flag_reestablecer);
	}
	if (strstr(ALGORITMO_DISTRIBUCION, "LSU") != NULL) {
		printf("INFO: Algoritmo LSU\n");
		return LeastSpaceUsed();
	}
	if (strstr(ALGORITMO_DISTRIBUCION, "KE") != NULL) {
		printf("INFO: Algoritmo KE\n");
		return keyExplicit(clave);
	}
	return NULL;
}

char ** get_clave_valor(int fd_esi) {

		int leng_clave = 0;
		int leng_valor = 0;
		int numbytes = 0;
		if ((numbytes = recv(fd_esi, &leng_clave, sizeof(int32_t), 0)) == -1) {
			printf("No se pudo recibir el tamaño de la clave\n");
			//MUERO
			exit(1);
		}
		char* clave = malloc(sizeof(char) * leng_clave);
		if ((numbytes = recv(fd_esi, clave, sizeof(char) * leng_clave, 0))
				== -1) {
			printf("No se pudo recibir la clave\n");
			//MUERO
			exit(1);
		}

		if ((numbytes = recv(fd_esi, &leng_valor, sizeof(int32_t), 0)) == -1) {
			printf("No se pudo recibir el tamaño del valor\n");
			//MUERO
			exit(1);
		}
		char* valor = malloc(sizeof(char) * leng_valor);
		if ((numbytes = recv(fd_esi, valor, sizeof(char) * leng_valor, 0))
				== -1) {
			printf("No se pudo recibir la valor\n");
			//MUERO
			exit(1);
		}
		printf("Recibi clave: %s valor: %s correctamente\n", clave, valor);

		char ** resultado = malloc(sizeof(char*) * 2);
		resultado[0] = clave;
		resultado[1] = valor;

		return resultado;
	}

int envio_recibo_tarea_store_instancia(int32_t id_operacion, char* clave,t_Instancia* instancia){

	int32_t len_clave = strlen(clave) + 1; // Tomo el CLAVE de la sentencia SET q me llega de la instacia
	int resultado_instancia = 0;
	void* bufferEnvio = malloc(sizeof(int32_t) * 2 + len_clave);
	memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
	memcpy(bufferEnvio+sizeof(int32_t), &len_clave, sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t)*2,clave,len_clave);

	//limpio mi lista de instancia respuestasds
	limpia_destruye_elemt_lista_respuesta_instancia();

	if(instancia == NULL){
		printf("Se desconecto la instancia\n");
		free(bufferEnvio);
		//la saco de mi tabla de registro instancia
		remove_registro_instancia(clave);
		return ABORTA_ESI_CLAVE_INNACCESIBLE;
	}

	if (send(instancia->fd, bufferEnvio,sizeof(int32_t) * 2 + len_clave, 0) == -1) {
		printf("No se pudo enviar la info a la INSTANCIA\n");
		free(bufferEnvio);
		resultado_instancia = ABORTA_ESI_CLAVE_INNACCESIBLE; //para q esi sepa que falle

		//la saco de mi tabla de registro instancia
		remove_registro_instancia(instancia->nombre_instancia);

		return resultado_instancia;
	}
	printf("Se envio STORE clave: %s a la INSTANCIA correctamente\n",clave);

	//mientras la instancia no mande su respuesta loopeo, cuando me active la busco
	bool _esNombreInstancia(t_instancia_respuesta* instancia_respuesta) { return (strcmp(instancia_respuesta->nombre_instancia,instancia->nombre_instancia)==0);}

	pthread_mutex_lock(&MUTEX_RECV_INSTANCIA);
	t_instancia_respuesta * instancia_resp = list_find(LIST_INSTANCIA_RESPUESTA, (void*)_esNombreInstancia);
	pthread_mutex_unlock(&MUTEX_RECV_INSTANCIA);

	pthread_mutex_lock(&MUTEX_RECV_INSTANCIA);
	while(instancia_resp == NULL){
		pthread_cond_wait(&CONDICION_RECV_INSTANCIA,&MUTEX_RECV_INSTANCIA); //espero a la respuesta de la instancia (si es q la instancia esta) por 10 segundos
		instancia_resp = list_find(LIST_INSTANCIA_RESPUESTA, (void*)_esNombreInstancia);
	}
	pthread_mutex_unlock(&MUTEX_RECV_INSTANCIA);
	resultado_instancia = instancia_resp->respuesta;
	if(resultado_instancia == OK_STORE_INSTANCIA){
		printf("Sentencia STORE realizado correctamente\n");
	}else if(resultado_instancia == FALLO_INSTANCIA_CLAVE_SOBREESCRITA){
		//esto significa que la clave ya no existe en la instancia, entonces la saco de mi tabla de registro instancia (INSTANCIA-CLAVE)
		remove_registro_instancia(instancia->nombre_instancia);
	}
	//limpio mi lista de instancia respuesta
	limpia_destruye_elemt_lista_respuesta_instancia();

	free(bufferEnvio);

	return resultado_instancia;
}

t_Instancia* get_instancia_by_name(char* name){
	bool _getInstancia(t_Instancia* instancia) { return strcmp(instancia->nombre_instancia,name)== 0;}
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	t_Instancia* instancia = list_find(LIST_INSTANCIAS, (void*)_getInstancia);
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
	return instancia;

}
int envio_tarea_instancia(int32_t id_operacion, t_Instancia * instancia,char** clave_valor_recibido) {

		int32_t long_clave = strlen(clave_valor_recibido[0]) + 1; // Tomo el CLAVE de la sentencia SET q me llega de la instacia
		int32_t long_valor = strlen(clave_valor_recibido[1]) + 1; // Tomo la VALOR  de la sentencia SET q me llega de la instacia
		void* bufferEnvio = malloc(sizeof(int32_t) * 3 + long_clave + long_valor);
		int resultado_instancia = 0;
		memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t), &long_clave, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t)*2,clave_valor_recibido[0],long_clave);
		memcpy(bufferEnvio + sizeof(int32_t)*2 + long_clave, &long_valor,sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 3) + long_clave,clave_valor_recibido[1], long_valor);

		//limpio mi lista de instancia respuesta
		limpia_destruye_elemt_lista_respuesta_instancia();

		if(instancia == NULL){
			//la saco de mi tabla de registro instancia
			remove_registro_instancia(clave_valor_recibido[0]);
			return ABORTA_ESI_CLAVE_INNACCESIBLE;
		}
		if (send(instancia->fd, bufferEnvio,
				sizeof(int32_t) * 3 + long_clave + long_valor, 0) == -1) {
			printf("Se desconecto la instancia\n");
			free(bufferEnvio);

			//la saco de mi tabla de registro instancia
			remove_registro_instancia(clave_valor_recibido[0]);
			return ABORTA_ESI_CLAVE_INNACCESIBLE;
		}
		printf("Se envio SET clave: %s valor: %s a la %s correctamente\n",clave_valor_recibido[0], clave_valor_recibido[1],instancia->nombre_instancia);

		//mientras la instancia no mande su respuesta loopeo, cuando me active la busco
		bool _esNombreInstancia(t_instancia_respuesta* instancia_respuesta) { return (strcmp(instancia_respuesta->nombre_instancia,instancia->nombre_instancia)==0);}

		pthread_mutex_lock(&MUTEX_RECV_INSTANCIA);
		t_instancia_respuesta * instancia_resp = list_find(LIST_INSTANCIA_RESPUESTA, (void*)_esNombreInstancia);
		pthread_mutex_unlock(&MUTEX_RECV_INSTANCIA);

		pthread_mutex_lock(&MUTEX_RECV_INSTANCIA);
		while(instancia_resp == NULL){
			pthread_cond_wait(&CONDICION_RECV_INSTANCIA,&MUTEX_RECV_INSTANCIA); //espero a la respuesta de la instancia (si es q la instancia esta) por 10 segundos
			instancia_resp = list_find(LIST_INSTANCIA_RESPUESTA, (void*)_esNombreInstancia);
		}
		pthread_mutex_unlock(&MUTEX_RECV_INSTANCIA);

		resultado_instancia = instancia_resp->respuesta;
		if(resultado_instancia == OK_SET_INSTANCIA){
			printf("Sentencia SET realizado correctamente\n");
			//actualizo su espacio (en lista_instnacias solo si es SET OK)
			instancia->espacio_libre = instancia_resp->espacio_libre;
			printf("Actualizo para la instancia %s la nueva cant. de entradas libres sera de %d\n",instancia->nombre_instancia,instancia->espacio_libre);
			//registro la INSTANCIA para esa clave si es que no esta registrado antes
			pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
			bool _registrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave_valor_recibido[0])== 0;}
			t_registro_instancia* reg = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_registrInstancia);
			pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
			strcpy(reg->nombre_instancia,instancia_resp->nombre_instancia);
			printf("Se registra en instancia-clave la %s con clave: %s\n",instancia_resp->nombre_instancia,clave_valor_recibido[0]);
		}else if(resultado_instancia == FALLO_INSTANCIA_CLAVE_SOBREESCRITA){
			//esto significa que la clave ya no existe en la instancia, entonces la saco de mi tabla de registro instancia
			remove_registro_instancia(clave_valor_recibido[0]);
		}

		free(bufferEnvio);

		//limpio mi lista de instancia respuesta
		limpia_destruye_elemt_lista_respuesta_instancia();

		return resultado_instancia;
}
void loggeo_respuesta(char* operacion, int32_t id_esi,int32_t resultado_linea){
	char* registro = malloc(sizeof(char) * 500);
	strcpy(registro, "ESI ");
	char* idEsi = string_itoa(id_esi);
	strcat(registro, idEsi);
	strcat(registro, " ");
	strcat(registro, operacion);
	strcat(registro, "  => ");
	switch (resultado_linea) {
	case OK_PLANIFICADOR:
		strcat(registro, "PLANIFICADOR INFORMA QUE SE REALIZO CORRECTAMENTE");
		break;
	case OK_BLOQUEADO_PLANIFICADOR:
		strcat(registro, "SE BLOQUEO AL ESI CORRECTAMENTE");
		break;
	case OK_SET_INSTANCIA:
		strcat(registro, "INSTANCIA INFORMA SET REALIZADO CORRECTAMENTE");
		break;
	case OK_STORE_INSTANCIA:
		strcat(registro, "INSTANCIA INFORMA STORE REALIZADO CORRECTAMENTE");
		break;
	case ABORTA_ESI_CLAVE_NO_IDENTIFICADA:
		strcat(registro, "SE ABORTA EL ESI POR CLAVE NO IDENTIFICADA");
		break;
	case ABORTA_ESI_CLAVE_NO_BLOQUEADA:
		strcat(registro, "SE ABORTA EL ESI POR CLAVE NO BLOQUEADA");
		break;
	case ABORTA_ESI_CLAVE_INNACCESIBLE:
		strcat(registro, "SE ABORTA EL ESI POR CLAVE INNACCESIBLE");
		break;
	case FALLO_PLANIFICADOR:
		strcat(registro, "OCURRIO UN ERROR CON EL PLANIFICADOR, MIRAR CONSOLA DEL PLANIFICADOR");
		break;
	case FALLO_INSTANCIA_CLAVE_SOBREESCRITA:
		strcat(registro, "OCURRIO UN ERROR CON LA INSTANCIA, LA CLAVE NO EXISTE MAS EN LA INSTANCIA DADO QUE SE SOBREESCRIBIO");
		break;
	case FALLA_PLANIFICADOR_DESCONECTADO:
		strcat(registro, "FALLA: planificador desconectado");
		break;
	case FALLA_ELEGIR_INSTANCIA:
		strcat(registro, "FALLA: instancia desconectado");
		break;
	case FALLA_SIN_INSTANCIA_CLAVE_STORE:
		strcat(registro, "FALLA: No existe ninguna instancia con esa clave");
		break;
	case ABORTA_ESI_ERROR_TAMANIIO_CLAVE:
			strcat(registro, "SE ABORTA ESI POR CLAVE SUPERA TAMAÑO MAXIMO DE 40 CARACTERES");
			break;
	case FALLO_CASO_BORDE:
			strcat(registro, "OCURRIO UN ERROR CON LA INSTANCIA, LA CLAVE NO SE INSERTO POR FALTA DE ESPACIO FINAL, CASO BORDE");
			break;
	case ABORTA_ESI_DESCONECTADO:
			strcat(registro, "Error de comunicacion, ESI desconectado");
			break;
	case FALLO_ENTRADA_MAS_GRANDE:
			strcat(registro, "OCURRIO UN ERROR CON LA INSTANCIA, EL VALOR DE LA CLAVE OCUPA MAS ENTRADAS DE LAS QUE TENIA ANTES");
			break;
	default:
		strcat(registro, "606 - ERROR INTERNO ");
		break;
	}
	log_info(LOGGER, registro);
	free(idEsi);
	free(registro);
}

void loggeo_info(int32_t id_operacion, int32_t id_esi, char* clave_recibida,char* valor_recibida) {
	char* registro = malloc(sizeof(char) * 500);
	strcpy(registro, "ESI ");
	char* idEsi = string_itoa(id_esi);
	strcat(registro, idEsi);
	switch (id_operacion) {
	case GET:
		strcat(registro, " GET ");
		strcat(registro, clave_recibida);
		break;
	case SET:
		strcat(registro, " SET ");
		strcat(registro, clave_recibida);
		strcat(registro, " ");
		strcat(registro, valor_recibida);
		break;
	case STORE:
		strcat(registro, " STORE ");
		strcat(registro, clave_recibida);
		break;

	default:
		strcat(registro, " - ");
		break;
	}
	log_info(LOGGER, registro);
	free(idEsi);
	free(registro);
}

int32_t envio_tarea_planificador(int32_t id_operacion, char* clave_recibida,
			int32_t id_esi) {

		int32_t len_clave = strlen(clave_recibida) + 1;

		//limpio respuestas viejas
		pthread_mutex_lock(&MUTEX_RESPUESTA_PLANIFICADOR);
		RESPUESTA_PLANIFICADOR = 0;
		pthread_mutex_unlock(&MUTEX_RESPUESTA_PLANIFICADOR);

		//envio: ID_OPERACION,ID_ESI,LENG_CLAVE,CLAVE
		void* bufferEnvio = malloc(
				sizeof(int32_t) * 3 + sizeof(char) * len_clave);
		memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t), &id_esi, sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 2), &len_clave,
				sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 3), clave_recibida,
				sizeof(char) * len_clave);

		if (send(FD_PLANIFICADOR, bufferEnvio,
				(sizeof(int32_t) * 3) + sizeof(char) * len_clave, 0) == -1) {
			printf("No se pudo enviar la info al planificador\n");
			exit(1);
		} else {
			printf("Se envio la tarea con clave: %s ID de ESI:%d al PLANIFICADOR correctamente\n",clave_recibida, id_esi);
		}

		pthread_mutex_lock(&MUTEX_RESPUESTA_PLANIFICADOR);
		int32_t resultado_linea = RESPUESTA_PLANIFICADOR;
		pthread_mutex_unlock(&MUTEX_RESPUESTA_PLANIFICADOR);

		pthread_mutex_lock(&MUTEX_RESPUESTA_PLANIFICADOR);
		while(resultado_linea <= 0){
			pthread_cond_wait(&CONDICION_RESPUESTA_PLANIFICADOR,&MUTEX_RESPUESTA_PLANIFICADOR);
			resultado_linea = RESPUESTA_PLANIFICADOR;
		}
		pthread_mutex_unlock(&MUTEX_RESPUESTA_PLANIFICADOR);

		free(bufferEnvio);
		return (resultado_linea);
}


int reciboRespuestaInstancia(int fd_instancia){
	int32_t respuestaInstacia = 0;
	int32_t numbytes = 0;
	if ((numbytes = recv(fd_instancia, &respuestaInstacia, sizeof(int32_t), 0)) <= 0) {
		if (numbytes == 0) {
		// conexión cerrada
			printf("Error de Comunicación: Se fue el INSTANCIA FD: %d\n",fd_instancia);

		} else {
			perror("ERROR: al recibir respuesta de la INSTANCIA");

		}
		respuestaInstacia = ABORTA_ESI_CLAVE_INNACCESIBLE;
	}else{
		printf("Recibimos respuesta de Instancia FD: %d\n", fd_instancia);

	}
	return respuestaInstacia;
}


int reciboEspacioLibre(int fd_instancia){
	int32_t espacioLibre = 0;
	int32_t numbytes = 0;
	if ((numbytes = recv(fd_instancia, &espacioLibre, sizeof(int32_t), 0)) <= 0) {
		//aca es si se desconecto la instancia, cosa q manejara el reciboResultadoInstancia
		espacioLibre = 0;
	}else{
		//printf("Recibimos nuevo tamaño de Instancia FD: %d\n", fd_instancia);

	}
	return espacioLibre;
}

void free_instancia(t_Instancia * instancia){
	free(instancia->nombre_instancia);
	free(instancia);
}

void free_registro_instancia(t_registro_instancia* registro_instancia){
	free(registro_instancia->clave);
	free(registro_instancia->nombre_instancia);
	free(registro_instancia);
}

void free_instancia_respuesta(t_instancia_respuesta* instancia_respuesta){
	free(instancia_respuesta->nombre_instancia);
	free(instancia_respuesta);
}

void remove_instancia(int fd_instancia){
	bool _esInstanciaFd(t_Instancia* instancia) { return instancia->fd == fd_instancia;}
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	t_Instancia* unInstancia = list_find(LIST_INSTANCIAS,(void*)_esInstanciaFd);
	if(unInstancia != NULL){
		printf("Sacamos a la INSTANCIA: %s de la lista\n", unInstancia->nombre_instancia);
		list_remove_and_destroy_by_condition(LIST_INSTANCIAS,(void*) _esInstanciaFd,(void*) free_instancia);
	}
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
}


void remove_registro_instancia( char * clave){
	pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
	bool _esRegistroInstancia(t_registro_instancia* reg) { return strcmp(reg->clave,clave) == 0;}
	t_registro_instancia* registro_inst = list_find(LIST_REGISTRO_INSTANCIAS,(void*)_esRegistroInstancia);
	if(registro_inst != NULL){
		printf("Sacamos de mi registro instancia - clave a la %s\n", registro_inst->nombre_instancia);
		list_remove_and_destroy_by_condition(LIST_REGISTRO_INSTANCIAS,(void*) _esRegistroInstancia,(void*) free_registro_instancia);
	}
	pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
}

//Cargo la instancia_respuesta sin repetir! (el tamaño solo lo tomo en cuenta del otro lado cuando es set)
void cargo_instancia_respuesta(char * nombre_instancia,int nueva_respuesta,int espacio_libre){

	if(nueva_respuesta == COMPACTACION_GLOBAL){
		printf("La instancia:%s me dijo que todos deben compactar\n",nombre_instancia);
		//envio mensaje a todas las demas instancias q tengo conectadas para q compacten menos a la mia
		envio_mensaje_masivo_compactacion_instancias(nombre_instancia);
	}else{
		//Solo cargo el resultado si la respuesta es diferente de q todos compacten
		bool _existInstanciaRespuesta(t_instancia_respuesta* inst_respue) { return strcmp(inst_respue->nombre_instancia,nombre_instancia)== 0;}
		pthread_mutex_lock(&MUTEX_RECV_INSTANCIA);
		if( list_find(LIST_INSTANCIA_RESPUESTA, (void*)_existInstanciaRespuesta) != NULL){
			t_instancia_respuesta * insta_respu = list_find(LIST_INSTANCIA_RESPUESTA, (void*)_existInstanciaRespuesta);
			insta_respu->respuesta = nueva_respuesta;
			insta_respu->espacio_libre = espacio_libre;
		}else{
			t_instancia_respuesta* instancia_respuesta = creo_instancia_respuesta(nombre_instancia,nueva_respuesta,espacio_libre);
			list_add(LIST_INSTANCIA_RESPUESTA,instancia_respuesta);

		}
		pthread_mutex_unlock(&MUTEX_RECV_INSTANCIA);
	}


}

void envio_mensaje_masivo_compactacion_instancias(char* nombre_instancia){
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	int control = 0;
	if(!list_is_empty(LIST_INSTANCIAS)){
		void _envioPedidoCompacta(t_Instancia* instancia) {
			if(strcmp(instancia->nombre_instancia,nombre_instancia) != 0){
				int32_t operacion = COMPACTACION_LOCAL;
				if (send(instancia->fd, &operacion,sizeof(int32_t), 0) == -1) {
					printf("No se pudo enviar pedido de compactacion local a la instancia :%s\n",instancia->nombre_instancia);
				}else{
					control++;
					printf("Pedido de compactacion local envia a %s correctamente\n",instancia->nombre_instancia);
				}

			}
		}
		list_iterate(LIST_INSTANCIAS,(void*)_envioPedidoCompacta);
	}

	if(control == 0){
		printf("No hay ninguna instancia para enviar pedido de compactacion local\n");
	}
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
}

bool exist_clave_registro_instancias(char * clave){
	bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave)== 0;}
	pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
	if(list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia) != NULL){
		pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
		return true;
	}
	pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
	return false;
}

t_Instancia* equitativeLoad(int flag_reestablecer){
	printf("Aplico Algoritmo EL\n");
	t_Instancia* instancia;
	int index_anterior = 0;
	pthread_mutex_lock(&MUTEX_INDEX);
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	if(flag_reestablecer){index_anterior = INDEX;}
	if((INDEX == list_size(LIST_INSTANCIAS)) || (INDEX > list_size(LIST_INSTANCIAS)) ){
		INDEX = 0;
		instancia = list_get(LIST_INSTANCIAS,INDEX);//ojo q list_get si no encuentra nada retorna NULL
		INDEX ++;
	}else{
		instancia = list_get(LIST_INSTANCIAS,INDEX);
		if(instancia)
			INDEX ++;
		else
			INDEX = 0;
	}
	if(instancia != NULL){//pregunto si efectivamente hay algo
		if(flag_reestablecer){INDEX = index_anterior;}
		pthread_mutex_unlock(&MUTEX_INSTANCIA);
		pthread_mutex_unlock(&MUTEX_INDEX);
		return instancia;
	}
	printf("Error al tratar de elegir una instancia\n");
	if(flag_reestablecer){INDEX = index_anterior;}
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
	pthread_mutex_unlock(&MUTEX_INDEX);
	return NULL;
}

t_Instancia* LeastSpaceUsed() {
	int i;
	t_Instancia* instancia = NULL;
	t_Instancia* instancia_max = NULL;
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	int size_lista =list_size(LIST_INSTANCIAS);

	for (i = 0; i < size_lista; i++) {
		if (i == 0) {

			instancia_max = list_get(LIST_INSTANCIAS, i);

		} else {

			instancia = list_get(LIST_INSTANCIAS, i);

			if ((instancia_max->espacio_libre) < (instancia->espacio_libre)) {

				instancia_max = list_get(LIST_INSTANCIAS, i);

			}
		}
	}
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
	if (instancia_max != NULL) {			//pregunto si efectivamente hay algo
		return instancia_max;
	}

	printf("Error al tratar de elegir una instancia\n");
	return NULL;

}

t_Instancia* keyExplicit(char* clave) {
	//a:97--z:124
	if((!(clave[0]>= 97 && clave[0]<=123) && !(clave[0]>= 65 && clave[0]<=91)) ||
			(clave[0] == 164) || (clave[0] == 165)){
		printf("Error al tratar de elegir una instancia, la clave no empieza con una letra del [a-z] (sin ñ) o [A-Z] (sin Ñ)\n");
		return NULL;
	}
	int i;
	pthread_mutex_lock(&MUTEX_INSTANCIA);

	int cant_instancias = LIST_INSTANCIAS->elements_count;

	if (cant_instancias > 0) {

		int letras_por_instancia = 26 / cant_instancias;
		int resto = 26 % cant_instancias;
		int letras_ultima_instancia = letras_por_instancia;

		//veo si hay decimal
		if(resto>0){
			letras_por_instancia = letras_por_instancia + 1;
			letras_ultima_instancia = 26 - (letras_por_instancia*(cant_instancias - 1));
		}

		t_Instancia * instancia;
		for (i = 0; i < cant_instancias; i++) {
			//ultima
			if(((i+1) == cant_instancias)){
				if(esta_grupo_ultimo(clave[0],i,letras_por_instancia,letras_ultima_instancia)){
					instancia = list_get(LIST_INSTANCIAS,i);
					pthread_mutex_unlock(&MUTEX_INSTANCIA);
					return instancia;
				}
			//otras
			}else{
				if(esta_grupo(clave[0],i,letras_por_instancia)){
					instancia = list_get(LIST_INSTANCIAS,i);
					pthread_mutex_unlock(&MUTEX_INSTANCIA);
					return instancia;
				}
			}

		}
	}
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
	printf("Error al tratar de elegir una instancia\n");
	return NULL;
}

bool esta_grupo(char primeraLetra, int numGrupo,int cantLetrasPorInstancia){
	int numeroCaracter;
	//dependiendo que case sea, lo convertimos en 1 en adelante
	if((primeraLetra >= 97) && (primeraLetra<=123)){
		numeroCaracter = primeraLetra - 96;
	}else if((primeraLetra>= 65) && (primeraLetra<=91)){
		numeroCaracter = primeraLetra - 64;
	}

	if((numeroCaracter >= (numGrupo*cantLetrasPorInstancia+1)) && (numeroCaracter <= (numGrupo+1)*cantLetrasPorInstancia)){
		return true;
	}
	return false;
}

bool esta_grupo_ultimo(char primeraLetra, int numGrupo,int cantLetrasPorInstancia,int cantLetrasUltimaInstancia){
	int numeroCaracter;
	//dependiendo que case sea, lo convertimos en 1 en adelante
	if((primeraLetra >= 97) && (primeraLetra<=123)){
		numeroCaracter = primeraLetra - 96;
	}else if((primeraLetra>= 65) && (primeraLetra<=91)){
		numeroCaracter = primeraLetra - 64;
	}

	if((numeroCaracter >= (numGrupo*cantLetrasPorInstancia+1)) && (numeroCaracter <= (numGrupo*cantLetrasPorInstancia+cantLetrasUltimaInstancia))){
		return true;
	}
	return false;
}

int aplicar_filtro_respuestas(int resultado_linea){
	if(resultado_linea == OK_SET_INSTANCIA || resultado_linea == OK_STORE_INSTANCIA || resultado_linea == OK_PLANIFICADOR){
		return OK_ESI;
	}
	if(resultado_linea == OK_BLOQUEADO_PLANIFICADOR){
		return BLOQUEADO_ESI;
	}

	return ABORTA_ESI;

}

void limpia_destruye_elemt_lista_respuesta_instancia(){
	pthread_mutex_lock(&MUTEX_RECV_INSTANCIA);
	list_clean_and_destroy_elements(LIST_INSTANCIA_RESPUESTA,(void*)free_instancia_respuesta);
	pthread_mutex_unlock(&MUTEX_RECV_INSTANCIA);
}



