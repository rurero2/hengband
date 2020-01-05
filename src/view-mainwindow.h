﻿#pragma once
#include "feature.h"

extern void get_screen_size(TERM_LEN *wid_p, TERM_LEN *hgt_p);
extern int panel_col_of(int col);
extern void apply_default_feat_lighting(TERM_COLOR f_attr[F_LIT_MAX], SYMBOL_CODE f_char[F_LIT_MAX]);
extern void prt_map(void);
extern void map_info(POSITION y, POSITION x, TERM_COLOR *ap, SYMBOL_CODE *cp, TERM_COLOR *tap, SYMBOL_CODE *tcp);
extern void display_map(floor_type *floor_ptr, int *cy, int *cx);
extern void do_cmd_view_map(void);

extern void health_track(MONSTER_IDX m_idx);
extern void prt_time(void);
extern concptr map_name(void);
extern void print_monster_list(floor_type *floor_ptr, TERM_LEN x, TERM_LEN y, TERM_LEN max_lines);
extern void move_cursor_relative(int row, int col);
extern void prt_path(floor_type *floor_ptr, POSITION y, POSITION x);
extern void monster_race_track(MONRACE_IDX r_idx);
extern void object_kind_track(KIND_OBJECT_IDX k_idx);
extern void resize_map(void);
extern void redraw_window(void);
extern bool change_panel(POSITION dy, POSITION dx);

extern void window_stuff(void);

#define ROW_MAP                 0
#define COL_MAP                 12


/*
 * Bit flags for the "redraw" variable
 */
extern void redraw_stuff(player_type *creature_ptr);

extern POSITION panel_row_min, panel_row_max;
extern POSITION panel_col_min, panel_col_max;
extern POSITION panel_col_prt, panel_row_prt;

/*
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(Y,X)" always implies "in_bounds2(Y,X)".
 */
#define panel_contains(Y,X) \
  (((Y) >= panel_row_min) && ((Y) <= panel_row_max) && \
   ((X) >= panel_col_min) && ((X) <= panel_col_max))

