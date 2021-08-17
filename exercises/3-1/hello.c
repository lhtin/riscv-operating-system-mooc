#include <stdio.h>

/**
 * gcc -c hello.c -o hello.o
 * readelf -h hello.o
 * readelf -S hello.o
 * objdump -d hello.o
 */

void main() {
  printf("Hello World!\n");
}