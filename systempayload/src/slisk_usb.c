/* Slisk Xbox 360 Wireless Receiver -> virtual DualShock 4.
 * Host process: GoldHEN system payload, NOT a game PRX.
 * Direct receiver access via Sony libSceUsbd. No network transport.
 * Virtual device routines supplied by PS4-verified GPL-3.0 Control4Free core.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include "xw_core.h"
#include "c4f_vda.h"
#include "c4f_log.h"

#define MAX_RADIOS 4
#define USB_TIMEOUT_MS 12
#define TEST_MAX_MS (20u*60u*1000u)
#define OWNER_SELECT_TIMEOUT_MS 30000u
#define USB_IDLE_SHUTDOWN_MS 120000u

/* libusb 1.0 USB descriptor ABI (used by libSceUsbd). */
typedef struct usb_device usb_device;
typedef struct usb_handle usb_handle;
typedef struct { uint8_t bLength,bDescriptorType; uint16_t bcdUSB; uint8_t bDeviceClass,bDeviceSubClass,bDeviceProtocol,bMaxPacketSize0; uint16_t idVendor,idProduct,bcdDevice; uint8_t iManufacturer,iProduct,iSerialNumber,bNumConfigurations; } UsbDevDesc;
typedef struct { uint8_t bLength,bDescriptorType,bEndpointAddress,bmAttributes; uint16_t wMaxPacketSize; uint8_t bInterval,bRefresh,bSynchAddress; const uint8_t *extra; int extra_length; } UsbEndDesc;
typedef struct { uint8_t bLength,bDescriptorType,bInterfaceNumber,bAlternateSetting,bNumEndpoints,bInterfaceClass,bInterfaceSubClass,bInterfaceProtocol,iInterface; const UsbEndDesc *endpoint; const uint8_t *extra; int extra_length; } UsbInterfaceDesc;
typedef struct { const UsbInterfaceDesc *altsetting; int num_altsetting; } UsbInterface;
typedef struct { uint8_t bLength,bDescriptorType; uint16_t wTotalLength; uint8_t bNumInterfaces,bConfigurationValue,iConfiguration,bmAttributes,MaxPower; const UsbInterface *interface; const uint8_t *extra; int extra_length; } UsbConfig;

typedef struct {
 void *library;
 int32_t (*init)(void);
 int32_t (*exit)(void);
 int32_t (*devices)(usb_device***);
 int32_t (*free_devices)(usb_device**);
 int32_t (*descriptor)(usb_device*,UsbDevDesc*);
 int32_t (*config)(usb_device*,UsbConfig**);
 int32_t (*free_config)(UsbConfig*);
 int32_t (*open)(usb_device*,usb_handle**);
 int32_t (*claim)(usb_handle*,int32_t);
 int32_t (*release)(usb_handle*,int32_t);
 int32_t (*transfer)(usb_handle*,unsigned char,unsigned char*,int32_t,int32_t*,uint32_t);
 void (*close)(usb_handle*);
} Usbd;
typedef struct { Usbd f; usb_handle *handle; unsigned intf,ep_in,ep_out; int claimed; } Receiver;

static void slisk_status(const char *stage){
 int fd=open("/data/slisk-v092-status.txt",O_WRONLY|O_CREAT|O_APPEND,0666);
 if(fd>=0){ssize_t nw=write(fd,stage,strlen(stage));if(nw>=0){nw=write(fd,"\n",1);}(void)nw;(void)close(fd);}
 c4fLog("SLISK STAGE: %s\n",stage);
}
static uint64_t now_ms(void){ struct timespec ts={0}; clock_gettime(CLOCK_MONOTONIC,&ts); return (uint64_t)ts.tv_sec*1000u + (uint64_t)ts.tv_nsec/1000000u; }
static int symbol(void *h,const char *name,void *out){void *p=dlsym(h,name); if(!p)return -1; memcpy(out,&p,sizeof(p));return 0;}
#define GETSYM(name,field) do{if(symbol(u->library,name,&u->field)){slisk_status("USB_SYMBOL_MISSING: " name);c4fNotify("Slisk: USB-funktion saknas, stoppar");return -1;}}while(0)
static int usbd_init(Usbd *u){
 static const char *paths[]={"/system/common/lib/libSceUsbd.sprx","libSceUsbd.sprx","libSceUsbd"};
 memset(u,0,sizeof(*u));
 for(size_t i=0;i<sizeof(paths)/sizeof(paths[0]);i++){u->library=dlopen(paths[i],RTLD_NOW|RTLD_GLOBAL);if(u->library)break;}
 if(!u->library){slisk_status("USB_LIBRARY_MISSING");c4fNotify("Slisk: libSceUsbd saknas, stoppar");return -1;}
 GETSYM("sceUsbdInit",init);GETSYM("sceUsbdExit",exit);
 GETSYM("sceUsbdGetDeviceList",devices); GETSYM("sceUsbdFreeDeviceList",free_devices);
 GETSYM("sceUsbdGetDeviceDescriptor",descriptor); GETSYM("sceUsbdGetActiveConfigDescriptor",config);
 GETSYM("sceUsbdFreeConfigDescriptor",free_config); GETSYM("sceUsbdOpen",open);
 GETSYM("sceUsbdClaimInterface",claim);GETSYM("sceUsbdReleaseInterface",release);
 GETSYM("sceUsbdInterruptTransfer",transfer);GETSYM("sceUsbdClose",close);
 return 0;
}
static void receiver_close(Receiver *r){if(r->handle){if(r->claimed)r->f.release(r->handle,(int32_t)r->intf);r->f.close(r->handle);}r->handle=NULL;r->claimed=0;}
static int receiver_open(Receiver *r){
 usb_device **list=NULL;int32_t n=r->f.devices(&list); if(n<=0||!list)return -1;
 for(int32_t i=0;i<n;i++){
  UsbDevDesc d={0}; UsbConfig *cfg=NULL;int found=0;
  if(r->f.descriptor(list[i],&d)!=0 || !xw_is_supported_receiver(d.idVendor,d.idProduct))continue;
  if(r->f.config(list[i],&cfg)!=0||!cfg)continue;
  if(!cfg->interface){r->f.free_config(cfg);continue;}
  for(unsigned j=0;j<cfg->bNumInterfaces&&!found;j++){
   const UsbInterface *ifc=&cfg->interface[j];
   if(!ifc->altsetting) continue;
   for(int k=0;k<ifc->num_altsetting&&!found;k++){
    const UsbInterfaceDesc *alt=&ifc->altsetting[k];unsigned in=0,out=0;
    if(!alt->endpoint)continue;
    if(alt->bAlternateSetting||alt->bInterfaceClass!=0xff||alt->bInterfaceSubClass!=0x5d||alt->bInterfaceProtocol!=0x81)continue;
    for(unsigned e=0;e<alt->bNumEndpoints;e++){
     const UsbEndDesc *ep=&alt->endpoint[e];if((ep->bmAttributes&3u)!=3u)continue;
     if(ep->bEndpointAddress&0x80u)in=ep->bEndpointAddress;else out=ep->bEndpointAddress;
    }
    if(in&&out){r->intf=alt->bInterfaceNumber;r->ep_in=in;r->ep_out=out;found=1;}
   }
  }
  r->f.free_config(cfg);
  if(!found)continue;
  if(r->f.open(list[i],&r->handle)==0&&r->handle){
   if(r->f.claim(r->handle,(int32_t)r->intf)==0){r->claimed=1;c4fLog("SLISK receiver claimed %04x:%04x interface=%u IN=%u OUT=%u\n",d.idVendor,d.idProduct,r->intf,r->ep_in,r->ep_out);break;}
   receiver_close(r);
  }
 }
 r->f.free_devices(list);
 if(!r->claimed)return -1;
 uint8_t inquiry[12]={0x08,0,0x0f,0xc0,0,0,0,0,0,0,0,0};int32_t sent=0;
 (void)r->f.transfer(r->handle,(uint8_t)r->ep_out,inquiry,sizeof(inquiry),&sent,100);
 return 0;
}
/* Wireless Xbox 360 player ring command, as documented by Linux xpad.
 * Player 2 uses command 7 (top-right steady). This sends on the already
 * claimed receiver OUT endpoint, and never changes the input path. */
static void receiver_set_player_two_led(Receiver *r){
 if(!r || !r->handle || !r->claimed || !r->ep_out)return;
 uint8_t packet[12]={0x00,0x00,0x08,0x47,0,0,0,0,0,0,0,0};
 int32_t sent=0;
 int32_t rc=r->f.transfer(r->handle,(uint8_t)r->ep_out,packet,
                          (int32_t)sizeof(packet),&sent,100);
 c4fLog("SLISK PLAYER2_LED rc=0x%08x sent=%d command=0x47\n",
        (unsigned)rc,(int)sent);
 if(rc==0 && sent==(int32_t)sizeof(packet))slisk_status("PLAYER2_LED_SENT");
 else slisk_status("PLAYER2_LED_WRITE_FAILED");
}
/* Xbox 360 wireless force feedback via receiver OUT endpoint.
 * Linux xpad uses these exact twelve bytes with two motor intensities. */
static void receiver_set_rumble(Receiver *r,uint8_t large,uint8_t small){
 if(!r || !r->handle || !r->claimed || !r->ep_out)return;
 uint8_t pkt[12]={0x00,0x01,0x0f,0xc0,0x00,large,small,0,0,0,0,0};
 int32_t sent=0;
 int32_t rc=r->f.transfer(r->handle,(uint8_t)r->ep_out,pkt,12,&sent,100);
 static unsigned messages=0;
 if(messages++<12)c4fLog("SLISK RUMBLE_WRITE rc=0x%08x sent=%d large=%u small=%u\n",
                          (unsigned)rc,(int)sent,(unsigned)large,(unsigned)small);
}
/* Diagnostic capture is bounded: 24 packets and 8 non-timeout errors.
 * A receiver being enumerated is not proof that controller input reaches us. */
static XWPacketResult receiver_poll(Receiver *r,XWReceiver *state,uint64_t ms){
 static unsigned samples=0, errors=0;
 uint8_t packet[64]={0};int32_t n=0;
 int32_t rc=r->f.transfer(r->handle,(uint8_t)r->ep_in,packet,sizeof(packet),&n,USB_TIMEOUT_MS);
 if(rc!=0){
  /* libusb-style timeout is -7. Record a few other failures only. */
  if(rc!=-7 && errors++<8) c4fLog("SLISK USB_TRANSFER_ERROR rc=%d n=%d ep=0x%02x\n",(int)rc,(int)n,r->ep_in);
  return XW_PACKET_IGNORED;
 }
 if(n<=0||(size_t)n>sizeof(packet)){
  if(errors++<8)c4fLog("SLISK USB_LENGTH_ERROR n=%d\n",(int)n);
  return XW_PACKET_IGNORED;
 }
 XWPacketResult result=xw_receive(state,0,packet,(size_t)n,ms);
 if(samples<24){
  char line[256];size_t pos=0;unsigned dump=(unsigned)n<32u?(unsigned)n:32u;
  pos+=(size_t)snprintf(line+pos,sizeof(line)-pos,"SLISK USB_PACKET len=%d result=%d hex=",(int)n,(int)result);
  for(unsigned i=0;i<dump && pos+4<sizeof(line);i++)
   pos+=(size_t)snprintf(line+pos,sizeof(line)-pos,"%02x",packet[i]);
  c4fLog("%s\n",line);
  samples++;
 }
 if(result==XW_PACKET_INPUT && (state->radios[0].last.buttons&XW_BTN_CROSS)) {
  static int seen_a=0;
  if(!seen_a++){c4fLog("SLISK USB_A_DECODED len=%d\n",(int)n);slisk_status("XBOX_A_RAW_INPUT_DECODED");}
 }
 return result;
}
static uint32_t ps4_buttons(uint32_t b){
 uint32_t out=0; const struct {uint32_t from,to;} m[]={
 {XW_BTN_CROSS,0x4000},{XW_BTN_CIRCLE,0x2000},{XW_BTN_SQUARE,0x8000},{XW_BTN_TRIANGLE,0x1000},
 {XW_BTN_L1,0x400},{XW_BTN_R1,0x800},{XW_BTN_L3,0x2},{XW_BTN_R3,0x4},
 {XW_BTN_OPTIONS,0x8},{XW_BTN_SHARE,0x1},{XW_BTN_UP,0x10},{XW_BTN_DOWN,0x40},
 {XW_BTN_LEFT,0x80},{XW_BTN_RIGHT,0x20},{XW_BTN_PS,0x10000}};
 for(size_t i=0;i<sizeof(m)/sizeof(m[0]);i++) { if(b&m[i].from)out|=m[i].to; }
 return out;
}
static uint32_t slisk_pre_owner_buttons(uint32_t buttons){
 return ps4_buttons(buttons & (XW_BTN_CROSS|XW_BTN_UP|XW_BTN_DOWN|
                              XW_BTN_LEFT|XW_BTN_RIGHT|XW_BTN_PS));
}
static void mapped(const XWPadFrame *f,ScePadData *d){
 c4fPadDataNeutral(d); if(!f)return;
 d->buttons=ps4_buttons(f->buttons); d->lx=f->left_x;d->ly=f->left_y;
 d->rx=f->right_x;d->ry=f->right_y;d->l2=f->l2;d->r2=f->r2;
 if(f->l2>32) d->buttons|=0x100u;
 if(f->r2>32) d->buttons|=0x200u;
}

/* A klog read must first show a marker written by this process. Never adopt an
 * old event from before device creation; abort on ambiguous multiple devices. */
typedef struct {int fd; char line[1024];size_t used;char marker[64];int seen_marker;uint64_t candidate;unsigned candidates;uint64_t first_candidate_ms;uint32_t owner;int owner_seen;} LogScan;
static void handle_line(LogScan *s){
 s->line[s->used]=0;
 /* Capture only relevant MBus lifecycle events, bounded to 32 lines.
  * No general kernel logging or account names. */
 if(s->seen_marker && (strstr(s->line,"DEVICE_OWNER_CHANGED") ||
                       strstr(s->line,"DEVICE_ADDED") ||
                       strstr(s->line,"DEVICE_REMOVED"))) {
  static unsigned trace_count=0;
  if(trace_count++<32)c4fLog("SLISK VDA_KLOG_EVENT %.220s\n",s->line);
 }
 if(strstr(s->line,s->marker))s->seen_marker=1;
 if(s->seen_marker&&c4fKlogIsVirtualAdd(s->line)){
  uint64_t id=c4fKlogDeviceId(s->line);
  if(id && (!s->candidate||s->candidate!=id)){
   if(!s->candidates)s->first_candidate_ms=now_ms();
   s->candidate=id;s->candidates++;
  }
 }
 if(s->candidate){
  unsigned long long id=0;unsigned owner=0;
  /* Locate the event anywhere within a decorated klog line. Accept extra
   * event fields between the two bracketed keys, but never another device. */
  const char *event=strstr(s->line,"DEVICE_OWNER_CHANGED");
  if(event){
   const char *device=strstr(event,"[DeviceId:0x");
   const char *user=strstr(event,"[UserId:0x");
   if(device && user && sscanf(device,"[DeviceId:0x%llx]",&id)==1 &&
      sscanf(user,"[UserId:0x%x]",&owner)==1 &&
      (uint64_t)id==s->candidate) {
    s->owner=owner;s->owner_seen=1;
   }
  }
 }
 s->used=0;
}
static void scan(LogScan *s){char b[1024];for(int r=0;r<6;r++){
 ssize_t n=read(s->fd,b,sizeof(b));if(n<=0)break;
 for(ssize_t i=0;i<n;i++){
  if(b[i]=='\n')handle_line(s);
  else if(b[i]!='\r'){if(s->used+1<sizeof(s->line))s->line[s->used++]=b[i];else s->used=0;}
 }
}}
static int verify_reader(LogScan *s){
 s->fd=c4fKlogOpenDevice();if(s->fd<0)return -1;
 c4fLogSetKlog(0); /* Avoid echoing our own file log into klog reader. */
 /* Flush historical log lines before setting a new marker. */
 for(unsigned i=0;i<20;i++){scan(s);usleep(12000);}
 snprintf(s->marker,sizeof(s->marker),"slisk-unique-%llu",(unsigned long long)now_ms());
 s->candidate=0;s->candidates=0;s->first_candidate_ms=0;s->seen_marker=0;
 c4fKlogMark(s->marker);
 uint64_t until=now_ms()+1600;
 while(now_ms()<until){scan(s);if(s->seen_marker)return 0;usleep(10000);}
 c4fLog("SLISK klog marker not received, refusing AddDevice\n");return -1;
}

static int slisk_owner_allowed(uint32_t owner,int32_t ds4){
 return owner>0xffu && owner!=0xffffffffu && (ds4<=0 || owner!=(uint32_t)ds4);
}
int slisk_usb_run(void){
 Receiver rx={0};XWReceiver state; C4fVirtualPad pad={0};LogScan log={0};
 int32_t before_users[4]={-1,-1,-1,-1};int32_t first_user=-1;
 uint64_t begun=now_ms(), last_open=0, deadline=0, last_frame=0, last_feedback=0;
 uint8_t last_rumble_large=0,last_rumble_small=0;int phase=0, missing_alerted=0;
 xw_init(&state);log.fd=-1;
 slisk_status("LOOP_ENTERED");if(usbd_init(&rx.f)!=0)return 1;slisk_status("USB_SYMBOLS_OK");int32_t usb_init_rc=rx.f.init();if(usb_init_rc!=0){slisk_status("USB_INIT_FAILED");c4fNotify("Slisk: sceUsbdInit misslyckades");return 3;}slisk_status("USB_INITIALIZED");c4fNotify("Slisk: USB initierat; söker mottagare");
 c4fLog("SLISK Xbox USB backend initialized, waiting for receiver\n");
 int login_rc=c4fLoginUsers(before_users);
 if(login_rc==0)for(int i=0;i<4;i++)if(before_users[i]>0){first_user=before_users[i];break;}
 if(login_rc!=0||first_user<=0){slisk_status("NO_DS4_OWNER");c4fNotify("Slisk: kan inte hitta DS4-anvandare, stoppar");receiver_close(&rx);if(rx.f.exit)(void)rx.f.exit();if(rx.f.library)dlclose(rx.f.library);return 2;}
 while(now_ms()-begun<TEST_MAX_MS){
  uint64_t now=now_ms();
  if(!rx.handle){if(!missing_alerted && now-begun>=15000){missing_alerted=1;slisk_status("USB_RECEIVER_NOT_FOUND_15S");c4fNotify("Slisk: ingen Xbox-mottagare efter 15 sekunder");}if(now-last_open>=1000){last_open=now;if(receiver_open(&rx)==0){slisk_status("USB_RECEIVER_READY");c4fNotify("Slisk: USB-mottagare hittad; paketdiagnostik aktiv, tryck A");}}usleep(25000);continue;}
  XWPacketResult res=receiver_poll(&rx,&state,now);
  if(res==XW_PACKET_CONNECTED)slisk_status("XBOX_CONTROLLER_CONNECTED");
  if(res==XW_PACKET_DISCONNECTED){
   static unsigned disconnect_notices=0;
   if(disconnect_notices++==0){
    c4fLog("SLISK Xbox radio disconnected or idle; continuing to poll receiver\n");
    slisk_status("XBOX_RADIO_DISCONNECTED_WAITING");
   }
   if(pad.owned){ScePadData neut;c4fPadDataNeutral(&neut);(void)c4fVirtualPadInsert(&pad,&neut);c4fVirtualPadRemove(&pad);}
   phase=0;
   continue;
  }
  XWPadFrame f=xw_frame_for_virtual(&state,0,now);
  /* Back+Start emergency cancellation works even before account ownership. */
  if((phase==2||phase==3) && res==XW_PACKET_INPUT &&
     (f.buttons&(XW_BTN_OPTIONS|XW_BTN_SHARE))==(XW_BTN_OPTIONS|XW_BTN_SHARE)){
   slisk_status("XBOX_MANUAL_CANCEL_CLEANUP");
   c4fNotify("Slisk: avbrutet med Back+Start");
   break;
  }
  if(!phase && res==XW_PACKET_INPUT && (f.buttons&XW_BTN_CROSS)){slisk_status("XBOX_A_RECEIVED");c4fNotify("Slisk: Xbox A registrerat, kontrollerar enhet");
   c4fLog("SLISK Xbox A pressed, starting verified native controller registration\n");
   if(verify_reader(&log)!=0){slisk_status("KLOG_VERIFY_FAILED");break;}
   /* AddDevice status may be nonzero even if device appears: only the MBus
      ID observed after our verified marker decides whether it exists. */
   int32_t rc=c4fVirtualPadAdd(C4F_VDA_USER_SELECT);
   c4fLog("SLISK AddDevice raw status 0x%08x\n",(unsigned)rc);
   log.candidate=0;log.candidates=0;log.first_candidate_ms=0;log.owner_seen=0;
   slisk_status("VDA_ADD_CALLED");phase=1;deadline=now+3000;
  }
  if(phase && log.fd>=0)scan(&log);
  if(phase==1){
   if(log.candidates>1){c4fLog("SLISK ambiguous MBus add events: aborting without adopting a device\n");break;}
   if(log.candidate&&log.candidates==1&&now>=log.first_candidate_ms+200){slisk_status("VDA_ID_SEEN");
    c4fVirtualPadAdopt(&pad,-1,C4F_VDA_USER_SELECT,log.candidate);
    c4fLog("SLISK virtual controller created id=0x%llx: select a different user/guest\n",(unsigned long long)pad.deviceId);
    /* No controller frames before the system confirms a distinct owner.
       Bound the user-selection experiment; never hold the prompt for 90s. */
    slisk_status("VDA_WAITING_FOR_OWNER");
    /* Diagnostic only: send ONE neutral frame before ownership is known.
       Never forward Xbox buttons until a distinct owner is confirmed. */
    { ScePadData neutral; c4fPadDataNeutral(&neutral);
      int32_t probe_rc=c4fVirtualPadInsert(&pad,&neutral);
      c4fLog("SLISK PRE_OWNER_NEUTRAL_INSERT_RC=0x%08x\n",(unsigned)probe_rc);
      if(probe_rc!=0){slisk_status("PRE_OWNER_NEUTRAL_REJECTED_CLEANUP");break;}
      slisk_status("PRE_OWNER_NEUTRAL_ACCEPTED_WAIT_OWNER"); }
    phase=2;deadline=now_ms()+OWNER_SELECT_TIMEOUT_MS;
   }else if(now>deadline){c4fLog("SLISK AddDevice was not confirmed within timeout\n");break;}
  }
  if(phase==2||phase==3){
   if(state.radios[0].has_input && now>=state.radios[0].input_time_ms && now-state.radios[0].input_time_ms>USB_IDLE_SHUTDOWN_MS){c4fLog("SLISK USB input silent 120s; stopping\n");break;}
   if(log.owner_seen && phase==2){static int owner_logged=0;if(!owner_logged++){c4fLog("SLISK VDA_OWNER_SEEN owner=0x%08x expected_not=0x%08x\n",(unsigned)log.owner,(unsigned)first_user);}}
   if(log.owner_seen&&log.owner!=0xffffffffu&&log.owner>0xffu){
    if(!slisk_owner_allowed(log.owner,first_user)){c4fLog("SLISK same user as DS4; choose guest, stopping\n");break;}
    if(phase==2){c4fLog("SLISK VIRTUAL OWNER assigned: 0x%08x\n",log.owner);phase=3;receiver_set_player_two_led(&rx);}
   }
   if(phase==2 && now>deadline){
    slisk_status("VDA_OWNER_TIMEOUT_CLEANUP");
    c4fNotify("Slisk: inget kontoval bekraftat; tar bort virtuell kontroll");
    c4fLog("SLISK virtual owner not confirmed in 30 seconds; removing device\n");break;
   }
   if(phase==3 && now-last_feedback>=100){
    last_feedback=now;
    C4fPadFeedback feedback={0};
    int32_t fb_rc=c4fVirtualPadFeedback(&pad,&feedback);
    static unsigned feedback_failures=0, feedback_successes=0;
    if(fb_rc==0){
     if(feedback_successes++==0)slisk_status("RUMBLE_FEEDBACK_AVAILABLE");
     if(feedback.large!=last_rumble_large || feedback.small!=last_rumble_small){
      receiver_set_rumble(&rx,feedback.large,feedback.small);
      last_rumble_large=feedback.large;last_rumble_small=feedback.small;
     }
    }else if(feedback_failures++<4)c4fLog("SLISK RUMBLE_FEEDBACK_ERROR rc=0x%08x\n",(unsigned)fb_rc);
   }
   if(now-last_frame>=16){
    ScePadData d;
    if(phase==3 && state.radios[0].has_input && now>=state.radios[0].input_time_ms && now-state.radios[0].input_time_ms<=2500)
       mapped(&f,&d);
    else c4fPadDataNeutral(&d);
    /* During login selection, only pass explicit navigation presses.
     * No sticks, triggers, shoulder buttons or game input until owner verified.
     * This is an experimental interaction path; the owner timeout remains active. */
    if(phase==2){
     c4fPadDataNeutral(&d);
     if(state.radios[0].has_input && now>=state.radios[0].input_time_ms &&
        now-state.radios[0].input_time_ms<=2500){
      d.buttons=slisk_pre_owner_buttons(f.buttons);
     }
     int32_t rc=c4fVirtualPadInsert(&pad,&d);
     static unsigned pre_owner_reports=0;
     if(pre_owner_reports<5)c4fLog("SLISK PRE_OWNER_NAV_INSERT rc=0x%08x buttons=0x%08x\n",(unsigned)rc,(unsigned)d.buttons);
     pre_owner_reports++;
     if(rc!=0){slisk_status("PRE_OWNER_NAV_REJECTED_CLEANUP");break;}
    }
    if(phase==3){
     int32_t rc=c4fVirtualPadInsert(&pad,&d);
     if(rc!=0){c4fLog("SLISK InsertData returned 0x%08x: neutralize and stop\n",(unsigned)rc);break;}
    }
    last_frame=now;
   }
  }
  if(phase==3 && res==XW_PACKET_INPUT && ((f.buttons&(XW_BTN_OPTIONS|XW_BTN_SHARE))==(XW_BTN_OPTIONS|XW_BTN_SHARE))){c4fLog("SLISK start+back requested shutdown\n");break;}
 }
 if(last_rumble_large||last_rumble_small)receiver_set_rumble(&rx,0,0);
 if(pad.owned){ScePadData d;c4fPadDataNeutral(&d);(void)c4fVirtualPadInsert(&pad,&d);c4fVirtualPadRemove(&pad);}
 if(log.fd>=0)close(log.fd);
 c4fLogSetKlog(1);
 receiver_close(&rx);
 if(rx.f.exit)(void)rx.f.exit();
 if(rx.f.library)dlclose(rx.f.library);
 c4fLog("SLISK native USB loop exited\n");return 0;
}
#ifdef SLISK_TEST
uint32_t slisk_test_pre_owner_buttons(uint32_t b){return slisk_pre_owner_buttons(b);}
int slisk_test_owner_event(const char *line,uint64_t candidate,uint32_t *out_owner){
 LogScan scan_state={0};
 scan_state.candidate=candidate;
 scan_state.seen_marker=1;
 size_t len=strlen(line);
 if(!out_owner || len>=sizeof(scan_state.line))return 0;
 memcpy(scan_state.line,line,len);
 scan_state.used=len;
 handle_line(&scan_state);
 if(!scan_state.owner_seen)return 0;
 *out_owner=scan_state.owner;
 return 1;
}
void slisk_test_map_frame(const XWPadFrame *f,ScePadData *out){mapped(f,out);}
int slisk_test_owner_allowed(uint32_t owner,int32_t active_ds4){return slisk_owner_allowed(owner,active_ds4);}
#endif
