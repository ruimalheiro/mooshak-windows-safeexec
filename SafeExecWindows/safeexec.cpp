#include <stdio.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "safeexec_windows.h"
#else
#include "safeexec_linux.h"
#endif

int main(int argc, char ** argv)
{
  return safe_execute(argc, argv);
}