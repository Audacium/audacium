/**********************************************************************

  Audacium: A Digital Audio Editor

  Wrapper.c

  Audacium(R) is copyright (c) 2020-2020 Audacium Team.
  License: GPL v2.  See License.txt.

*******************************************************************//**

\file
Give the user more control over where libraries such as FFmpeg get
loaded from.

Since absolute pathnames are used when loading these libraries, the
normal search path would be DYLD_LIBRARY_PATH, absolute path,
DYLD_FALLBACK_LIBRARY_PATH.  This means that DYLD_LIBRARY_PATH can
override what the user actually wants.

So, we unset DYLD_LIBRARY_PATH variable and then invoke the main
executable.

*//*******************************************************************/

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char audacium[] = "Audacium";
extern char **environ;

int main(int argc, char *argv[])
{
   size_t len = strlen(argv[0]);
   char *path = alloca(len + sizeof(audacium)); // not precise, but we don't need it to be

   strcpy(path, argv[0]);

   char *slash = strrchr(path, '/');
   if (slash)
   {
      strcpy(++slash, audacium);
   }

   unsetenv("DYLD_LIBRARY_PATH");

   execve(path, argv, environ);
}
