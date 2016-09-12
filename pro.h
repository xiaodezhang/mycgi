#ifndef RPO 
#define RRO

#include<fastcgi/fcgiapp.h>
#include"deco.h"

#define SIGLE       0
#define RTK         1
#define SBASSPP     2
#define SBASPPP     3
#define SSRSPP      4
#define SSRPPP      5

#define MAXIP   50
#define MAXPORT 10
#define MAXTIMELEN 1024

#define MAXPRN 30
#define MAXACC  50
#define MAXPASS 50

#define GPSNUM   40
#define GLONUM   40
#define CMPNUM   40
#define SBASNUM  70
#define GALINUM  40
#define MAXINTERFACE 100

#define MAXPRNGPS 32
#define MAXPRNGLO 32
#define MAXPRNGAL 32
#define MAXPRNCMP 32
#define MAXPRNSBS 32
typedef struct{
    unsigned char mainantnworksta;         /*�����߹������ 0������ 1 ���� */
    unsigned char mainantnrfsta;           /*��������Ƶͨ�����������ͬ�� */
    unsigned char aantnworksta;            /*�����߹���״����ͬ��         */
    unsigned char aantnrfsta;              /*�����߹���״����ͬ��         */
}antenna_t;

typedef struct{
    unsigned char acpuload;                /*armоƬcpuռ����             */
    unsigned char dcpuload;                /*dspоƬcpuռ����             */
    unsigned char astatus;                 /*armоƬ������� 0������  1������*/
    unsigned char dstatus;                 /*dspоƬ������� 0������  1������*/
#if 1
    unsigned char fstatus;                 /*fpgaоƬ���������ͬ��        */
#endif
}cpu_t;

typedef struct{
    char utctime[MAXTIMELEN];               /*     UTCʱ��       */
    antenna_t antn;                   /* ���ߺ�������Ƶͨ���������*/
    cpu_t     cpu;                   /*  оƬ�������      */
}state_t;

typedef struct{
    unsigned char satnu[5];                /* satnu[0]:GPS���������� 
                                              satnu[1]:GLONASS����������Ϊ
                                              Galileo,������SBAS  */
    unsigned char gps[MAXPRNGPS];          /* ���ٵ���GPS���ǵ�prn��   */
    unsigned char glo[MAXPRNGLO];
    unsigned char gali[MAXPRNGAL];
    unsigned char cmp[MAXPRNCMP];
    unsigned char sbas[MAXPRNSBS];
}sateprn_t;

typedef struct{
    unsigned char sys;                       /* GPS || BDS || GLONASS   */
    unsigned char obstype;                   /*L1,L2,L5 || B1,B2,B3 || L1,L2*/
    unsigned char state;                     /*0 : close 1: open         */
}channel_t;


typedef struct{                              /*�ַ���             */
    unsigned char ip[MAXIP];      
    unsigned char gateway[MAXIP];
    unsigned char netmask[MAXIP];
    unsigned char dns[MAXIP];
    unsigned char serverip[MAXIP];
}net_t;

typedef struct{
    double mlat;
    double mlon;
    double mhgt;
    double alat;
    double alon;
    double ahgt;
    double pitch;
    double heading;
    double roll;
    unsigned char qf;
    float hdop;
    unsigned char satnum;
    char utctime[MAXTIMELEN];
}direcdata_t;

typedef struct{
    double lat;                   /*      ����       */
    double lon;
    double hgt;
    double undulation;
    unsigned char postype;        /*��λ����        */ 
    float difftime;               /*����ӳ�        */
    char stnid[4];                /*��վid          */
    int  gpsweek;                 /*GPS��           */
    float gpssec;                 /*GPS��           */
    float pdop;
    float hdop;
    float vdop;
    float tdop;
    unsigned char solsatnu;          /*ʹ�ö�λ�������� */
    sateprn_t sateprn;            /*prn              */
}posstate_t;

typedef struct{
    unsigned char prn;                     /*prn                 */
    double        elev;                    /*�߶Ƚ�              */
    double        az;                      /*��λ��              */
    float cn[3];                   /*����Ƶ�������      */
    unsigned char satstatus;
}prncn_t;

typedef struct{
    unsigned char satnu[5];                /* satnu[0]:GPS���������� 
                                              satnu[1]:GLONASS����������Ϊ
                                              Galileo,������SBAS  */
    prncn_t gps[GPSNUM];                 /*GPS��������           */
    prncn_t glo[GLONUM];
    prncn_t gali[GALINUM];
    prncn_t cmp[CMPNUM];
    prncn_t sbas[SBASNUM];
    float   ecutoff;
}tracetable_t;

typedef struct{
    double lat;
    double lon;
    double hgt;
    float ecutoff;
}basepos_t;


typedef struct{
    unsigned char locksys[5];            /*locksys[0]:GPS
                                           locksys[1]:GLONASS,��������Ϊ
                                           Galileo,������SBAS  */
    unsigned char lockprn[5][MAXPRN];
}locksat_t;

typedef struct{
    unsigned char closefre[8];            /*0-7 : "L1","L2","L3","B1","B2",
                                            "B3","GL1","GL2" */
    unsigned char channel;               /*����ͨ����        */
    unsigned char chsta;                 /*ͨ��״̬ 0������ 1���ر� 2������*/
}channelset_t;

typedef struct{
    unsigned char type;                    /*0:��Ƶ����  1��˫Ƶ����*/
    float         baseline;                /*���߳���               */
}utab_t;

typedef struct{
    unsigned char clientbool;               /*0���ǿͻ���ģʽ 1���ͻ���ģʽ */
    unsigned char ip[MAXIP];                /*ip*/
    unsigned char port[MAXPORT];            /*�˿ں�*/
    unsigned char datatype;                 /*0:ԭʼ���� 
                                              1:rtcm3.0 
                                              2:rtcm3.2
                                              3:nmea  */
    unsigned char com;
    float datatime[10];                    /* ���ݴ���Ƶ�� 0��ʾ�����      */
}data_t;

typedef struct{
   unsigned char account[MAXACC];
   unsigned char password[MAXPASS];
}user_t;

typedef struct{
    unsigned char clientbool;
    unsigned char ip[MAXIP];                /*ip*/
    unsigned char port[MAXPORT];            /*�˿ں�*/
}guidenet_t;

typedef struct{
    unsigned char port;
    unsigned char baud;
    unsigned char databit;
    unsigned char check;
    unsigned char stop;
}guideserial_t;

typedef struct{
    unsigned char comid;
    unsigned char flag;
    unsigned short msgid;
    unsigned short period;
}interfaces_t;

typedef struct{
    unsigned char num;
    interfaces_t interfaces[MAXINTERFACE];
    unsigned char retype;
    unsigned char comtype[3];
    unsigned char sytype;
    unsigned char sidotype;
    unsigned char dirtype;
    double        dirlen;
}interface_t;


typedef struct{
    state_t state;
    sateprn_t sateprn;
    posstate_t posstate;
    tracetable_t tracetable;
    basepos_t basepos;
    user_t user;
    direcdata_t direcdata;
    char entype;
    net_t net;
    interface_t interface;
    unsigned char chansta[FRENUM];
    unsigned char serclient;
}guide_t;

extern void request_pro(FCGX_Request *);
extern void deal_pro(int,short,void*);

#endif
