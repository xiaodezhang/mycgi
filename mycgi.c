#include"pro.h"
#include<math.h> 
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<fastcgi/fcgiapp.h>
#include<pthread.h>
#include<event2/event.h>
#include<event2/event_struct.h>
#include<unistd.h>
#include"logger.h"
#include<pthread.h>
#include<string.h>
#include<stdlib.h>

#define MSTATE           0
#define MACTION          1
#define MPOSTION         2
#define MTRACETABLE      3
#define MGETPOS          4
#define MGETPASSWORD     5
#define MGETDIR          6
#define MGETENGINE       7
#define MGETIP           8
#define MGTINTER         9

#define DEFD  "/tmp/fifo_web_decode"
#define PROFD "/tmp/fifo_web_decode_protocol"
#define HTTP_CONENT_HEAD "Content-type: text/html\r\n\r\n"
static struct event_base *cgi_eb;
static struct event defifo,weserver;
int fd[2];

guide_t *guide;

extern void deal_pro(int sock,short event,void *arg);
extern void web_pro(FCGX_Request *);

static void web_accept(int sock,short event,void *arg){

    FCGX_Request request;
    
    FCGX_InitRequest(&request,FCGI_LISTENSOCK_FILENO,0);

    if(FCGX_Accept_r(&request) == 0){
       web_pro(&request);
       FCGX_Finish_r(&request);
    }
}

static int wconnect(){

    FCGX_Init();
   
    event_assign(&weserver,cgi_eb,FCGI_LISTENSOCK_FILENO,EV_READ | EV_PERSIST,
                 web_accept,NULL);
    event_add(&weserver,NULL);
    return 1;
}

static int deconnect(){

    char *fdpath[2] = { DEFD,PROFD };
    int i,wronly[2] = { O_WRONLY,O_RDONLY };
    char *cmd[] = {
        "\r\nlog oemstatus ontime 1\r\n","\r\nlog satinfo ontime 1\r\n",
        "\r\nlog interfaceinfo ontime 1\r\n",
        "\r\nlog channelinfo ontime 1\r\n"
    };

    for(i = 0;i < 2;i++){
        if(access(fdpath[i],F_OK) == -1){
            if(mkfifo(fdpath[i],0777) != 0){
                loge("mkfifo error\n");
                return 0;
            }
        }
        if((fd[i] = open(fdpath[i],wronly[i])) == -1){
            loge("open error\n");
            return 0;
        }
    }
    for(i = 0;i < sizeof(cmd)/sizeof(cmd[0]);i++){
        if(write(fd[0],cmd[i],strlen(cmd[i])) == -1){
            loge("write cmd error\n");
            return 0;
        }
    }
#if 1
    event_assign(&defifo,cgi_eb,fd[1],EV_READ | EV_PERSIST,deal_pro,NULL);
    event_add(&defifo,NULL);
#endif

    return 1;
}

int main(){


    if(!(guide = calloc(sizeof(guide_t),1))){
        loge("guide calloc error\n");
        return 0;
    }
    logger_init("./cgi.log");
    cgi_eb = event_base_new();

    if(!wconnect()){
        loge("web connect error\n");
        return 0;
    }
    if(!deconnect()){
        loge("de connect error\n");
        return 0;
    }

    event_base_dispatch(cgi_eb);
    event_base_free(cgi_eb);
    free(guide);
    return 0;
}
