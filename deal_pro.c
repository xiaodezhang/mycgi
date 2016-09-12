#include"deco.h"
#include<stdio.h>
#include"pro.h"
#include<string.h>
#include"logger.h"

extern guide_t *guide;
extern int fd[2];

static void statesend(hcnraw_t *hcnraw){

    gtime_t utc;
    double timer[6];

    utc=gpst2utc(hcnraw->time);
    time2epoch(utc,timer);
    sprintf(guide->state.utctime,"%.0f-%.0f-%.0f-%.0f-%.0f-%.0f",
            timer[0],timer[1],timer[2],timer[3],timer[4],timer[5]);
    guide->state.antn.mainantnworksta = hcnraw->board.mainantnworksta;
    guide->state.antn.mainantnrfsta   = hcnraw->board.mainantnrfsta;
    guide->state.antn.aantnworksta    = hcnraw->board.aantnworksta;
    guide->state.antn.aantnrfsta      = hcnraw->board.aantnrfsta;
    guide->state.cpu.acpuload         = hcnraw->board.acpuload;
    guide->state.cpu.dcpuload         = hcnraw->board.dcpuload;
    guide->state.cpu.astatus          = hcnraw->board.astatus;
    guide->state.cpu.dstatus          = hcnraw->board.dstatus;
    guide->state.cpu.fstatus          = hcnraw->board.fstatus;
}

static void enginesend(hcnraw_t *hcnraw){

    guide->entype = hcnraw->board.workstatus;
}
static void ipsend(hcnraw_t *hcnraw){

    sprintf((char*)guide->net.ip,"%d.%d.%d.%d.%d",hcnraw->board.ip[0],
            hcnraw->board.ip[1],hcnraw->board.ip[2], hcnraw->board.ip[3],
            hcnraw->board.port);
    sprintf((char*)guide->net.gateway,"%d.%d.%d.%d",hcnraw->board.gateway[0],
            hcnraw->board.gateway[1],hcnraw->board.gateway[2],
            hcnraw->board.gateway[3]);
    sprintf((char*)guide->net.netmask,"%d.%d.%d.%d",hcnraw->board.netmask[0],
            hcnraw->board.netmask[1],hcnraw->board.netmask[2],
            hcnraw->board.netmask[3]);
    sprintf((char*)guide->net.dns,"%d.%d.%d.%d",hcnraw->board.dns[0],
            hcnraw->board.dns[1],hcnraw->board.dns[2],
            hcnraw->board.dns[3]);
    sprintf((char*)guide->net.serverip,"%d.%d.%d.%d",hcnraw->board.serverip[0],
            hcnraw->board.serverip[1],hcnraw->board.serverip[2],
            hcnraw->board.serverip[3]);

}

static void serclientsend(hcnraw_t *hcnraw){

    guide->serclient = hcnraw->board.socktype;
}

static void getpossend(hcnraw_t *hcnraw){

    double r[3],e[3];

    if(hcnraw->satpro.satnum == 0) {
        printf("demo:satnum==0\n");
        return;
    }
    r[0]=hcnraw->satpro.x;
    r[1]=hcnraw->satpro.y;
    r[2]=hcnraw->satpro.z;
    ecef2pos(r,e);
    guide->basepos.lat = e[0]*R2D;
    guide->basepos.lon = e[1]*R2D;
    guide->basepos.hgt = e[2];
    guide->basepos.ecutoff = hcnraw->satpro.ecutoff;

}
static void actionsend(hcnraw_t *hcnraw){

    int i;
    char sys[] = {
        0x01,0x04,0x20,0x20,0x01,0x04,0x20,0x01,0x04,
        0x08,0x02,0x02,0x08,0x08
    };

    if(hcnraw->satpro.satnum == 0) return;
    for(i = 0;i < 5;i++) guide->sateprn.satnu[i] = 0;
    for(i = 0;i < hcnraw->satpro.satnum;i++){
        switch(sys[hcnraw->satpro.satprosat[i].sigtype]){
            case SYS_GPS: guide->sateprn.gps[guide->sateprn.satnu[0]++]=
                          hcnraw->satpro.satprosat[i].svid;
                          break;
            case SYS_GLO: guide->sateprn.glo[guide->sateprn.satnu[1]++]=
                          hcnraw->satpro.satprosat[i].svid;
                          break;
            case SYS_GAL:guide->sateprn.gali[guide->sateprn.satnu[2]++]=
                         hcnraw->satpro.satprosat[i].svid;
                          break;
            case SYS_CMP: guide->sateprn.cmp[guide->sateprn.satnu[3]++]=
                          hcnraw->satpro.satprosat[i].svid;
                          break;
            case SYS_SBS: guide->sateprn.sbas[guide->sateprn.satnu[4]++]=
                          hcnraw->satpro.satprosat[i].svid;
                          break;
        }
    }
}
static void possend(hcnraw_t *hcnraw){

    int i;
    double r[3],e[3];
    char sys[] = {
        0x01,0x04,0x20,0x20,0x01,0x04,0x20,0x01,0x04,
        0x08,0x02,0x02,0x08,0x08
    };

      r[0]=hcnraw->satpro.x;
      r[1]=hcnraw->satpro.y;
      r[2]=hcnraw->satpro.z;
      ecef2pos(r,e);
      guide->posstate.lat = e[0]*R2D;
      guide->posstate.lon = e[1]*R2D;
      guide->posstate.hgt = e[2];
      guide->posstate.undulation = hcnraw->satpro.undulation;
      guide->posstate.postype = hcnraw->satpro.pvtstatus;
      guide->posstate.difftime= hcnraw->satpro.age;
      for(i = 0;i < 4;i++) guide->posstate.stnid[i] = hcnraw->satpro.stnid[i];
      guide->posstate.gpssec = (float)time2gpst(hcnraw->time,&guide->posstate.gpsweek);
      guide->posstate.pdop = hcnraw->satpro.pdop;
      guide->posstate.hdop = hcnraw->satpro.hdop;
      guide->posstate.vdop = hcnraw->satpro.vdop;
      guide->posstate.tdop = hcnraw->satpro.tdop;
      guide->posstate.solsatnu= hcnraw->satpro.solsatnum;
      for(i = 0;i < 5;i++) guide->posstate.sateprn.satnu[i] = 0;
      for(i = 0;i < hcnraw->satpro.satnum;i++){
          if(hcnraw->satpro.satprosat[i].satstatus != 0) continue;
          switch(sys[hcnraw->satpro.satprosat[i].sigtype]){
              case SYS_GPS: guide->posstate.sateprn.gps[guide->posstate.sateprn.satnu[0]++]= 
                            hcnraw->satpro.satprosat[i].svid;
                            break;
              case SYS_GLO: guide->posstate.sateprn.glo[guide->posstate.sateprn.satnu[1]++]=
                            hcnraw->satpro.satprosat[i].svid;
                            break;
              case SYS_GAL:guide->posstate.sateprn.gali[guide->posstate.sateprn.satnu[2]++]=
                            hcnraw->satpro.satprosat[i].svid;
                            break;
              case SYS_CMP: guide->posstate.sateprn.cmp[guide->posstate.sateprn.satnu[3]++]=
                            hcnraw->satpro.satprosat[i].svid;
                            break;
              case SYS_SBS: guide->posstate.sateprn.sbas[guide->posstate.sateprn.satnu[4]++]=
                            hcnraw->satpro.satprosat[i].svid;
                            break;
          }

      }
}
static void tracesend(hcnraw_t *hcnraw){

    int i,j;
    char sys[] = {
        0x01,0x04,0x20,0x20,0x01,0x04,0x20,0x01,0x04,
        0x08,0x02,0x02,0x08,0x08
    };

    if(hcnraw->satpro.satnum <= 0) return;
    
    guide->tracetable.ecutoff = hcnraw->satpro.ecutoff;
    for(i = 0;i < 5;i++) guide->tracetable.satnu[i] = 0;
    for(i = 0;i < hcnraw->satpro.satnum;i++){
        switch(sys[hcnraw->satpro.satprosat[i].sigtype]){
            case SYS_GPS: guide->tracetable.gps[guide->tracetable.satnu[0]].prn =
                              hcnraw->satpro.satprosat[i].svid;
                          guide->tracetable.gps[guide->tracetable.satnu[0]].elev =
                              hcnraw->satpro.satprosat[i].elevation;
                          guide->tracetable.gps[guide->tracetable.satnu[0]].az=
                              hcnraw->satpro.satprosat[i].azimuth;
                          guide->tracetable.gps[guide->tracetable.satnu[0]].satstatus =
                              hcnraw->satpro.satprosat[i].satstatus;

                          for(j = 0;j < 3;j++)
                             guide->tracetable.gps[guide->tracetable.satnu[0]].cn[j]=
                                 hcnraw->satpro.satprosat[i].cn[j];
                          guide->tracetable.satnu[0]++;
                          break;

            case SYS_GLO: guide->tracetable.glo[guide->tracetable.satnu[1]].prn =
                              hcnraw->satpro.satprosat[i].svid;
                          guide->tracetable.glo[guide->tracetable.satnu[1]].elev =
                              hcnraw->satpro.satprosat[i].elevation;
                          guide->tracetable.glo[guide->tracetable.satnu[1]].az=
                              hcnraw->satpro.satprosat[i].azimuth;
                          guide->tracetable.glo[guide->tracetable.satnu[1]].satstatus =
                              hcnraw->satpro.satprosat[i].satstatus;

                          for(j = 0;j < 3;j++)
                             guide->tracetable.glo[guide->tracetable.satnu[1]].cn[j]=
                                 hcnraw->satpro.satprosat[i].cn[j];
                          guide->tracetable.satnu[1]++;
                          break;

            case SYS_GAL: guide->tracetable.gali[guide->tracetable.satnu[2]].prn =
                              hcnraw->satpro.satprosat[i].svid;
                          guide->tracetable.gali[guide->tracetable.satnu[2]].elev =
                              hcnraw->satpro.satprosat[i].elevation;
                          guide->tracetable.gali[guide->tracetable.satnu[2]].az=
                              hcnraw->satpro.satprosat[i].azimuth;
                          guide->tracetable.gali[guide->tracetable.satnu[2]].satstatus =
                              hcnraw->satpro.satprosat[i].satstatus;

                          for(j = 0;j < 3;j++)
                             guide->tracetable.gali[guide->tracetable.satnu[2]].cn[j]=
                                 hcnraw->satpro.satprosat[i].cn[j];
                          guide->tracetable.satnu[2]++;
                          break;

            case SYS_CMP: guide->tracetable.cmp[guide->tracetable.satnu[3]].prn =
                             hcnraw->satpro.satprosat[i].svid;
                          guide->tracetable.cmp[guide->tracetable.satnu[3]].elev =
                              hcnraw->satpro.satprosat[i].elevation;
                          guide->tracetable.cmp[guide->tracetable.satnu[3]].az=
                              hcnraw->satpro.satprosat[i].azimuth;
                          guide->tracetable.cmp[guide->tracetable.satnu[3]].satstatus =
                              hcnraw->satpro.satprosat[i].satstatus;

                          for(j = 0;j < 3;j++)
                             guide->tracetable.cmp[guide->tracetable.satnu[3]].cn[j]=
                                 hcnraw->satpro.satprosat[i].cn[j];
                          guide->tracetable.satnu[3]++;
                          break;

            case SYS_SBS: guide->tracetable.sbas[guide->tracetable.satnu[4]].prn =
                              hcnraw->satpro.satprosat[i].svid;
                          guide->tracetable.sbas[guide->tracetable.satnu[4]].elev =
                              hcnraw->satpro.satprosat[i].elevation;
                          guide->tracetable.sbas[guide->tracetable.satnu[4]].az=
                              hcnraw->satpro.satprosat[i].azimuth;
                          guide->tracetable.sbas[guide->tracetable.satnu[4]].satstatus =
                              hcnraw->satpro.satprosat[i].satstatus;

                          for(j = 0;j < 3;j++)
                             guide->tracetable.sbas[guide->tracetable.satnu[4]].cn[j]=
                                 hcnraw->satpro.satprosat[i].cn[j];
                          guide->tracetable.satnu[4]++;
                          break;
        }
    }
}


static void rboard(hcnraw_t *hcnraw){

    statesend(hcnraw);
    enginesend(hcnraw);
    ipsend(hcnraw);
    serclientsend(hcnraw);
}

static void rinter(hcnraw_t *hcnraw){

    int i;

    guide->interface.num = hcnraw->inter.num;
    guide->interface.retype= hcnraw->inter.retype;
    for(i = 0;i < 3;i++)
        guide->interface.comtype[i]= hcnraw->inter.comtype[i];
    guide->interface.sytype= hcnraw->inter.sytype;
    guide->interface.sidotype= hcnraw->inter.sidotype;
    guide->interface.dirtype= hcnraw->inter.dirtype;
    guide->interface.dirlen= hcnraw->inter.dirlen;
    for(i = 0;i < guide->interface.num;i++){
        guide->interface.interfaces[i].comid = hcnraw->inter.inters[i].comid;
        guide->interface.interfaces[i].flag= hcnraw->inter.inters[i].flag;
        guide->interface.interfaces[i].msgid= hcnraw->inter.inters[i].msgid;
        guide->interface.interfaces[i].period= hcnraw->inter.inters[i].period;
    }
}

static void rsat(hcnraw_t *hcnraw){

    getpossend(hcnraw);
    actionsend(hcnraw);
    possend(hcnraw);
    tracesend(hcnraw);
}

static void rdirec(hcnraw_t *hcnraw){

    gtime_t utc;
    double timer[6];

    utc = gpst2utc(hcnraw->time);
    time2epoch(utc,timer);
    sprintf(guide->direcdata.utctime,"%.0f-%.0f-%.0f-%.0f-%.0f-%.0f",
              timer[0],timer[1],timer[2],timer[3],timer[4],timer[5]);
    guide->direcdata.mlat = hcnraw->direc.baselat;
    guide->direcdata.mlon = hcnraw->direc.baselon;
    guide->direcdata.mhgt = hcnraw->direc.basehgt;
    guide->direcdata.alat = hcnraw->direc.rovelat;
    guide->direcdata.alon = hcnraw->direc.rovelon;
    guide->direcdata.ahgt = hcnraw->direc.rovehgt;
    guide->direcdata.pitch = hcnraw->direc.pitch;
    guide->direcdata.heading = hcnraw->direc.heading;
    guide->direcdata.roll = hcnraw->direc.roll;
    guide->direcdata.qf = hcnraw->direc.qf;
    guide->direcdata.hdop= hcnraw->direc.hdop;
    guide->direcdata.satnum = hcnraw->direc.satnum;
}

static void rchansta(hcnraw_t *hcnraw){
    
    int i;

    memset(guide->chansta,0,FRENUM);
    for(i = 0;i < hcnraw->pipes.channum;i++){
        guide->chansta[(int)hcnraw->pipes.pipe[i].staword.frenu]
            = 1;
    }
}

extern void deal_pro(int sock,short event,void *arg){

    int res,type;
    hcnraw_t hcnraw = { { 0 } };

    if((res = input_hcnoem4p(&hcnraw,fd[1],&type)) > 0){
        switch(type){
            case ID_HCNBOARD: rboard(&hcnraw);   break;
            case ID_DIREC:    rdirec(&hcnraw);   break;
            case ID_SAT:      rsat(&hcnraw);     break;
            case ID_INTER:    rinter(&hcnraw);   break;
            case ID_HCNPIPE:  rchansta(&hcnraw); break;
        }
    }
}
