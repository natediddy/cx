#ifndef __CX_PATH_H__
#define __CX_PATH_H__

#include <stdbool.h>
#include <sys/param.h>

#include "cx.h"

#define CX_PATHMAX MAXPATHLEN

typedef struct
{
  char str[CX_PATHMAX];
  int  len;
} CxPath;

void cx_path_init (CxPath *path, const char *path_str, int path_len);
void cx_path_init_copy (CxPath *dst, const CxPath *src);
void cx_path_init_parent_of (CxPath *path);

void cx_path_set_as_home_dir (CxPath *path);
void cx_path_set_as_current_dir (CxPath *path);

void cx_path_dir_item (CxPath *path, const CxPath *parent,
                       const char *name_str, int name_len);

bool cx_path_is_root (const CxPath *path);
bool cx_path_is_home_dir (const CxPath *path);

#endif /* __CX_PATH_H__ */
