#include"pro.h"
#include<string.h>
#include<unistd.h>
#include"parson.h"
#include"logger.h"
#include"url_parser.h"
#include"deco.h"
#include<stdio.h>

#define HTTP_CONENT_HEAD "Content-type: text/html\r\n\r\n"
#define JFILE "/web/sbin/msgid"
#define JPASSWORD "/web/sbin/password"

#define COMNUM  3
#define CMDLEN  100
#define DATANUM 10
extern guide_t *guide;
extern int fd[2];

static int sendcmd(int cn){

    int res;
    char *cmd[] = {
        "\r\nlog oemstatus ontime 1\r\n","\r\nlog satinfo ontime 1\r\n",
        "\r\nlog interfaceinfo ontime 1\r\n",
        "\r\nlog gputab ontime 1\r\n",
        "\r\nlog channelinfo ontime 1\r\n"

    };
    if((res = write(fd[0],cmd[cn],strlen(cmd[cn]))) == -1){
        return 0;
    }

    return res;
}

static char *state(){

    JSON_Value *val;
    JSON_Object *object;
    char *response;

    if(!sendcmd(0)){
        myerror("send cmd error\n");
        return NULL;
    }
    val = json_value_init_object();
    object = json_value_get_object( val );
    if ( !object ){
        json_value_free( val );
        myerror( "json_value_get_object error\n" );
        return NULL;
    }
    json_object_set_string(object,"utc",guide->state.utctime);
    json_object_set_number(object,"mainantnworksta",guide->state.antn.mainantnworksta);
    json_object_set_number(object,"mainantnrfsta",guide->state.antn.mainantnrfsta);
    json_object_set_number(object,"aantnworksta",guide->state.antn.aantnworksta);
    json_object_set_number(object,"aantnrfstan",guide->state.antn.aantnrfsta);
    json_object_set_number(object,"acpuload",guide->state.cpu.acpuload);
    json_object_set_number(object,"dcpuload",guide->state.cpu.dcpuload);
    json_object_set_number(object,"astatus",guide->state.cpu.astatus);
    json_object_set_number(object,"dstatus",guide->state.cpu.dstatus);
    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *action(){

    JSON_Value *val;
    JSON_Object *object;
    JSON_Array *sysarr[5];
    char *response;
    int i;
    char *sys[] = {
        "gps_prn","glo_prn","gali_prn","bd_prn","sbas_prn"
    },*sysnum[] = {
        "gps_num","glo_num","gali_num","bd_num","sbas_num"
    };

    if(!sendcmd(1)){
        myerror("send cmd error\n");
        return NULL;
    }
    val = json_value_init_object();
    object = json_value_get_object( val );
    if ( !object ){
       json_value_free( val );
       myerror( "json_value_get_object error\n" );
       return NULL;
    }
    for(i = 0;i < 5;i++){
        json_object_set_number(object,sysnum[i],guide->sateprn.satnu[i]);
        if(guide->sateprn.satnu[i] > 0){
           json_object_set_value(object,sys[i], json_value_init_array());
           sysarr[i] = json_object_get_array(object,sys[i]);
        }
    }


    if(guide->sateprn.satnu[0] > 0)
        for(int i = 0; i < guide->sateprn.satnu[0]; i++){
              json_array_append_number(sysarr[0],guide->sateprn.gps[i]);
    }
    
    if(guide->sateprn.satnu[1] > 0) 
        for(int i = 0; i < guide->sateprn.satnu[1]; i++){
              json_array_append_number(sysarr[1],guide->sateprn.glo[i]);
        }

    if(guide->sateprn.satnu[2] > 0)
        for(int i = 0; i < guide->sateprn.satnu[2]; i++){
              json_array_append_number(sysarr[2],guide->sateprn.gali[i]);
        }
    if(guide->sateprn.satnu[3] > 0)
        for(int i = 0; i < guide->sateprn.satnu[3]; i++){
              json_array_append_number(sysarr[3],guide->sateprn.cmp[i]);

        }
    if(guide->sateprn.satnu[4] > 0)
        for(int i = 0; i < guide->sateprn.satnu[4]; i++){
              json_array_append_number(sysarr[4],guide->sateprn.sbas[i]);
    }

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *position(){

    JSON_Value *val;
    JSON_Object *object;
    JSON_Array *sysarr[5];
    char *response;
    char *type[] ={
        "invalid","single","diff","invalid","rtk",
        "rtk_float","invalid","invalid","invalid",
        "sbas_single","ssr_float","ssr","ssr_single",
        "sbas","sbas_float", "ppp_single"
    },*sysprn[] = {
        "gps_prn","glo_prn","gali_prn",
        "bd_prn","sbas_prn"
    },*sysnum[] = {
        "gps_num","glo_num","gali_num","bd_num","sbas_num"
    };
    int i;

    if(!sendcmd(1)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    object = json_value_get_object( val );
    if ( !object ){
        json_value_free( val );
        myerror( "json_value_get_object error\n" );
        return NULL;
    }
    

    json_object_set_number(object,"lat",guide->posstate.lat);
    json_object_set_number(object,"lon",guide->posstate.lon);
    json_object_set_number(object,"high",guide->posstate.hgt);
    json_object_set_number(object,"undulation",guide->posstate.undulation);
    json_object_set_string(object,"postype",type[guide->posstate.postype]);
    json_object_set_number(object,"difftime",guide->posstate.difftime);
    json_object_set_string(object,"baseid",guide->posstate.stnid);
    json_object_set_number(object,"gpsweek",guide->posstate.gpsweek);
    json_object_set_number(object,"gpssec",guide->posstate.gpssec);
    json_object_set_number(object,"pdop",guide->posstate.pdop);
    json_object_set_number(object,"hdop",guide->posstate.hdop);
    json_object_set_number(object,"vdop",guide->posstate.vdop);
    json_object_set_number(object,"tdop",guide->posstate.tdop);
    json_object_set_number(object,"solution",guide->posstate.solsatnu);

    json_object_set_number(object,"gps_num",guide->posstate.sateprn.satnu[0]);
    json_object_set_number(object,"glo_num",guide->posstate.sateprn.satnu[1]);
    json_object_set_number(object,"gali_num",guide->posstate.sateprn.satnu[2]);
    json_object_set_number(object,"bd_num",guide->posstate.sateprn.satnu[3]);
    json_object_set_number(object,"sbas_num",guide->posstate.sateprn.satnu[4]);

    for(i = 0;i < 5;i++){
        json_object_set_number(object,sysnum[i],guide->posstate.sateprn.satnu[i]);
        if(guide->posstate.sateprn.satnu[i] > 0)
        json_object_set_value(object,sysprn[i],json_value_init_array());
        sysarr[i]= json_object_get_array(object,sysprn[i]);
    }

    if(guide->posstate.sateprn.satnu[0] > 0)
        for(int i = 0; i < guide->posstate.sateprn.satnu[0]; i++){
              json_array_append_number(sysarr[0],guide->posstate.sateprn.gps[i]);
    }
    
    if(guide->posstate.sateprn.satnu[1] > 0) 
        for(int i = 0; i < guide->posstate.sateprn.satnu[1]; i++){
              json_array_append_number(sysarr[1],guide->posstate.sateprn.glo[i]);
        }

    if(guide->posstate.sateprn.satnu[2] > 0)
        for(int i = 0; i < guide->posstate.sateprn.satnu[2]; i++){
              json_array_append_number(sysarr[2],guide->posstate.sateprn.gali[i]);
        }
    if(guide->posstate.sateprn.satnu[3] > 0)
        for(int i = 0; i < guide->posstate.sateprn.satnu[3]; i++){
              json_array_append_number(sysarr[3],guide->posstate.sateprn.cmp[i]);

        }
    if(guide->posstate.sateprn.satnu[4] > 0)
        for(int i = 0; i < guide->posstate.sateprn.satnu[4]; i++){
              json_array_append_number(sysarr[4],guide->posstate.sateprn.sbas[i]);
    }

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}


static char *tracetable(){

    JSON_Value *val;
    JSON_Object *object ;
    JSON_Array *gps_array;
    JSON_Array *glo_array;
    JSON_Array *gali_array;
    JSON_Array *bd_array;
    JSON_Array *sbas_array;
    char *response;

    JSON_Value *val_loop;
    JSON_Object *object_loop;

    if(!sendcmd(1)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    object = json_value_get_object( val );
    if ( !object ){
        json_value_free( val );
        myerror( "json_value_get_object error\n" );
        return NULL;
    }

    json_object_set_number(object,"gps_num",guide->tracetable.satnu[0]);
    json_object_set_number(object,"glo_num",guide->tracetable.satnu[1]);
    json_object_set_number(object,"gali_num",guide->tracetable.satnu[2]);
    json_object_set_number(object,"bd_num",guide->tracetable.satnu[3]);
    json_object_set_number(object,"sbas_num",guide->tracetable.satnu[4]);
    json_object_set_number(object,"ecutoff",guide->tracetable.ecutoff);

    json_object_set_value(object, "gps_sat", json_value_init_array());
    json_object_set_value(object, "glo_sat", json_value_init_array());
    json_object_set_value(object, "gali_sat", json_value_init_array());
    json_object_set_value(object, "bd_sat", json_value_init_array());
    json_object_set_value(object, "sbas_sat", json_value_init_array());

    gps_array = json_object_get_array(object, "gps_sat");
    glo_array = json_object_get_array(object, "glo_sat");
    gali_array = json_object_get_array(object, "gali_sat");
    bd_array = json_object_get_array(object, "bd_sat");
    sbas_array = json_object_get_array(object, "sbas_sat");

    for(int i = 0; i < guide->tracetable.satnu[0]; i++){
          val_loop = json_value_init_object();
          object_loop = json_value_get_object(val_loop);
          
          json_object_set_number(object_loop,"prn",guide->tracetable.gps[i].prn);
          json_object_set_number(object_loop,"elevation",guide->tracetable.gps[i].elev);
          json_object_set_number(object_loop,"azimuth",guide->tracetable.gps[i].az);
          json_object_set_number(object_loop,"satstatus",guide->tracetable.gps[i].satstatus);
          json_object_set_number(object_loop,"L1",guide->tracetable.gps[i].cn[0]);
          json_object_set_number(object_loop,"L2",guide->tracetable.gps[i].cn[1]);
          json_object_set_number(object_loop,"L5",guide->tracetable.gps[i].cn[2]);
          json_array_append_value(gps_array,val_loop);
    }
    for(int i = 0; i < guide->tracetable.satnu[1]; i++){
          val_loop = json_value_init_object();
          object_loop = json_value_get_object(val_loop);
          json_object_set_number(object_loop,"prn",guide->tracetable.glo[i].prn);
          json_object_set_number(object_loop,"elevation",guide->tracetable.glo[i].elev);
          json_object_set_number(object_loop,"azimuth",guide->tracetable.glo[i].az);
          json_object_set_number(object_loop,"satstatus",guide->tracetable.glo[i].satstatus);
          json_object_set_number(object_loop,"L1",guide->tracetable.glo[i].cn[0]);
          json_object_set_number(object_loop,"L2",guide->tracetable.glo[i].cn[1]);
          json_object_set_number(object_loop,"L5",guide->tracetable.glo[i].cn[2]);
          json_array_append_value(glo_array,val_loop);
    }
    for(int i = 0; i < guide->tracetable.satnu[2]; i++){
          val_loop = json_value_init_object();
          object_loop = json_value_get_object(val_loop);

          json_object_set_number(object_loop,"prn",guide->tracetable.gali[i].prn);
          json_object_set_number(object_loop,"elevation",guide->tracetable.gali[i].elev);
          json_object_set_number(object_loop,"azimuth",guide->tracetable.gali[i].az);
          json_object_set_number(object_loop,"satstatus",guide->tracetable.gali[i].satstatus);
          json_object_set_number(object_loop,"L1",guide->tracetable.gali[i].cn[0]);
          json_object_set_number(object_loop,"L2",guide->tracetable.gali[i].cn[1]);
          json_object_set_number(object_loop,"L5",guide->tracetable.gali[i].cn[2]);
          json_array_append_value(gali_array,val_loop);
    }
    for(int i = 0; i < guide->tracetable.satnu[3]; i++){
          val_loop = json_value_init_object();
          object_loop = json_value_get_object(val_loop);

          json_object_set_number(object_loop,"prn",guide->tracetable.cmp[i].prn);
          json_object_set_number(object_loop,"elevation",guide->tracetable.cmp[i].elev);
          json_object_set_number(object_loop,"azimuth",guide->tracetable.cmp[i].az);
          json_object_set_number(object_loop,"satstatus",guide->tracetable.cmp[i].satstatus);
          json_object_set_number(object_loop,"L1",guide->tracetable.cmp[i].cn[0]);
          json_object_set_number(object_loop,"L2",guide->tracetable.cmp[i].cn[1]);
          json_object_set_number(object_loop,"L5",guide->tracetable.cmp[i].cn[2]);
          json_array_append_value(bd_array,val_loop);
    }

    for(int i = 0; i < guide->tracetable.satnu[4]; i++){
          val_loop = json_value_init_object();
          object_loop = json_value_get_object(val_loop);

          json_object_set_number(object_loop,"prn",guide->tracetable.sbas[i].prn);
          json_object_set_number(object_loop,"elevation",guide->tracetable.sbas[i].elev);
          json_object_set_number(object_loop,"azimuth",guide->tracetable.sbas[i].az);
          json_object_set_number(object_loop,"satstatus",guide->tracetable.sbas[i].satstatus);
          json_object_set_number(object_loop,"L1",guide->tracetable.sbas[i].cn[0]);
          json_object_set_number(object_loop,"L2",guide->tracetable.sbas[i].cn[1]);
          json_object_set_number(object_loop,"L5",guide->tracetable.sbas[i].cn[2]);
          json_array_append_value(sbas_array,val_loop);
    }

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *getpos(){

    JSON_Value *val;
    JSON_Object *object;
    char *response;
    double latdms[3],londms[3];

    if(!sendcmd(1)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    object = json_value_get_object( val );
    if ( !object ){
        json_value_free( val );
        myerror( "json_value_get_object error\n" );
        return NULL;
    }

    deg2dms(guide->basepos.lat,latdms);
    deg2dms(guide->basepos.lon,londms);

    json_object_set_number(object,"hgt",guide->basepos.hgt);
    json_object_set_number(object,"latdeg",latdms[0]);
    json_object_set_number(object,"latmin",latdms[1]);
    json_object_set_number(object,"latsec",latdms[2]);
    json_object_set_number(object,"londeg",londms[0]);
    json_object_set_number(object,"lonmin",londms[1]);
    json_object_set_number(object,"lonsec",latdms[2]);

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *getpassword(){

    JSON_Value * jv;
    char *response;

    jv = json_parse_file(JPASSWORD);
    response = json_serialize_to_string(jv);
    return response;
}

static char *getdir(){

    JSON_Value *val;
    JSON_Object *ob;
    char *response;

    if(!sendcmd(3)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    ob = json_value_get_object(val);
    if(!ob){
        json_value_free(val);
        myerror("json value get object error\n");
        return NULL;
    }
    json_object_set_number(ob,"mlat",guide->direcdata.mlat);
    json_object_set_number(ob,"mlon",guide->direcdata.mlon);
    json_object_set_number(ob,"mhgt",guide->direcdata.mhgt);
    json_object_set_number(ob,"alat",guide->direcdata.alat);
    json_object_set_number(ob,"alon",guide->direcdata.alon);
    json_object_set_number(ob,"ahgt",guide->direcdata.ahgt);
    json_object_set_number(ob,"pitch",guide->direcdata.pitch);
    json_object_set_number(ob,"heading",guide->direcdata.heading);
    json_object_set_number(ob,"roll",guide->direcdata.roll);
    json_object_set_number(ob,"qf",guide->direcdata.qf);
    json_object_set_number(ob,"hdop",guide->direcdata.hdop);
    json_object_set_number(ob,"satnum",guide->direcdata.satnum);
    json_object_set_string(ob,"utctime",guide->direcdata.utctime);
    
    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *getengine(){

    JSON_Value *val;
    JSON_Object *ob;
    char *response;

    if(!sendcmd(0)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    ob = json_value_get_object(val);
    if(!ob){
        json_value_free(val);
        myerror("json value get object error\n");
        return NULL;
    }
    json_object_set_number(ob,"engine",guide->entype);

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *getip(){

    JSON_Value *val;
    JSON_Object *ob;
    char *response,*p;

    if(!sendcmd(2)){
        myerror("send cmd error\n");
        return NULL;
    }
    if(!sendcmd(0)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    ob = json_value_get_object(val);
    if(!ob){
        json_value_free(val);
        myerror("json value get object error\n");
        return NULL;
    }
    if(!(p = strrchr((char*)guide->net.ip,'.')))
        return NULL;
    *p = '\0';
    json_object_set_string(ob,"ip",(char*)guide->net.ip);
    json_object_set_string(ob,"port",p+1);
    json_object_set_string(ob,"gateway",(char*)guide->net.gateway);
    json_object_set_string(ob,"netmask",(char*)guide->net.netmask);
    json_object_set_string(ob,"dns",(char*)guide->net.dns);
    json_object_set_string(ob,"serverip",(char*)guide->net.serverip);
    json_object_set_number(ob,"serclient",guide->serclient);

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;
}

static char *getinter(){

    JSON_Value *val,*val_loop,*jv;
    JSON_Object *ob,*ob_loop,*jo;
    JSON_Array *interfaces;
    char *response,msgid[20];
    const char *jmsgid;
    int i;

    if(!sendcmd(0)){
        myerror("send cmd error\n");
        return NULL;
    }
    jv = json_parse_file(JFILE);
    val = json_value_init_object();
    ob = json_value_get_object(val);
    jo = json_value_get_object(jv);
    if(!ob || !jo){
        json_value_free(val);
        json_value_free(jv);
        myerror("json value get object error\n");
        return NULL;
    }

    json_object_set_value(ob,"interfaces",json_value_init_array());
    interfaces = json_object_get_array(ob,"interfaces");
    json_object_set_number(ob,"num",guide->interface.num);
    json_object_set_number(ob,"retype",guide->interface.retype);
    json_object_set_number(ob,"com1",guide->interface.comtype[0]);
    json_object_set_number(ob,"com2",guide->interface.comtype[1]);
    json_object_set_number(ob,"com3",guide->interface.comtype[2]);
    json_object_set_number(ob,"sytype",guide->interface.sytype);
    json_object_set_number(ob,"sidotype",guide->interface.sidotype);
    json_object_set_number(ob,"dirtype",guide->interface.dirtype);
    json_object_set_number(ob,"dirlen",guide->interface.dirlen);
    for(i = 0;i < guide->interface.num;i++){
        val_loop = json_value_init_object();
        ob_loop = json_value_get_object(val_loop);
        sprintf(msgid,"%d",guide->interface.interfaces[i].msgid);
        jmsgid = json_object_get_string(jo,msgid);
        if(!jmsgid){
            myerror("json object get string error\n");
            continue;
        }
        json_object_set_number(ob_loop,"comid",guide->interface.interfaces[i].comid);
        json_object_set_number(ob_loop,"flag",guide->interface.interfaces[i].flag);
        json_object_set_string(ob_loop,"msgid",jmsgid);
        json_object_set_number(ob_loop,"period",guide->interface.interfaces[i].period);
        json_array_append_value(interfaces,val_loop);
    }

    response = json_serialize_to_string(val);
    json_value_free(val);
    json_value_free(jv);

    return response;
}

static char *getchansta(){

    JSON_Value *val;
    JSON_Object *ob;
    char *response;

    if(!sendcmd(4)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    ob = json_value_get_object(val);
    if(!ob){
        json_value_free(val);
        myerror("json value get object error\n");
        return NULL;
    }
    json_object_set_number(ob,"L1",guide->chansta[0]);
    json_object_set_number(ob,"L2",guide->chansta[4]);
    json_object_set_number(ob,"L5",guide->chansta[7]);
    json_object_set_number(ob,"B1",guide->chansta[2]);
    json_object_set_number(ob,"B2",guide->chansta[6]);
    json_object_set_number(ob,"B3",guide->chansta[3]);
    json_object_set_number(ob,"GL1",guide->chansta[1]);
    json_object_set_number(ob,"GL2",guide->chansta[5]);

    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;

}

static char *getserclient(){

    JSON_Value *val;
    JSON_Object *ob;
    char *response;

    if(!sendcmd(0)){
        myerror("send cmd error\n");
        return NULL;
    }

    val = json_value_init_object();
    ob = json_value_get_object(val);
    if(!ob){
        json_value_free(val);
        myerror("json value get object error\n");
        return NULL;
    }
    json_object_set_number(ob,"serclient",guide->serclient);
    response = json_serialize_to_string(val);
    json_value_free(val);
    return response;

}

static void pro_guide(FCGX_Request *request){

    URL_BUF_STRUCT url_buf = { { 0 } };
    char *querys,*ps,*response;
    char *guidetype[]={
      "state", "action", "position",
      "tracetable", "getpos", "getpassword",
      "getdir", "getengine", "getip",
      "getinter","getchansta","getserclient"
    };
    int i;

    if(!(querys = FCGX_GetParam("QUERY_STRING",request->envp))){
        myerror("fcgx query string error\n");
        return;
    }
    url_parse(&url_buf,querys);
    if(!(ps = get_param_value(&url_buf,"guide"))){
        myerror("get param error\n");
        return;
    }
    for(i = 0;i < sizeof(guidetype)/sizeof(guidetype[0]);i++){
        if(strcmp(ps,guidetype[i]) == 0){
            switch(i){
               case 0:  response = state();       break;
               case 1:  response = action();      break;
               case 2:  response = position();    break;
               case 3:  response = tracetable();  break;
               case 4:  response = getpos();      break;
               case 5:  response = getpassword(); break;
               case 6:  response = getdir();      break;
               case 7:  response = getengine();   break;
               case 8:  response = getip();       break;
               case 9:  response = getinter();    break;
               case 10: response = getchansta();  break;
               case 11: response = getserclient();  break;
            }
        }
    }
    if(!response){
        myerror("json_serialize to strng error\n");
        return;
    }

    FCGX_FPrintF( request->out,
                  HTTP_CONENT_HEAD
                  "%s", response);
    free(response);
}

static void setrov(){

    char *cmd = "\r\nfix none\r\n";

    if(write(fd[0],cmd,strlen(cmd)) == -1)
        myerror("write error\n");
}

static void setbasepos(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*lat,*lon,
         *hgt;

    if(!(lat = get_param_value(url,"lat")))
        return;
    if(!(lon= get_param_value(url,"lon")))
        return;
    if(!(hgt= get_param_value(url,"hgt")))
        return;

    sprintf(cmd,"\r\nfix position %s %s %s\r\n",lat,lon,hgt);
    if(write(fd[0],cmd,strlen(cmd)) == -1)
        myerror("write error\n");
}

static void engineset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*entype[] ={
        "single","rtk","sbasppp","sbasspp",
        "ssrppp","ssrspp"
    },*eng;
    int engi;

    if(!(eng = get_param_value(url,"eng")))
        return;
    engi = atoi(eng);
    sprintf(cmd,"\r\nsetpvt %s\r\n",entype[engi]);
    if(write(fd[0],cmd,strlen(cmd)) == -1)
        myerror("write error\n");
}

static void eleset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*ele;

    if(!(ele = get_param_value(url,"ele")))
      return;
    sprintf(cmd,"\r\necutoff %s\r\n",ele);
    if(write(fd[0],cmd,strlen(cmd)) == -1)
        myerror("write error\n");

}

static void satset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*s,*p,sys[5],
         prn[5][MAXPRN],*systype[] = {
              "gps","gln","gal","bds","sbas"
         };
    int i,j;

    for(i = 0,s = p = url->buf;(p = get_param_array(s,"sys",
                       url->len -= (p-s)));s=p,i++){
        sys[i] = atoi(p);
    }
    for(i = 0,s = p = url->buf;(p = get_param_array(s,"gpsprn",
                       url->len -= (p-s)));s=p,i++){
        prn[0][i] = atoi(p);
    }
    for(i = 0,s = p = url->buf;(p = get_param_array(s,"gloprn",
                       url->len -= (p-s)));s=p,i++){
        prn[1][i] = atoi(p);
    }
    for(i = 0,s = p = url->buf;(p = get_param_array(s,"bdsprn",
                       url->len -= (p-s)));s=p,i++){
        prn[3][i] = atoi(p);
    }
    for(i = 0,s = p = url->buf;(p = get_param_array(s,"sbasprn",
                       url->len -= (p-s)));s=p,i++){
        prn[4][i] = atoi(p);
    }

    for(j = 0;j < 5;++j){
        for(i = 0;i < MAXPRN;++i){
            if(prn[j][i] != 0){
                s = prn[j][i] == 1 ? "lockout" : "unlockout";
                sprintf(cmd,"%s %s %d\r\n",s,systype[j],i);
                if(( write(fd[0],cmd,strlen(cmd))) == -1){
                    myerror("write error\n");
                     return;
                }
            }
        }
    }

    for(i = 0;i < 5;i++){
        if(sys[i] != 0){
            s = sys[i] == 1 ? "lockout" : "unlockout";
            sprintf(cmd,"\r\n%s %s\r\n",s,systype[i]);
            if(( write(fd[0],cmd,strlen(cmd))) == -1){
                myerror("write error\n");
                 return;
            }
        }
    }

}

static void channelset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*s,*p,closefre[8],
         *channel,chsta,*fre[8] = {
            "L1","L2","L5","B1","B2","B3","GL1","GL2"
         };
    int i;

   for(i = 0,s = p = url->buf;(p = get_param_array(s,"closefre",
                    url->len -= (p-s)));s=p,i++){
        closefre[i] = atoi(p);
    }
    if(!(channel =  get_param_value(url,"channel"))) 
        return;
    if(!(s = get_param_value(url,"chsta"))) 
        return;
    chsta = atoi(s);

    for(i = 0;i < 8;i++)
        if(closefre[i] != 0){
            s = closefre[i] == 1 ? "close" : "unclose";
            sprintf(cmd,"%s %s\r\n",s,fre[i]);
            if((write(fd[0],cmd,strlen(cmd))) == -1){
                myerror("write error\n");
                 return;
            }
        }
    if(chsta != 0){
        s = chsta == 1 ? "close" : "unclose";
        sprintf(cmd,"%s %s\r\n",s,channel);
        if((write(fd[0],cmd,strlen(cmd))) == -1){
             myerror("write error");
             return;
        }
    }
}

static void netset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*ip,*gateway,*netmask,
         *dns,*np[4],*p;
    int i;

    if(!(ip= get_param_value(url,"ip"))) 
        return;
    if(!(gateway= get_param_value(url,"gateway")))
        return;
    if(!(netmask= get_param_value(url,"netmask"))) 
        return;
    if(!(dns = get_param_value(url,"dns"))) 
        return;

    np[0] = ip; np[1] = gateway; 
    np[2] = netmask; np[3] = dns;

    for(i = 0;i < 4;i++)
        for(p = np[i];*p;p++)
            if(*p == '.')
                *p = ' ';

    sprintf(cmd,"\r\nsetip %s %s %s %s\r\nsaveconfig\r\n",ip,gateway,netmask,dns);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
         myerror("write error");
         return;
    }
}

static void dirset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*type,
         *mode[] = {
           "snb","swb","dnb","dwb"
         },basebool,*s;
    int typei;
    float baseline;

    if(!(type= get_param_value(url,"type"))) 
        return;
    typei = atoi(type);
    if(!(s = get_param_value(url,"baseline"))) 
        return;
    baseline = atof(s);

    basebool = baseline == 0 ? 0 : 1;
    if(basebool)
        sprintf(cmd,"\r\nset %s %f\r\n",mode[typei+basebool],baseline);
    else        
        sprintf(cmd,"\r\nset %s\r\n",mode[typei+basebool]);

    if((write(fd[0],cmd,strlen(cmd))) == -1){
         myerror("write error");
         return;
    }
}

static void dirsyset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*type,
         *mode[] = { "dynamic","static" };
    int typei;

    if(!(type = get_param_value(url,"type")))
        return;
    typei = atoi(type);
    sprintf(cmd,"\r\nsetpvt %s\r\n",mode[typei]);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
        myerror("write error\n");
        return;
    }
}

static void antnset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*s,*p,com[COMNUM];
    int i;

 
    for(i = 0,s = p = url->buf;(p = get_param_array(s,"com",
                   url->len -= (p-s)));s=p,i++){
        com[i] = atoi(p);
    }
    for(i = 0;i < COMNUM;i++){
        s = com[i] == 1 ? "antn1":"antn2";
        sprintf(cmd,"set com%d %s\r\n",i+1,s);
        if(( write(fd[0],cmd,strlen(cmd))) == -1){
             myerror("write error");
             return;
        }
    }
}

static void ioset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],clientbool,*ip,*port,
         *com,*datatype,
         *s,*p,*upraw[4][DATANUM] = {
            { "rangeb","gpsephemb","bdsephemb","gloephemb"},
            { "rtcm1033b","rtcm1005b","rtcm1006b","rtcm1004b",
                "rtcm1104b","rtcm1012b","rtcm1019b","rtcm1020b",
                "rtcm4011b" },
            { "rtcm1033b","rtcm1005b","rtcm1006b","rtcm1074b",
                "rtcm1124b","rtcm1084b","rtcm1019b","rtcm1020b",
                "rtcm4011b" },
            { "gpgga","gpgsv","gprmc","gpgrs","gpgsa",
                "gputa","gpgll","gpzda","gpvtg","gpgst"}
         };
    int typenum[] = { 4,9,9,10 },i,datatypei;
    float datatime[DATANUM];

    if(!(s = get_param_value(url,"clientbool")))
        return;
    clientbool = atoi(s);
    if(!(ip= get_param_value(url,"ip")))
        return;
    if(!(port = get_param_value(url,"port")))
        return;
    if(!(com = get_param_value(url,"com")))
        return;
    if(!(datatype = get_param_value(url,"datatype")))
        return;
    datatypei = atoi(datatype);
    for(i = 0,s = p = url->buf;(p = get_param_array(s,"datatime",
                    url->len -= (p-s)));s=p,i++){
        datatime[i] = atof(p);
    }

    for(p = ip;*p;p++)
        if(*p == '.')
            *p = ' ';

    if(clientbool != 0)
        sprintf(cmd,"setclient %s %s\r\n",ip,port);
    else
        sprintf(cmd,"setserver %s\r\n",port);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
      myerror("write error");
      return;
    }

    for(i = 0;i < 10;i++){
        if(i >= typenum[datatypei]) break;
        if(datatime[i] == -1)
            sprintf(cmd,"log com%s ranged onchanged\r\n",com);
        else if(datatime[i] == 0)
           sprintf(cmd,"unlog com%s %s\r\n",com,upraw[datatypei][i]);
        else
           sprintf(cmd,"log com%s %s ontime %f\r\n",com,upraw[datatypei][i], datatime[i]);
       if((write(fd[0],cmd,strlen(cmd))) == -1){
          myerror("write error");
          return;
       }

    }
}

static void setpassword(URL_BUF_STRUCT *url){

    char *account,*password;
    JSON_Value *jv;
    JSON_Object *jo;

    if(!(account= get_param_value(url,"account"))) 
        return;
    if(!(password= get_param_value(url,"password"))) 
        return;

    jv = json_value_init_object();
    if(!jv){
        myerror("json_init failed\n");
        return;
    }
    jo = json_value_get_object(jv);
    json_object_set_string(jo,"account",account);
    json_object_set_string(jo,"password",password);
    if(json_serialize_to_file(jv,JPASSWORD) == JSONFailure){
        myerror("json serialize to file failed\n");
        json_value_free(jv);
        return;
    }
    json_value_free(jv);
}

static void guideserial(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*port,*databit,*baud,
         *check,*stop,*baudtype[] = {
            "921600","460800","115200","38400",
            "19200","9600","4800","2400",
            "1200","300"
         };
    int baudi;

    if(!(port= get_param_value(url,"port")))
        return;
    if(!(baud= get_param_value(url,"baud")))
        return;
    baudi = atoi(baud);
    if(!(databit= get_param_value(url,"databit")))
        return;
    if(!(check= get_param_value(url,"check"))) 
        return;
    if(!(stop= get_param_value(url,"stop"))) 
        return;
    sprintf(cmd,"com com%s %s %s %s %s\r\n",port,baudtype[baudi],check,databit,stop);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
      myerror("write error");
      return;
    }
}

static void guidenet(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],clientbool,*ip,*port,
         *p;

    if(!(p = get_param_value(url,"clientbool"))) 
        return;
    clientbool = atoi(p);
    if(!(ip= get_param_value(url,"ip")))
        return;
    if(!(port= get_param_value(url,"port"))) 
        return;

    for(p = ip;*p;p++)
        if(*p == '.')
            *p = ' ';

    if(clientbool != 0)
        sprintf(cmd,"setclient %s %s\r\n",ip,port);
    else
        sprintf(cmd,"setserver %s\r\n",port);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
      myerror("write error");
      return;
    }
}

static void output(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],rtkout,*com,*p;

    if(!(p = get_param_value(url,"rtkout"))) 
        return;
    rtkout = atoi(p);
    if(!(com= get_param_value(url,"com")))
        return;
    if(!rtkout)
        sprintf(cmd,"\r\nlog com%s gpgga ontime 1\r\n",com);
    else
        sprintf(cmd,"\r\nlog com%s gputa ontime 1\r\n",com);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
      myerror("write error");
      return;
    }

}

static void rebootset(URL_BUF_STRUCT *url){

    char cmd[CMDLEN],*type,
         *mode[] = { "reboot","freset" };
    int typei;

    if(!(type = get_param_value(url,"type")))
        return;
    typei = atoi(type);
    sprintf(cmd,"\r\n%s\r\n",mode[typei]);
    if((write(fd[0],cmd,strlen(cmd))) == -1){
        myerror("write error\n");
        return;
    }
}

static void pro_config(FCGX_Request *request){

    URL_BUF_STRUCT url_buf = { { 0 } };
    char *querys,*ps;
    char *configtype[]={
        "setrov","setbasepos",
        "engineset","eleset","satset",
        "channelset","netset","dirset",
        "antnset","ioset","setpassword",
        "guideserial","guidenet","output",
        "dirsyset","rebootset"
    };
    int i;

    if(!(querys = FCGX_GetParam("QUERY_STRING",request->envp))){
        myerror("fcgx query string error\n");
        return;
    }
    url_parse(&url_buf,querys);
    if(!(ps = get_param_value(&url_buf,"config"))){
        myerror("get param error\n");
        return;
    }
    for(i = 0;i < sizeof(configtype)/sizeof(configtype[0]);i++){
        if(strcmp(ps,configtype[i]) == 0){
            switch(i){
               case 0:  setrov();              break;
               case 1:  setbasepos(&url_buf);  break;
               case 2:  engineset(&url_buf);   break;
               case 3:  eleset(&url_buf);      break;
               case 4:  satset(&url_buf);      break;
               case 5:  channelset(&url_buf);  break;
               case 6:  netset(&url_buf);      break;
               case 7:  dirset(&url_buf);      break;
               case 8:  antnset(&url_buf);     break;
               case 9:  ioset(&url_buf);       break;
               case 10: setpassword(&url_buf); break;
               case 11: guideserial(&url_buf); break;
               case 12: guidenet(&url_buf);    break;
               case 13: output(&url_buf);      break;
               case 14: dirsyset(&url_buf);    break;
               case 15: rebootset(&url_buf);    break;
            }
        }
    }
}

extern void web_pro(FCGX_Request *request){

    char *scname;

    if(!(scname = FCGX_GetParam("SCRIPT_NAME",request->envp))){
        myerror("fcgx getparam error\n");
        return;
    }
    if(strcmp(scname,"/get_receiver_guide.cmd") == 0)
        pro_guide(request);
    else if(strcmp(scname,"/get_receiver_config.cmd") == 0)
        pro_config(request);
    else
        myerror("script name error\n");
}


