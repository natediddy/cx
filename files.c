#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "util.h"

#define BINARY_K_FACTOR CX_BYTE_C (1024)
#define BINARY_M_FACTOR CX_BYTE_C (1048576)
#define BINARY_G_FACTOR CX_BYTE_C (1073741824)
#define BINARY_T_FACTOR CX_BYTE_C (1099511627776)

#define METRIC_K_FACTOR CX_BYTE_C (1000)
#define METRIC_M_FACTOR CX_BYTE_C (1000000)
#define METRIC_G_FACTOR CX_BYTE_C (1000000000)
#define METRIC_T_FACTOR CX_BYTE_C (1000000000000)

#define B_SYMBOL "B"

#define BINARY_K_SYMBOL "Ki" B_SYMBOL
#define BINARY_M_SYMBOL "Mi" B_SYMBOL
#define BINARY_G_SYMBOL "Gi" B_SYMBOL
#define BINARY_T_SYMBOL "Ti" B_SYMBOL

#define METRIC_K_SYMBOL "k" B_SYMBOL
#define METRIC_M_SYMBOL "M" B_SYMBOL
#define METRIC_G_SYMBOL "G" B_SYMBOL
#define METRIC_T_SYMBOL "T" B_SYMBOL

#define UNKNOWN_TYPE_STR "unknown"
#define BLOCK_DEVICE_TYPE_STR "block dev"
#define CHAR_DEVICE_TYPE_STR "char dev"
#define DIRECTORY_TYPE_STR "dir"
#define FIFO_TYPE_STR "fifo"
#define FILE_TYPE_STR "file"
#define SOCKET_TYPE_STR "socket"
#define SYMLINK_TYPE_STR "symlink"

#define PARENT_ITEM_NAME "[..]"

extern CxSizeUnits g_size_units;
extern bool        g_include_hidden_files;

static void
set_type_str (char *buffer, int *len, CxFileType type)
{
  static const int unknown_len   = strlen (UNKNOWN_TYPE_STR);
  static const int block_dev_len = strlen (BLOCK_DEVICE_TYPE_STR);
  static const int char_dev_len  = strlen (CHAR_DEVICE_TYPE_STR);
  static const int dir_len       = strlen (DIRECTORY_TYPE_STR);
  static const int fifo_len      = strlen (FIFO_TYPE_STR);
  static const int file_len      = strlen (FILE_TYPE_STR);
  static const int socket_len    = strlen (SOCKET_TYPE_STR);
  static const int symlink_len   = strlen (SYMLINK_TYPE_STR);

#define __SET_TYPE_STR(__str, __len)                                          \
  *len = __len;                                                               \
  memcpy (buffer, __str, *len);                                               \
  buffer[*len] = '\0'

  switch (type)
  {
    case CX_FILE_TYPE_BLOCK_DEVICE:
      __SET_TYPE_STR (BLOCK_DEVICE_TYPE_STR, block_dev_len);
      break;
    case CX_FILE_TYPE_CHARACTER_DEVICE:
      __SET_TYPE_STR (CHAR_DEVICE_TYPE_STR, char_dev_len);
      break;
    case CX_FILE_TYPE_DIRECTORY:
      __SET_TYPE_STR (DIRECTORY_TYPE_STR, dir_len);
      break;
    case CX_FILE_TYPE_FIFO:
      __SET_TYPE_STR (FIFO_TYPE_STR, fifo_len);
      break;
    case CX_FILE_TYPE_FILE:
      __SET_TYPE_STR (FILE_TYPE_STR, file_len);
      break;
    case CX_FILE_TYPE_SOCKET:
      __SET_TYPE_STR (SOCKET_TYPE_STR, socket_len);
      break;
    case CX_FILE_TYPE_SYMLINK:
      __SET_TYPE_STR (SYMLINK_TYPE_STR, symlink_len);
      break;
    default:
      __SET_TYPE_STR (UNKNOWN_TYPE_STR, unknown_len);
      break;
  }

#undef __SET_TYPE_STR
}

static void
set_size_str (char *buffer, int *len, cx_byte_t bytes)
{
  if ((g_size_units == CX_SIZE_UNITS_BINARY && bytes < BINARY_K_FACTOR) ||
      (g_size_units == CX_SIZE_UNITS_METRIC && bytes < METRIC_K_FACTOR))
  {
    snprintf (buffer, CX_SMALL_BUFMAX, "%" CX_PRIbyte " " B_SYMBOL, bytes);
    *len = strlen (buffer);
    return;
  }

#define __FORMAT_SIZE_STR(__factor, __symbol)                                 \
  snprintf (buffer, CX_SMALL_BUFMAX, "%.1Lf " __symbol,                       \
            ((long double) bytes / (long double) __factor))

  switch (g_size_units)
  {
    case CX_SIZE_UNITS_BINARY:
      if (bytes < BINARY_M_FACTOR)
        __FORMAT_SIZE_STR (BINARY_K_FACTOR, BINARY_K_SYMBOL);
      else if (bytes < BINARY_G_FACTOR)
        __FORMAT_SIZE_STR (BINARY_M_FACTOR, BINARY_M_SYMBOL);
      else if (bytes < BINARY_T_FACTOR)
        __FORMAT_SIZE_STR (BINARY_G_FACTOR, BINARY_G_SYMBOL);
      else
        __FORMAT_SIZE_STR (BINARY_T_FACTOR, BINARY_T_SYMBOL);
      break;
    case CX_SIZE_UNITS_METRIC:
      if (bytes < METRIC_M_FACTOR)
        __FORMAT_SIZE_STR (METRIC_K_FACTOR, METRIC_K_SYMBOL);
      else if (bytes < METRIC_G_FACTOR)
        __FORMAT_SIZE_STR (METRIC_M_FACTOR, METRIC_M_SYMBOL);
      else if (bytes < METRIC_T_FACTOR)
        __FORMAT_SIZE_STR (METRIC_G_FACTOR, METRIC_G_SYMBOL);
      else
        __FORMAT_SIZE_STR (METRIC_T_FACTOR, METRIC_T_SYMBOL);
      break;
    default:;
  }

#undef __FORMAT_SIZE_STR

  *len = strlen (buffer);
}

static void
dir_item_file_info_set (CxDirItem *item, const CxPath *parent)
{
  cx_path_dir_item (&item->info.path, parent, item->name, item->name_len);

  memset (&item->info.st, 0, sizeof (struct stat));
  if (stat (item->info.path.str, &item->info.st) == 0)
  {
    if (S_ISBLK (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_BLOCK_DEVICE;
    else if (S_ISCHR (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_CHARACTER_DEVICE;
    else if (S_ISDIR (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_DIRECTORY;
    else if (S_ISFIFO (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_FIFO;
    else if (S_ISREG (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_FILE;
    else if (S_ISSOCK (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_SOCKET;
    else if (S_ISLNK (item->info.st.st_mode))
      item->info.type = CX_FILE_TYPE_SYMLINK;
    else
      item->info.type = CX_FILE_TYPE_UNKNOWN;
    set_type_str (item->info.type_str, &item->info.type_str_len,
                  item->info.type);
    set_size_str (item->info.size_str, &item->info.size_str_len,
                  (cx_byte_t) item->info.st.st_size);
  }
  else
    cx_die (errno, "failed to stat `%s'", item->info.path.str);
}

static int
num_children (const char *path)
{
  DIR *          dp;
  struct dirent *de;
  int            num;

  dp = opendir (path);
  if (!dp)
    cx_die (errno, "failed to open directory `%s'", path);

  num = 0;
  for (;;)
  {
    de = readdir (dp);
    if (!de)
    {
      if (errno != 0)
        cx_die (errno, "failed to read directory `%s'", path);
      break;
    }
    if (cx_streq (de->d_name, ".") || cx_streq (de->d_name, "..") ||
        (!g_include_hidden_files && *de->d_name == '.'))
      continue;
    ++num;
  }
  closedir (dp);
  return num;
}

static void
set_parent_item (CxDirItem *item)
{
  static const int parent_name_len = strlen (PARENT_ITEM_NAME);

  item->name_len = parent_name_len;
  memcpy (item->name, PARENT_ITEM_NAME, item->name_len);
  item->name[item->name_len] = '\0';
}

void
cx_dir_listing_init (CxDirListing *listing, CxPath *path)
{
  DIR *          dp;
  struct dirent *de;
  int            pos;

  listing->path = path;
  if (!cx_path_is_root (listing->path))
  {
    listing->total = num_children (listing->path->str) + 1;
    listing->list  = malloc (sizeof (CxDirItem) * listing->total);
    if (!listing->list)
      cx_die (errno, "failed to allocate memory");
    set_parent_item (&listing->list[0]);
    pos = 1;
  }
  else
  {
    listing->total = num_children (listing->path->str);
    listing->list  = malloc (sizeof (CxDirItem) * listing->total);
    if (!listing->list)
      cx_die (errno, "failed to allocate memory");
    pos = 0;
  }

  dp = opendir (listing->path->str);
  if (!dp)
    cx_die (errno, "failed to open directory `%s'", listing->path->str);

  for (;;)
  {
    de = readdir (dp);
    if (!de)
    {
      if (errno != 0)
        cx_die (errno, "failed to read directory `%s'", listing->path->str);
      break;
    }
    if (cx_streq (de->d_name, ".") || cx_streq (de->d_name, "..") ||
        (!g_include_hidden_files && *de->d_name == '.'))
      continue;
    listing->list[pos].name_len = de->d_namlen;
    memcpy (listing->list[pos].name, de->d_name, de->d_namlen);
    listing->list[pos].name[de->d_namlen] = '\0';
    dir_item_file_info_set (&listing->list[pos++], listing->path);
  }
  closedir (dp);
}

bool
cx_dir_listing_has_parent_item (const CxDirListing *listing)
{
  return !cx_path_is_root (listing->path);
}

void
cx_dir_listing_free (CxDirListing *listing)
{
  if (listing->list)
  {
    free (listing->list);
    listing->list = NULL;
  }

  listing->path  = NULL;
  listing->total = 0;
}
