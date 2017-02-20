#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "path.h"
#include "util.h"

void
cx_path_init (CxPath *path, const char *path_str, int path_len)
{
  path->len = path_len;
  memcpy (path->str, path_str, path->len);
  path->str[path->len] = '\0';
}

void
cx_path_init_copy (CxPath *dst, const CxPath *src)
{
  memcpy (dst, src, sizeof (*dst));
}

void
cx_path_init_parent_of (CxPath *path)
{
  int i = path->len - 1;

  while (path->str[i] != '/')
    --i;

  if (path->str[i] == '/')
  {
    if (i == 0)
    {
      /* we are in the root directory `/' */
      path->len    = 1;
      path->str[1] = '\0';
    }
    else
    {
      path->len            = i;
      path->str[path->len] = '\0';
    }
  }
}

void
cx_path_set_as_home_dir (CxPath *path)
{
  const char *home_dir = getenv ("HOME");

  if (home_dir && *home_dir)
    cx_path_init (path, home_dir, strlen (home_dir));
  else
    cx_die (0, "could not determine home directory");
}

void
cx_path_set_as_current_dir (CxPath *path)
{
  char buf[CX_PATHMAX];

  if (getcwd (buf, CX_PATHMAX))
    cx_path_init (path, buf, strlen (buf));
  else
    cx_die (errno, "could not determine the current working directory");
}

void
cx_path_dir_item (CxPath *path, const CxPath *parent, const char *name_str,
                  int name_len)
{
  if (!cx_path_is_root (parent))
  {
    path->len = parent->len + 1 + name_len;
    memcpy (path->str, parent->str, parent->len);
    path->str[parent->len] = '/';
    memcpy (path->str + (parent->len + 1), name_str, name_len);
    path->str[path->len] = '\0';
  }
  else
  {
    path->len    = name_len + 1;
    path->str[0] = '/';
    memcpy (&path->str[1], name_str, name_len);
    path->str[path->len] = '\0';
  }
}

bool
cx_path_is_root (const CxPath *path)
{
  return path->len == 1 && *path->str == '/';
}

bool
cx_path_is_home_dir (const CxPath *path)
{
  const char *home_dir = getenv ("HOME");

  if (home_dir && *home_dir)
    return cx_strneq (home_dir, strlen (home_dir), path->str, path->len);

  cx_die (0, "could not determine home directory");
  return false; /* suppress compiler warning */
}
