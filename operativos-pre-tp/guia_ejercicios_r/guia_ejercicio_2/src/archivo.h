/*
 * archivo.h
 *
 *  Created on: 31/3/2018
 *      Author: utnso
 */

#ifndef ARCHIVO_H_
#define ARCHIVO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Condicion
"r":Abre un archivo para leer. El archivo debe existir
"w":Crea nuevo archivo, si existe lo sobreescribe como vacio
"r+":Abre un archivo para actualizar tanto la lectura como la escritura. El archivo debe existir.
"w+":Crea un archivo vacío para leer y escribir
"w+b": crea un archivo para leer y escribir, si ya existe lo "limpia"
"r+b": crea un archivo para leer y escribir, si ya existe lo usa sin "Limpiarlo"*/

//Abre un archivo
FILE* txt_open_file(char* path, char* condicion);

//Carga inicial
void txt_carga_inicial(FILE* file);

//Cierra el archivo
void txt_close_file(FILE* file);

//Lee todos los datos
void txt_read_all(FILE* file);

//Lee una linea especifica
void txt_read_especific_line(FILE* file, int line);

//Ejecuta una operacion sobre cada linea del file
void txt_execute_operation_file(FILE* file, void* operation);

//Ejecuta una operacion sobre una lista
void txt_execute_operation_list(char** list, void* operation);


#endif /* ARCHIVO_H_ */
