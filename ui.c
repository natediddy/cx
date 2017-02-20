#include <signal.h>
#include <string.h>

#include <ncurses.h>

#include "ui.h"
#include "util.h"

#define ENTER_KEY 10
#define ESC_KEY 27

#define HELP_ITEM_PADDING 4
#define HELP_WINDOW_LINE_PADDING 2
#define HELP_WINDOW_COLUMN_PADDING 4

#define UP_ARROW_HELP_KEY "\u2191"
#define UP_ARROW_HELP_DESC "Move up"

#define DOWN_ARROW_HELP_KEY "\u2193"
#define DOWN_ARROW_HELP_DESC "Move down"

#define LEFT_ARROW_HELP_KEY "\u2190"
#define LEFT_ARROW_HELP_DESC "Change to parent directory"

#define RIGHT_ARROW_HELP_KEY "\u2192"
#define RIGHT_ARROW_HELP_DESC "Change to hilighted directory"

#define HOME_HELP_KEY "H"
#define HOME_HELP_DESC "Change to home directory"

#define INFO_HELP_KEY "i"
#define INFO_HELP_DESC "Display information about hilighted file"

#define UNITS_HELP_KEY "u"
#define UNITS_HELP_DESC "Toggle between binary and metric units for file sizes"

#define HIDDEN_HELP_KEY "."
#define HIDDEN_HELP_DESC "Toggle show hidden files"

#define EXIT_HELP_KEY "Esc"
#define EXIT_HELP_DESC "Exit program"

#define ENTER_HELP_KEY "Enter"
#define ENTER_HELP_DESC "Change to hilighted directory"

#define HELP_HELP_KEY "h"
#define HELP_HELP_DESC "Toggle this help window"

#define CURDIR_COLOR 1
#define HILIGHT_COLOR 2
#define PARENT_ITEM_COLOR 3

extern CxSizeUnits g_size_units;
extern bool        g_include_hidden_files;
extern bool        g_state_changed;

static struct
{
  int  first_listing_item;
  int  hilighted;
  int  height;
  int  width;
  int  listing_area_h;
  int  listing_area_w;
  bool running;
  bool keep_running;
} ui;

static void
resize_callback (int sig)
{
  endwin ();
  refresh ();
  clear ();
  getmaxyx (stdscr, ui.height, ui.width);

  ui.listing_area_h = ui.height - 1;
  ui.listing_area_w = ui.width;
}

void
cx_ui_start (void)
{
  struct sigaction sa;

  if (ui.running)
    return;

  if (!initscr ())
    cx_die (0, "failed to initialize Ncurses");

  raw ();
  noecho ();
  cbreak ();
  curs_set (0);
  keypad (stdscr, true);
  set_escdelay (25);

  if (has_colors ())
  {
    start_color ();
    use_default_colors ();
    init_pair (CURDIR_COLOR, COLOR_GREEN, -1);
    init_pair (HILIGHT_COLOR, COLOR_CYAN, -1);
    init_pair (PARENT_ITEM_COLOR, COLOR_BLUE, -1);
  }

  memset (&sa, 0, sizeof (struct sigaction));
  sa.sa_handler = resize_callback;
  sigaction (SIGWINCH, &sa, NULL);

  getmaxyx (stdscr, ui.height, ui.width);

  ui.listing_area_h     = ui.height - 1;
  ui.listing_area_w     = ui.width;
  ui.first_listing_item = 0;
  ui.hilighted          = -1;
  ui.running            = true;
  ui.keep_running       = true;
}

void
cx_ui_stop (void)
{
  if (!ui.running)
    return;

  endwin ();

  ui.running      = false;
  ui.keep_running = false;
}

void
cx_ui_draw (const CxDirListing *listing)
{
  CxDirItem *item;
  char       item_count_str[CX_SMALL_BUFMAX];
  int        item_count_str_len;
  int        i;
  int        y;
  int        x;
  int        info_len;
  int        info_start;
  bool       is_hilighted;

  clear ();

  snprintf (item_count_str, CX_SMALL_BUFMAX, "%d items", listing->total);
  item_count_str_len = strlen (item_count_str);

  attron (COLOR_PAIR (CURDIR_COLOR) | A_BOLD | A_UNDERLINE);
  mvaddnstr (0, 0, listing->path->str, listing->path->len);

  info_start = ui.width - item_count_str_len;
  for (x = listing->path->len; x < info_start; ++x)
    mvaddch (0, x, ' ');

  mvaddnstr (0, x, item_count_str, item_count_str_len);
  attroff (COLOR_PAIR (CURDIR_COLOR) | A_BOLD | A_UNDERLINE);

  if (ui.first_listing_item == 0 && cx_dir_listing_has_parent_item (listing))
  {
    x = 0;

    mvaddch (1, x++, ' ');

    attron (COLOR_PAIR (CURDIR_COLOR) | A_BOLD);
    if (listing->total > 1)
      mvaddch (1, x++, ACS_LTEE);
    else
      mvaddch (1, x++, ACS_LLCORNER);

    mvaddch (1, x++, ACS_HLINE);

    if (ui.hilighted == 0)
      mvaddch (1, x++, ACS_RARROW);
    attroff (COLOR_PAIR (CURDIR_COLOR) | A_BOLD);

    mvaddch (1, x++, ' ');

    if (ui.hilighted == 0)
      attron (COLOR_PAIR (HILIGHT_COLOR) | A_UNDERLINE);
    else
      attron (COLOR_PAIR (PARENT_ITEM_COLOR) | A_BOLD);

    mvaddnstr (1, x, listing->list[0].name, listing->list[0].name_len);
    x += listing->list[0].name_len;

    if (ui.hilighted == 0)
    {
      while (x < ui.width)
        mvaddch (1, x++, ' ');
      attroff (COLOR_PAIR (HILIGHT_COLOR) | A_UNDERLINE);
    }
    else
      attroff (COLOR_PAIR (PARENT_ITEM_COLOR) | A_BOLD);

    i = 1;
    y = 2;
  }
  else
  {
    i = ui.first_listing_item;
    y = 1;
  }

  for (; i < listing->total && y < ui.height; ++i, ++y)
  {
    item         = &listing->list[i];
    is_hilighted = (ui.hilighted == i);
    x            = 0;

    mvaddch (y, x++, ' ');

    attron (COLOR_PAIR (CURDIR_COLOR) | A_BOLD);
    if (i < (listing->total - 1))
      mvaddch (y, x++, ACS_LTEE);
    else
      mvaddch (y, x++, ACS_LLCORNER);

    mvaddch (y, x++, ACS_HLINE);

    if (is_hilighted)
      mvaddch (y, x++, ACS_RARROW);
    attroff (COLOR_PAIR (CURDIR_COLOR) | A_BOLD);

    mvaddch (y, x++, ' ');

    if (is_hilighted)
      attron (COLOR_PAIR (HILIGHT_COLOR) | A_UNDERLINE);

    mvaddnstr (y, x, item->name, item->name_len);
    x += item->name_len;

    info_len   = item->info.type_str_len + item->info.size_str_len + 5;
    info_start = ui.width - info_len;
    for (; x < info_start; ++x)
      mvaddch (y, x, ' ');

    mvaddch (y, x++, '(');
    mvaddnstr (y, x, item->info.type_str, item->info.type_str_len);
    x += item->info.type_str_len;
    mvaddch (y, x++, ')');

    mvaddch (y, x++, ' ');
    mvaddch (y, x++, '[');
    mvaddnstr (y, x, item->info.size_str, item->info.size_str_len);
    x += item->info.size_str_len;
    mvaddch (y, x, ']');

    if (is_hilighted)
      attroff (COLOR_PAIR (HILIGHT_COLOR) | A_UNDERLINE);
  }

  refresh ();
}

static void
show_help_window (void)
{
  static const struct
  {
    const char *key;
    const char *desc;
    int         key_len;
    int         desc_len;
    bool        unicode;
  } help[] = {
    { UP_ARROW_HELP_KEY, UP_ARROW_HELP_DESC, 1, strlen (UP_ARROW_HELP_DESC),
      true },

    { DOWN_ARROW_HELP_KEY, DOWN_ARROW_HELP_DESC, 1,
      strlen (DOWN_ARROW_HELP_DESC), true },

    { LEFT_ARROW_HELP_KEY, LEFT_ARROW_HELP_DESC, 1,
      strlen (LEFT_ARROW_HELP_DESC), true },

    { RIGHT_ARROW_HELP_KEY, RIGHT_ARROW_HELP_DESC, 1,
      strlen (RIGHT_ARROW_HELP_DESC), true },

    { HOME_HELP_KEY, HOME_HELP_DESC, strlen (HOME_HELP_KEY),
      strlen (HOME_HELP_DESC), false },

    { INFO_HELP_KEY, INFO_HELP_DESC, strlen (INFO_HELP_KEY),
      strlen (INFO_HELP_DESC), false },

    { UNITS_HELP_KEY, UNITS_HELP_DESC, strlen (UNITS_HELP_KEY),
      strlen (UNITS_HELP_DESC), false },

    { HIDDEN_HELP_KEY, HIDDEN_HELP_DESC, strlen (HIDDEN_HELP_KEY),
      strlen (HIDDEN_HELP_DESC), false },

    { EXIT_HELP_KEY, EXIT_HELP_DESC, strlen (EXIT_HELP_KEY),
      strlen (EXIT_HELP_DESC), false },

    { ENTER_HELP_KEY, ENTER_HELP_DESC, strlen (ENTER_HELP_KEY),
      strlen (ENTER_HELP_DESC), false },

    { HELP_HELP_KEY, HELP_HELP_DESC, strlen (HELP_HELP_KEY),
      strlen (HELP_HELP_DESC), false },

    { NULL, NULL, 0, 0, 0 },
  };

  WINDOW *win;
  int     y;
  int     x;
  int     i;
  int     n_longest_key  = 0;
  int     n_longest_desc = 0;
  int     n_lines        = 0;
  int     n_columns      = 0;

  for (i = 0; help[i].key && help[i].desc; ++i, ++n_lines)
  {
    if (help[i].key_len > n_longest_key)
      n_longest_key = help[i].key_len;

    if (help[i].desc_len > n_longest_desc)
      n_longest_desc = help[i].desc_len;
  }

  n_lines += HELP_WINDOW_LINE_PADDING * 2;
  n_columns = (n_longest_key + n_longest_desc + HELP_ITEM_PADDING +
               (HELP_WINDOW_COLUMN_PADDING * 2));

  win = newwin (n_lines, n_columns, (ui.height / 2) - (n_lines / 2),
                (ui.width / 2) - (n_columns / 2));
  if (!win)
    cx_die (0, "failed to create help window");

  wbkgd (win, A_BOLD | A_REVERSE);
  wborder (win, 0, 0, 0, 0, 0, 0, 0, 0);

  for (i = 0, y = HELP_WINDOW_LINE_PADDING; help[i].key; ++i, ++y)
  {
    x = HELP_WINDOW_COLUMN_PADDING;

    if (!help[i].unicode)
      mvwaddnstr (win, y, x, help[i].key, help[i].key_len);
    else
      mvwaddstr (win, y, x, help[i].key);

    x += help[i].key_len +
         ((n_longest_key - help[i].key_len) + HELP_ITEM_PADDING);

    mvwaddnstr (win, y, x, help[i].desc, help[i].desc_len);
  }

  wrefresh (win);

  for (;;)
    if (wgetch (win) == 'h')
      break;

  delwin (win);
}

static void
show_info_window (const CxFileInfo *info)
{
}

void
cx_ui_handle_next_event (CxPath *location, const CxDirListing *listing)
{
  switch (getch ())
  {
    case KEY_LEFT:
      if (cx_dir_listing_has_parent_item (listing))
      {
        cx_path_init_parent_of (location);
        g_state_changed = true;
        ui.hilighted    = 0;
      }
      break;

    case KEY_RIGHT:
    case ENTER_KEY:
      if (listing->list[ui.hilighted].info.type == CX_FILE_TYPE_DIRECTORY)
      {
        cx_path_init_copy (location, &listing->list[ui.hilighted].info.path);
        g_state_changed = true;
        ui.hilighted    = 0;
      }
      else if (ui.hilighted == 0 && cx_dir_listing_has_parent_item (listing))
      {
        cx_path_init_parent_of (location);
        g_state_changed = true;
        ui.hilighted    = 0;
      }
      break;

    case KEY_UP:
      if (listing->total > 0)
      {
        if (ui.hilighted == -1)
          ui.hilighted = 0;
        else if (ui.hilighted == 0)
        {
          ui.hilighted = listing->total - 1;
          if (ui.listing_area_h < listing->total)
            ui.first_listing_item = listing->total - ui.listing_area_h;
        }
        else
        {
          if (ui.listing_area_h < listing->total &&
              ui.hilighted == ui.first_listing_item)
            --ui.first_listing_item;
          --ui.hilighted;
        }
      }
      break;

    case KEY_DOWN:
      if (listing->total > 0)
      {
        if (ui.hilighted == -1)
          ui.hilighted = 0;
        else if (ui.hilighted == (listing->total - 1))
        {
          ui.hilighted = 0;
          if (ui.listing_area_h < listing->total)
            ui.first_listing_item = 0;
        }
        else
        {
          if (ui.listing_area_h < listing->total &&
              ui.hilighted == (ui.first_listing_item + ui.listing_area_h - 1))
            ++ui.first_listing_item;
          ++ui.hilighted;
        }
      }
      break;

    case 'h':
      show_help_window ();
      break;

    case 'H':
      if (!cx_path_is_home_dir (location))
      {
        cx_path_set_as_home_dir (location);
        g_state_changed = true;
      }
      break;

    case 'i':
      show_info_window (&listing->list[ui.hilighted].info);
      break;

    case 'u':
      g_size_units = (g_size_units == CX_SIZE_UNITS_BINARY)
                       ? CX_SIZE_UNITS_METRIC
                       : CX_SIZE_UNITS_BINARY;
      g_state_changed = true;
      break;

    case '.':
      g_include_hidden_files = !g_include_hidden_files;
      g_state_changed        = true;
      break;

    case ESC_KEY:
      ui.keep_running = false;
      break;

    default:;
  }
}

int
cx_ui_hilighted_index (void)
{
  return ui.hilighted;
}

void
cx_ui_set_hilighted_index (int index)
{
  ui.hilighted = index;
}

int
cx_ui_first_listing_item_index (void)
{
  return ui.first_listing_item;
}

void
cx_ui_set_first_listing_item_index (int index)
{
  ui.first_listing_item = index;
}

bool
cx_ui_keep_running (void)
{
  return ui.keep_running;
}

void
cx_ui_set_keep_running (bool keep_running)
{
  ui.keep_running = keep_running;
}
