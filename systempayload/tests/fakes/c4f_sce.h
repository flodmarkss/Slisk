#ifndef SLISK_FAKE_SCE_H
#define SLISK_FAKE_SCE_H
#include <stdint.h>
int32_t sceUserServiceInitialize(void*);
int32_t scePadInit(void);
int32_t scePadSetProcessPrivilege(int32_t);
#endif
