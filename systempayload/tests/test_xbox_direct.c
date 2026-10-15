#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "xw_core.h"
#include "c4f_vda.h"
void slisk_test_map_frame(const XWPadFrame*,ScePadData*);
int slisk_test_owner_allowed(uint32_t,int32_t);
int slisk_test_owner_event(const char*,uint64_t,uint32_t*);
uint32_t slisk_test_pre_owner_buttons(uint32_t);
void c4fLog(const char*fmt,...){(void)fmt;}
void c4fNotify(const char*fmt,...){(void)fmt;}
void c4fLogSetKlog(int enabled){(void)enabled;}
int c4fKlogOpenDevice(void){return -1;}
void c4fKlogMark(const char*s){(void)s;}
int c4fKlogIsVirtualAdd(const char*s){(void)s;return 0;}
uint64_t c4fKlogDeviceId(const char*s){(void)s;return 0;}
int32_t c4fVirtualPadAdd(int32_t id){(void)id;return -1;}
void c4fVirtualPadAdopt(C4fVirtualPad*p,int32_t a,int32_t b,uint64_t id){(void)a;(void)b;(void)id;p->owned=1;}
int32_t c4fVirtualPadInsert(const C4fVirtualPad*p,const ScePadData*f){(void)p;(void)f;return 0;}
void c4fVirtualPadRemove(C4fVirtualPad*p){p->owned=0;}
int32_t c4fVirtualPadFeedback(const C4fVirtualPad*p,C4fPadFeedback*f){(void)p;memset(f,0,sizeof(*f));return 0;}
void c4fPadDataNeutral(ScePadData*d){memset(d,0,sizeof(*d));d->lx=d->ly=d->rx=d->ry=128;d->quat[3]=1;}
int32_t c4fLoginUsers(int32_t ids[4]){for(int i=0;i<4;i++)ids[i]=-1;return 0;}
int main(void){
 XWReceiver r;ScePadData d;XWPadFrame f;
 const uint8_t press[]={0,1,0xf0,0x00,0x00,0x13,0x00,0x10,0x00,0x00,0x0c,0xf1,0xfd,0x0d,0xba,0xfb,0xd8,0x02};
 const uint8_t release[]={0,1,0xf0,0x00,0x00,0x13,0,0,0,0,0x0c,0xf1,0xfd,0x0d,0xba,0xfb,0xd8,0x02};
 xw_init(&r);
 assert(xw_receive(&r,0,press,sizeof(press),100)==XW_PACKET_INPUT);
 f=xw_frame_for_virtual(&r,0,100);
 assert(f.connected==1 && (f.buttons&XW_BTN_CROSS));
 slisk_test_map_frame(&f,&d);assert(d.buttons==0x4000);
 assert(xw_receive(&r,0,release,sizeof(release),200)==XW_PACKET_INPUT);
 f=xw_frame_for_virtual(&r,0,200);slisk_test_map_frame(&f,&d);assert(d.buttons==0);
 f=xw_frame_for_virtual(&r,0,2800);slisk_test_map_frame(&f,&d);assert(d.buttons==0 && d.lx==128);
 assert(!slisk_test_owner_allowed(0xffffffffu,407270272));
 assert(!slisk_test_owner_allowed(407270272,407270272));
 assert(slisk_test_owner_allowed(407270273,407270272));
 uint32_t parsed=0;
 assert(slisk_test_owner_event("<118>#LOGIN MGR# SCE_MBUS_EVENT_DEVICE_OWNER_CHANGED [DeviceId:0xa030d][UserId:0x18400001]",0xa030d,&parsed));
 assert(parsed==0x18400001u);
 assert(slisk_test_owner_event("<118> DEVICE_OWNER_CHANGED [DeviceId:0xa030d][type:1][UserId:0x18400002]",0xa030d,&parsed));
 assert(parsed==0x18400002u);
 assert(!slisk_test_owner_event("<118> DEVICE_OWNER_CHANGED [DeviceId:0x9030d][UserId:0x18400001]",0xa030d,&parsed));
 assert(!slisk_test_owner_event("<118> SCE_MBUS_EVENT_DEVICE_ADDED [DeviceId:0xa030d][subType:2]",0xa030d,&parsed));
 assert(slisk_test_pre_owner_buttons(XW_BTN_CROSS)==0x4000u);
 assert(slisk_test_pre_owner_buttons(XW_BTN_PS)==0x10000u);
 assert(slisk_test_pre_owner_buttons(XW_BTN_UP)==0x10u);
 assert(slisk_test_pre_owner_buttons(XW_BTN_CIRCLE|XW_BTN_TRIANGLE|XW_BTN_L1)==0u);
 assert(slisk_test_pre_owner_buttons(XW_BTN_CROSS|XW_BTN_CIRCLE)==0x4000u);
 /* Two independent radio slots must never mirror each other's buttons. */
 XWReceiver pair; xw_init(&pair);
 assert(xw_receive(&pair,0,press,sizeof(press),100)==XW_PACKET_INPUT);
 assert(xw_receive(&pair,1,release,sizeof(release),100)==XW_PACKET_INPUT);
 assert(xw_radio_for_virtual(&pair,0)==0);
 assert(xw_radio_for_virtual(&pair,1)==1);
 assert((xw_frame_for_virtual(&pair,0,100).buttons&XW_BTN_CROSS)!=0);
 assert((xw_frame_for_virtual(&pair,1,100).buttons&XW_BTN_CROSS)==0);
 assert(xw_receive(&pair,1,press,sizeof(press),200)==XW_PACKET_INPUT);
 assert((xw_frame_for_virtual(&pair,1,200).buttons&XW_BTN_CROSS)!=0);
 assert(xw_receive(&pair,0,release,sizeof(release),210)==XW_PACKET_INPUT);
 assert((xw_frame_for_virtual(&pair,0,210).buttons&XW_BTN_CROSS)==0);
 assert((xw_frame_for_virtual(&pair,1,210).buttons&XW_BTN_CROSS)!=0);
 puts("Slisk recorded Xbox USB -> virtual PS4 sample & owner guards: PASS");
 return 0;
}
