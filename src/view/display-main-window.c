﻿/*!
 * @brief プレイヤーのステータス処理 / status
 * @date 2018/09/25
 * @author
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke\n
 * This software may be copied and distributed for educational, research, and\n
 * not for profit purposes provided that this copyright and statement are\n
 * included in all such copies.\n
 * 2014 Deskull rearranged comment for Doxygen.
 */

#include "view/display-main-window.h"
#include "core/player-processor.h"
#include "core/stuff-handler.h"
#include "dungeon/dungeon.h"
#include "dungeon/quest.h"
#include "effect/effect-characteristics.h"
#include "effect/spells-effect-util.h"
#include "floor/floor-town.h"
#include "floor/floor.h"
#include "game-option/map-screen-options.h"
#include "game-option/special-options.h"
#include "grid/feature.h"
#include "grid/grid.h"
#include "io/input-key-acceptor.h"
#include "io/targeting.h"
#include "monster/monster-update.h"
#include "player/mimic-info-table.h"
#include "term/screen-processor.h"
#include "term/term-color-types.h"
#include "util/bit-flags-calculator.h"
#include "view/display-map.h"
#include "view/main-window-row-column.h"
#include "view/main-window-util.h"
#include "world/world.h"

/*!
 * todo ここにplayer_type を追加するとz-termに影響が行くので保留
 * @brief コンソールのリサイズに合わせてマップを再描画する /
 * Map resizing whenever the main term changes size
 * @return なし
 */
void resize_map()
{
    if (!current_world_ptr->character_dungeon)
        return;

    panel_row_max = 0;
    panel_col_max = 0;
    panel_row_min = p_ptr->current_floor_ptr->height;
    panel_col_min = p_ptr->current_floor_ptr->width;
    verify_panel(p_ptr);

    p_ptr->update |= (PU_TORCH | PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_MON_LITE);
    p_ptr->update |= (PU_MONSTERS);
    p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY);

    handle_stuff(p_ptr);
    Term_redraw();

    if (can_save)
        move_cursor_relative(p_ptr->y, p_ptr->x);

    Term_fresh();
}

/*!
 * @brief フォーカスを当てるべきマップ描画の基準座標を指定する（サブルーチン）
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param dy 変更先のフロアY座標
 * @param dx 変更先のフロアX座標
 * Handle a request to change the current panel
 * Return TRUE if the panel was changed.
 * Also used in do_cmd_locate
 * @return 実際に再描画が必要だった場合TRUEを返す
 */
bool change_panel(player_type *player_ptr, POSITION dy, POSITION dx)
{
    TERM_LEN wid, hgt;
    get_screen_size(&wid, &hgt);

    POSITION y = panel_row_min + dy * hgt / 2;
    POSITION x = panel_col_min + dx * wid / 2;

    floor_type *floor_ptr = player_ptr->current_floor_ptr;
    if (y > floor_ptr->height - hgt)
        y = floor_ptr->height - hgt;
    if (y < 0)
        y = 0;

    if (x > floor_ptr->width - wid)
        x = floor_ptr->width - wid;
    if (x < 0)
        x = 0;

    if ((y == panel_row_min) && (x == panel_col_min))
        return FALSE;

    panel_row_min = y;
    panel_col_min = x;
    panel_bounds_center();

    player_ptr->update |= (PU_MONSTERS);
    player_ptr->redraw |= (PR_MAP);
    handle_stuff(player_ptr);
    return TRUE;
}

/*!
 * @brief 現在のコンソール表示の縦横を返す。 /
 * Get term size and calculate screen size
 * @param wid_p コンソールの表示幅文字数を返す
 * @param hgt_p コンソールの表示行数を返す
 * @return なし
 */
void get_screen_size(TERM_LEN *wid_p, TERM_LEN *hgt_p)
{
    Term_get_size(wid_p, hgt_p);
    *hgt_p -= ROW_MAP + 2;
    *wid_p -= COL_MAP + 2;
    if (use_bigtile)
        *wid_p /= 2;
}

/*
 * Moves the cursor to a given MAP (y,x) location
 */
void move_cursor_relative(int row, int col)
{
    row -= panel_row_prt;
    Term_gotoxy(panel_col_of(col), row);
}

/*
 * print project path
 */
void print_path(player_type *player_ptr, POSITION y, POSITION x)
{
    int path_n;
    u16b path_g[512];
    byte default_color = TERM_SLATE;

    if (!display_path)
        return;
    if (project_length == -1)
        return;

    floor_type *floor_ptr = player_ptr->current_floor_ptr;
    path_n = project_path(player_ptr, path_g, (project_length ? project_length : MAX_RANGE), player_ptr->y, player_ptr->x, y, x, PROJECT_PATH | PROJECT_THRU);
    player_ptr->redraw |= (PR_MAP);
    handle_stuff(player_ptr);
    for (int i = 0; i < path_n; i++) {
        POSITION ny = GRID_Y(path_g[i]);
        POSITION nx = GRID_X(path_g[i]);
        grid_type *g_ptr = &floor_ptr->grid_array[ny][nx];
        if (panel_contains(ny, nx)) {
            TERM_COLOR a = default_color;
            SYMBOL_CODE c;

            TERM_COLOR ta = default_color;
            SYMBOL_CODE tc = '*';

            if (g_ptr->m_idx && floor_ptr->m_list[g_ptr->m_idx].ml) {
                map_info(player_ptr, ny, nx, &a, &c, &ta, &tc);

                if (!is_ascii_graphics(a))
                    a = default_color;
                else if (c == '.' && (a == TERM_WHITE || a == TERM_L_WHITE))
                    a = default_color;
                else if (a == default_color)
                    a = TERM_WHITE;
            }

            if (!use_graphics) {
                if (current_world_ptr->timewalk_m_idx)
                    a = TERM_DARK;
                else if (is_invuln(player_ptr) || player_ptr->timewalk)
                    a = TERM_WHITE;
                else if (player_ptr->wraith_form)
                    a = TERM_L_DARK;
            }

            c = '*';
            Term_queue_bigchar(panel_col_of(nx), ny - panel_row_prt, a, c, ta, tc);
        }

        if ((g_ptr->info & CAVE_MARK) && !cave_have_flag_grid(g_ptr, FF_PROJECT))
            break;

        if (nx == x && ny == y)
            default_color = TERM_L_DARK;
    }
}

/*
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(Y,X)" always implies "in_bounds2(Y,X)".
 */
bool panel_contains(POSITION y, POSITION x) { return (y >= panel_row_min) && (y <= panel_row_max) && (x >= panel_col_min) && (x <= panel_col_max); }
