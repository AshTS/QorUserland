#include <libc/sys/syscalls.h>

#include <libc/stdio.h>

int main()
{
  char c;
  while (read(0, &c, 1) && c != 'q');
  return 0;
}