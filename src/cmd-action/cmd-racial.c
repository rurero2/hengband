﻿#include "cmd-action/cmd-racial.h"
#include "action/action-limited.h"
#include "action/mutation-execution.h"
#include "core/asking-player.h"
#include "core/player-redraw-types.h"
#include "core/player-update-types.h"
#include "core/window-redrawer.h"
#include "game-option/text-display-options.h"
#include "io/command-repeater.h"
#include "io/input-key-requester.h"
#include "main/sound-of-music.h"
#include "mutation/mutation-flag-types.h"
#include "player/attack-defense-types.h"
#include "player/player-damage.h"
#include "player/special-defense-types.h"
#include "racial/class-racial-switcher.h"
#include "racial/mutation-racial-selector.h"
#include "racial/race-racial-switcher.h"
#include "racial/racial-switcher.h"
#include "racial/racial-util.h"
#include "status/action-setter.h"
#include "term/screen-processor.h"
#include "util/int-char-converter.h"

/*!
 * @brief レイシャル・パワーコマンドのメインルーチン / Allow user to choose a power (racial / mutation) to activate
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @return なし
 */
void do_cmd_racial_power(player_type *creature_ptr)
{
    if (creature_ptr->wild_mode)
        return;

    if (cmd_limit_confused(creature_ptr)) {
        free_turn(creature_ptr);
        return;
    }

    if (creature_ptr->special_defense & (KATA_MUSOU | KATA_KOUKIJIN))
        set_action(creature_ptr, ACTION_NONE);

    rc_type tmp_rc;
    rc_type *rc_ptr = initialize_rc_type(creature_ptr, &tmp_rc);
    switch_class_racial(creature_ptr, rc_ptr);
    if (creature_ptr->mimic_form)
        switch_mimic_racial(creature_ptr, rc_ptr);
    else
        switch_race_racial(creature_ptr, rc_ptr);

    select_mutation_racial(creature_ptr, rc_ptr);
    rc_ptr->flag = FALSE;
    rc_ptr->redraw = FALSE;

    (void)strnfmt(rc_ptr->out_val, 78,
        _("(特殊能力 %c-%c, *'で一覧, ESCで中断) どの特殊能力を使いますか？", "(Powers %c-%c, *=List, ESC=exit) Use which power? "),
        I2A(0), (rc_ptr->num <= 26) ? I2A(rc_ptr->num - 1) : '0' + rc_ptr->num - 27);

    if (!repeat_pull(&rc_ptr->command_code) || rc_ptr->command_code < 0 || rc_ptr->command_code >= rc_ptr->num) {
        if (use_menu)
            screen_save();

        rc_ptr->choice = (always_show_list || use_menu) ? ESCAPE : 1;
        while (!rc_ptr->flag) {
            if (rc_ptr->choice == ESCAPE)
                rc_ptr->choice = ' ';
            else if (!get_com(rc_ptr->out_val, &rc_ptr->choice, FALSE))
                break;

            if (use_menu && rc_ptr->choice != ' ') {
                switch (rc_ptr->choice) {
                case '0': {
                    screen_load();
                    free_turn(creature_ptr);
                    return;
                }

                case '8':
                case 'k':
                case 'K': {
                    rc_ptr->menu_line += (rc_ptr->num - 1);
                    break;
                }

                case '2':
                case 'j':
                case 'J': {
                    rc_ptr->menu_line++;
                    break;
                }

                case '6':
                case 'l':
                case 'L':
                case '4':
                case 'h':
                case 'H': {
                    if (rc_ptr->menu_line > 18)
                        rc_ptr->menu_line -= 18;
                    else if (rc_ptr->menu_line + 18 <= rc_ptr->num)
                        rc_ptr->menu_line += 18;
                    break;
                }

                case 'x':
                case 'X':
                case '\r': {
                    rc_ptr->command_code = rc_ptr->menu_line - 1;
                    rc_ptr->ask = FALSE;
                    break;
                }
                }
                if (rc_ptr->menu_line > rc_ptr->num)
                    rc_ptr->menu_line -= rc_ptr->num;
            }

            if ((rc_ptr->choice == ' ') || (rc_ptr->choice == '*') || (rc_ptr->choice == '?') || (use_menu && rc_ptr->ask)) {
                if (!rc_ptr->redraw || use_menu) {
                    byte y = 1, x = 0;
                    int ctr = 0;
                    char dummy[80];
                    char letter;
                    TERM_LEN x1, y1;
                    strcpy(dummy, "");
                    rc_ptr->redraw = TRUE;
                    if (!use_menu)
                        screen_save();

                    if (rc_ptr->num < 18)
                        prt(_("                            Lv   MP 失率", "                            Lv Cost Fail"), y++, x);
                    else
                        prt(_("                            Lv   MP 失率                            Lv   MP 失率",
                                "                            Lv Cost Fail                            Lv Cost Fail"),
                            y++, x);

                    while (ctr < rc_ptr->num) {
                        x1 = ((ctr < 18) ? x : x + 40);
                        y1 = ((ctr < 18) ? y + ctr : y + ctr - 18);
                        if (use_menu) {
                            if (ctr == (rc_ptr->menu_line - 1))
                                strcpy(dummy, _(" 》 ", " >  "));
                            else
                                strcpy(dummy, "    ");
                        } else {
                            if (ctr < 26)
                                letter = I2A(ctr);
                            else
                                letter = '0' + ctr - 26;
                            sprintf(dummy, " %c) ", letter);
                        }

                        strcat(dummy,
                            format("%-23.23s %2d %4d %3d%%", rc_ptr->power_desc[ctr].racial_name, rc_ptr->power_desc[ctr].min_level, rc_ptr->power_desc[ctr].cost,
                                100 - racial_chance(creature_ptr, &rc_ptr->power_desc[ctr])));
                        prt(dummy, y1, x1);
                        ctr++;
                    }
                } else {
                    rc_ptr->redraw = FALSE;
                    screen_load();
                }

                continue;
            }

            if (!use_menu) {
                if (rc_ptr->choice == '\r' && rc_ptr->num == 1)
                    rc_ptr->choice = 'a';

                if (isalpha(rc_ptr->choice)) {
                    rc_ptr->ask = (isupper(rc_ptr->choice));
                    if (rc_ptr->ask)
                        rc_ptr->choice = (char)tolower(rc_ptr->choice);

                    rc_ptr->command_code = (islower(rc_ptr->choice) ? A2I(rc_ptr->choice) : -1);
                } else {
                    rc_ptr->ask = FALSE;
                    rc_ptr->command_code = rc_ptr->choice - '0' + 26;
                }
            }

            if ((rc_ptr->command_code < 0) || (rc_ptr->command_code >= rc_ptr->num)) {
                bell();
                continue;
            }

            if (rc_ptr->ask) {
                char tmp_val[160];
                (void)strnfmt(tmp_val, 78, _("%sを使いますか？ ", "Use %s? "), rc_ptr->power_desc[rc_ptr->command_code].racial_name);
                if (!get_check(tmp_val))
                    continue;
            }

            rc_ptr->flag = TRUE;
        }

        if (rc_ptr->redraw)
            screen_load();

        if (!rc_ptr->flag) {
            free_turn(creature_ptr);
            return;
        }

        repeat_push(rc_ptr->command_code);
    }

    switch (racial_aux(creature_ptr, &rc_ptr->power_desc[rc_ptr->command_code])) {
    case 1:
        if (rc_ptr->power_desc[rc_ptr->command_code].number < 0)
            rc_ptr->cast = exe_racial_power(creature_ptr, rc_ptr->power_desc[rc_ptr->command_code].number);
        else
            rc_ptr->cast = exe_mutation_power(creature_ptr, rc_ptr->power_desc[rc_ptr->command_code].number);
        break;
    case 0:
        rc_ptr->cast = FALSE;
        break;
    case -1:
        rc_ptr->cast = TRUE;
        break;
    }

    if (!rc_ptr->cast) {
        free_turn(creature_ptr);
        return;
    }

    int racial_cost = rc_ptr->power_desc[rc_ptr->command_code].racial_cost;
    if (racial_cost == 0)
        return;

    int actual_racial_cost = racial_cost / 2 + randint1(racial_cost / 2);
    if (creature_ptr->csp < actual_racial_cost) {
        actual_racial_cost -= creature_ptr->csp;
        creature_ptr->csp = 0;
        take_hit(creature_ptr, DAMAGE_USELIFE, actual_racial_cost, _("過度の集中", "concentrating too hard"), -1);
    } else
        creature_ptr->csp -= actual_racial_cost;

    creature_ptr->redraw |= PR_HP | PR_MANA;
    creature_ptr->window |= PW_PLAYER | PW_SPELL;
}
