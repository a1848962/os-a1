#include <signal.h>  // signals
#include <stdio.h>   // io
#include <stdlib.h>  // exit
#include <unistd.h>  // sleep

void handleSIGHUP(int sig) { printf("Ouch!"); }

void handleSIGINT(int sig) { printf("Yeah!"); }

int main(int argc, char** argv) {
  signal(SIGHUP, handleSIGHUP);
  signal(SIGINT, handleSIGINT);

  if (argc != 2) {
    printf("Enter 1 argument");
    exit(1);
  }

  int even_nums_to_print = atoi(argv[1]);

  for (int i = 0; i < even_nums_to_print; i++) {
    printf("%d\n", i * 2);
    sleep(5);
  }
}