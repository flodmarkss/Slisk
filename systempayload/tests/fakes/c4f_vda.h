#ifndef FAKE_VDA_H
#define FAKE_VDA_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
 uint32_t buttons;uint8_t lx,ly,rx,ry,l2,r2;uint16_t pad;
 float quat[4];uint8_t rest[120-28];
} ScePadData;
typedef struct {int owned;int32_t handle;uint64_t deviceId;} C4fVirtualPad;
#define C4F_VDA_USER_SELECT 1
int c4fKlogOpenDevice(void);
void c4fKlogMark(const char *);
int c4fKlogIsVirtualAdd(const char *);
uint64_t c4fKlogDeviceId(const char *);
int32_t c4fVirtualPadAdd(int32_t);
void c4fVirtualPadAdopt(C4fVirtualPad *,int32_t,int32_t,uint64_t);
int32_t c4fVirtualPadInsert(const C4fVirtualPad *,const ScePadData *);
void c4fVirtualPadRemove(C4fVirtualPad *);
typedef struct {uint8_t large,small,r,g,b;} C4fPadFeedback;
int32_t c4fVirtualPadFeedback(const C4fVirtualPad *,C4fPadFeedback *);
void c4fPadDataNeutral(ScePadData *);
int32_t c4fLoginUsers(int32_t userIds[4]);
#endif
int c4fMbusInit(void);
int c4fPadInit(void);
