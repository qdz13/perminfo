#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const char *progname = "perminfo";
const char *version  = "1.0.0";

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
/* #define BLUE    "\033[34m" */
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define GRAY    "\033[38;5;243m"
#define BOLD    "\033[1m"
/* #define LINE    "\033[4m" */
#define RESET   "\033[0m"

bool
isperm (const char *s)
{
	int i;
	for (i = 0; *(s+i) != '\0'; i++) {
		if (*(s+i) < '0' || *(s+i) > '7') {
			return false;
		}
	}

	if (strlen(s) < 3 || strlen(s) > 4)
		return false;

	return true;
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
		n == '7') *setuid = true;

	if (n == '2' ||
		n == '3' ||
		n == '6' ||
		n == '7') *setgid = true;

	if (n == '1' ||
		n == '3' ||
		n == '5' ||
		n == '7') *sticky = true;
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
separator(void)
{
	int i;
	for (i = 0; i < 37; i++) {
		putchar('-');
	}
	putchar('\n');
}

void
render(const char *s)
{
	separator();
	printf("| %sSymbolic%s   | ", CYAN, RESET);
	int i;
	for (i = 0; s[i] != '\0'; i++) {
		switch(s[i]) {
			case 'r':
				printf("%s%s%c%s", BOLD, YELLOW, s[i], RESET);
				break;
			case 'w':
				printf("%s%s%c%s", BOLD, RED, s[i], RESET);
				break;
			case 'x':
				printf("%s%s%c%s", BOLD, GREEN, s[i], RESET);
				break;
			case 's':
			case 'S':
			case 't':
			case 'T':
				printf("%s%s%c%s", BOLD, MAGENTA, s[i], RESET);
				break;
			case '-':
				printf("%s%s%c%s", BOLD, GRAY, s[i], RESET);
				break;
		}
	}
	puts("            |");
	bool setuid = false;
	bool setgid = false;
	bool sticky = false;
	for (i = 0; i < 3; i++) {
		separator();
		printf("| %s", CYAN);
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
		printf("%s|", RESET);
		if (*s == 'r') {
			printf("%s%s read%s", BOLD, YELLOW, RESET);
		} else {
			printf("%s read%s", GRAY, RESET);
		}
		s++;
		if (*s == 'w') {
			printf("%s%s write%s", BOLD, RED, RESET);
		} else {
			printf("%s write%s", GRAY, RESET);
		}
		s++;
		switch(*s) {
			case 'x':
				printf("%s%s execute%s", BOLD, GREEN, RESET);
				break;
			case 's':
				printf("%s%s execute%s", BOLD, GREEN, RESET);
				if (i == 0) {
					setuid = true;
				} else {
					setgid = true;
				}
				break;
			case 't':
				printf("%s%s execute%s", BOLD, GREEN, RESET);
				sticky = true;
				break;
			case 'S':
				printf("%s execute%s", GRAY, RESET);
				if (i == 0) {
					setuid = true;
				} else {
					setgid = true;
				}
				break;
			case 'T':
				printf("%s execute%s", GRAY, RESET);
				sticky = true;
				break;
			default:
				printf("%s execute%s", GRAY, RESET);
				break;
		}
		s++;
		puts("   |");
	}
	separator();
	printf("| %sAttributes%s |", CYAN, RESET);
	if (setuid) {
		printf("%s%s setuid%s", BOLD, MAGENTA, RESET);
	} else {
		printf("%s setuid%s", GRAY, RESET);
	}
	if (setgid) {
		printf("%s%s setgid%s", BOLD, MAGENTA, RESET);
	} else {
		printf("%s setgid%s", GRAY, RESET);
	}
	if (sticky) {
		printf("%s%s sticky%s", BOLD, MAGENTA, RESET);
	} else {
		printf("%s sticky%s", GRAY, RESET);
	}
	puts(" |");
	separator();
}

void
usage(int status)
{
	fprintf(stderr, "Usage: %s [OPTIONS] [PERMISSION]\n", progname);
	fprintf(stderr, "\n"
			"Options:\n"
			"  -h, --help    Display this help message and exit\n"
			"  -v, --version Display version information and exit\n");
	exit(status);
}

int
main(int argc, char *argv[])
{
	if (argc == 1) {
		usage(1);
	}

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			usage(0);
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			printf("%s %s\n", progname, version);
			return 0;
		} else if (*argv[i] == '-') {
			fprintf(stderr, "%s: Unknown option: \"%s\"\n", progname, argv[i]);
			return 1;
		} else if (argc != 2) {
			usage(1);
		}
	}

	if (!isperm(argv[1])) {
		fprintf(stderr, "%s: Invalid file mode: %s\n", progname, argv[1]);
		return 1;
	}

	char full[10];
	tosymbolic(full, argv[1]);

	render(full);

	return 0;
}
