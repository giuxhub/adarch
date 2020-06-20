#include "utils.h"

static struct timeval null_time = { 0, 0 };

struct timeval *utils_timediff(const struct timeval *a, const struct timeval *b)
{
	static struct timeval rslt;

	if (a->tv_sec < b->tv_sec)
		return &null_time;

	else if (a->tv_sec == b->tv_sec) {
		if (a->tv_usec < b->tv_usec)
			return &null_time;
		else {
			rslt.tv_sec = 0;
			rslt.tv_usec = a->tv_usec - b->tv_usec;
			return &rslt;
		}
	} else {
		rslt.tv_sec = a->tv_sec - b->tv_sec;

		if (a->tv_usec < b->tv_usec) {
			rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
			rslt.tv_sec--;
		} else
			rslt.tv_usec = a->tv_usec - b->tv_usec;

		return &rslt;
	}
}

struct timeval *utils_timeadd(struct timeval *a, struct timeval *b)
{
	static struct timeval rslt;

	rslt.tv_sec = a->tv_sec + b->tv_sec;
	rslt.tv_usec = a->tv_usec + b->tv_usec;

	while (rslt.tv_usec >= 1000000) {
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

size_t utils_readline(FILE *fp, void *buffer, size_t len)
{
	size_t n = 0; /* # of bytes fetched by last read() */
	size_t r = 0; /* Total bytes read so far */
	char *buf = NULL;
	char ch = '\0';

	if (len <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}

	buf = buffer;

	r = 0;
	for (;;) {

		if ((n = fread(&ch, 1, 1, fp)) <= 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else if (n == 0) {
			if (r == 0)
				return 0;
			else
				break;
		} else {
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

size_t utils_socket_read(int fd, void *buffer, size_t len)
{
	size_t n; /* # of bytes fetched by last read() */
	size_t r; /* Total bytes read so far */
	char *buf;
	char ch;

	if (len <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}

	buf = buffer;

	r = 0;
	for (;;) {

		if ((n = read(fd, &ch, 1)) == -1) {
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
	if ((mode1 & mode2) == mode2)
		return true;

	return false;
}

void utils_sleep(const struct timespec *req)
{
	nanosleep(req, NULL);
}

void utils_ms_sleep(long dwMillisecond)
{
	struct timespec tv_sleep;
	tv_sleep.tv_sec = dwMillisecond / 1000;
	tv_sleep.tv_nsec = dwMillisecond * 1000;
	utils_sleep(&tv_sleep);
}

void utils_daemonize(const char *path, const char *pid_filename)
{
	pid_t pid, sid;
	char tmp_path[256];

	pid = fork();

	if (pid < 0)
	exit(EXIT_FAILURE);

	if (pid > 0)
	exit(EXIT_SUCCESS);

	umask(0);

	sid = setsid();

	if (sid < 0)
	exit(EXIT_FAILURE);

	if ((chdir(path)) < 0)
	exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	buffer_clean(tmp_path);

	strncpy(tmp_path, path, strlen(path));
	strncat(tmp_path, pid_filename, strlen(pid_filename));

	FILE* fp = fopen(tmp_path, "w");
	fprintf(fp, "%u\n", getpid());
	fclose(fp);
}

FILE *utils_create_temp_file(char *path)
{
	char temp_name[strlen(path) + 8];
	FILE *temp_file;

	sprintf(temp_name, ".%s-XXXXXX", utils_get_filename_from_path(path));

	int fd = -1;
	if ((fd = mkstemp(temp_name)) < 0)
	return NULL;

	unlink(temp_name);
	temp_file = fdopen(fd, "w+b");

	return temp_file;
}

unsigned long utils_getpid(bool thread)
{
	return thread ? pthread_self() : getpid();
}

void utils_exit(bool thread)
{
	thread ? pthread_exit(EXIT_SUCCESS) : exit(EXIT_SUCCESS);
}

