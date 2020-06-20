#include "utils.h"

static struct timeval null_time = { 0, 0 };

struct timeval *utils_timediff(const struct timeval *a, const struct timeval *b)
{
	static struct timeval rslt;

	if (a->tv_sec < b->tv_sec)
		return &null_time;

	else if (a->tv_sec == b->tv_sec){
		if (a->tv_usec < b->tv_usec)
			return &null_time;
		else {
			rslt.tv_sec = 0;
			rslt.tv_usec = a->tv_usec - b->tv_usec;
			return &rslt;
		}
	}
	else{
		rslt.tv_sec = a->tv_sec - b->tv_sec;

		if (a->tv_usec < b->tv_usec){
			rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
			rslt.tv_sec--;
		}
		else
			rslt.tv_usec = a->tv_usec - b->tv_usec;

		return &rslt;
	}
}

struct timeval *utils_timeadd(struct timeval *a, struct timeval *b)
{
	static struct timeval rslt;

	rslt.tv_sec = a->tv_sec + b->tv_sec;
	rslt.tv_usec = a->tv_usec + b->tv_usec;

	while (rslt.tv_usec >= 1000000){
		rslt.tv_usec -= 1000000;
		rslt.tv_sec++;
	}
	return &rslt;
}

unsigned long utils_filesize(FILE *fp)
{
	long pos;
	long size;

	pos = ftell(fp);

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, pos, SEEK_SET);

	return (size);
}

int utils_get_char_len_from_int(unsigned long n)
{
	int i = 1;

	while (n >= 10) {
		n /= 10;
		i++;
	}
	return i;
}

char *utils_get_filename_from_path(char *path)
{
	char *filename = strrchr(path, '\\');

	if (filename == NULL)
		filename = path;
	else
		filename++;

	return filename;
}

int utils_min(int a, int b)
{
	return a < b ? a : b;
}

int utils_max(int a, int b)
{
	return a > b ? a : b;
}

size_t utils_strip_newline(char *in, char *out, size_t len)
{
	size_t r = 0, n = 0;
	char *buf = NULL;

	if (in == NULL)
		return -1;

	buf = in;

	while ((r = strcspn(buf, "\n")) > 0) {
		r = strcspn(buf, "\n");
		n += r + 1;

		*(buf + r - 1) == '\r' ? *(buf + r - 1) = '\0' : r;
		*(buf + r) = '\0';	
		
		strncat_s(out, n, buf, _TRUNCATE);
		buf += r + 1;
	}
	return n;
}

size_t utils_socket_read(int fd, void *buffer, size_t len)
{
	size_t n; /* # of bytes fetched by last read() */
	size_t r; /* Total bytes read so far */
	LPSTR buf;
	char ch;

	if (len <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}

	buf = buffer;

	r = 0;
	for (;;) {

		if ((n = recv(fd, &ch, 1, 0)) == -1) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
		else if (n == 0) {
			if (r == 0)
				return 0;
			else
				break;
		}
		else {
			if (ch == '\n')
				break;

			if (r < (len - 1)) {
				r++;
				*buf++ = ch;
			}
		}
	}
	*(buf - 1) == '\r' ? --buf : buf;

	*buf = '\0';

	return r;
}

bool utils_open_mode_compare(int mode1, int mode2)
{
	if ((mode1&mode2) == mode2)
		return true;

	return false;
}

void utils_sleep(struct timeval *timeout) {
	Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
}

void utils_ms_sleep(long lMillisecond){
	Sleep(lMillisecond);
}

DWORD utils_getpid(bool pre_threaded){
	return pre_threaded ? GetCurrentThreadId() : GetCurrentProcessId();
}

void utils_exit(bool pre_threaded){
	pre_threaded ? ExitThread(EXIT_SUCCESS) : exit(EXIT_SUCCESS);
}
