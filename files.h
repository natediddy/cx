#ifndef __CX_FILES_H__
#define __CX_FILES_H__

#include <inttypes.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "cx.h"
#include "path.h"

#ifdef _DARWIN_FEATURE_64_BIT_INODE
#define CX_DIR_ITEM_NAME_MAX 1024
#else
#define CX_DIR_ITEM_NAME_MAX 256
#endif

#define CX_BYTE_C(n) UINT64_C (n)
#define CX_PRIbyte PRIu64

typedef uint64_t cx_byte_t;

typedef enum
{
  CX_SIZE_UNITS_BINARY,
  CX_SIZE_UNITS_METRIC,
} CxSizeUnits;

typedef enum
{
  CX_FILE_TYPE_UNKNOWN,
  CX_FILE_TYPE_BLOCK_DEVICE,
  CX_FILE_TYPE_CHARACTER_DEVICE,
  CX_FILE_TYPE_DIRECTORY,
  CX_FILE_TYPE_FIFO,
  CX_FILE_TYPE_FILE,
  CX_FILE_TYPE_SOCKET,
  CX_FILE_TYPE_SYMLINK
} CxFileType;

typedef struct
{
  struct stat st;
  CxPath      path;
  char        type_str[CX_SMALL_BUFMAX];
  char        size_str[CX_SMALL_BUFMAX];
  int         type_str_len;
  int         size_str_len;
  CxFileType  type;
} CxFileInfo;

typedef struct
{
  CxFileInfo info;
  char       name[CX_DIR_ITEM_NAME_MAX];
  int        name_len;
} CxDirItem;

typedef struct
{
  CxDirItem *   list;
  const CxPath *path;
  int           total;
} CxDirListing;

void cx_dir_listing_init (CxDirListing *listing, CxPath *path);
bool cx_dir_listing_has_parent_item (const CxDirListing *listing);
void cx_dir_listing_free (CxDirListing *listing);

#endif /* __CX_FILES_H__ */
