﻿#include "mspell/specified-summon.h"
#include "effect/effect-characteristics.h"
#include "floor/cave.h"
#include "floor/floor.h"
#include "monster-floor/monster-summon.h"
#include "monster-floor/place-monster-types.h"
#include "monster-race/monster-race.h"
#include "monster-race/race-indice-types.h"
#include "monster/monster-info.h"
#include "mspell/mspell-util.h"
#include "mspell/mspells1.h"
#include "spell-kind/spells-launcher.h"
#include "spell/process-effect.h"
#include "spell/spell-types.h"
#include "spell/spells-summon.h"
#include "view/display-messages.h"

/*!
 * @brief 鷹召喚の処理。 /
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param y 対象の地点のy座標
 * @param x 対象の地点のx座標
 * @param rlev 呪文を唱えるモンスターのレベル
 * @param m_idx 呪文を唱えるモンスターID
 * @return 召喚したモンスターの数を返す。
 */
MONSTER_NUMBER summon_EAGLE(player_type *target_ptr, POSITION y, POSITION x, int rlev, MONSTER_IDX m_idx)
{
    int count = 0;
    int num = 4 + randint1(3);
    for (int k = 0; k < num; k++) {
        count += summon_specific(target_ptr, m_idx, y, x, rlev, SUMMON_EAGLES, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE);
    }

    return count;
}

/*!
 * @brief インターネット・エクスプローダー召喚の処理。 /
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param y 対象の地点のy座標
 * @param x 対象の地点のx座標
 * @param rlev 呪文を唱えるモンスターのレベル
 * @param m_idx 呪文を唱えるモンスターID
 * @return 召喚したモンスターの数を返す。
 */
MONSTER_NUMBER summon_IE(player_type *target_ptr, POSITION y, POSITION x, int rlev, MONSTER_IDX m_idx)
{
    int count = 0;
    int num = 2 + randint1(1 + rlev / 20);
    for (int k = 0; k < num; k++) {
        count += summon_named_creature(target_ptr, m_idx, y, x, MON_IE, PM_NONE);
    }

    return count;
}

/*!
 * @brief ダンジョンの主召喚の処理。 /
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param y 対象の地点のy座標
 * @param x 対象の地点のx座標
 * @param rlev 呪文を唱えるモンスターのレベル
 * @param m_idx 呪文を唱えるモンスターID
 * @param t_idx 呪文を受けるモンスターID。プレイヤーの場合はdummyで0とする。
 * @param TARGET_TYPE プレイヤーを対象とする場合MONSTER_TO_PLAYER、モンスターを対象とする場合MONSTER_TO_MONSTER
 * @return 召喚したモンスターの数を返す。
 */
MONSTER_NUMBER summon_guardian(player_type *target_ptr, POSITION y, POSITION x, int rlev, MONSTER_IDX m_idx, MONSTER_IDX t_idx, int TARGET_TYPE)
{
    int num = 2 + randint1(3);
    bool mon_to_mon = (TARGET_TYPE == MONSTER_TO_MONSTER);
    bool mon_to_player = (TARGET_TYPE == MONSTER_TO_PLAYER);

    if (r_info[MON_JORMUNGAND].cur_num < r_info[MON_JORMUNGAND].max_num && one_in_(6)) {
        simple_monspell_message(target_ptr, m_idx, t_idx, _("地面から水が吹き出した！", "Water blew off from the ground!"),
            _("地面から水が吹き出した！", "Water blew off from the ground!"), TARGET_TYPE);

        if (mon_to_player)
            fire_ball_hide(target_ptr, GF_WATER_FLOW, 0, 3, 8);
        else if (mon_to_mon)
            project(target_ptr, t_idx, 8, y, x, 3, GF_WATER_FLOW, PROJECT_GRID | PROJECT_HIDE, -1);
    }

    int count = 0;
    for (int k = 0; k < num; k++) {
        count += summon_specific(target_ptr, m_idx, y, x, rlev, SUMMON_GUARDIANS, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE));
    }

    return count;
}

/*!
 * @brief ロックのクローン召喚の処理。 /
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param y 対象の地点のy座標
 * @param x 対象の地点のx座標
 * @param m_idx 呪文を唱えるモンスターID
 * @return 召喚したモンスターの数を返す。
 */
MONSTER_NUMBER summon_LOCKE_CLONE(player_type *target_ptr, POSITION y, POSITION x, MONSTER_IDX m_idx)
{
    int count = 0;
    int num = randint1(3);
    for (int k = 0; k < num; k++) {
        count += summon_named_creature(target_ptr, m_idx, y, x, MON_LOCKE_CLONE, PM_NONE);
    }

    return count;
}

/*!
 * @brief シラミ召喚の処理。 /
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param y 対象の地点のy座標
 * @param x 対象の地点のx座標
 * @param rlev 呪文を唱えるモンスターのレベル
 * @param m_idx 呪文を唱えるモンスターID
 * @return 召喚したモンスターの数を返す。
 */
MONSTER_NUMBER summon_LOUSE(player_type *target_ptr, POSITION y, POSITION x, int rlev, MONSTER_IDX m_idx)
{
    int count = 0;
    int num = 2 + randint1(3);
    for (int k = 0; k < num; k++) {
        count += summon_specific(target_ptr, m_idx, y, x, rlev, SUMMON_LOUSE, PM_ALLOW_GROUP);
    }

    return count;
}

MONSTER_NUMBER summon_MOAI(player_type *target_ptr, POSITION y, POSITION x, MONSTER_IDX m_idx)
{
    int count = 0;
    int num = 3 + randint1(3);
    for (int k = 0; k < num; k++)
        count += summon_named_creature(target_ptr, m_idx, y, x, MON_SMALL_MOAI, PM_NONE);

    return count;
}

MONSTER_NUMBER summon_DEMON_SLAYER(player_type *target_ptr, POSITION y, POSITION x, MONSTER_IDX m_idx)
{
    int count = 0;
    const int num = 5;
    for (int k = 0; k < num; k++)
        count += summon_named_creature(target_ptr, m_idx, y, x, MON_DEMON_SLAYER_MEMBER, PM_NONE);

    return count;
}

/*!
 * @brief ナズグル戦隊召喚の処理。 /
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param y 対象の地点のy座標
 * @param x 対象の地点のx座標
 * @param m_idx 呪文を唱えるモンスターID
 * @return 召喚したモンスターの数を返す。
 */
MONSTER_NUMBER summon_NAZGUL(player_type *target_ptr, POSITION y, POSITION x, MONSTER_IDX m_idx)
{
    BIT_FLAGS mode = 0L;
    POSITION cy = y;
    POSITION cx = x;
    GAME_TEXT m_name[MAX_NLEN];
    monster_name(target_ptr, m_idx, m_name);

    if (target_ptr->blind)
        msg_format(_("%^sが何かをつぶやいた。", "%^s mumbles."), m_name);
    else
        msg_format(_("%^sが魔法で幽鬼戦隊を召喚した！", "%^s magically summons rangers of Nazgul!"), m_name);

    msg_print(NULL);

    int count = 0;
    for (int k = 0; k < 30; k++) {
        if (!summon_possible(target_ptr, cy, cx) || !is_cave_empty_bold(target_ptr, cy, cx)) {
            int j;
            for (j = 100; j > 0; j--) {
                scatter(target_ptr, &cy, &cx, y, x, 2, 0);
                if (is_cave_empty_bold(target_ptr, cy, cx))
                    break;
            }

            if (!j)
                break;
        }

        if (!is_cave_empty_bold(target_ptr, cy, cx))
            continue;

        if (!summon_named_creature(target_ptr, m_idx, cy, cx, MON_NAZGUL, mode))
            continue;

        y = cy;
        x = cx;
        count++;
        if (count == 1)
            msg_format(_("「幽鬼戦隊%d号、ナズグル・ブラック！」", "A Nazgul says 'Nazgul-Rangers Number %d, Nazgul-Black!'"), count);
        else
            msg_format(_("「同じく%d号、ナズグル・ブラック！」", "Another one says 'Number %d, Nazgul-Black!'"), count);

        msg_print(NULL);
    }

    msg_format(_("「%d人そろって、リングレンジャー！」", "They say 'The %d meets! We are the Ring-Ranger!'."), count);
    msg_print(NULL);
    return count;
}
