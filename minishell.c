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

#define NV 20  /* max number of command tokens */
#define NL 100 /* input buffer size */
#define NB 20  /* max number of active background processes */
char line[NL]; /* command input buffer */
int job_counter = 1;
int num_active_processes = 0;

/* ---------------------------------------------------------------
 * RUDIMENTARY HASHMAP IMPLEMENTATION CONTAINING ACTIVE PROCESSES
 * --------------------------------------------------------------- */
struct process {
  int pid;             // pid of child
  int job_number;      // job number of child
  bool is_bg_process;  // bg process flag
};

struct process processes[NB];

void init_process_map() {
  for (int i = 0; i < NB; i++) {
    processes[i].pid = -1;
    processes[i].job_number = -1;
  }
  num_active_processes = 0;
}

bool add_process(int pid, int job_number, bool is_bg_process) {
  if (num_active_processes >= NB) {
    return false;
  } else {
    processes[num_active_processes].pid = pid;
    processes[num_active_processes].job_number = job_number;
    processes[num_active_processes].is_bg_process = is_bg_process;
    num_active_processes++;
    return true;
  }
}

bool remove_process(struct process* removed_process, int pid) {
  int i = 0;
  while ((i < num_active_processes) && (processes[i].pid != pid)) {
    i++;
  }

  if (i == num_active_processes) {
    return false;
  } else {
    *removed_process = processes[i]; // copy process to provided pointer
    num_active_processes--;
    for (int j = i; j < num_active_processes; j++) {
      processes[j] = processes[j + 1];
    }
    return true;
  }
}
/* ---------------------------------------------------------------
 *                    END HASHMAP IMPLEMENTATION
 * --------------------------------------------------------------- */

void handleSIGCHLD(int sig) {
  pid_t pid;
  int status;
  while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    struct process finished_process;
    if (!remove_process(&finished_process, pid)) {
      perror("process not found in list");
    } else if (finished_process.is_bg_process == true) {
      printf("[%d]+ Done\n", finished_process.job_number);
    } else {
      printf("[%d] [%d]\n", finished_process.job_number, pid);
    }
  }
}

/* argc - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argc, char *argv[], char *envp[]) {
  int frkRtnVal;       /* value returned by fork sys call */
  int wpid;            /* value returned by wait */
  char *v[NV];         /* array of pointers to command line tokens */
  char *sep = " \t\n"; /* command line token separators    */
  int i;               /* parse index */
  bool run_in_bg;      /* set by & */

  init_process_map();

  // designate signal handler
  // child processes send SIGCHLD signal when finished
  signal(SIGCHLD, handleSIGCHLD);

  /* prompt for and process one command line at a time  */

  while (1) { /* do Forever */
    fflush(stdout);
    fgets(line, NL, stdin);
    fflush(stdin);

    if (feof(stdin)) { /* non-zero on EOF  */
      perror("non-zero on EOF");
      exit(0);
    }

    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000') {
      continue; /* to prompt */
    }

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL) {
        break;
      }
    }
    /* assert i is number of tokens + 1 */
    run_in_bg = false;
    if (strcmp(v[i - 1], "&") == 0) {
      v[i - 1] = NULL;   // remove & if present
      run_in_bg = true;  // set run_in_bg flag
    }

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
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
      }
      default: /* code executed only by parent process */
      {
        if (!add_process(frkRtnVal, job_counter, run_in_bg)) {
          perror("maximum number of background processes exceeded");
        }
        job_counter++;
        if (run_in_bg) {
          // execute in background

        } else {
          wpid = wait(0);
          if (wpid == -1) {
            // perror("no children exist");
          }

          // REMOVE PRINTF STATEMENT BEFORE SUBMISSION
          // printf("%s done \n", v[0]);
        }
        break;
      }
    } /* switch */
  } /* while */
} /* main */
