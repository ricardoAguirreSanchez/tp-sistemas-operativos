///*
// * guia_ejercicio_2.c
// *
// *  Created on: 30/3/2018
// *      Author: utnso
// */
//
//
///*
// ============================================================================
// Name        : guia_ejercicios.c
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
// ============================================================================
// */
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
///*
//*   Asigna en el tercer parámetro, la concatenación
//*   de los primeros dos Strings
//*   Ejemplo:
//*   char* nombre = "Ritchie";
//*   char* saludo;
//*   string_concat("Hola ", nombre, &saludo);
//*   =>
//*   saludo = "Hola Ritchie"
//*/
//void string_concat_dinamyc(char* primero,char* segundo,char** resultado){
//	strcpy(resultado[0],primero);
//	strcat(resultado[0],segundo);
//}
//
//
//
//int main(void) {
//	char* nombre1 = malloc(sizeof(char)*5 + 1);
//	strcpy(nombre1,"Hola ");
//	nombre1[strlen(nombre1)] = '\0';
//	char* nombre2 = malloc(sizeof(char)*7 + 1);
//	strcpy(nombre2,"Ritchie");
//	nombre2[strlen(nombre2)] = '\0';
//
//	char* resultado = malloc(strlen(nombre1) + strlen(nombre2) + 1);
//
//	string_concat_dinamyc(nombre1,nombre2,&resultado);
//	resultado[strlen(resultado)] = '\0';
//
//	printf("Saludo del ejercicio 1: %s", resultado);
//
//
//	free(nombre1);
//	free(nombre2);
//	free(resultado);
//	return 0;
//
//
//}