#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

const char *progname = "perminfo";
const char *version  = "3.2.0";

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

const char *
get_currentdir(void)
{
	static char path[PATH_MAX + 1];

	if (!getcwd(path, sizeof(path))) {
		fprintf(stderr, "%s: Failed to get current directory\n", progname);
		exit(EXIT_FAILURE);
	}

	return path;
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
get(const char *file, char octal[], bool p[][3], bool *isdir)
{
	struct stat stbuf;

	if (stat(file, &stbuf) == -1) {
		fprintf(stderr, "%s: %s: %s\n", progname, file, strerror(errno));
		exit(EXIT_FAILURE);
	}

	p[SPECIAL][SETUID] = (stbuf.st_mode & S_ISUID);
	p[SPECIAL][SETGID] = (stbuf.st_mode & S_ISGID);
	p[SPECIAL][STICKY] = (stbuf.st_mode & S_ISVTX);
	octal[0] = '0' + 4 * p[SPECIAL][SETUID] + 2 * p[SPECIAL][SETGID] + p[SPECIAL][STICKY];

	*isdir = S_ISDIR(stbuf.st_mode);

	octal[1] = set_perm(stbuf.st_mode & S_IRWXU, 100, p[USER]);
	octal[2] = set_perm(stbuf.st_mode & S_IRWXG,  10, p[GROUP]);
	octal[3] = set_perm(stbuf.st_mode & S_IRWXO,   1, p[OTHERS]);
	octal[4] = '\0';
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
render_perm(const char *label, const bool *p)
{
	printf("├────────────┼──────────────────────┤\n"
		   "│ %s%-11s%s│", COLOR_TYPE, label, RESET);

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
		printf("%s%s execute%s   │\n", BOLD, COLOR_EXECUTE, RESET);
	} else {
		printf("%s execute%s   │\n", COLOR_GRAYED_OUT, RESET);
	}
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
render(const char *file, const char *octal, const bool p[][3], const bool isdir)
{
	printf("┌────────────┬──────────────────────┐\n"
		   "│ %sFilename%s   │ ", COLOR_TYPE, RESET);

	if (strlen(file) > 20) {
		printf("%s...%s%-17s%s │\n", COLOR_GRAYED_OUT, COLOR_FILENAME,
			   strlen(file) - 17 + file, RESET);
	} else {
		printf("%s%-20s%s │\n", COLOR_FILENAME, file, RESET);
	}

	printf("├────────────┼──────────────────────┤\n"
		   "│ %sOctal      │ %s%-20s%s │\n", COLOR_TYPE, COLOR_OCTAL, octal, RESET);


	printf("├────────────┼──────────────────────┤\n"
		   "│ %sSymbolic%s   │ ", COLOR_TYPE, RESET);

	if (isdir) {
		printf("%s%sd%s", BOLD, COLOR_DIRECTORY, RESET);
	}

	render_char(p[USER],   p[SPECIAL][SETUID], 's');
	render_char(p[GROUP],  p[SPECIAL][SETGID], 's');
	render_char(p[OTHERS], p[SPECIAL][STICKY], 't');

	if (!isdir) {
		putchar(' ');
	}

	puts("           │");

	render_perm("User",   p[USER]);
	render_perm("Group",  p[GROUP]);
	render_perm("Others", p[OTHERS]);

	printf("├────────────┼──────────────────────┤\n"
		   "│ %sAttributes%s │", COLOR_TYPE, RESET);

	render_special(p[SPECIAL][SETUID], "setuid");
	render_special(p[SPECIAL][SETGID], "setgid");
	render_special(p[SPECIAL][STICKY], "sticky");

	printf(" │\n"
		   "└────────────┴──────────────────────┘\n");
}

void
run(const char *file)
{
	bool isdir;
	char octal[5];
	bool perms[4][3];
	get(file, octal, perms, &isdir);
	render(file, octal, perms, isdir);
}

void
usage(const int status)
{
	fprintf(stderr,
			"Usage: %s [-hv] [file ...]\n"
			"\n"
			"Options:\n"
			"  -h, --help     Display this help message and exit\n"
			"  -v, --version  Display version information and exit\n",
			progname);
	exit(status);
}

int
main(int argc, char **argv)
{
	if (argc == 1) {
		run(get_currentdir());
		return EXIT_SUCCESS;
	}

	int i;
	bool optend = false;
	for (i = 1; i < argc; i++) {
		if (optend) {
			run(argv[i]);
		} else if (strcmp("--", argv[i]) == 0) {
			optend = true;
		} else if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
			usage(EXIT_SUCCESS);
		} else if (strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) {
			printf("%s %s\n", progname, version);
			return EXIT_SUCCESS;
		} else if ('-' == *argv[i]) {
			fprintf(stderr, "%s: Unknown option: \"%s\"\n", progname, argv[i]);
			return EXIT_FAILURE;
		} else {
			run(argv[i]);
		}
	}

	return EXIT_SUCCESS;
}
