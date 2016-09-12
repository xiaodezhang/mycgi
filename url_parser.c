#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#include "url_parser.h"

static unsigned  char hexchars[] = "0123456789ABCDEF";


static int url_htoi( char *s )
{
    int value;
    int c;
    
    c = ( ( unsigned  char * )s )[0];
    if ( isupper( c ) )
    {
        c = tolower( c );
    }
    value = ( c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10 ) * 16;
    
    c = ( ( unsigned  char * )s )[1];
    if ( isupper( c ) )
    {
        c = tolower( c );
    }
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
    
    return ( value );
}


char *url_encode( char const *s, int len, int *new_length )
{
    register unsigned  char c;
    unsigned  char *to, *start;
    unsigned  char const *from, *end;
    
    from = ( unsigned  char * )s;
    end  = ( unsigned  char * )s + len;
    start = to = ( unsigned  char * ) calloc( 1, 3 * len + 1 );
    
    while ( from < end )
    {
        c = *from++;
        
        if ( c == ' ' )
        {
            *to++ = '+';
        }
        else if ( ( c < '0' && c != '-' && c != '.' ) ||
                  ( c < 'A' && c > '9' ) ||
                  ( c > 'Z' && c < 'a' && c != '_' ) ||
                  ( c > 'z' ) )
        {
            to[0] = '%';
            to[1] = hexchars[c >> 4];
            to[2] = hexchars[c & 15];
            to += 3;
        }
        else
        {
            *to++ = c;
        }
    }
    *to = 0;
    if ( new_length )
    {
        *new_length = to - start;
    }
    return ( char * ) start;
}


int url_decode( char *str, int len )
{
    char *dest = str;
    char *data = str;
    
    while ( len-- )
    {
        if ( *data == '+' )
        {
            *dest = ' ';
        }
        else if ( ( *data == '%' )
                  && ( len >= 2 )
                  && isxdigit( ( int ) * ( data + 1 ) )
                  && isxdigit( ( int ) * ( data + 2 ) ) )
        {
            *dest = ( char ) url_htoi( data + 1 );
            data += 2;
            len -= 2;
        }
#if 1
        else  if ( *data == '&' )
        {
            *dest = '\0';
        }
#endif
        else
        {
            *dest = *data;
        }
        
        data++;
        dest++;
    }
    
    *dest = '\0';
    
    return dest - str;
}



/* decode url to url_buf */
int url_parse( URL_BUF_STRUCT *url_buf, char *url )
{
    if ( url_buf  && url )
    {
        memcpy( url_buf->buf, url, strlen( url ) );
        url_buf->len  = url_decode( url_buf->buf, strlen( url_buf->buf ) );
        
        return 1;
    }
    
    return 0;
}

#if 0
extern void upurldecode(char *url){

    char *s;
    int len;

    for(s = url;*s;s++){
        if(*s == '+')
            *s = ' ';
        else if((*s == '%') && (len >= 2) &&
                 isxdigit((int)*(s+1)) && isxdigit((int)*(s+2)))
            *s = (char)url_htoi(s+1);
    }
}
#endif
/* get a name relavtived value */
char *get_param_value( URL_BUF_STRUCT *url_buf, char *name )
{
    char *url = url_buf->buf;
    int url_len = url_buf->len;
    char *pos = NULL;
    char *cur = NULL;
    int  namelen  = 0;
    
    if ( !name || ( url_len <= 0 ) )
    {
        return NULL;
    }
    
    namelen = strlen( name );
    cur = url;
    
    while ( cur )
    {
       	if ( !strncmp( cur, name, namelen ) )
       	{
			pos = cur;

			if ( pos && ( *( pos + namelen ) == '=' ) )
	        {

	            return ( pos + namelen + 1 );
	        }
       	}
        
        cur += strlen( cur ) + 1;
        
        if ( cur >= ( url + url_len ) )
        {
            cur = NULL;
        }
    }
    
    return NULL;
}

char *get_param_array(char *buf,char *name,int len){

    char *s;
    int i;

    for(s = buf,i = 0;i < len;s++,i++){
        if(!strncmp(s,name,strlen(name)) &&
                *(s+strlen(name)) == '=')
            return s+strlen(name)+1;
    }

    return NULL;
}

extern int getparam(char *buf,char *name,char *param){

    char *s,*p = buf+strlen(buf);

    for(s = buf;;s++){
        if(!strncmp(s,name,strlen(name)) &&
                *(s+strlen(name)) == '=')
            p = s+strlen(name)+1;
        if((*s == '&' || !(*s)) && (s-p > 0)){
            memcpy(param,p,s-p);
            param[s-p] = '\0';
            return s-p;
        }
        if(!(*s))
            break;
    }
    return 0;
}
extern char *getparam_data(char *buf,char *name,int len){

    char *s,*p;

    for(s = buf;*s;s++){
        if(strlen(s) < strlen(name)) 
            break;
        if(!strncmp(s,name,strlen(name)) &&
                *(s+strlen(name)) == '='){
            if(strlen(s) > strlen(name)+len){
                p = s+strlen(name)+1;
                *(p+len) = '\0';
                return p;
            }
        }
    }
    return NULL;
}
