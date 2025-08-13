#include <signal.h>  // signals
#include <stdio.h>   // io
#include <stdlib.h>  // exit
#include <unistd.h>  // sleep

// added newline after handler outputs to pass autograder
void handleSIGHUP(int sig) { printf("Ouch!\n"); }
void handleSIGINT(int sig) { printf("Yeah!\n"); }

int main(int argc, char** argv) {
  signal(SIGHUP, handleSIGHUP); // designate signal handlers
  signal(SIGINT, handleSIGINT);

  // validate number of arguments
  if (argc != 2) {
    printf("Enter a single integer");
    exit(1);
  }

  // parse argument to integer
  int even_nums_to_print = atoi(argv[1]);

  for (int i = 0; i < even_nums_to_print; i++) {
    // new line required to sleep after each print
    printf("%d\n", i * 2);
    sleep(5);
  }
}