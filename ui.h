#ifndef __CX_UI_H__
#define __CX_UI_H__

#include <stdbool.h>

#include "files.h"
#include "path.h"

void cx_ui_start (void);
void cx_ui_stop (void);

void cx_ui_draw (const CxDirListing *listing);

void cx_ui_handle_next_event (CxPath *location, const CxDirListing *listing);

void cx_ui_help_window (void);

int  cx_ui_hilighted_index (void);
void cx_ui_set_hilighted_index (int index);

int  cx_ui_first_listing_item_index (void);
void cx_ui_set_first_listing_item_index (int index);

bool cx_ui_keep_running (void);
void cx_ui_set_keep_running (bool keep_running);

#endif /* __CX_UI_H__ */
