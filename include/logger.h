#ifndef __LOGGER_H__
#define __LOGGER_H__

#define LOG_LEVEL_ERROR    7
#define LOG_LEVEL_WARN     6
#define LOG_LEVEL_INFO     5
#define LOG_LEVEL_DEBUG    4

#ifdef LOGFLAG
#define loge(fmt, args...) \
    debug(LOG_LEVEL_ERROR, "%s %d : " fmt,  __FUNCTION__, __LINE__, ##args)

#define logw(fmt, args...) \
    debug(LOG_LEVEL_WARN,  "%s %d : " fmt,  __FUNCTION__, __LINE__, ##args)

#define logi(fmt, args...) \
    debug(LOG_LEVEL_INFO,  "%s %d : " fmt,  __FUNCTION__, __LINE__, ##args)

#define logd(fmt, args...) \
    debug(LOG_LEVEL_DEBUG, "%s %d : " fmt,  __FUNCTION__, __LINE__, ##args)
#else
#define loge(fmt, args...)
#define logw(fmt, args...)
#define logd(fmt, args...)
#define logi(fmt, args...)
#endif


void logger_init( char *path );
void debug( int level, const char *fmt, ... );
void myerror(const char *s);

#endif

