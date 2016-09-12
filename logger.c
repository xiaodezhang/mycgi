/*
 *  Author  : liwei
 *  Date    : 2015.07.29
 *  Brief   : logger
 *  Version : 1.0
 *  Histroy : NONE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>

#include <logger.h>


#define DEFAUT_LOG_FILE_PATH "./cm.log"


static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE * s_logger_fp = NULL;

void logger_init( char *path )
{

#if 1
	if( !path )
	{
		s_logger_fp = fopen( DEFAUT_LOG_FILE_PATH, "a+" );
	}
	else
	{
		s_logger_fp = fopen( path, "a+" );
	}
	
	if ( s_logger_fp == NULL )
	{
		fprintf( stderr, "open logger file failure :%s\n", DEFAUT_LOG_FILE_PATH );
	}
#else	
	s_logger_fp = stderr;
#endif
}

void debug( int level, const char *fmt, ... )
{
	va_list ap;
	static time_t  time_now  = 0;
	static time_t  time_last = 0;
	static struct  tm now_tm = { 0 };
	static char    time_str_buf[64] = { 0 };

	pthread_mutex_lock( &( s_mutex ) );

	va_start( ap, fmt );

	time( &time_now );
	if ( time_last < time_now )
	{
		time_last = time_now;

		memset( &now_tm, 0, sizeof( struct tm ) );
		gmtime_r( &time_now, &now_tm );

		memset( time_str_buf, 0, sizeof( time_str_buf ) );
		strftime( time_str_buf , sizeof( time_str_buf ), "%F %T", &now_tm);
	}

	switch ( level )
	{
	case LOG_LEVEL_DEBUG:
		fprintf( s_logger_fp, "[ DEBUG ] %s ", time_str_buf );
		break;

	case LOG_LEVEL_INFO:
		fprintf( s_logger_fp, "[ INFO  ] %s ", time_str_buf );
		break;

	case LOG_LEVEL_WARN:
		fprintf( s_logger_fp, "[ WARN  ] %s ", time_str_buf );
		break;

	case LOG_LEVEL_ERROR:
		fprintf( s_logger_fp, "[ ERROR ] %s ", time_str_buf );
		break;

	default:
		pthread_mutex_unlock( &( s_mutex ) );
		break;
	}

	vfprintf( s_logger_fp, fmt, ap );

	va_end( ap );


	pthread_mutex_unlock( &( s_mutex ) );

	
	fflush( NULL );
}

void myerror(const char * s){
    fputs(s,s_logger_fp);
}

