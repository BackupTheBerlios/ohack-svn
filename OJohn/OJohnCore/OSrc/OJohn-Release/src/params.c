/*
 * This file is part of John the Ripper password cracker,
 * Copyright (c) 1996-98 by Solar Designer
 */



#include "params.h"

int password_hash_sizes[3] = {
	PASSWORD_HASH_SIZE_0,
	PASSWORD_HASH_SIZE_1,
	PASSWORD_HASH_SIZE_2
};

int password_hash_thresholds[3] = {
	PASSWORD_HASH_THRESHOLD_0,
	PASSWORD_HASH_THRESHOLD_1,
	PASSWORD_HASH_THRESHOLD_2
};



/*Modificaciones por Omar Zapata*/
/*esta es para los archivos de usuario*/
char* OGetJOhnPrivateHome(char *PATH_ADD)
{
const char PATH[100];
sprintf(PATH,"%s/%s",getenv("HOME"),PATH_ADD);
return PATH;
}

/*Esta es para los archivos del systema*/
char* OGetJohnSystemWide(char *PATH_ADD)
{
const char PATH[100];
sprintf(PATH,"%s/%s",getenv("OHACK"),PATH_ADD);
return PATH;
}
/*finaliza modificaciones Omar*/
