#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<deco.h>
#include<math.h>
#include<sys/time.h>
#include"logger.h"


#define OEM4SYNC1   0xAA        /* oem4 message start sync code 1 */
#define OEM4SYNC2   0x44        /* oem4 message start sync code 2 */
#define OEM4SYNC3   0x12        /* oem4 message start sync code 3 */

#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

const static double leaps[][7]={ /* leap seconds {y,m,d,h,m,s,utc-gpst,...} */
    {2012,7,1,0,0,0,-16},
    {2009,1,1,0,0,0,-15},
    {2006,1,1,0,0,0,-14},
    {1999,1,1,0,0,0,-13},
    {1997,7,1,0,0,0,-12},
    {1996,1,1,0,0,0,-11},
    {1994,7,1,0,0,0,-10},
    {1993,7,1,0,0,0, -9},
    {1992,7,1,0,0,0, -8},
    {1991,1,1,0,0,0, -7},
    {1990,1,1,0,0,0, -6},
    {1988,1,1,0,0,0, -5},
    {1985,7,1,0,0,0, -4},
    {1983,7,1,0,0,0, -3},
    {1982,7,1,0,0,0, -2},
    {1981,7,1,0,0,0, -1}
};


const static double gpst0[]={1980,1, 6,0,0,0}; /* gps time reference */

extern double timediff(gtime_t t1, gtime_t t2)
{
    return difftime(t1.time,t2.time)+t1.sec-t2.sec;
}

extern gtime_t timeadd(gtime_t t, double sec)
{
    double tt;
    
    t.sec+=sec; tt=floor(t.sec); t.time+=(int)tt; t.sec-=tt;
    return t;
}

extern void ecef2pos(const double *r, double *pos)
{
    double e2=FE_WGS84*(2.0-FE_WGS84),r2=dot(r,r,2),z,zk,v=RE_WGS84,sinp;
    
    for (z=r[2],zk=0.0;fabs(z-zk)>=1E-4;) {
        zk=z;
        sinp=z/sqrt(r2+z*z);
        v=RE_WGS84/sqrt(1.0-e2*sinp*sinp);
        z=r[2]+v*e2*sinp;
    }
    pos[0]=r2>1E-12?atan(z/sqrt(r2)):(r[2]>0.0?PI/2.0:-PI/2.0);
    pos[1]=r2>1E-12?atan2(r[1],r[0]):0.0;
    pos[2]=sqrt(r2+z*z)-v;
}

extern gtime_t epoch2time(const double *ep)
{
    const int doy[]={1,32,60,91,121,152,182,213,244,274,305,335};
    gtime_t time={0};
    int days,sec,year=(int)ep[0],mon=(int)ep[1],day=(int)ep[2];
    
    if (year<1970||2099<year||mon<1||12<mon) return time;
    
    /* leap year if year%4==0 in 1901-2099 */
    days=(year-1970)*365+(year-1969)/4+doy[mon-1]+day-2+(year%4==0&&mon>=3?1:0);
    sec=(int)floor(ep[5]);
    time.time=(time_t)days*86400+(int)ep[3]*3600+(int)ep[4]*60+sec;
    time.sec=ep[5]-sec;
    return time;
}

extern gtime_t gpst2utc(gtime_t t)
{
    gtime_t tu;
    int i;
    
    for (i=0;i<(int)sizeof(leaps)/(int)sizeof(*leaps);i++) {
        tu=timeadd(t,leaps[i][6]);
        if (timediff(tu,epoch2time(leaps[i]))>=0.0) return tu;
    }
    return t;
}

extern void deg2dms(double deg, double *dms)
{
    double sign=deg<0.0?-1.0:1.0,a=fabs(deg);
    dms[0]=floor(a); a=(a-dms[0])*60.0;
    dms[1]=floor(a); a=(a-dms[1])*60.0;
    dms[2]=a; dms[0]*=sign;
}

extern void time2epoch(gtime_t t, double *ep)
{
    const int mday[]={ /* # of days in a month */
        31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
        31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
    };
    int days,sec,mon,day;
    
    /* leap year if year%4==0 in 1901-2099 */
    days=(int)(t.time/86400);
    sec=(int)(t.time-(time_t)days*86400);
    for (day=days%1461,mon=0;mon<48;mon++) {
        if (day>=mday[mon]) day-=mday[mon]; else break;
    }
    ep[0]=1970+days/1461*4+mon/12; ep[1]=mon%12+1; ep[2]=day+1;
    ep[3]=sec/3600; ep[4]=sec%3600/60; ep[5]=sec%60+t.sec;
}

extern void time2str(gtime_t t, char *s, int n)
{
    double ep[6];
    
    if (n<0) n=0; else if (n>12) n=12;
    if (1.0-t.sec<0.5/pow(10.0,n)) {t.time++; t.sec=0.0;};
    time2epoch(t,ep);
    sprintf(s,"%04.0f/%02.0f/%02.0f %02.0f:%02.0f:%0*.*f",ep[0],ep[1],ep[2],
            ep[3],ep[4],n<=0?2:n+3,n<=0?0:n,ep[5]);
}

extern char *time_str(gtime_t t, int n)
{
    static char buff[64];
    time2str(t,buff,n);
    return buff;
}

static int exsign(unsigned int v, int bits)
{
    return (int)(v&(1<<(bits-1))?v|(~0u<<bits):v);
}


static double timeoffset_=0.0;        /* time offset (s) */

extern gtime_t timeget(void)
{
    double ep[6]={0};
#ifdef WIN32
    SYSTEMTIME ts;
    
    GetSystemTime(&ts); /* utc */
    ep[0]=ts.wYear; ep[1]=ts.wMonth;  ep[2]=ts.wDay;
    ep[3]=ts.wHour; ep[4]=ts.wMinute; ep[5]=ts.wSecond+ts.wMilliseconds*1E-3;
#else
    struct timeval tv;
    struct tm *tt;
    
    if (!gettimeofday(&tv,NULL)&&(tt=gmtime(&tv.tv_sec))) {
        ep[0]=tt->tm_year+1900; ep[1]=tt->tm_mon+1; ep[2]=tt->tm_mday;
        ep[3]=tt->tm_hour; ep[4]=tt->tm_min; ep[5]=tt->tm_sec+tv.tv_usec*1E-6;
    }
#endif
    return timeadd(epoch2time(ep),timeoffset_);
}


extern gtime_t utc2gpst(gtime_t t)
{
    int i;
    
    for (i=0;i<(int)sizeof(leaps)/(int)sizeof(*leaps);i++) {
        if (timediff(t,epoch2time(leaps[i]))>=0.0) return timeadd(t,-leaps[i][6]);
    }
    return t;
}

extern double time2gpst(gtime_t t, int *week)
{
    gtime_t t0=epoch2time(gpst0);
    time_t sec=t.time-t0.time;
    int w=(int)(sec/(86400*7));
    
    if (week) *week=w;
    return (double)(sec-w*86400*7)+t.sec;
}


extern gtime_t gpst2time(int week, double sec)
{
    gtime_t t=epoch2time(gpst0);
    
    if (sec<-1E9||1E9<sec) sec=0.0;
    t.time+=86400*7*week+(int)sec;
    t.sec=sec-(int)sec;
    return t;
}

extern int adjgpsweek(int week)
{
    int w;
    (void)time2gpst(utc2gpst(timeget()),&w);
    if (w<1560) w=1560; /* use 2009/12/1 if time is earlier than 2009/12/1 */
    return week+(w-week+512)/1024*1024;
}

extern unsigned int crc32(const unsigned char *buff, int len)
{
    unsigned int crc=0;
    int i,j;
    
    
    for (i=0;i<len;i++) {
        crc^=buff[i];
        for (j=0;j<8;j++) {
            if (crc&1) crc=(crc>>1)^POLYCRC32; else crc>>=1;
        }
    }
    return crc;
}

static int sync_oem4(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
    return buff[0]==OEM4SYNC1&&buff[1]==OEM4SYNC2&&buff[2]==OEM4SYNC3;
}

static int decode_hcnboard(hcnraw_t *hcnraw){

    unsigned char *p = hcnraw->buff;
    int i;

    if(hcnraw->len < 71){
        printf("board len < 71\n");
        return -1;
    };
    hcnraw->board.acpuload     = U1(p+28);
    hcnraw->board.dcpuload     = U1(p+29);
    hcnraw->board.astatus      = U1(p+30);
    hcnraw->board.dstatus      = U1(p+31);
    hcnraw->board.fstatus      = U1(p+32);
    hcnraw->board.workstatus   = U1(p+33);
    hcnraw->board.mainantnworksta= U1(p+34);
    hcnraw->board.mainantnrfsta  = U1(p+35);
    hcnraw->board.aantnworksta = U1(p+36);
    hcnraw->board.aantnrfsta   = U1(p+37);
    hcnraw->board.flaread      = U1(p+38);
    hcnraw->board.flawrite     = U1(p+39);
    
#if 1
    for(i = 0;i < 4;i++)
        hcnraw->board.ip[i] = U1(p+48+i);
    for(i = 0;i < 4;i++)
        hcnraw->board.netmask[i] = U1(p+52+i);
    for(i = 0;i < 4;i++)
        hcnraw->board.gateway[i] = U1(p+56+i);
    for(i = 0;i < 4;i++)
        hcnraw->board.dns[i] = U1(p+60+i);
    hcnraw->board.socktype = U1(p+64);
    hcnraw->board.port= U2(p+65);
    for(i = 0;i < 4;i++)
        hcnraw->board.serverip[i] = U1(p+67+i);

#if 0
    printf("ip:%d %d %d %d\n",hcnraw->board.ip[0],hcnraw->board.ip[1],
            hcnraw->board.ip[2],hcnraw->board.ip[3]);
    printf("netmsk:%d %d %d %d\n",hcnraw->board.netmask[0],hcnraw->board.netmask[1],
            hcnraw->board.netmask[2],hcnraw->board.netmask[3]);
    printf("gateway:%d %d %d %d\n",hcnraw->board.gateway[0],hcnraw->board.gateway[1],
            hcnraw->board.gateway[2],hcnraw->board.gateway[3]);
    printf("dns:%d %d %d %d\n",hcnraw->board.dns[0],hcnraw->board.dns[1],
            hcnraw->board.dns[2],hcnraw->board.dns[3]);
    printf("socktype:%d port:%d\n",hcnraw->board.socktype,hcnraw->board.port);
    printf("serverip:%d %d %d %d\n",hcnraw->board.serverip[0],hcnraw->board.serverip[1],
            hcnraw->board.serverip[2],hcnraw->board.serverip[3]);
#endif
#endif
#if 0
    printf("board:\n");
    printf("%d %d %d %d %d %d %d %d %d %d %d %d \n",hcnraw->board.acpuload,
            hcnraw->board.dcpuload,hcnraw->board.astatus,hcnraw->board.dstatus,
            hcnraw->board.fstatus,hcnraw->board.workstatus,hcnraw->board.mantnworksta,
            hcnraw->board.mantnrfsta,hcnraw->board.aantnworksta,hcnraw->board.aantnrfsta,
            hcnraw->board.flaread,hcnraw->board.flawrite
            );
#endif
    return 12;
}

static int decode_hcnpipe(hcnraw_t *hcnraw){

    unsigned char *p = hcnraw->buff;
    int i;

    hcnraw->pipes.channum = U2(p+28);

    if(hcnraw->len < hcnraw->pipes.channum*37+2) return -1;
#if 1
    for(i = 0;i < hcnraw->pipes.channum;i++){
        hcnraw->pipes.pipe[i].chanid = U2(p+30+i*37);
        hcnraw->pipes.pipe[i].sateid = U1(p+32+i*37);
        hcnraw->pipes.pipe[i].tick   = U4(p+33+i*37);
        hcnraw->pipes.pipe[i].staword.frenu = U1(p+37+i*37)&0x3F;
        hcnraw->pipes.pipe[i].staword.codetype = (U1(p+37+i*37)>>6)&0x3;
        hcnraw->pipes.pipe[i].staword.obsstate = U1(p+38+i*37)&0x7;
        hcnraw->pipes.pipe[i].staword.sigstate= (U1(p+38+i*37)>>3)&0x1F;
        hcnraw->pipes.pipe[i].staword.elestate= (U1(p+39+i*37))&0x3F;
        hcnraw->pipes.pipe[i].staword.ltag= (U1(p+39+i*37)>>6)&0x1;
        hcnraw->pipes.pipe[i].staword.ptag= (U1(p+39+i*37)>>7)&0x1;
        hcnraw->pipes.pipe[i].staword.glofreq= (U1(p+40+i*37))&0xFF;
        hcnraw->pipes.pipe[i].pipeobs.D= exsign(U4(p+41+i*37)&0xFFFFFFF,28)/256.0;
        hcnraw->pipes.pipe[i].pipeobs.P= (U4(p+44+i*37)>>4)/128.0+U1(p+48+i*36)*2097152.0;
        hcnraw->pipes.pipe[i].pipeobs.L= (I4(p+49+i*37))/2000.0+hcnraw->pipes.pipe[i].pipeobs.P;
        hcnraw->pipes.pipe[i].pipeobs.pstd = ((U1(p+53+i*37))&0xF)/128.0;
        hcnraw->pipes.pipe[i].pipeobs.lstd = (((U1(p+53+i*37))>> 4)&0xF)/256.0;
        hcnraw->pipes.pipe[i].cn0 = U1(p+54+i*37);
        hcnraw->pipes.pipe[i].locktime = R4(p+55+i*37);
#if 0
    printf("dop:%f psr:%f pstd:%f adr:%f lstd:%f\n",dop,psr,pstd,adr,lstd);
#endif
    }
#endif
#if 1
    i = 10;
#if 0
    printf("chanid:%d sateid:%d tick:%d frenu:%d codetype:%d\n",hcnraw->pipes.pipe[i].chanid,
            hcnraw->pipes.pipe[i].sateid,hcnraw->pipes.pipe[i].tick,hcnraw->pipes.pipe[i].staword.frenu,
            hcnraw->pipes.pipe[i].staword.codetype);
#endif
#if 0
    printf("obsstate:%d sigstate:%d elestate:%d ltag:%d ptag:%d glofreq:%d\n",hcnraw->pipes.pipe[i].staword.obsstate,
            hcnraw->pipes.pipe[i].staword.sigstate,hcnraw->pipes.pipe[i].staword.elestate,hcnraw->pipes.pipe[i].staword.ltag,
            hcnraw->pipes.pipe[i].staword.ptag,hcnraw->pipes.pipe[i].staword.glofreq);
#endif
#if 0
    printf("dop:%f psr:%f pstd:%f adr:%f lstd:%f\n",dop,psr,pstd,adr,lstd);
#endif
#if 0
    printf("cn0:%d locktime:%f\n",hcnraw->pipes.pipe[i].cn0,hcnraw->pipes.pipe[i].locktime);
#endif
#endif

#if 0
    printf("meslength:%d\n",U4(p+28));
    printf("channum:%d\n",hcnraw->pipes.channum);
#endif
    return hcnraw->pipes.channum*7+1;
}

static int decode_bestpos(hcnraw_t *hcnraw){

    unsigned char *p = hcnraw->buff+OEM4HLEN;

    if(hcnraw->len < OEM4HLEN+72) return -1;
    hcnraw->bestpos.postype = U4(p+4);
    hcnraw->bestpos.lat = R8(p+8);
    hcnraw->bestpos.lon = R8(p+16);
    hcnraw->bestpos.hgt = R8(p+24);
    hcnraw->bestpos.stnid = U4(p+52);
    hcnraw->bestpos.diffage = U4(p+56);
#if 0
    printf("%f  %f  %f  %f  %f  %f\n",hcnraw->bestpos.lat,hcnraw->bestpos.lon,hcnraw->bestpos.hgt,
            hcnraw->bestpos.stnid,hcnraw->bestpos.diffage,hcnraw->bestpos.postype);
#endif

    return 5;
}

static int decode_direc(hcnraw_t *hcnraw){

    unsigned char *p = hcnraw->buff;

    if(hcnraw->len < 154) return -1;
    hcnraw->direc.qf = U1(p+28);
    hcnraw->direc.satnum = U1(p+29);
    hcnraw->direc.pitch = R8(p+30);
    hcnraw->direc.roll= R8(p+38);
    hcnraw->direc.heading= R8(p+46);
    hcnraw->direc.baselat= R8(p+54);
    hcnraw->direc.baselon= R8(p+62);
    hcnraw->direc.basehgt= R8(p+70);
    hcnraw->direc.rovelat= R8(p+78);
    hcnraw->direc.rovelon= R8(p+86);
    hcnraw->direc.rovehgt= R8(p+94);
    hcnraw->direc.hdop= R4(p+102);

#if 0
    printf("direc:\n");
    printf("%d %d %f %f %f %f %f %f %f %f %f %f\n",hcnraw->direc.qf,hcnraw->direc.satnum,
            hcnraw->direc.pitch,hcnraw->direc.roll,hcnraw->direc.heading,hcnraw->direc.baselat,hcnraw->direc.baselon,
            hcnraw->direc.basehgt,hcnraw->direc.rovelat,hcnraw->direc.rovelon,hcnraw->direc.rovehgt,hcnraw->direc.hdop);
#endif
#if 0
    printf("lat:%f  lon:%f  hgt:%f\n",hcnraw->direc.baselat,hcnraw->direc.baselon,hcnraw->direc.basehgt);
#endif
    return 12;
}

static int decode_sat(hcnraw_t *hcnraw){

    unsigned char *p = hcnraw->buff;
    int i,j,k;

#if 0
    if(hcnraw->len < 207) return -1;
    printf("len:%d\n",hcnraw->len);
#endif
    hcnraw->satpro.pvtstatus = U1(p+28);
    hcnraw->satpro.solstacode= U1(p+29);
    hcnraw->satpro.velcode   = U1(p+30);
    j = hcnraw->satpro.satnum    = U1(p+31);
    hcnraw->satpro.solsatnum = U1(p+32);
    hcnraw->satpro.age       = R4(p+33);
    for(i = 0;i < 4;i++) hcnraw->satpro.stnid[0] = U1(p+37+i);
    hcnraw->satpro.undulation = R4(p+41);
    hcnraw->satpro.successcnt = U4(p+45);
    hcnraw->satpro.x          = R8(p+49);
    hcnraw->satpro.y          = R8(p+57);
    hcnraw->satpro.z          = R8(p+65);
    hcnraw->satpro.vx         = R8(p+73);
    hcnraw->satpro.vy         = R8(p+81);
    hcnraw->satpro.vz         = R8(p+89);
    hcnraw->satpro.pdop       = R4(p+97);
    hcnraw->satpro.gdop       = R4(p+101);
    hcnraw->satpro.tdop       = R4(p+105);
    hcnraw->satpro.vdop       = R4(p+109);
    hcnraw->satpro.hdop       = R4(p+113);
    hcnraw->satpro.ecutoff    = R4(p+117);
    hcnraw->satpro.posrms     = R8(p+121);
    hcnraw->satpro.velrms     = R8(p+129);
    hcnraw->satpro.clkbias    = R8(p+137);
    hcnraw->satpro.clkdrift   = R8(p+145);
    hcnraw->satpro.measnum    = U2(p+153);
    for(k = 0;k < j;k++){
        hcnraw->satpro.satprosat[k].svid       = U2(p+155+k*68);
        hcnraw->satpro.satprosat[k].satstatus  = U1(p+157+k*68);
        hcnraw->satpro.satprosat[k].sigtype    = U1(p+158+k*68);
        hcnraw->satpro.satprosat[k].solcnt     = U4(p+159+k*68);
        hcnraw->satpro.satprosat[k].elevation  = R4(p+163+k*68);
        hcnraw->satpro.satprosat[k].azimuth    = R4(p+167+k*68);
        hcnraw->satpro.satprosat[k].troerror   = R8(p+171+k*68);
        hcnraw->satpro.satprosat[k].ionerror   = R8(p+179+k*68);
        hcnraw->satpro.satprosat[k].poserror   = R8(p+187+k*68);
        for(i = 0;i < 3;i++) hcnraw->satpro.satprosat[k].cn[i] = R4(p+i*4+195+k*68);
    }
#if 0
    printf("satnum:%d\n",hcnraw->satpro.satnum);
#endif
#if 0
    printf("pvtstatus:%d solstacode:%d velcode:%dsatnum:%d age:%f stnid0:%d stnid1:%d stnid2:%d stnid3:%d\n",hcnraw->satpro.pvtstatus,raw->satpro.solstacode,raw->satpro.velcode,
             hcnraw->satpro.satnum,raw->satpro.age,raw->satpro.stnid[0],raw->satpro.stnid[1],
             hcnraw->satpro.stnid[2],raw->satpro.stnid[3]);
#endif
#if 0
    printf("solsatnum:%d undulation:%d successcnt:%d pdop:%f gdop:%f tdop:%f vdop:%f hdop:%f\n",
            hcnraw->satpro.solsatnum,raw->satpro.undulation,raw->satpro.successcnt,
            hcnraw->satpro.pdop,raw->satpro.gdop,raw->satpro.tdop,raw->satpro.vdop);
#endif
#if 0
    printf("vx:%f vy:%f vz:%f\n",hcnraw->satpro.vx,raw->satpro.vy,raw->satpro.vz);
#endif
#if 0
    printf("x:%f y:%f z:%f\n",hcnraw->satpro.x,raw->satpro.y,raw->satpro.z);
#endif
#if 0
    printf("ecutoff:%f posrms:%f velrms:%f clkbias:%f clkdrift:%f measnum:%d\n",hcnraw->satpro.ecutoff,
            hcnraw->satpro.posrms,raw->satpro.velrms,raw->satpro.clkbias,raw->satpro.clkdrift,
            hcnraw->satpro.measnum);
#endif
#if 0
#if 0
    for(k = 0;k < j;k++){
#endif
        k = 8;
        printf("svid:%d satstatus:%d sigtype:%d solcnt:%d elevation:%f azimuth:%f troerror:%f ionerror:%f poserror:%f\n",
                hcnraw->satpro.satprosat[k].svid,raw->satpro.satprosat[k].satstatus,raw->satpro.satprosat[k].sigtype,
                hcnraw->satpro.satprosat[k].solcnt,raw->satpro.satprosat[k].elevation,raw->satpro.satprosat[k].azimuth,
                hcnraw->satpro.satprosat[k].troerror,raw->satpro.satprosat[k].ionerror,raw->satpro.satprosat[k].poserror);
#if 0
    }
#endif
#endif
    return 41;
}
static int decode_satvis(hcnraw_t *hcnraw){

    unsigned char *p = hcnraw->buff+OEM4HLEN;

    if(hcnraw->len < OEM4HLEN+44) return -1;
    hcnraw->satvis.elev = R8(p+20);
    hcnraw->satvis.az    = R8(p+28);
#if 0
    printf("%f  %f\n",hcnraw->satvis.elev,hcnraw->satvis.az);
#endif
    return 2;
}

static int decode_inter(hcnraw_t* hcnraw){

    unsigned char *p = hcnraw->buff;
    int i;

    hcnraw->inter.num = U1(p+28);
    for(i = 0;i < hcnraw->inter.num;i++){
        hcnraw->inter.inters[i].comid = U1(p+29+i*6);
        hcnraw->inter.inters[i].flag= U1(p+30+i*6);
        hcnraw->inter.inters[i].msgid= U2(p+31+i*6);
        hcnraw->inter.inters[i].period= U2(p+33+i*6);
    }
    hcnraw->inter.retype = U1(p+29+i*6*hcnraw->inter.num);
    for(i = 0;i < 3;i++){
        hcnraw->inter.comtype[i]= U1(p+30+6*hcnraw->inter.num+i);
    }
    hcnraw->inter.sytype = U1(p+33+6*hcnraw->inter.num);
    hcnraw->inter.sidotype= U1(p+34+6*hcnraw->inter.num);
    hcnraw->inter.dirtype= U1(p+35+6*hcnraw->inter.num);
    hcnraw->inter.dirlen= R8(p+36+6*hcnraw->inter.num);
    return 20;
}

static int decode_hcnoem4(hcnraw_t *hcnraw,int *type)
{
    double tow;
    int msg,week;
    *type=U2(hcnraw->buff+4);
    
    /* check crc32 */
    if (crc32(hcnraw->buff,hcnraw->len)!=U4(hcnraw->buff+hcnraw->len)) {
        myerror("oem4 crc error: type=%3d len=%d\n");
        return -1;
    }

    msg =(U1(hcnraw->buff+6)>>4)&0x3;
    week=adjgpsweek(U2(hcnraw->buff+14));
    tow =U4(hcnraw->buff+16)*0.001;
    hcnraw->time=gpst2time(week,tow);
    
    if (hcnraw->outtype) {
        sprintf(hcnraw->msgtype,"OEM4 %4d (%4d): msg=%d %s",*type,hcnraw->len,msg,
                time_str(gpst2time(week,tow),2));
    }
    if (msg!=0) return 0; /* message type: 0=binary,1=ascii */
    
    switch (*type) {
        case ID_HCNBOARD      : return decode_hcnboard       (hcnraw);
        case ID_HCNPIPE       : return decode_hcnpipe        (hcnraw);
        case ID_BESTPOS       : return decode_bestpos        (hcnraw);
        case ID_SATVIS        : return decode_satvis         (hcnraw);
        case ID_DIREC         : return decode_direc          (hcnraw);
        case ID_SAT           : return decode_sat            (hcnraw);
        case ID_INTER         : return decode_inter          (hcnraw);
    }
    return 0;
}
extern double dot(const double *a, const double *b, int n)
{
    double c=0.0;
    
    while (--n>=0) c+=a[n]*b[n];
    return c;
}

extern int input_hcnoem4p(hcnraw_t *hcnraw,int fd,int *type)
{
    int i,res;
    unsigned char data;
    /* synchronize frame */
    if (hcnraw->nbyte==0) {
        for (i=0;;i++) {
            if ((res=read(fd,&data,1)) != 1) return -2;
            if (sync_oem4(hcnraw->buff,data)) break;
            if (i>=4096) return 0;
        }
    }
    if (read(fd,hcnraw->buff+3,7)<1) return -2;
    hcnraw->nbyte=10;
    
    if ((hcnraw->len=U2(hcnraw->buff+8)+OEM4HLEN)>MAXRAWLEN-4) {
        hcnraw->nbyte=0;
        return -1;
    }
    if (read(fd,hcnraw->buff+10,hcnraw->len-6)<1) return -2;
    hcnraw->nbyte=0;
    
    /* decode oem4 message */
    return decode_hcnoem4(hcnraw,type);
}

