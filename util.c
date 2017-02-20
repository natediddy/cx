#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cx.h"
#include "ui.h"
#include "util.h"

#define LOG_FILENAME "." CX_PROGRAM_NAME ".log"

extern const char *g_program_name;

void
cx_log (CxLogStatus status, const char *fmt, ...)
{
  FILE *  fp;
  va_list ap;

  fp = fopen (LOG_FILENAME, "a+");
  if (!fp)
    cx_die (errno, "failed to open log file");

  switch (status)
  {
    case CX_LOG_STATUS_INFO:
      fputs ("INFO: ", fp);
      break;
    case CX_LOG_STATUS_WARN:
      fputs ("WARNING: ", fp);
      break;
    case CX_LOG_STATUS_ERROR:
      fputs ("ERROR: ", fp);
      break;
    default:;
  }

  va_start (ap, fmt);
  vfprintf (fp, fmt, ap);
  va_end (ap);

  fputc ('\n', fp);
  fclose (fp);
}

void
cx_die (int errno_val, const char *fmt, ...)
{
  va_list ap;

  cx_ui_stop ();
  fprintf (stderr, "%s: error: ", g_program_name);

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  if (errno_val != 0)
    fprintf (stderr, " - %s", strerror (errno_val));

  fputc ('\n', stderr);
  exit (EXIT_FAILURE);
}

bool
cx_streq (const char *s1, const char *s2)
{
  int n = strlen (s1);
  return (n == strlen (s2) && memcmp (s1, s2, n) == 0);
}

bool
cx_strneq (const char *s1, int n1, const char *s2, int n2)
{
  return (n1 == n2 && memcmp (s1, s2, n1) == 0);
}
