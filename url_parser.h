#ifndef __URL_PARSER_H__
#define __URL_PARSER_H__



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#define URL_BUF_MAX_LEN  4096

typedef struct _url_buf
{
    char   buf[ URL_BUF_MAX_LEN ];
    int  len;
} URL_BUF_STRUCT;


char   *url_encode( char const *s, int len, int *new_length );
int   url_decode( char *str, int len );
int   url_parse( URL_BUF_STRUCT *url_buf, char *url );
char   *get_param_value( URL_BUF_STRUCT *url_buf, char *name );
char *get_param_array(char *buf,char *name,int len);
int getparam(char *buf,char *name,char *param);
char *getparam_data(char *buf,char *name,int len);


#endif
