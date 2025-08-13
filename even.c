#include <stdio.h> // io
#include <stdlib.h> // malloc
#include <unistd.h> // sleep
#include <signal.h> // signals

int main(int argc, char** argv) {
  int even_nums_to_print;
  scanf("%d", &even_nums_to_print);

  for (int i=0; i<even_nums_to_print; i++) {
    printf("%d ", i*2);
  }
}