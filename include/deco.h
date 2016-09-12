#ifndef DECO_H
#define DECO_H

#include<time.h>

#define OEM4HLEN    28          /* oem4 message header length (bytes) */
#define MAXRAWLEN   4096                /* max length of receiver raw message */
#define MAXINTER    100
#define MAXPIPE     120
#define PRNNUMBER   16
#define MAXSAT      75

#define SYS_GPS     0x01                /* navigation system: GPS */
#define SYS_SBS     0x02                /* navigation system: SBAS */
#define SYS_GLO     0x04                /* navigation system: GLONASS */
#define SYS_GAL     0x08                /* navigation system: Galileo */
#define SYS_QZS     0x10                /* navigation system: QZSS */
#define SYS_CMP     0x20                /* navigation system: BeiDou */
#define SYS_ALL     0xFF                /* navigation system: all */

#define PI          3.1415926535897932  /* pi */
#define FE_WGS84    (1.0/298.257223563) /* earth flattening (WGS84) */
#define RE_WGS84    6378137.0           /* earth semimajor axis (WGS84) (m) */

#define R2D         (180.0/PI)          /* rad to deg */

#define ID_HCNBOARD      32002
#define ID_HCNPIPE       32000
#define ID_BESTPOS       42
#define ID_SATVIS        48
#define ID_GPGSA         221
#define ID_DIREC         32003
#define ID_SAT           32001
#define ID_INTER         32007

#define FRENUM       14

typedef struct {        /* time struct */
    time_t time;        /* time (s) expressed by standard time_t */
    double sec;         /* fraction of second under 1 s */
} gtime_t;

typedef struct{
    unsigned char comid;
    unsigned char flag;
    unsigned short msgid;
    unsigned short period;
}inters_t;

typedef struct{
    unsigned char num;
    inters_t inters[MAXINTER];
    unsigned char retype;
    unsigned char comtype[3];
    unsigned char sytype;
    unsigned char sidotype;
    unsigned char dirtype;
    double        dirlen;
}inter_t;

typedef struct{
    unsigned char acpuload;
    unsigned char dcpuload;
    unsigned char astatus;
    unsigned char dstatus;
    unsigned char fstatus;
    unsigned char workstatus;
    unsigned char mainantnworksta;
    unsigned char mainantnrfsta;
    unsigned char aantnworksta;
    unsigned char aantnrfsta;
    unsigned char flaread;
    unsigned char flawrite;
#if 1
    unsigned char ip[4];
    unsigned char netmask[4];
    unsigned char gateway[4];
    unsigned char dns[4];
    unsigned char socktype;
    unsigned short port;
    unsigned char serverip[4];
#endif
} board_t;

typedef struct{
    unsigned char frenu;
    unsigned char codetype;
    unsigned char obsstate;
    unsigned char sigstate;
    unsigned char elestate;
    unsigned char ltag;
    unsigned char ptag;
    unsigned char glofreq;
} staword_t;

typedef struct{
    float D;
    double P;
    float pstd;
    float L;
    float lstd;
}pipeobs_t;

typedef struct{
    unsigned short chanid;
    unsigned char sateid;
    unsigned int tick;
    staword_t staword;
    pipeobs_t pipeobs;
    unsigned char cn0;
    float locktime;
} pipe_t;

typedef struct{
    unsigned short channum;
    pipe_t pipe[MAXPIPE];
} pipes_t;

typedef struct{
    unsigned char prn[PRNNUMBER];
    float pdop;
    float hdop;
    float vdop;
} gpgsa_t;

typedef struct{
    double lat;
    double lon;
    double hgt;
    float  postype;
    float diffage;
    float stnid;
} bestpos_t;

typedef struct{
    double elev;
    double az;
} satvis_t;

typedef struct{
    unsigned char qf;
    unsigned char satnum;
    double        pitch;
    double        roll;
    double        heading;
    double        baselat;
    double        baselon;
    double        basehgt;
    double        rovelat;
    double        rovelon;
    double        rovehgt;
    float         hdop;
} direc_t;

typedef struct{
   unsigned short svid;
   unsigned char satstatus;
   unsigned char sigtype;
   unsigned int  solcnt;
   float         elevation;
   float         azimuth;
   double        troerror;
   double        ionerror;
   double        poserror;
   float         cn[3];
}satprosat_t;

typedef struct{
   unsigned char pvtstatus;
   unsigned char solstacode;
   unsigned char velcode;
   unsigned char satnum;
   unsigned char solsatnum;
   float         age;
   unsigned char stnid[4];
   float         undulation;
   unsigned int  successcnt;
   double        x;
   double        y;
   double        z;
   double        vx;
   double        vy;
   double        vz;
   float         pdop;
   float         gdop;
   float         tdop;
   float         vdop;
   float         hdop;
   float         ecutoff;
   double        posrms;
   double        velrms;
   double        clkbias;
   double        clkdrift;
   unsigned short measnum;
   satprosat_t  satprosat[MAXSAT];
} satpro_t;

typedef struct{
    gtime_t time;
    int nbyte;
    int len;
    char msgtype[256];
    int outtype;
    unsigned char buff[MAXRAWLEN];
    board_t board;
    pipes_t pipes;
    bestpos_t bestpos;
    satvis_t satvis;
    gpgsa_t gpgsa;
    direc_t direc;
    satpro_t satpro;
    inter_t inter;
} hcnraw_t;

extern void ecef2pos(const double *r, double *pos);
extern gtime_t gpst2utc(gtime_t t);
extern void deg2dms(double deg, double *dms);
extern void time2epoch(gtime_t t,double *ep);
extern double time2gpst(gtime_t t, int *week);
extern int input_hcnoem4p(hcnraw_t *hcnraw,int fd,int *type);
extern double dot(const double *a, const double *b, int n);



#endif
