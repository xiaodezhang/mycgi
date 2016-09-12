TARGET = N72.fcgi

CC_PRE = arm-none-linux-gnueabi-
CC = $(CC_PRE)gcc


SRCS = mycgi.c \
       web_pro.c  \
	 deal_pro.c \
	 logger.c \
	 parson.c \
	 url_parser.c \
	 oem6/deco.c




CFLAGS= -Wall -Werror -lm -O0 -std=gnu99 \
        -I./ \
        -I./include \
        -I./oem6 \
        -I./parson



LDFLAGS = -L./lib \
         -lfcgi \
         -lpthread \
         -lrt  \
         -levent_core \
         -levent_extra


all:
	$(CC) -o $(TARGET)  $(SRCS) $(CFLAGS) $(LDFLAGS) 

clean:
		rm  -rf $(TARGET)


