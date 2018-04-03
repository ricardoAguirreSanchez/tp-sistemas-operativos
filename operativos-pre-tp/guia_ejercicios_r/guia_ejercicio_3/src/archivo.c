/*
 * archivo.c
 *
 *  Created on: 31/3/2018
 *      Author: utnso
 */


#include "archivo.h"


FILE* txt_open_file(char* path, char* condicion) {
	return fopen(path,condicion);
}


void txt_close_file(FILE* file) {
	fclose(file);
}

void txt_read_all(FILE* file){
	//tamaño maximo q podemos usar para traer toda una linea entera (hasta el /n inclusive)
	char* line = malloc(sizeof(char)*256);
	while (fgets(line, 256, file)) {
		printf("%s", line);
	}
	free(line);

}

void txt_read_especific_line(FILE* file, int line_number){
	char* line = malloc(sizeof(char)*256);
	int contador = 1;
	while (fgets(line, 256, file)) {

		if(contador == line_number){
			printf("Fila n° %d elegida: %s\n",line_number,line);
			break;
		}
		contador++;
	}
	free(line);


}

void txt_carga_inicial(FILE* file){
	char* fila = malloc(sizeof(char) * 256);
	strcpy(fila,"abuenos; juan ; 12; 2233442344; 55665555; 9\n");
	fwrite(fila,sizeof(char),strlen(fila),file);
	strcpy(fila,"djujuy; pepe; 9; 2233442344; 55665555; 9\n");
	fwrite(fila,sizeof(char),strlen(fila),file);
	strcpy(fila,"atucuman; marcos ; 10; 2233442344; 5435665555; 9\n");
	fwrite(fila,sizeof(char),strlen(fila),file);
	strcpy(fila,"erivadavia; julia; 10; 2233442344; 55665555; 9\n");
	fwrite(fila,sizeof(char),strlen(fila),file);


	strcpy(fila,"erivadavia; julia; 10; 2233442344; 55665555; 9\n");
	fwrite(fila,sizeof(char),strlen(fila),file);


	free(fila);
}

void persona_split(char* line, t_Persona * p){
	strcpy(p->localidad,strsep(&line,";"));
	p->localidad[strlen(p->localidad)] = '\0';

	strcpy(p->nombre,strsep(&line,";"));
	p->nombre[strlen(p->nombre)] = '\0';

	p->edad = atoi(strsep(&line,";"));
	p->telef = atol(strsep(&line,";"));
	p->dni = atol(strsep(&line,";"));
	p->saldo = atol(strsep(&line,";"));

}

t_list* txt_save_in_list(FILE* archivo){
	t_list * listaGeneral =  list_create();
	char* line = malloc(sizeof(char)*256);
	while (fgets(line, 256, archivo)) {
		//creo la persona
		t_Persona * persona = malloc(sizeof(t_Persona));
		persona->nombre = malloc(sizeof(char) * 30);
		persona->localidad = malloc(sizeof(char) * 30);
		//la cargo de datos
		persona_split(line,persona);
		//la guardo en la lista general
		list_add(listaGeneral, persona);

	}
	free(line);
	return listaGeneral;
}

//t_list* txt_save_in_list(FILE* archivo){
//	t_list * listaGeneral =  list_create();
//	char* line = malloc(sizeof(char)*256);
//	while (1) {
//		if (fgets(line,256, archivo) == NULL) break;
//		//creo la persona
//		t_Persona * persona = malloc(sizeof(t_Persona));
//		persona->nombre = malloc(sizeof(char) * 30);
//		persona->localidad = malloc(sizeof(char) * 30);
//		//la cargo de datos
//		strcpy(persona->localidad,strsep(&line,";"));
//		strcpy(persona->nombre,strsep(&line,";"));
//		persona->edad = atoi(strsep(&line,";"));
//		persona->telef = atol(strsep(&line,";"));
//		persona->dni = atol(strsep(&line,";"));
//		persona->saldo = atol(strsep(&line,";"));
//		//la guardo en la lista general
//		list_add(listaGeneral, persona);
//		strcpy(line,"fin\0");
//	}
//
//	free(line);
//	return listaGeneral;
//}

bool menor_edad(t_Persona* person_menor, t_Persona* person) {
	bool condicion = (person_menor->edad < person->edad);
	return condicion;
}

//El comparador devuelve si el primer parametro debe aparecer antes que el
bool localidad(t_Persona *person_menor, t_Persona *person) {
	return (person_menor->localidad[0] < person->localidad[0]);
}


void txt_order_list(t_list* lista){

	list_sort(lista, (void*) localidad);

	//list_sort(lista, (void*) menor_edad);

}

void txt_save_in_file(t_list* lista_personas){
	FILE * archivo = txt_open_file("prueba_archivo_ordenado.txt","w");

	void guardar(t_Persona* persona) {
		char* fila = malloc(sizeof(char) * 256);
		strcpy(fila,persona->localidad);
		strcat(fila," |");
		strcat(fila,persona->nombre);
		strcat(fila,"\n");
		fwrite(fila,sizeof(char),strlen(fila),archivo);
		free(fila);
	}

	list_iterate(lista_personas,(void *) guardar);
	txt_close_file(archivo);

}

static void persona_destroy(t_Persona *self) {
    free(self->localidad);
    free(self->nombre);
    free(self);
}

void txt_clear_list(t_list* lista_personas){

//	int size = list_size(lista_personas);
//	int i;
//	for(i = 0; i < size; i++){
//		list_remove_and_destroy_element(lista_personas,i, (void*)persona_destroy);
//	}
//	free(lista_personas);
	list_destroy_and_destroy_elements(lista_personas, (void*) persona_destroy);
}



