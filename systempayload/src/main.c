/* Slisk standalone PS4 GoldHEN payload.
 * GPL-3.0-only because the build links GPL-3.0 VDA support from
 * MoHadiShibli/Control4Free (copyright remains with original contributors).
 * No Control4Free installation, network server or game plugin at runtime.
 */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include <ps4/kernel.h>
#include "c4f_log.h"
#include "c4f_sce.h"
#include "c4f_vda.h"

#define SLISK_AUTHID 0x3800000000010003ULL
extern int slisk_usb_run(void);
static uint64_t original_authid;
static uint8_t original_caps[16];
static pid_t host_pid;
static int have_snapshot, modified_authid, modified_caps;
static void restore_credentials(void){
 if(modified_caps){int rc=kernel_set_ucred_caps(host_pid,original_caps);c4fLog("SLISK restore caps=%d\n",rc);}
 if(modified_authid){int rc=kernel_set_ucred_authid(host_pid,original_authid);c4fLog("SLISK restore authid=%d\n",rc);}
 modified_caps=modified_authid=0;
}
static int authorize(void){
 uint8_t caps[16];
 host_pid=getpid();original_authid=kernel_get_ucred_authid(host_pid);
 if(!original_authid || kernel_get_ucred_caps(host_pid,original_caps)!=0){c4fLog("SLISK no original credentials: abort\n");return -1;}
 have_snapshot=1;memset(caps,0xff,sizeof(caps));
 if(kernel_set_ucred_authid(host_pid,SLISK_AUTHID)!=0){c4fLog("SLISK failed authid change\n");return -1;}
 modified_authid=1;
 if(kernel_set_ucred_caps(host_pid,caps)!=0){c4fLog("SLISK failed capabilities change\n");restore_credentials();return -1;}
 modified_caps=1;
 return 0;
}
int main(void){
 int lock=-1,ret=1;
 /* Lock must be acquired before touching process credentials or USB. */
 (void)mkdir(C4F_LOG_DIR,0777);
 lock=open(C4F_LOG_DIR "/instance.lock",O_WRONLY|O_CREAT,0666);
 if(lock<0 || flock(lock,LOCK_EX|LOCK_NB)!=0){if(lock>=0)close(lock);return 1;}
 c4fLogOpen();
 /* Legacy diagnostic files from older builds are no longer used. */
 if(unlink("/data/slisk-v092-status.txt")==0)c4fLog("SLISK removed obsolete v092 status text file\n");
 if(unlink("/data/slisk-v092.status")==0)c4fLog("SLISK removed obsolete v092 status file\n");

 c4fLog("SLISK native Xbox 360 PS4 payload starting, firmware=%08x\n",(unsigned)kernel_get_fw_version());
 if(authorize()!=0)goto done;
 int32_t rc=sceUserServiceInitialize(NULL);
 c4fLog("SLISK UserService init=0x%08x\n",(unsigned)rc);
 if(c4fMbusInit()!=0){c4fLog("SLISK could not load Mbus: no Pad API touched\n");goto done;}
 int32_t pad_rc=scePadInit();
 c4fLog("SLISK Pad init=0x%08x\n",(unsigned)pad_rc);
 if(pad_rc!=0)goto done;
 pad_rc=scePadSetProcessPrivilege(1);
 c4fLog("SLISK Pad privilege=0x%08x\n",(unsigned)pad_rc);
 if(pad_rc!=0)goto done;
 c4fNotify("Slisk Xbox direct USB: connect receiver, press Xbox A");
 ret=slisk_usb_run();
 done:
 if(have_snapshot)restore_credentials();
 c4fLog("SLISK exit=%d\n",ret);
 c4fLogClose();
 if(lock>=0)close(lock);
 return ret;
}
