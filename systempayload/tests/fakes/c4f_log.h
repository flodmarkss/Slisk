#ifndef FAKE_LOG_H
#define FAKE_LOG_H
#define C4F_LOG_DIR "/data/slisk_x360"
void c4fLog(const char *,...);
void c4fLogOpen(void);
void c4fLogClose(void);
void c4fNotify(const char *,...);
#endif
