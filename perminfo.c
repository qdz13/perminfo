#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "config.h"

const char *progname = "perminfo";

const char *
get_currentdir(void)
{
	static char path[PATH_MAX + 1];
	if (!getcwd(path, PATH_MAX + 1)) {
		fprintf(stderr, "%s: Failed to get current directory\n", progname);
		exit(EXIT_FAILURE);
	}
	return path;
}

bool
isperm(const char *s)
{
	int i;
	for (i = 0; s[i] != '\0'; i++) {
		if (s[i] < '0' || s[i] > '7') {
			return false;
		}
	}

	if (strlen(s) < 3 || strlen(s) > 4) {
		return false;
	}

	return true;
}

void
file_perm(char *target, const size_t len, const char *file, bool *isdir)
{
		struct stat stbuf;

		if (stat(file, &stbuf) == -1) {
			fprintf(stderr, "%s: %s: No such file or directory\n", progname, file);
			exit(EXIT_FAILURE);
		}

		int special = 0;
		if (stbuf.st_mode & S_ISUID) {
			special += 4;
		}
		if (stbuf.st_mode & S_ISGID) {
			special += 2;
		}
		if (stbuf.st_mode & S_ISVTX) {
			special++;
		}
		snprintf(target, len, "%d%o", special,
				(stbuf.st_mode & S_IRWXU) +
				(stbuf.st_mode & S_IRWXG) +
				(stbuf.st_mode & S_IRWXO));

		*isdir = S_ISDIR(stbuf.st_mode);
}

void
set_rwx(char *target, const char n, const bool special, const char lower, const char upper)
{
	if (n == '4' ||
		n == '5' ||
		n == '6' ||
		n == '7') {
		target[0] = 'r';
	} else {
		target[0] = '-';
	}

	if (n == '2' ||
		n == '3' ||
		n == '6' ||
		n == '7') {
		target[1] = 'w';
	} else {
		target[1] = '-';
	}

	if (n == '1' ||
		n == '3' ||
		n == '5' ||
		n == '7') {
		if (special) {
			target[2] = lower;
		} else {
			target[2] = 'x';
		}
	} else {
		if (special) {
			target[2] = upper;
		} else {
			target[2] = '-';
		}
	}

	target[3] = '\0';
}

void
setspecial(const char n, bool *setuid, bool *setgid, bool *sticky)
{
	if (n == '4' ||
		n == '5' ||
		n == '6' ||
		n == '7') {
		*setuid = true;
	}

	if (n == '2' ||
		n == '3' ||
		n == '6' ||
		n == '7') {
		*setgid = true;
	}

	if (n == '1' ||
		n == '3' ||
		n == '5' ||
		n == '7') {
		*sticky = true;
	}
}

void
tosymbolic(char *target, const char *n)
{
	bool setuid = false;
	bool setgid = false;
	bool sticky = false;

	if (strlen(n) == 4) {
		setspecial(*n, &setuid, &setgid, &sticky);
		n++;
	}

	set_rwx(target,     *n,     setuid, 's', 'S');
	set_rwx(&target[3], *(n+1), setgid, 's', 'S');
	set_rwx(&target[6], *(n+2), sticky, 't', 'T');
}

void
render(const char *s, const bool isdir)
{
	puts("┌────────────┬──────────────────────┐");
	printf("│ %sSymbolic%s   │ ", COLOR_TYPE, RESET);
	if (isdir) {
		printf("%s%sd%s", BOLD, COLOR_DIRECTORY, RESET);
	}
	int i;
	for (i = 0; s[i] != '\0'; i++) {
		switch(s[i]) {
			case 'r':
				printf("%s%s%c%s", BOLD, COLOR_READ, s[i], RESET);
				break;
			case 'w':
				printf("%s%s%c%s", BOLD, COLOR_WRITE, s[i], RESET);
				break;
			case 'x':
				printf("%s%s%c%s", BOLD, COLOR_EXECUTE, s[i], RESET);
				break;
			case 's':
			case 'S':
			case 't':
			case 'T':
				printf("%s%s%c%s", BOLD, COLOR_SPECIAL, s[i], RESET);
				break;
			case '-':
				printf("%s%s%c%s", BOLD, COLOR_GRAYED_OUT, s[i], RESET);
				break;
		}
	}
	if (!isdir) {
		putchar(' ');
	}
	puts("           │");
	bool setuid = false;
	bool setgid = false;
	bool sticky = false;
	for (i = 0; i < 3; i++) {
		puts("├────────────┼──────────────────────┤");
		printf("│ %s", COLOR_TYPE);
		switch(i) {
			case 0:
				printf("User       ");
				break;
			case 1:
				printf("Group      ");
				break;
			case 2:
				printf("Others     ");
				break;
		}
		printf("%s│", RESET);
		if (*s == 'r') {
			printf("%s%s read%s", BOLD, COLOR_READ, RESET);
		} else {
			printf("%s read%s", COLOR_GRAYED_OUT, RESET);
		}
		s++;
		if (*s == 'w') {
			printf("%s%s write%s", BOLD, COLOR_WRITE, RESET);
		} else {
			printf("%s write%s", COLOR_GRAYED_OUT, RESET);
		}
		s++;
		switch(*s) {
			case 'x':
				printf("%s%s execute%s", BOLD, COLOR_EXECUTE, RESET);
				break;
			case 's':
				printf("%s%s execute%s", BOLD, COLOR_EXECUTE, RESET);
				if (i == 0) {
					setuid = true;
				} else {
					setgid = true;
				}
				break;
			case 't':
				printf("%s%s execute%s", BOLD, COLOR_EXECUTE, RESET);
				sticky = true;
				break;
			case 'S':
				printf("%s execute%s", COLOR_GRAYED_OUT, RESET);
				if (i == 0) {
					setuid = true;
				} else {
					setgid = true;
				}
				break;
			case 'T':
				printf("%s execute%s", COLOR_GRAYED_OUT, RESET);
				sticky = true;
				break;
			default:
				printf("%s execute%s", COLOR_GRAYED_OUT, RESET);
				break;
		}
		s++;
		puts("   │");
	}
	puts("├────────────┼──────────────────────┤");
	printf("│ %sAttributes%s │", COLOR_TYPE, RESET);
	if (setuid) {
		printf("%s%s setuid%s", BOLD, COLOR_SPECIAL, RESET);
	} else {
		printf("%s setuid%s", COLOR_GRAYED_OUT, RESET);
	}
	if (setgid) {
		printf("%s%s setgid%s", BOLD, COLOR_SPECIAL, RESET);
	} else {
		printf("%s setgid%s", COLOR_GRAYED_OUT, RESET);
	}
	if (sticky) {
		printf("%s%s sticky%s", BOLD, COLOR_SPECIAL, RESET);
	} else {
		printf("%s sticky%s", COLOR_GRAYED_OUT, RESET);
	}
	puts(" │");
	puts("└────────────┴──────────────────────┘");
}

void
usage(int status)
{
	fprintf(stderr, "Usage: %s [OPTIONS] [PERMISSION]\n", progname);
	fprintf(stderr, "       %s [OPTIONS] [FILE]\n", progname);
	fprintf(stderr, "\n"
			"Options:\n"
			"  -h, --help    Display this help message and exit\n");
	exit(status);
}

int
main(int argc, char *argv[])
{
	int string = 1;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			usage(EXIT_SUCCESS);
		} else if (strcmp(argv[i], "--") == 0) {
			if (argc == 3) {
				string = 2;
				break;
			} else {
				usage(EXIT_FAILURE);
			}
		} else if (*argv[i] == '-') {
			fprintf(stderr, "%s: Unknown option: \"%s\"\n", progname, argv[i]);
			return EXIT_FAILURE;
		} else if (argc != 2) {
			usage(EXIT_FAILURE);
		}
	}

	char perm[5];
	bool isdir = false;
	if (argc == 1) {
		file_perm(perm, sizeof(perm), get_currentdir(), &isdir);
	} else if (isperm(argv[string])) {
		strncpy(perm, argv[string], sizeof(perm));
	} else {
		file_perm(perm, sizeof(perm), argv[string], &isdir);
	}

	char full[10];
	tosymbolic(full, perm);

	render(full, isdir);

	return EXIT_SUCCESS;
}
