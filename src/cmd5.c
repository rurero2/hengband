﻿/*!
 *  @file cmd5.c
 *  @brief プレイヤーの魔法に関するコマンドの実装 / Spell/Prayer commands
 *  @date 2014/01/02
 *  @author
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */


#include "angband.h"

/*!
 * @brief 領域魔法に応じて技能の名称を返す。
 * @param tval 魔法書のtval
 * @return 領域魔法の技能名称を保管した文字列ポインタ
 */
cptr spell_category_name(OBJECT_TYPE_VALUE tval)
{
	switch (tval)
	{
	case TV_HISSATSU_BOOK:
		return _("必殺技", "art");
	case TV_LIFE_BOOK:
		return _("祈り", "prayer");
	case TV_MUSIC_BOOK:
		return _("歌", "song");
	default:
		return _("呪文", "spell");
	}
}


bool select_the_force = FALSE;

/*!
 * @brief 領域魔法の閲覧、学習、使用選択するインターフェイス処理
 * Allow user to choose a spell/prayer from the given book.
 * @param sn 選択した魔法IDを返す参照ポインタ
 * @param prompt 魔法を利用する際の動詞表記
 * @param sval 魔道書のsval
 * @param learned 閲覧/使用選択ならばTRUE、学習処理ならFALSE
 * @param use_realm 魔法領域ID
 * @return
 * <pre>
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 * The "prompt" should be "cast", "recite", or "study"
 * The "known" should be TRUE for cast/pray, FALSE for study
 * </pre>
 */
static int get_spell(SPELL_IDX *sn, cptr prompt, OBJECT_SUBTYPE_VALUE sval, bool learned, REALM_IDX use_realm)
{
	int         i;
	SPELL_IDX   spell = -1;
	int         num = 0;
	int         ask = TRUE;
	MANA_POINT  need_mana;
	SPELL_IDX   spells[64];
	bool        flag, redraw, okay;
	char        choice;
	const magic_type  *s_ptr;
	char        out_val[160];
	cptr        p;
	COMMAND_CODE code;
#ifdef JP
	char jverb_buf[128];
#endif
	int menu_line = (use_menu ? 1 : 0);

	/* Get the spell, if available */
	if (repeat_pull(&code))
	{
		*sn = (SPELL_IDX)code;
		/* Verify the spell */
		if (spell_okay(*sn, learned, FALSE, use_realm))
		{
			/* Success */
			return (TRUE);
		}
	}

	p = spell_category_name(mp_ptr->spell_book);

	/* Extract spells */
	for (spell = 0; spell < 32; spell++)
	{
		/* Check for this spell */
		if ((fake_spell_flags[sval] & (1L << spell)))
		{
			/* Collect this spell */
			spells[num++] = spell;
		}
	}

	/* Assume no usable spells */
	okay = FALSE;

	/* Assume no spells available */
	(*sn) = -2;

	/* Check for "okay" spells */
	for (i = 0; i < num; i++)
	{
		/* Look for "okay" spells */
		if (spell_okay(spells[i], learned, FALSE, use_realm)) okay = TRUE;
	}

	/* No "okay" spells */
	if (!okay) return (FALSE);
	if (((use_realm) != p_ptr->realm1) && ((use_realm) != p_ptr->realm2) && (p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE)) return FALSE;
	if (((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE)) && !is_magic(use_realm)) return FALSE;
	if ((p_ptr->pclass == CLASS_RED_MAGE) && ((use_realm) != REALM_ARCANE) && (sval > 1)) return FALSE;

	/* Assume cancelled */
	*sn = (-1);

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Show choices */
	p_ptr->window |= (PW_SPELL);

	window_stuff();

	/* Build a prompt (accept all spells) */
#ifdef JP
	jverb( prompt, jverb_buf, JVERB_AND );
	(void) strnfmt(out_val, 78, "(%^s:%c-%c, '*'で一覧, ESCで中断) どの%sを%^sますか? ",
		p, I2A(0), I2A(num - 1), p, jverb_buf );
#else
	(void)strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit) %^s which %s? ",
		p, I2A(0), I2A(num - 1), prompt, p);
#endif

	/* Get a spell from the user */

	choice = (always_show_list || use_menu) ? ESCAPE : 1;
	while (!flag)
	{
		if (choice == ESCAPE) choice = ' '; 
		else if (!get_com(out_val, &choice, TRUE))break;

		if (use_menu && choice != ' ')
		{
			switch (choice)
			{
				case '0':
				{
					screen_load();
					return FALSE;
				}

				case '8':
				case 'k':
				case 'K':
				{
					menu_line += (num - 1);
					break;
				}

				case '2':
				case 'j':
				case 'J':
				{
					menu_line++;
					break;
				}

				case 'x':
				case 'X':
				case '\r':
				case '\n':
				{
					i = menu_line - 1;
					ask = FALSE;
					break;
				}
			}
			if (menu_line > num) menu_line -= num;
			/* Display a list of spells */
			print_spells(menu_line, spells, num, 1, 15, use_realm);
			if (ask) continue;
		}
		else
		{
			/* Request redraw */
			if ((choice == ' ') || (choice == '*') || (choice == '?'))
			{
				/* Show the list */
				if (!redraw)
				{
					/* Show list */
					redraw = TRUE;

					/* Save the screen */
					screen_save();

					/* Display a list of spells */
					print_spells(menu_line, spells, num, 1, 15, use_realm);
				}

				/* Hide the list */
				else
				{
					if (use_menu) continue;

					/* Hide list */
					redraw = FALSE;

					/* Restore the screen */
					screen_load();
				}

				/* Redo asking */
				continue;
			}


			/* Note verify */
			ask = (isupper(choice));

			/* Lowercase */
			if (ask) choice = (char)tolower(choice);

			/* Extract request */
			i = (islower(choice) ? A2I(choice) : -1);
		}

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell();
			continue;
		}

		/* Save the spell index */
		spell = spells[i];

		/* Require "okay" spells */
		if (!spell_okay(spell, learned, FALSE, use_realm))
		{
			bell();
#ifdef JP
			msg_format("その%sを%sことはできません。", p, prompt);
#else
			msg_format("You may not %s that %s.", prompt, p);
#endif

			continue;
		}

		/* Verify it */
		if (ask)
		{
			char tmp_val[160];

			/* Access the spell */
			if (!is_magic(use_realm))
			{
				s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
			}
			else
			{
				s_ptr = &mp_ptr->info[use_realm - 1][spell];
			}

			/* Extract mana consumption rate */
			if (use_realm == REALM_HISSATSU)
			{
				need_mana = s_ptr->smana;
			}
			else
			{
				need_mana = mod_need_mana(s_ptr->smana, spell, use_realm);
			}

			/* Prompt */
#ifdef JP
			jverb( prompt, jverb_buf, JVERB_AND );
			/* 英日切り替え機能に対応 */
			(void) strnfmt(tmp_val, 78, "%s(MP%d, 失敗率%d%%)を%sますか? ",
				do_spell(use_realm, spell, SPELL_NAME), need_mana,
				       spell_chance(spell, use_realm),jverb_buf);
#else
			(void)strnfmt(tmp_val, 78, "%^s %s (%d mana, %d%% fail)? ",
				prompt, do_spell(use_realm, spell, SPELL_NAME), need_mana,
				spell_chance(spell, use_realm));
#endif


			/* Belay that order */
			if (!get_check(tmp_val)) continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}


	/* Restore the screen */
	if (redraw) screen_load();


	/* Show choices */
	p_ptr->window |= (PW_SPELL);

	window_stuff();


	/* Abort if needed */
	if (!flag) return FALSE;

	/* Save the choice */
	(*sn) = spell;

	repeat_push((COMMAND_CODE)spell);

	/* Success */
	return TRUE;
}

/*!
 * @brief オブジェクトがプレイヤーが使用可能な魔道書かどうかを判定する
 * @param o_ptr 判定したいオブ会ジェクトの構造体参照ポインタ
 * @return 学習できる魔道書ならばTRUEを返す
 */
static bool item_tester_learn_spell(object_type *o_ptr)
{
	s32b choices = realm_choices2[p_ptr->pclass];

	if (p_ptr->pclass == CLASS_PRIEST)
	{
		if (is_good_realm(p_ptr->realm1))
		{
			choices &= ~(CH_DEATH | CH_DAEMON);
		}
		else
		{
			choices &= ~(CH_LIFE | CH_CRUSADE);
		}
	}

	if ((o_ptr->tval < TV_LIFE_BOOK) || (o_ptr->tval > (TV_LIFE_BOOK + MAX_REALM - 1))) return (FALSE);
	if ((o_ptr->tval == TV_MUSIC_BOOK) && (p_ptr->pclass == CLASS_BARD)) return (TRUE);
	else if (!is_magic(tval2realm(o_ptr->tval))) return FALSE;
	if ((REALM1_BOOK == o_ptr->tval) || (REALM2_BOOK == o_ptr->tval)) return (TRUE);
	if (choices & (0x0001 << (tval2realm(o_ptr->tval) - 1))) return (TRUE);
	return (FALSE);
}

/*!
 * @brief プレイヤーが魔道書を一冊も持っていないかを判定する
 * @return 魔道書を一冊も持っていないならTRUEを返す
 */
static bool player_has_no_spellbooks(void)
{
	int         i;
	object_type *o_ptr;

	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];
		if (o_ptr->k_idx && check_book_realm(o_ptr->tval, o_ptr->sval)) return FALSE;
	}

	for (i = cave[p_ptr->y][p_ptr->x].o_idx; i; i = o_ptr->next_o_idx)
	{
		o_ptr = &o_list[i];
		if (o_ptr->k_idx && (o_ptr->marked & OM_FOUND) && check_book_realm(o_ptr->tval, o_ptr->sval)) return FALSE;
	}

	return TRUE;
}

/*!
 * @brief プレイヤーの職業が練気術師の時、領域魔法と練気術を切り換える処理のインターフェイス
 * @param browse_only 魔法と技能の閲覧を行うならばTRUE
 * @return 魔道書を一冊も持っていないならTRUEを返す
 */
static void confirm_use_force(bool browse_only)
{
	char which;
	COMMAND_CODE code;

	/* Get the item index */
	if (repeat_pull(&code) && (code == INVEN_FORCE))
	{
		browse_only ? do_cmd_mind_browse() : do_cmd_mind();
		return;
	}

	/* Show the prompt */
	prt(_("('w'練気術, ESC) 'w'かESCを押してください。 ", "(w for the Force, ESC) Hit 'w' or ESC. "), 0, 0);

	while (1)
	{
		/* Get a key */
		which = inkey();

		if (which == ESCAPE) break;
		else if (which == 'w')
		{

			repeat_push(INVEN_FORCE);

			break;
		}
	}

	/* Clear the prompt line */
	prt("", 0, 0);

	if (which == 'w')
	{
		browse_only ? do_cmd_mind_browse() : do_cmd_mind();
	}
}


/*!
 * @brief プレイヤーの魔法と技能を閲覧するコマンドのメインルーチン /
 * Peruse the spells/prayers in a book
 * @return なし
 * @details
 * <pre>
 * Note that *all* spells in the book are listed
 *
 * Note that browsing is allowed while confused or blind,
 * and in the dark, primarily to allow browsing in stores.
 * </pre>
 */
void do_cmd_browse(void)
{
	OBJECT_IDX item;
	OBJECT_SUBTYPE_VALUE sval;
	REALM_IDX use_realm = 0;
	int j, line;
	SPELL_IDX spell = -1;
	int num = 0;

	SPELL_IDX spells[64];
	char temp[62*4];

	object_type	*o_ptr;

	cptr q, s;

	/* Warriors are illiterate */
	if (!(p_ptr->realm1 || p_ptr->realm2) && (p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE))
	{
		msg_print(_("本を読むことができない！", "You cannot read books!"));
		return;
	}

	if (p_ptr->special_defense & KATA_MUSOU)
	{
		set_action(ACTION_NONE);
	}

	if (p_ptr->pclass == CLASS_FORCETRAINER)
	{
		if (player_has_no_spellbooks())
		{
			confirm_use_force(TRUE);
			return;
		}
		select_the_force = TRUE;
	}

	/* Restrict choices to "useful" books */
	if (p_ptr->realm2 == REALM_NONE) item_tester_tval = mp_ptr->spell_book;
	else item_tester_hook = item_tester_learn_spell;

	q = _("どの本を読みますか? ", "Browse which book? ");
	s = _("読める本がない。", "You have no books that you can read.");

	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR)))
	{
		select_the_force = FALSE;
		return;
	}
	select_the_force = FALSE;

	if (item == INVEN_FORCE) /* the_force */
	{
		do_cmd_mind_browse();
		return;
	}

	/* Get the item (in the pack) */
	else if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Access the item's sval */
	sval = o_ptr->sval;

	use_realm = tval2realm(o_ptr->tval);

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);
	handle_stuff();


	/* Extract spells */
	for (spell = 0; spell < 32; spell++)
	{
		/* Check for this spell */
		if ((fake_spell_flags[sval] & (1L << spell)))
		{
			/* Collect this spell */
			spells[num++] = spell;
		}
	}


	/* Save the screen */
	screen_save();

	/* Clear the top line */
	prt("", 0, 0);

	/* Keep browsing spells.  Exit browsing on cancel. */
	while(TRUE)
	{
		/* Ask for a spell, allow cancel */
		if (!get_spell(&spell, _("読む", "browse"), o_ptr->sval, TRUE, use_realm))
		{
			/* If cancelled, leave immediately. */
			if (spell == -1) break;

			/* Display a list of spells */
			print_spells(0, spells, num, 1, 15, use_realm);

			/* Notify that there's nothing to see, and wait. */
			if (use_realm == REALM_HISSATSU)
				prt(_("読める技がない。", "No techniques to browse."), 0, 0);
			else
				prt(_("読める呪文がない。", "No spells to browse."), 0, 0);
			(void)inkey();


			/* Restore the screen */
			screen_load();

			return;
		}

		/* Clear lines, position cursor  (really should use strlen here) */
		Term_erase(14, 14, 255);
		Term_erase(14, 13, 255);
		Term_erase(14, 12, 255);
		Term_erase(14, 11, 255);

		roff_to_buf(do_spell(use_realm, spell, SPELL_DESC), 62, temp, sizeof(temp));

		for (j = 0, line = 11; temp[j]; j += 1 + strlen(&temp[j]))
		{
			prt(&temp[j], line, 15);
			line++;
		}
	}

	/* Restore the screen */
	screen_load();
}

/*!
 * @brief プレイヤーの第二魔法領域を変更する /
 * @param next_realm 変更先の魔法領域ID
 * @return なし
 */
static void change_realm2(CHARACTER_IDX next_realm)
{
	int i, j = 0;
	char tmp[80];

	for (i = 0; i < 64; i++)
	{
		p_ptr->spell_order[j] = p_ptr->spell_order[i];
		if (p_ptr->spell_order[i] < 32) j++;
	}
	for (; j < 64; j++)
		p_ptr->spell_order[j] = 99;

	for (i = 32; i < 64; i++)
	{
		p_ptr->spell_exp[i] = SPELL_EXP_UNSKILLED;
	}
	p_ptr->spell_learned2 = 0L;
	p_ptr->spell_worked2 = 0L;
	p_ptr->spell_forgotten2 = 0L;

	sprintf(tmp,_("魔法の領域を%sから%sに変更した。", "change magic realm from %s to %s."), realm_names[p_ptr->realm2], realm_names[next_realm]);
	do_cmd_write_nikki(NIKKI_BUNSHOU, 0, tmp);
	p_ptr->old_realm |= 1 << (p_ptr->realm2-1);
	p_ptr->realm2 = next_realm;

	p_ptr->notice |= (PN_REORDER);
	p_ptr->update |= (PU_SPELLS);
	handle_stuff();

	/* Load an autopick preference file */
	autopick_load_pref(FALSE);
}


/*!
 * @brief 魔法を学習するコマンドのメインルーチン /
 * Study a book to gain a new spell/prayer
 * @return なし
 */
void do_cmd_study(void)
{
	int	i;
	OBJECT_IDX item;
	OBJECT_SUBTYPE_VALUE sval;
	int	increment = 0;
	bool    learned = FALSE;

	/* Spells of realm2 will have an increment of +32 */
	SPELL_IDX spell = -1;

	cptr p = spell_category_name(mp_ptr->spell_book);

	object_type *o_ptr;

	cptr q, s;

	if (!p_ptr->realm1)
	{
		msg_print(_("本を読むことができない！", "You cannot read books!"));
		return;
	}

	if (p_ptr->blind || no_lite())
	{
		msg_print(_("目が見えない！", "You cannot see!"));
		return;
	}

	if (p_ptr->confused)
	{
		msg_print(_("混乱していて読めない！", "You are too confused!"));
		return;
	}

	if (!(p_ptr->new_spells))
	{
		msg_format(_("新しい%sを覚えることはできない！", "You cannot learn any new %ss!"), p);
		return;
	}

	if (p_ptr->special_defense & KATA_MUSOU)
	{
		set_action(ACTION_NONE);
	}

#ifdef JP
	if( p_ptr->new_spells < 10 ){
		msg_format("あと %d つの%sを学べる。", p_ptr->new_spells, p);
	}else{
		msg_format("あと %d 個の%sを学べる。", p_ptr->new_spells, p);
	}
#else
	msg_format("You can learn %d new %s%s.", p_ptr->new_spells, p,
		(p_ptr->new_spells == 1?"":"s"));
#endif

	msg_print(NULL);


	/* Restrict choices to "useful" books */
	if (p_ptr->realm2 == REALM_NONE) item_tester_tval = mp_ptr->spell_book;
	else item_tester_hook = item_tester_learn_spell;

	q = _("どの本から学びますか? ", "Study which book? ");
	s = _("読める本がない。", "You have no books that you can read.");

	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Access the item's sval */
	sval = o_ptr->sval;

	if (o_ptr->tval == REALM2_BOOK) increment = 32;
	else if (o_ptr->tval != REALM1_BOOK)
	{
		if (!get_check(_("本当に魔法の領域を変更しますか？", "Really, change magic realm? "))) return;
		change_realm2(tval2realm(o_ptr->tval));
		increment = 32;
	}

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);
	handle_stuff();

	/* Mage -- Learn a selected spell */
	if (mp_ptr->spell_book != TV_LIFE_BOOK)
	{
		/* Ask for a spell, allow cancel */
		if (!get_spell(&spell, _("学ぶ", "study"), sval, FALSE, o_ptr->tval - TV_LIFE_BOOK + 1) && (spell == -1)) return;
	}

	/* Priest -- Learn a random prayer */
	else
	{
		int k = 0;
		int gift = -1;

		/* Extract spells */
		for (spell = 0; spell < 32; spell++)
		{
			/* Check spells in the book */
			if ((fake_spell_flags[sval] & (1L << spell)))
			{
				/* Skip non "okay" prayers */
				if (!spell_okay(spell, FALSE, TRUE,
					(increment ? p_ptr->realm2 : p_ptr->realm1))) continue;

				/* Hack -- Prepare the randomizer */
				k++;

				/* Hack -- Apply the randomizer */
				if (one_in_(k)) gift = spell;
			}
		}

		/* Accept gift */
		spell = gift;
	}

	/* Nothing to study */
	if (spell < 0)
	{
		msg_format(_("その本には学ぶべき%sがない。", "You cannot learn any %ss in that book."), p);

		/* Abort */
		return;
	}

	if (increment) spell += increment;

	/* Learn the spell */
	if (spell < 32)
	{
		if (p_ptr->spell_learned1 & (1L << spell)) learned = TRUE;
		else p_ptr->spell_learned1 |= (1L << spell);
	}
	else
	{
		if (p_ptr->spell_learned2 & (1L << (spell - 32))) learned = TRUE;
		else p_ptr->spell_learned2 |= (1L << (spell - 32));
	}

	if (learned)
	{
		int max_exp = (spell < 32) ? SPELL_EXP_MASTER : SPELL_EXP_EXPERT;
		int old_exp = p_ptr->spell_exp[spell];
		int new_rank = EXP_LEVEL_UNSKILLED;
		cptr name = do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell%32, SPELL_NAME);

		if (old_exp >= max_exp)
		{
			msg_format(_("その%sは完全に使いこなせるので学ぶ必要はない。", "You don't need to study this %s anymore."), p);
			return;
		}
#ifdef JP
		if (!get_check(format("%sの%sをさらに学びます。よろしいですか？", name, p)))
#else
		if (!get_check(format("You will study a %s of %s again. Are you sure? ", p, name)))
#endif
		{
			return;
		}
		else if (old_exp >= SPELL_EXP_EXPERT)
		{
			p_ptr->spell_exp[spell] = SPELL_EXP_MASTER;
			new_rank = EXP_LEVEL_MASTER;
		}
		else if (old_exp >= SPELL_EXP_SKILLED)
		{
			if (spell >= 32) p_ptr->spell_exp[spell] = SPELL_EXP_EXPERT;
			else p_ptr->spell_exp[spell] += SPELL_EXP_EXPERT - SPELL_EXP_SKILLED;
			new_rank = EXP_LEVEL_EXPERT;
		}
		else if (old_exp >= SPELL_EXP_BEGINNER)
		{
			p_ptr->spell_exp[spell] = SPELL_EXP_SKILLED + (old_exp - SPELL_EXP_BEGINNER) * 2 / 3;
			new_rank = EXP_LEVEL_SKILLED;
		}
		else
		{
			p_ptr->spell_exp[spell] = SPELL_EXP_BEGINNER + old_exp / 3;
			new_rank = EXP_LEVEL_BEGINNER;
		}
		msg_format(_("%sの熟練度が%sに上がった。", "Your proficiency of %s is now %s rank."), name, exp_level_str[new_rank]);
	}
	else
	{
		/* Find the next open entry in "p_ptr->spell_order[]" */
		for (i = 0; i < 64; i++)
		{
			/* Stop at the first empty space */
			if (p_ptr->spell_order[i] == 99) break;
		}

		/* Add the spell to the known list */
		p_ptr->spell_order[i++] = spell;

		/* Mention the result */
#ifdef JP
		/* 英日切り替え機能に対応 */
		if (mp_ptr->spell_book == TV_MUSIC_BOOK)
		{
			msg_format("%sを学んだ。",
				    do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell % 32, SPELL_NAME));
		}
		else
		{
			msg_format("%sの%sを学んだ。",
				    do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell % 32, SPELL_NAME) ,p);
		}
#else
		msg_format("You have learned the %s of %s.",
			p, do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell % 32, SPELL_NAME));
#endif
	}

	p_ptr->energy_use = 100;

	switch (mp_ptr->spell_book)
	{
	case TV_LIFE_BOOK:
		chg_virtue(V_FAITH, 1);
		break;
	case TV_DEATH_BOOK:
		chg_virtue(V_UNLIFE, 1);
		break;
	case TV_NATURE_BOOK:
		chg_virtue(V_NATURE, 1);
		break;
	default:
		chg_virtue(V_KNOWLEDGE, 1);
		break;
	}

	sound(SOUND_STUDY);

	/* One less spell available */
	p_ptr->learned_spells++;
#
	/* Update Study */
	p_ptr->update |= (PU_SPELLS);
	update_stuff();

	/* Redraw object recall */
	p_ptr->window |= (PW_OBJECT);
}


/*!
 * @brief 魔法を詠唱するコマンドのメインルーチン /
 * Cast a spell
 * @return なし
 */
void do_cmd_cast(void)
{
	OBJECT_IDX item;
	OBJECT_SUBTYPE_VALUE sval;
	SPELL_IDX spell;
	REALM_IDX realm;
	int	chance;
	int	increment = 0;
	REALM_IDX use_realm;
	MANA_POINT need_mana;

	cptr prayer;
	object_type	*o_ptr;
	const magic_type *s_ptr;
	cptr q, s;

	bool over_exerted = FALSE;

	/* Require spell ability */
	if (!p_ptr->realm1 && (p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE))
	{
		msg_print(_("呪文を唱えられない！", "You cannot cast spells!"));
		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite())
	{
		if (p_ptr->pclass == CLASS_FORCETRAINER) confirm_use_force(FALSE);
		else
		{
			msg_print(_("目が見えない！", "You cannot see!"));
			flush();
		}
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print(_("混乱していて唱えられない！", "You are too confused!"));
		flush();
		return;
	}

	/* Hex */
	if (p_ptr->realm1 == REALM_HEX)
	{
		if (hex_spell_fully())
		{
			bool flag = FALSE;
			msg_print(_("これ以上新しい呪文を詠唱することはできない。", "Can not spell new spells more."));
			flush();
			if (p_ptr->lev >= 35) flag = stop_hex_spell();
			if (!flag) return;
		}
	}

	if (p_ptr->pclass == CLASS_FORCETRAINER)
	{
		if (player_has_no_spellbooks())
		{
			confirm_use_force(FALSE);
			return;
		}
		select_the_force = TRUE;
	}

	prayer = spell_category_name(mp_ptr->spell_book);

	/* Restrict choices to spell books */
	item_tester_tval = mp_ptr->spell_book;

	q = _("どの呪文書を使いますか? ", "Use which book? ");
	s = _("呪文書がない！", "You have no spell books!");

	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR)))
	{
		select_the_force = FALSE;
		return;
	}
	select_the_force = FALSE;

	if (item == INVEN_FORCE) /* the_force */
	{
		do_cmd_mind();
		return;
	}

	/* Get the item (in the pack) */
	else if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Access the item's sval */
	sval = o_ptr->sval;

	if ((p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE) && (o_ptr->tval == REALM2_BOOK)) increment = 32;


	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);
	handle_stuff();

	if ((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE))
		realm = o_ptr->tval - TV_LIFE_BOOK + 1;
	else if (increment) realm = p_ptr->realm2;
	else realm = p_ptr->realm1;

	/* Ask for a spell */
#ifdef JP
	if (!get_spell(&spell, ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "詠唱する" : (mp_ptr->spell_book == TV_MUSIC_BOOK) ? "歌う" : "唱える"), 
			sval, TRUE, realm))
	{
		if (spell == -2) msg_format("その本には知っている%sがない。", prayer);
		return;
	}
#else
	if (!get_spell(&spell, ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "recite" : "cast"),
		sval, TRUE, realm))
	{
		if (spell == -2)
			msg_format("You don't know any %ss in that book.", prayer);
		return;
	}
#endif


	use_realm = tval2realm(o_ptr->tval);

	/* Hex */
	if (use_realm == REALM_HEX)
	{
		if (hex_spelling(spell))
		{
			msg_print(_("その呪文はすでに詠唱中だ。", "You are already casting it."));
			return;
		}
	}

	if (!is_magic(use_realm))
	{
		s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
	}
	else
	{
		s_ptr = &mp_ptr->info[realm - 1][spell];
	}

	/* Extract mana consumption rate */
	need_mana = mod_need_mana(s_ptr->smana, spell, realm);

	/* Verify "dangerous" spells */
	if (need_mana > p_ptr->csp)
	{
		if (flush_failure) flush();

		/* Warning */
#ifdef JP
		msg_format("その%sを%sのに十分なマジックポイントがない。",prayer,
			 ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "詠唱する" : (mp_ptr->spell_book == TV_LIFE_BOOK) ? "歌う" : "唱える"));
#else
		msg_format("You do not have enough mana to %s this %s.",
			((mp_ptr->spell_book == TV_LIFE_BOOK) ? "recite" : "cast"),
			prayer);
#endif


		if (!over_exert) return;

		/* Verify */
		if (!get_check_strict(_("それでも挑戦しますか? ", "Attempt it anyway? "), CHECK_OKAY_CANCEL)) return;
	}

	/* Spell failure chance */
	chance = spell_chance(spell, use_realm);

	/* Sufficient mana */
	if (need_mana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= need_mana;
	}
	else over_exerted = TRUE;
	p_ptr->redraw |= (PR_MANA);

	/* Failed spell */
	if (randint0(100) < chance)
	{
		if (flush_failure) flush();
		
		msg_format(_("%sをうまく唱えられなかった！", "You failed to get the %s off!"), prayer);
		sound(SOUND_FAIL);

		switch (realm)
		{
		case REALM_LIFE:
			if (randint1(100) < chance) chg_virtue(V_VITALITY, -1);
			break;
		case REALM_DEATH:
			if (randint1(100) < chance) chg_virtue(V_UNLIFE, -1);
			break;
		case REALM_NATURE:
			if (randint1(100) < chance) chg_virtue(V_NATURE, -1);
			break;
		case REALM_DAEMON:
			if (randint1(100) < chance) chg_virtue(V_JUSTICE, 1);
			break;
		case REALM_CRUSADE:
			if (randint1(100) < chance) chg_virtue(V_JUSTICE, -1);
			break;
		case REALM_HEX:
			if (randint1(100) < chance) chg_virtue(V_COMPASSION, -1);
			break;
		default:
			if (randint1(100) < chance) chg_virtue(V_KNOWLEDGE, -1);
			break;
		}

		/* Failure casting may activate some side effect */
		do_spell(realm, spell, SPELL_FAIL);


		if ((o_ptr->tval == TV_CHAOS_BOOK) && (randint1(100) < spell))
		{
			msg_print(_("カオス的な効果を発生した！", "You produce a chaotic effect!"));
			wild_magic(spell);
		}
		else if ((o_ptr->tval == TV_DEATH_BOOK) && (randint1(100) < spell))
		{
			if ((sval == 3) && one_in_(2))
			{
				sanity_blast(0, TRUE);
			}
			else
			{
				msg_print(_("痛い！", "It hurts!"));
				take_hit(DAMAGE_LOSELIFE, damroll(o_ptr->sval + 1, 6), _("暗黒魔法の逆流", "a miscast Death spell"), -1);

				if ((spell > 15) && one_in_(6) && !p_ptr->hold_exp)
					lose_exp(spell * 250);
			}
		}
		else if ((o_ptr->tval == TV_MUSIC_BOOK) && (randint1(200) < spell))
		{
			msg_print(_("いやな音が響いた", "An infernal sound echoed."));
			aggravate_monsters(0);
		}
		if (randint1(100) >= chance)
			chg_virtue(V_CHANCE,-1);
	}

	/* Process spell */
	else
	{
		/* Canceled spells cost neither a turn nor mana */
		if (!do_spell(realm, spell, SPELL_CAST)) return;

		if (randint1(100) < chance)
			chg_virtue(V_CHANCE,1);

		/* A spell was cast */
		if (!(increment ?
		    (p_ptr->spell_worked2 & (1L << spell)) :
		    (p_ptr->spell_worked1 & (1L << spell)))
		    && (p_ptr->pclass != CLASS_SORCERER)
		    && (p_ptr->pclass != CLASS_RED_MAGE))
		{
			int e = s_ptr->sexp;

			/* The spell worked */
			if (realm == p_ptr->realm1)
			{
				p_ptr->spell_worked1 |= (1L << spell);
			}
			else
			{
				p_ptr->spell_worked2 |= (1L << spell);
			}

			/* Gain experience */
			gain_exp(e * s_ptr->slevel);

			/* Redraw object recall */
			p_ptr->window |= (PW_OBJECT);

			switch (realm)
			{
			case REALM_LIFE:
				chg_virtue(V_TEMPERANCE, 1);
				chg_virtue(V_COMPASSION, 1);
				chg_virtue(V_VITALITY, 1);
				chg_virtue(V_DILIGENCE, 1);
				break;
			case REALM_DEATH:
				chg_virtue(V_UNLIFE, 1);
				chg_virtue(V_JUSTICE, -1);
				chg_virtue(V_FAITH, -1);
				chg_virtue(V_VITALITY, -1);
				break;
			case REALM_DAEMON:
				chg_virtue(V_JUSTICE, -1);
				chg_virtue(V_FAITH, -1);
				chg_virtue(V_HONOUR, -1);
				chg_virtue(V_TEMPERANCE, -1);
				break;
			case REALM_CRUSADE:
				chg_virtue(V_FAITH, 1);
				chg_virtue(V_JUSTICE, 1);
				chg_virtue(V_SACRIFICE, 1);
				chg_virtue(V_HONOUR, 1);
				break;
			case REALM_NATURE:
				chg_virtue(V_NATURE, 1);
				chg_virtue(V_HARMONY, 1);
				break;
			case REALM_HEX:
				chg_virtue(V_JUSTICE, -1);
				chg_virtue(V_FAITH, -1);
				chg_virtue(V_HONOUR, -1);
				chg_virtue(V_COMPASSION, -1);
				break;
			default:
				chg_virtue(V_KNOWLEDGE, 1);
				break;
			}
		}
		switch (realm)
		{
		case REALM_LIFE:
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_TEMPERANCE, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_COMPASSION, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_VITALITY, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_DILIGENCE, 1);
			break;
		case REALM_DEATH:
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_UNLIFE, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_JUSTICE, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_FAITH, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_VITALITY, -1);
			break;
		case REALM_DAEMON:
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_JUSTICE, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_FAITH, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_HONOUR, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_TEMPERANCE, -1);
			break;
		case REALM_CRUSADE:
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_FAITH, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_JUSTICE, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_SACRIFICE, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_HONOUR, 1);
			break;
		case REALM_NATURE:
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_NATURE, 1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_HARMONY, 1);
			break;
		case REALM_HEX:
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_JUSTICE, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_FAITH, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_HONOUR, -1);
			if (randint1(100 + p_ptr->lev) < need_mana) chg_virtue(V_COMPASSION, -1);
			break;
		}
		if (mp_ptr->spell_xtra & MAGIC_GAIN_EXP)
		{
			s16b cur_exp = p_ptr->spell_exp[(increment ? 32 : 0)+spell];
			s16b exp_gain = 0;

			if (cur_exp < SPELL_EXP_BEGINNER)
				exp_gain += 60;
			else if (cur_exp < SPELL_EXP_SKILLED)
			{
				if ((dun_level > 4) && ((dun_level + 10) > p_ptr->lev))
					exp_gain = 8;
			}
			else if (cur_exp < SPELL_EXP_EXPERT)
			{
				if (((dun_level + 5) > p_ptr->lev) && ((dun_level + 5) > s_ptr->slevel))
					exp_gain = 2;
			}
			else if ((cur_exp < SPELL_EXP_MASTER) && !increment)
			{
				if (((dun_level + 5) > p_ptr->lev) && (dun_level > s_ptr->slevel))
					exp_gain = 1;
			}
			p_ptr->spell_exp[(increment ? 32 : 0) + spell] += exp_gain;
		}
	}

	p_ptr->energy_use = 100;


	/* Over-exert the player */
	if(over_exerted)
	{
		int oops = need_mana;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		msg_print(_("精神を集中しすぎて気を失ってしまった！", "You faint from the effort!"));

		/* Hack -- Bypass free action */
		(void)set_paralyzed(p_ptr->paralyzed + randint1(5 * oops + 1));

		switch (realm)
		{
		case REALM_LIFE:
			chg_virtue(V_VITALITY, -10);
			break;
		case REALM_DEATH:
			chg_virtue(V_UNLIFE, -10);
			break;
		case REALM_DAEMON:
			chg_virtue(V_JUSTICE, 10);
			break;
		case REALM_NATURE:
			chg_virtue(V_NATURE, -10);
			break;
		case REALM_CRUSADE:
			chg_virtue(V_JUSTICE, -10);
			break;
		case REALM_HEX:
			chg_virtue(V_COMPASSION, 10);
			break;
		default:
			chg_virtue(V_KNOWLEDGE, -10);
			break;
		}

		/* Damage CON (possibly permanently) */
		if (randint0(100) < 50)
		{
			bool perm = (randint0(100) < 25);

			msg_print(_("体を悪くしてしまった！", "You have damaged your health!"));

			/* Reduce constitution */
			(void)dec_stat(A_CON, 15 + randint1(10), perm);
		}
	}

	p_ptr->window |= (PW_PLAYER);
	p_ptr->window |= (PW_SPELL);
}
