#ifndef SLISK_FAKE_KERNEL_H
#define SLISK_FAKE_KERNEL_H
#include <stdint.h>
#include <sys/types.h>
uint64_t kernel_get_ucred_authid(pid_t pid);
int kernel_get_ucred_caps(pid_t pid,uint8_t caps[16]);
int kernel_set_ucred_authid(pid_t pid,uint64_t value);
int kernel_set_ucred_caps(pid_t pid,const uint8_t caps[16]);
uint32_t kernel_get_fw_version(void);
#endif
