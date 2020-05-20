﻿#pragma once

#include "market/bounty-prize-table.h"

bounty_prize_type prize_list[MAX_BOUNTY] = {
    { TV_POTION, SV_POTION_CURING },
    { TV_POTION, SV_POTION_SPEED },
    { TV_POTION, SV_POTION_SPEED },
    { TV_POTION, SV_POTION_RESISTANCE },
    { TV_POTION, SV_POTION_ENLIGHTENMENT },

    { TV_POTION, SV_POTION_HEALING },
    { TV_POTION, SV_POTION_RESTORE_MANA },
    { TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION },
    { TV_POTION, SV_POTION_STAR_ENLIGHTENMENT },
    { TV_SCROLL, SV_SCROLL_SUMMON_PET },

    { TV_SCROLL, SV_SCROLL_GENOCIDE },
    { TV_POTION, SV_POTION_STAR_HEALING },
    { TV_POTION, SV_POTION_STAR_HEALING },
    { TV_POTION, SV_POTION_NEW_LIFE },
    { TV_SCROLL, SV_SCROLL_MASS_GENOCIDE },

    { TV_POTION, SV_POTION_LIFE },
    { TV_POTION, SV_POTION_LIFE },
    { TV_POTION, SV_POTION_AUGMENTATION },
    { TV_POTION, SV_POTION_INVULNERABILITY },
    { TV_SCROLL, SV_SCROLL_ARTIFACT },
};
