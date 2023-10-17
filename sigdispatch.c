
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MIN_SIGNUM 1
#define MAX_SIGNUM (NSIG - 1)
#define LENGTH(x) (sizeof(x)/sizeof(x[0]))
#define SIG_IDENTICAL 0
#define SIG_SINK -1
#define SIG_KEEP -2


typedef struct {
	char * abbrev;
	int number;
} SignalMap;

SignalMap system_signals[] = {
#include "signames.h"
};

int signal_to_send[MAX_SIGNUM - MIN_SIGNUM + 1];
pid_t child_pid = 0;


void bad_invocation()
{
	errx(-1,
		"Usage: sigdispatch [SIG_A1:SIG_B1 [SIG_A2:SIG_B2 [...]]] -- <COMMAND> [ARGUMENTS]\n\n"
		"Execute COMMAND. Received SIG_A is translated to SIG_B and SIG_B sent to COMMAND.\n"
		"SIG_X can be a signal number or signal name optionally with \"SIG\" prefix.\n"
		"A dash \"-\" for SIG_B makes the corresponding SIG_A not dispatched.\n"
		"By default all signals map to itself and relayed to COMMAND (up to system limitations).\n");
}

int resolve_sigspec(char* sigspec)
{
	int signal = 0;
	char* rest;
	char* signame;
	int x;
	
	errno = 0;
	signal = strtol(sigspec, &rest, 10);
	if(errno != 0)
	{
		perror("strtol");
		bad_invocation();
	}
	if(rest == sigspec)
	{
		/* there was no numbers, just string */
		/* interpret it as a signal name */
		if(strncmp(sigspec, "SIG", 3)==0)
		{
			/* sigspec starts with "SIG" prefix, let's remove it */
			sigspec = &sigspec[3];
		}
		for(x = 0; x < LENGTH(system_signals); x++)
		{
			if(strcmp(system_signals[x].abbrev, sigspec)==0)
			{
				/* we have found a matching signal name, let's use its signal number */
				signal = system_signals[x].number;
				break;
			}
		}
		if(x >= LENGTH(system_signals))
		{
			errx(-2, "Unknown signal name: %s", sigspec);
		}
	}
	else if(rest[0] != '\0')
	{
		/* the whole sigspec string has NOT converted to number */
		bad_invocation();
	}
	/* okay, the whole sigspec string converted to number */
	if(signal < MIN_SIGNUM || signal > MAX_SIGNUM)
	{
		bad_invocation();
	}
	return signal;
}

static void signal_handler(int signal)
{
	if(child_pid != 0 && signal_to_send[signal - MIN_SIGNUM] != SIG_SINK)
	{
		kill(child_pid, signal_to_send[signal - MIN_SIGNUM]);
	}
}

int main(int argc, char** argv)
{
	int argind;
	char* sigspec_a;
	char* sigspec_b;
	int signal_a;
	int signal_b;
	struct sigaction sigact;
	int child_status;
	
	memset(signal_to_send, '\0', sizeof(signal_to_send));
	
	for(argind = 1; argind < argc; argind++)
	{
		if(strcmp(argv[argind], "--")==0)
		{
			argind++;
			break;
		}
		
		sigspec_a = argv[argind];
		sigspec_b = strchr(argv[argind], ':');
		if(sigspec_b == NULL)
		{
			bad_invocation();
		}
		/* terminiate sigspec_a at ':' char */
		*sigspec_b = '\0';
		/* sigspec_b starts right after ':' char */
		sigspec_b++;
		
		signal_a = resolve_sigspec(sigspec_a);
		if(strcmp(sigspec_b, "-") == 0)
			signal_b = SIG_SINK;
		else
			signal_b = resolve_sigspec(sigspec_b);
		signal_to_send[signal_a - MIN_SIGNUM] = signal_b;
	}
	
	if(argind >= argc)
	{
		bad_invocation();
	}
	
	child_pid = fork();
	if(child_pid == 0)
	{
		execvp(argv[argind], &argv[argind]);
		_exit(127);
	}
	
	for(signal_a = MIN_SIGNUM; signal_a <= MAX_SIGNUM; signal_a++)
	{
		if(signal_to_send[signal_a - MIN_SIGNUM] == SIG_IDENTICAL)
		{
			signal_to_send[signal_a - MIN_SIGNUM] = signal_a;
		}
		
		if(signal_to_send[signal_a - MIN_SIGNUM] != SIG_KEEP)
		{
			sigact.sa_handler = signal_handler;
			sigemptyset(&sigact.sa_mask);
			sigact.sa_flags = 0;
			sigaction(signal_a, &sigact, (struct sigaction *)NULL);
		}
	}
	
	while(wait(&child_status) == -1);
	
	if(WIFEXITED(child_status))
		exit(WEXITSTATUS(child_status));
	else
		exit(WTERMSIG(child_status) + 128);
}
