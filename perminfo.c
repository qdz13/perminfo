#define _XOPEN_SOURCE 600

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

const char *progname = "perminfo";
const char *version  = "3.8.0";

enum Type {
	USER,
	GROUP,
	OTHERS,
	SPECIAL
};

enum Rwx {
	READ,
	WRITE,
	EXECUTE
};

enum Special {
	SETUID,
	SETGID,
	STICKY
};

[[noreturn]] void
die(const char *fmt, ...)
{
	fprintf(stderr, "%s: ", progname);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

bool
set_rwx(enum Rwx type, const int perm)
{
	switch (type) {
		case READ:
			return (4 == perm ||
					5 == perm ||
					6 == perm ||
					7 == perm);
		case WRITE:
			return (2 == perm ||
					3 == perm ||
					6 == perm ||
					7 == perm);
		case EXECUTE:
			return (1 == perm ||
					3 == perm ||
					5 == perm ||
					7 == perm);
		default:
			exit(EXIT_FAILURE);
	}
}

char
set_perm(const int octalnum, const int divisor, bool *p)
{
	char tmp[4];
	snprintf(tmp, sizeof(tmp), "%o", octalnum);
	int perm = atoi(tmp) / divisor;

	p[READ]    = set_rwx(READ, perm);
	p[WRITE]   = set_rwx(WRITE, perm);
	p[EXECUTE] = set_rwx(EXECUTE, perm);

	return '0' + perm;
}

void
get(const char *file, const bool links, char octal[], bool p[][3], bool *isdir, bool *islnk)
{
	struct stat stbuf;

	if (links) {
		if (stat(file, &stbuf) == -1) {
			die("%s: %s", file, strerror(errno));
		}
	} else if (lstat(file, &stbuf) == -1) {
		die("%s: %s", file, strerror(errno));
	}

	p[SPECIAL][SETUID] = (stbuf.st_mode & S_ISUID);
	p[SPECIAL][SETGID] = (stbuf.st_mode & S_ISGID);
	p[SPECIAL][STICKY] = (stbuf.st_mode & S_ISVTX);
	octal[0] = '0' + 4 * p[SPECIAL][SETUID] + 2 * p[SPECIAL][SETGID] + p[SPECIAL][STICKY];

	*isdir = S_ISDIR(stbuf.st_mode);
	*islnk = S_ISLNK(stbuf.st_mode);

	octal[1] = set_perm(stbuf.st_mode & S_IRWXU, 100, p[USER]);
	octal[2] = set_perm(stbuf.st_mode & S_IRWXG,  10, p[GROUP]);
	octal[3] = set_perm(stbuf.st_mode & S_IRWXO,   1, p[OTHERS]);
	octal[4] = '\0';
}

void
render_separator(const int width)
{
	int i;
	printf("├────────────┼");
	for (i = 0; i < width - 15; i++) {
		printf("─");
	}
	printf("┤\n");
}

void
render_close(const int n)
{
	int i;
	for (i = 0; i < n; i++) {
		putchar(' ');
	}
	printf("│\n");
}

void
render_char(const bool *p, const bool special, const char lower)
{
	if (p[READ]) {
		printf("%s%sr%s", BOLD, COLOR_READ, RESET);
	} else {
		printf("%s-%s", COLOR_GRAYED_OUT, RESET);
	}

	if (p[WRITE]) {
		printf("%s%sw%s", BOLD, COLOR_WRITE, RESET);
	} else {
		printf("%s-%s", COLOR_GRAYED_OUT, RESET);
	}

	if (p[EXECUTE]) {
		if (special) {
			printf("%s%s%c%s", BOLD, COLOR_SPECIAL, lower, RESET);
		} else {
			printf("%s%sx%s", BOLD, COLOR_EXECUTE, RESET);
		}
	} else {
		if (special) {
			printf("%s%s%c%s", BOLD, COLOR_SPECIAL, toupper(lower), RESET);
		} else {
			printf("%s-%s", COLOR_GRAYED_OUT, RESET);
		}
	}
}

void
render_perm(const int width, const char *label, const bool *p)
{
	render_separator(width);
	printf("│ %s%-11s%s│", COLOR_TYPE, label, RESET);

	if (p[READ]) {
		printf("%s%s read%s", BOLD, COLOR_READ, RESET);
	} else {
		printf("%s read%s", COLOR_GRAYED_OUT, RESET);
	}

	if (p[WRITE]) {
		printf("%s%s write%s", BOLD, COLOR_WRITE, RESET);
	} else {
		printf("%s write%s", COLOR_GRAYED_OUT, RESET);
	}

	if (p[EXECUTE]) {
		printf("%s%s execute%s", BOLD, COLOR_EXECUTE, RESET);
	} else {
		printf("%s execute%s", COLOR_GRAYED_OUT, RESET);
	}

	render_close(width - 34);
}

void
render_special(const bool b, const char *s)
{
	if (b) {
		printf("%s%s %s%s", BOLD, COLOR_SPECIAL, s, RESET);
	} else {
		printf("%s %s%s", COLOR_GRAYED_OUT, s, RESET);
	}
}

void
render(const char *file, const char *octal, const bool p[][3], const bool isdir, const bool islnk)
{
	int width = 37; /* Minimum value of required width */
	struct winsize w;
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &w) == -1) {
		die("ioctl: %s", strerror(errno));
	}
	const int cols = w.ws_col;

	if (cols < width) {
		die("Terminal width is too small");
	}

	bool cutfilename = false;
	if (strlen(file) + 17 > (size_t)width) {
		if (strlen(file) + 17 > (size_t)cols) {
			cutfilename = true;
			width = cols;
		} else {
			width = strlen(file) + 17;
		}
	}

	printf("┌────────────┬");
	int i;
	for (i = 0; i < width - 15; i++) {
		printf("─");
	}
	printf("┐\n"
		   "│ %sFilename%s   │ ", COLOR_TYPE, RESET);

	if (cutfilename) {
		printf("%s…%s%s%s │\n", COLOR_GRAYED_OUT, COLOR_FILENAME,
			   strlen(file) - (width - 18) + file, RESET);
	} else {
		printf("%s%s%s", COLOR_FILENAME, file, RESET);
		render_close(width - (strlen(file) + 16));
	}

	render_separator(width);
	printf("│ %sOctal      │ %s%s%s", COLOR_TYPE, COLOR_OCTAL, octal, RESET);
	render_close(width - 20);

	render_separator(width);
	printf("│ %sSymbolic%s   │ ", COLOR_TYPE, RESET);

	if (isdir) {
		printf("%s%sd%s", BOLD, COLOR_DIRECTORY, RESET);
	} else if (islnk) {
		printf("%s%sl%s", BOLD, COLOR_LINK, RESET);
	}

	render_char(p[USER],   p[SPECIAL][SETUID], 's');
	render_char(p[GROUP],  p[SPECIAL][SETGID], 's');
	render_char(p[OTHERS], p[SPECIAL][STICKY], 't');

	if (!isdir && !islnk) {
		putchar(' ');
	}

	render_close(width - 26);
	render_perm(width, "User",   p[USER]);
	render_perm(width, "Group",  p[GROUP]);
	render_perm(width, "Others", p[OTHERS]);

	render_separator(width);
	printf("│ %sAttributes%s │", COLOR_TYPE, RESET);

	render_special(p[SPECIAL][SETUID], "setuid");
	render_special(p[SPECIAL][SETGID], "setgid");
	render_special(p[SPECIAL][STICKY], "sticky");

	render_close(width - 36);
	printf("└────────────┴");
	for (i = 0; i < width - 15; i++) {
		printf("─");
	}
	printf("┘\n");
}

void
run(const char *file, const bool links)
{
	bool isdir, islnk;
	char octal[5];
	bool perms[4][3];
	get(file, links, octal, perms, &isdir, &islnk);
	render(file, octal, perms, isdir, islnk);
}

[[noreturn]] void
usage(const int status)
{
	fprintf(stderr,
			"Usage: %s [-hv] [file ...]\n"
			"\n"
			"Options:\n"
			"  -h, --help          Display this help message and exit\n"
			"  -l, --follow-links  Follow symbolic links\n"
			"  -v, --version       Display version information and exit\n",
			progname);
	exit(status);
}

int
main(int argc, char **argv)
{
	int c, i;
	bool links = false;

	struct option longopts[] = {
		{ "help",         no_argument, NULL, 'b' },
		{ "follow-links", no_argument, NULL, 'l' },
		{ "version",      no_argument, NULL, 'v' },
		{ NULL,           0,           NULL,  0  }
	};

	while ((c = getopt_long(argc, argv, "hlv", longopts, NULL)) != -1) {
		switch (c) {
			case 'h':
				usage(EXIT_SUCCESS);
			case 'l':
				links = true;
				break;
			case 'v':
				printf("%s %s\n", progname, version);
				return EXIT_SUCCESS;
			default:
				usage(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0) {
		const long pathmax = pathconf(".", _PC_PATH_MAX);
		if (pathmax == -1) {
			die("Failed to get the maximum number of bytes in a pathname");
		}

		char path[pathmax + 1];
		if (getcwd(path, sizeof(path)) == NULL) {
			die("%s", strerror(errno));
		}

		run(path, links);

		return EXIT_SUCCESS;
	}

	for (i = 0; i < argc; i++) {
		run(argv[i], links);
	}

	return EXIT_SUCCESS;
}
