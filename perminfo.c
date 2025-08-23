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

struct _rwx {
	bool read;
	bool write;
	bool execute;
};

struct _special {
	bool setuid;
	bool setgid;
	bool sticky;
};

struct _permission {
	bool isdir;
	struct _rwx user;
	struct _rwx group;
	struct _rwx others;
	struct _special special;
};

enum Rwx {
	READ,
	WRITE,
	EXECUTE
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
set_rwx(enum Rwx type, const char *perm)
{
	switch (type) {
		case READ:
			return ('4' == *perm ||
					'5' == *perm ||
					'6' == *perm ||
					'7' == *perm);
		case WRITE:
			return ('2' == *perm ||
					'3' == *perm ||
					'6' == *perm ||
					'7' == *perm);
		case EXECUTE:
			return ('1' == *perm ||
					'3' == *perm ||
					'5' == *perm ||
					'7' == *perm);
	}
}

void
set_perm(const int octalnum, struct _permission *ptr)
{
	char perm[4];
	snprintf(perm, sizeof(perm), "%o", octalnum);

	/* user */
	ptr->user.read      = set_rwx(READ, perm);
	ptr->user.write     = set_rwx(WRITE, perm);
	ptr->user.execute   = set_rwx(EXECUTE, perm);

	/* group */
	ptr->group.read     = set_rwx(READ, &perm[1]);
	ptr->group.write    = set_rwx(WRITE, &perm[1]);
	ptr->group.execute  = set_rwx(EXECUTE, &perm[1]);

	/* others */
	ptr->others.read    = set_rwx(READ, &perm[2]);
	ptr->others.write   = set_rwx(WRITE, &perm[2]);
	ptr->others.execute = set_rwx(EXECUTE, &perm[2]);
}

void
get(const char *file, struct _permission *ptr)
{
	struct stat stbuf;

	if (stat(file, &stbuf) == -1) {
		fprintf(stderr, "%s: %s: %s\n", progname, file, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ptr->special.setuid = (stbuf.st_mode & S_ISUID);
	ptr->special.setgid = (stbuf.st_mode & S_ISGID);
	ptr->special.sticky = (stbuf.st_mode & S_ISVTX);

	ptr->isdir = S_ISDIR(stbuf.st_mode);

	set_perm((stbuf.st_mode & S_IRWXU) + (stbuf.st_mode & S_IRWXG) +
			 (stbuf.st_mode & S_IRWXO), ptr);
}

void
render_char(const bool read,    const bool write,
			const bool execute, const bool special,
			const char lower,   const char upper)
{
	if (read) {
		printf("%s%sr%s", BOLD, COLOR_READ, RESET);
	} else {
		printf("%s-%s", COLOR_GRAYED_OUT, RESET);
	}

	if (write) {
		printf("%s%sw%s", BOLD, COLOR_WRITE, RESET);
	} else {
		printf("%s-%s", COLOR_GRAYED_OUT, RESET);
	}

	if (execute) {
		if (special) {
			printf("%s%s%c%s", BOLD, COLOR_SPECIAL, lower, RESET);
		} else {
			printf("%s%sx%s", BOLD, COLOR_EXECUTE, RESET);
		}
	} else {
		if (special) {
			printf("%s%s%c%s", BOLD, COLOR_SPECIAL, upper, RESET);
		} else {
			printf("%s-%s", COLOR_GRAYED_OUT, RESET);
		}
	}
}

void
render_perm(const char *label, const bool read,
			const bool write,  const bool execute)
{
	puts("├────────────┼──────────────────────┤");
	printf("│ %s%-11s%s│", COLOR_TYPE, label, RESET);

	if (read) {
		printf("%s%s read%s", BOLD, COLOR_READ, RESET);
	} else {
		printf("%s read%s", COLOR_GRAYED_OUT, RESET);
	}

	if (write) {
		printf("%s%s write%s", BOLD, COLOR_WRITE, RESET);
	} else {
		printf("%s write%s", COLOR_GRAYED_OUT, RESET);
	}

	if (execute) {
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
render(struct _permission *ptr)
{
	puts("┌────────────┬──────────────────────┐");
	printf("│ %sSymbolic%s   │ ", COLOR_TYPE, RESET);

	if (ptr->isdir) {
		printf("%s%sd%s", BOLD, COLOR_DIRECTORY, RESET);
	}

	render_char(ptr->user.read,      ptr->user.write,
				ptr->user.execute,   ptr->special.setuid,
				's', 'S');
	render_char(ptr->group.read,     ptr->group.write,
				ptr->group.execute,  ptr->special.setgid,
				's', 'S');
	render_char(ptr->others.read,    ptr->others.write,
				ptr->others.execute, ptr->special.sticky,
				't', 'T');

	if (!ptr->isdir) {
		putchar(' ');
	}

	puts("           │");

	render_perm("User",            ptr->user.read,
				ptr->user.write,   ptr->user.execute);
	render_perm("Group",           ptr->group.read,
				ptr->group.write,  ptr->group.execute);
	render_perm("Others",          ptr->others.read,
				ptr->others.write, ptr->others.execute);

	puts("├────────────┼──────────────────────┤");
	printf("│ %sAttributes%s │", COLOR_TYPE, RESET);

	render_special(ptr->special.setuid, "setuid");
	render_special(ptr->special.setgid, "setgid");
	render_special(ptr->special.sticky, "sticky");

	puts(" │");
	puts("└────────────┴──────────────────────┘");
}

void
run(const char *file)
{
	struct _permission octal;
	get(file, &octal);
	render(&octal);
}

void
usage(const int status)
{
	fprintf(stderr,
			"Usage: %s [OPTION] [FILE]...\n"
			"\n"
			"Option:\n"
			"  -h, --help    Display this help message and exit\n",
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
		} else if ('-' == *argv[i]) {
			fprintf(stderr, "%s: Unknown option: \"%s\"\n", progname, argv[i]);
			return EXIT_FAILURE;
		} else {
			run(argv[i]);
		}
	}

	return EXIT_SUCCESS;
}
