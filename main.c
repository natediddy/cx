#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cx.h"
#include "files.h"
#include "ui.h"
#include "util.h"

const char *g_program_name;
CxSizeUnits g_size_units           = CX_SIZE_UNITS_BINARY;
bool        g_include_hidden_files = false;
bool        g_state_changed        = true;

static void
set_program_name (const char *argv0)
{
  const char *p;

  if (argv0 && *argv0)
  {
    p = strrchr (argv0, '/');
    if (p && *p && *(p + 1))
      g_program_name = p + 1;
    else
      g_program_name = argv0;
    return;
  }
  g_program_name = CX_PROGRAM_NAME;
}

static void
handle_state_change (CxPath *location, CxDirListing *listing)
{
  cx_dir_listing_free (listing);
  cx_dir_listing_init (listing, location);

  if (cx_ui_hilighted_index () >= listing->total)
    cx_ui_set_hilighted_index (listing->total - 1);

  if (cx_ui_first_listing_item_index () >= listing->total)
    cx_ui_set_first_listing_item_index (0);

  g_state_changed = false;
}

int
main (int argc, char **argv)
{
  CxDirListing listing;
  CxPath       location;

  set_program_name (argv[0]);
  setlocale (LC_ALL, "");

  if (argc > 2)
  {
    fprintf (stderr, "%s: error: too many arguments\n\n"
                     "Usage: %s [--help|--version] [DIRECTORY]\n",
             g_program_name, argv[0]);
    return EXIT_FAILURE;
  }
  else if (argc == 2)
  {
    if (cx_streq (argv[1], "-h") || cx_streq (argv[1], "--help"))
    {
      printf ("Usage: %s [DIRECTORY]\n"
              "Options:\n"
              "  -h, --help     Print this message and exit\n"
              "  -v, --version  Print version information and exit\n",
              argv[0]);
      return EXIT_SUCCESS;
    }
    else if (cx_streq (argv[1], "-v") || cx_streq (argv[1], "--version"))
    {
      printf ("%s %d.%d\n"
              "Written by Nathan Forbes (2017)\n",
              CX_PROGRAM_NAME, CX_VERSION_MAJOR, CX_VERSION_MINOR);
      return EXIT_SUCCESS;
    }
    else if (cx_streq (argv[1], "."))
      cx_path_set_as_current_dir (&location);
    else if (cx_streq (argv[1], "~"))
      cx_path_set_as_home_dir (&location);
    else
      cx_path_init (&location, argv[1], strlen (argv[1]));
  }
  else if (argc == 1)
    cx_path_set_as_home_dir (&location);

  cx_dir_listing_init (&listing, &location);
  cx_ui_start ();

  while (cx_ui_keep_running ())
  {
    if (g_state_changed)
      handle_state_change (&location, &listing);
    cx_ui_draw (&listing);
    cx_ui_handle_next_event (&location, &listing);
  }

  cx_dir_listing_free (&listing);
  cx_ui_stop ();
  return EXIT_SUCCESS;
}
