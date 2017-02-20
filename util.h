#ifndef __CX_UTIL_H__
#define __CX_UTIL_H__

#include <stdbool.h>

typedef enum
{
  CX_LOG_STATUS_INFO,
  CX_LOG_STATUS_WARN,
  CX_LOG_STATUS_ERROR,
} CxLogStatus;

void cx_log (CxLogStatus status, const char *fmt, ...);

void cx_die (int errno_val, const char *fmt, ...);

bool cx_streq (const char *s1, const char *s2);
bool cx_strneq (const char *s1, int n1, const char *s2, int n2);

#endif /* __CX_UTIL_H__ */
