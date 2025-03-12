#include <stdio.h>
#include <stdlib.h>
#include <math.h>
void main() {
  int* a =(int*)malloc(100*sizeof(int));
  for (int i = 0; i < 100; ++i) {
    a[i] = i;
  }
}

