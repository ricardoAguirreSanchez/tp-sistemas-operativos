/*
 ============================================================================
 Name        : Prueba_execv.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	char* arr[] = {"ls", NULL};
//	char* arr[] = {"ls", "-l", "-R", "-a", NULL};
	execv("/bin/ls", arr);

	return EXIT_SUCCESS;
}
