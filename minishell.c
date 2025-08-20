/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NB 20             /* max number of active background processes */
#define NV 20             /* max number of command tokens */
#define NL 100            /* input buffer size */
char line[NL];            /* command input buffer */
int job_counter = 1;      /* global job counter, starts at 1 */
int num_bg_processes = 0; /* number of active background processes */

/* ----------------------------------------------------------------
**   RUDIMENTARY HASHMAP IMPLEMENTATION TO TRACK ACTIVE PROCESSES
** ---------------------------------------------------------------- */

/**
 * Struct to represent a background process. Contains PID, job number, and
 * command string
 *
 */
struct bg_process {
  int pid;           // pid of background process
  int job_number;    // job number of bg process
  char command[NL];  // command e.g. sleep 2
};

/**
 * Array to store all active background processes. Functions implemented to act
 * as hashmap.
 *
 */
struct bg_process bg_processes[NB];

/**
 * Function to create a bg_process object and add it to the bg_processes hashmap
 *
 * @param pid the process ID of the background process
 * @param job_number the value of `job_counter` at the time of process start
 * @param command the executing command
 * @return `true` if the process was successfully stored, else `false`
 */
bool add_bg_process(int pid, int job_number, char *command) {
  if (num_bg_processes < NB) {
    // define bg_process properties:
    bg_processes[num_bg_processes].pid = pid;
    bg_processes[num_bg_processes].job_number = job_number;
    strcpy(bg_processes[num_bg_processes].command, command);

    num_bg_processes++;  // increment active bg processes counter
    return true;
  } else {
    // limit on number of bg processes reached
    return false;
  }
}

/**
 * Function to remove a bg_process from the hashmap, corresponding with pid.
 *
 * @param removed_process filled with the properties of the removed process
 * @param pid the pid of the process to be removed
 * @return `true` if a process was found and removed, else `false`
 */
bool remove_bg_process(struct bg_process *removed_process, int pid) {
  int i = 0;
  while ((i < num_bg_processes) && (bg_processes[i].pid != pid)) {
    i++;
  }

  if (i == num_bg_processes) {
    return false;
  } else {
    *removed_process = bg_processes[i];  // copy process to provided pointer
    num_bg_processes--;                  // decrement number of active processes
    // shift subsequent array items left to overwrite removed process:
    for (int j = i; j < num_bg_processes; j++) {
      bg_processes[j] = bg_processes[j + 1];
    }
    return true;
  }
}
/* ----------------------------------------------------------------
**                    END HASHMAP IMPLEMENTATION
** ---------------------------------------------------------------- */

/**
 * SIGCHLD handler. Only used to reap background processes, ignores foreground
 * processes.
 */
void handleSIGCHLD(int sig) {
  pid_t pid;
  while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    struct bg_process finished_process;
    if (remove_bg_process(&finished_process, pid)) {
      // remove finished process from hashmap and print output
      printf("[%d]+ Done                 %s\n", finished_process.job_number,
             finished_process.command);
    }  // foreground process are ignored (reaped in main loop)
  }
}

/* argc - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argc, char *argv[], char *envp[]) {
  int pid;             /* value returned by fork sys call */
  char *v[NV];         /* array of pointers to command line tokens */
  char *sep = " \t\n"; /* command line token separators    */
  int i;               /* parse index */
  char command[NL];    /* copy of command string for bg processes*/
  bool run_in_bg;      /* set by & */

  // designate signal handler
  // child processes send SIGCHLD signal when finished
  signal(SIGCHLD, handleSIGCHLD);

  while (1) { /* do Forever */
    fflush(stdout);
    fgets(line, NL, stdin);
    fflush(stdin);

    if (feof(stdin)) {
      while (num_bg_processes > 0) pause();  // wait for bg processes to finish
      exit(0);
    }

    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000') {
      continue; /* to prompt */
    }

    // copy command before tokenising
    strcpy(command, line);

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL) {
        break;
      }
    }

    // handle cd correctly
    if (strcmp(v[0], "cd") == 0) {
      chdir(v[1]);
      continue;
    }

    /* assert i is number of tokens + 1 */
    run_in_bg = false;
    if (strcmp(v[i - 1], "&") == 0) {
      v[i - 1] = NULL;                      // remove & from token list
      command[strlen(command) - 2] = '\0';  // remove & from str copy
      run_in_bg = true;                     // set run_in_bg flag
    }

    /* fork a child process to exec the command in v[0] */
    switch (pid = fork()) {
      case -1: /* fork returns error to parent process */
      {
        perror("fork failed");
        break;
      }
      case 0: /* code executed only by child process */
      {
        execvp(v[0], v);
        perror("execvp failed");
        exit(EXIT_FAILURE);
        break;
      }
      default: /* code executed only by parent process */
      {
        if (run_in_bg) {
          // add process info to active background processes hashmap
          if (!add_bg_process(pid, job_counter, command)) {
            // perror("maximum number of background processes exceeded");
          }
          printf("[%d] %d\n", job_counter, pid);
          job_counter++;
        } else {
          // wait for process to finish before continuing
          waitpid(pid, NULL, 0);
        }
        break;
      }
    } /* switch */
  } /* while */
} /* main */
