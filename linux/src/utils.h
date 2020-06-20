#ifndef __INC_UTILS_H__
#define __INC_UTILS_H__

#include "log.h"
#include "stdafx.h"

#define utils_free(p) {if(p){free((void *)p); (p) = NULL; }}
#define buffer_clean(buf) memset((buf), 0x00, sizeof((buf)));
#define fclose(fp) ((fp) ? fclose(fp) : 0, (fp) = 0);

#define utils_create(result, type, number)  do { \
if (!((result) = (type *)calloc((number), sizeof(type)))) { \
	logging(CRITICAL, L"calloc failed [%d] %s", errno, strerror(errno)); \
	abort(); } } while (0)

#define utils_realloc(result,type,number) do { \
if (!((result) = (type *)realloc((result), sizeof(type)* (number)))) { \
	logging(CRITICAL, "realloc failed [%d] %s", errno, strerror(errno)); \
	abort(); } } while (0)

struct timeval *utils_timediff(const struct timeval *a, const struct timeval *b);
struct timeval *utils_timeadd(struct timeval *a, struct timeval *b);
unsigned long utils_filesize(FILE *fp);
int utils_get_char_len_from_int(unsigned long n);
char *utils_get_filename_from_path(char *path);
int utils_min(int a, int b);
int utils_max(int a, int b);
void utils_sleep(const struct timespec *req);
bool utils_open_mode_compare(int mode1, int mode2);
void utils_daemonize(const char *path, const char *pid_filename);
void utils_ms_sleep(long dwMillisecond);
size_t utils_readline(FILE *fp, void *buffer, size_t len);
size_t utils_socket_read(int fd, void *buffer, size_t len);
unsigned long utils_getpid(bool thread);
void utils_exit(bool thread);

#endif
