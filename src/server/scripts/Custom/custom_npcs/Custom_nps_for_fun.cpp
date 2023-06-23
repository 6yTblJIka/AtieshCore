/*
* Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "AchievementMgr.h"
#include "Chat.h"
#include "Creature.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "Item.h"
#include "Language.h"
#include "Log.h"
#include "Player.h"

struct Teleport
{
    uint32 menu_id, next_menu_id;
    uint8 icon;
    std::string name;
    uint32 cost;
    uint8 level, faction; // 0 Both, 1 Horde, 2 Ally
    uint32 map;
    float x, y, z, o;
};
typedef struct Teleport Data;

//{menu_id, next_menu_id, icon, name, cost, level, faction, map, x, y, z, o}
Data Tele[] = // key = i & Key
{
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportStormWind:25|t Stormwind", 0, 0, 2, 0, -8842.09f, 626.358f, 94.0867f, 3.61363f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportOrgrimmar:25|t Orgrimmar", 0, 0, 1, 1, 1601.08f, -4378.69f, 9.9846f, 2.14362f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportDarnassus:25|t Darnass", 0, 0, 2, 1, 9869.91f, 2493.58f, 1315.88f, 2.78897f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportIronForge:25|t Ironforge", 0, 0, 2, 0, -4900.47f, -962.585f, 501.455f, 5.40538f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportExodar:25|t Exodar", 0, 0, 2, 530, -3864.92f, -11643.7f, -137.644f, 5.50862f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportThunderBluff:25|t Thunder Bluff", 0, 0, 1, 1, -1274.45f, 71.8601f, 128.159f, 2.80623f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportUnderCity:25|t Undercity", 0, 0, 1, 0, 1633.75f, 240.167f, -43.1034f, 6.26128f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportSilvermoon:25|t Silvermoon", 0, 0, 1, 530, 9738.28f, -7454.19f, 13.5605f, 0.043914f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportDalaran:25|t Dalaran", 0, 67, 0, 571, 5809.55f, 503.975f, 657.526f, 2.38338f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportShattrath:25|t Shattrath", 0, 57, 0, 530, -1887.62f, 5359.09f, -12.4279f, 4.40435f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_BG_killingblow_most:25|t Booty-Bay", 0, 40, 0, 1, -7177.15f, -3785.34f, 8.36981f, 6.10237f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_BG_killingblow_most:25|t Ratchet", 0, 10, 0, 1, -956.664f, -3754.709f, 5.33239f, 0.99663f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Duel zone", 0, 80, 0, 37, 1194.983887f, 186.221924f, 356.908295f, 5.434835f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Arena Gurubashi", 0, 30, 0, 0, -13181.8f, 339.356f, 42.9805f, 1.18013f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Arena in Circle of Blood", 0, 57, 0, 530, 2839.44f, 5930.17f, 11.1002f, 3.16284f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Circle of Challenge", 0, 57, 0, 530, -1999.94f, 6581.71f, 11.32f, 2.36528f },
    { 1, 6, 3, "|TInterface/ICONS/INV_BannerPVP_02:25|t Eastern Kingdoms", 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 7, 3, "|TInterface/ICONS/INV_BannerPVP_01:25|t Kalimdor", 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 8, 3, "|TInterface/ICONS/Achievement_Dungeon_Outland_DungeonMaster:25|t Outland", 0, 58, 0, 0, 0, 0, 0, 0 },
    { 1, 9, 3, "|TInterface/ICONS/Achievement_Zone_BoreanTundra_03:25|t Northrend", 0, 68, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_Heroic:25|t Classic instances", 0, 15, 0, 0, 0, 0, 0, 0 },
    { 1, 3, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_10man:25|t BC instances", 0, 59, 0, 0, 0, 0, 0, 0 },
    { 1, 4, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_Normal:25|t WotLK instances", 0, 69, 0, 0, 0, 0, 0, 0 },
    { 1, 5, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_25man:25|t Raids", 0, 57, 0, 0, 0, 0, 0, 0 },
    //{ 1, 10, 9, "|TInterface/ICONS/Spell_fire_felfireward:25|t ?????? ?????????", 0, 80, 0, 0, 0, 0, 0, 0 },

    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_Mekgineer_Thermaplugg:25|t Gnomeregan |cffff0000(24-34)|r", 0, 25, 2, 0, -5163.54f, 925.423f, 257.181f, 1.57423f },
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_EdwinVancleef:25|t The Deadmines |cffff0000(15-21)|r", 0, 17, 2, 0, -11209.6f, 1666.54f, 24.6974f, 1.42053f },
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_Bazil_Thredd:25|t The Stockade |cffff0000(20-30)|r", 0, 22, 2, 0, -8799.15f, 832.718f, 97.6348f, 6.04085f },
    { 2, 0, 2, "|TInterface/ICONS/Spell_Shadow_SummonFelGuard:25|t Ragefire-chasm |cffff0000(15-21)|r", 0, 15, 1, 1, 1811.78f, -4410.5f, -18.4704f, 5.20165f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_charlgarazorflank:25|t Razorfen Kraul |cffff0000(35-43)|r", 0, 34, 1, 1, -4657.3f, -2519.35f, 81.0529f, 4.54808f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_amnennar_the_coldbringer:25|t Razprfen Downs |cffff0000(30-40)|r", 0, 24, 1, 1, -4470.28f, -1677.77f, 81.3925f, 1.16302f },
    { 2, 0, 2, "|TInterface/ICONS/inv_helmet_52:25|t Scarlet Monastery |cffff0000(26-42)|r", 0, 32, 1, 0, 2873.15f, -764.523f, 160.332f, 5.10447f },
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_ArchmageArugal:25|t Shadowfang-keep |cffff0000(16-26)|r", 0, 18, 1, 0, -234.675f, 1561.63f, 76.8921f, 1.24031f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_mutanus_the_devourer:25|t Wailing-caverns |cffff0000(15-25)|r", 0, 17, 1, 1, -731.607f, -2218.39f, 17.0281f, 2.78486f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_bazil_akumai:25|t Blackfathom-deeps |cffff0000(20-30)|r", 0, 21, 0, 1, 4249.99f, 740.102f, -25.671f, 1.34062f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_emperordagranthaurissan:25|t Blackrock-depths |cffff0000(50-60)|r", 0, 53, 0, 0, -7179.34f, -921.212f, 165.821f, 5.09599f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_generaldrakkisath:25|t Blackrock-spire |cffff0000(50-60)|r", 0, 57, 0, 0, -7527.05f, -1226.77f, 285.732f, 5.29626f },
    { 2, 0, 2, "|TInterface/ICONS/ability_warrior_decisivestrike:25|t Dire-maul |cffff0000(50-60)|r", 0, 55, 0, 1, -3520.14f, 1119.38f, 161.025f, 4.70454f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_princesstheradras:25|t Maraudon |cffff0000(30-46)|r", 0, 45, 0, 1, -1421.42f, 2907.83f, 137.415f, 1.70718f },
    { 2, 0, 2, "|TInterface/ICONS/spell_holy_senseundead:25|t Scholomance |cffff0000(50-60)|r", 0, 55, 0, 0, 1269.64f, -2556.21f, 93.6088f, 0.620623f },
    { 2, 0, 2, "|TInterface/ICONS/spell_deathknight_armyofthedead:25|t Stratholme |cffff0000(50-60)|r", 0, 55, 0, 0, 3352.92f, -3379.03f, 144.782f, 6.25978f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_shadeoferanikus:25|t Sunken-temple |cffff0000(45-55)|r", 0, 47, 0, 0, -10177.9f, -3994.9f, -111.239f, 6.01885f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_archaedas:25|t Uldaman |cffff0000(35-45)|r", 0, 37, 0, 0, -6071.37f, -2955.16f, 209.782f, 0.015708f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_chiefukorzsandscalp:25|t Zul'Farrak |cffff0000(40-48)|r", 0, 35, 0, 1, -6801.19f, -2893.02f, 9.00388f, 0.158639f },
    { 2, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_exarch_maladaar:25|t Auchindoun |cffff0000(60-70)|r", 0, 64, 0, 530, -3324.49f, 4943.45f, -101.239f, 4.63901f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_epochhunter:25|t Caverns of Time |cffff0000(60-70)|r", 0, 66, 0, 1, -8369.65f, -4253.11f, -204.272f, -2.70526f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_warlord_kalithresh:25|t Coilfang Reservoir |cffff0000(60-69)|r", 0, 62, 0, 530, 738.865f, 6865.77f, -69.4659f, 6.27655f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_kelidanthebreaker:25|t Hellfire-citadel |cffff0000(60-68)|r", 0, 59, 0, 530, -347.29f, 3089.82f, 21.394f, 5.68114f },
    { 3, 0, 2, "|TInterface/ICONS/Achievement_Boss_Kael\'thasSunstrider_01:25|t Mgisters-terrace |cffff0000(68-72)|r", 0, 70, 0, 530, 12884.6f, -7317.69f, 65.5023f, 4.799f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_pathaleonthecalculator:25|t The-eye (instances) |cffff0000(60-70)|r", 0, 70, 0, 530, 3100.48f, 1536.49f, 190.3f, 4.62226f },
    { 3, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_azjoluppercity_heroic:25|t Azjol nerub |cffff0000(72-74)|r", 0, 73, 0, 571, 3707.86f, 2150.23f, 36.76f, 3.22f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_azjollowercity_heroic:25|t Ahnkahet the old-kingdom |cffff0000(73-75)|r", 0, 74, 0, 571, 3707.86f, 2150.23f, 36.76f, 3.22f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_cotstratholme_heroic:25|t The culling of stratholme |cffff0000(80)|r", 0, 79, 0, 1, -8756.39f, -4440.68f, -199.489f, 4.66289f },
    { 4, 0, 2, "|TInterface/ICONS/inv_spear_05:25|t Trial of the champion |cffff0000(80)|r", 0, 79, 0, 571, 8590.95f, 791.792f, 558.235f, 3.13127f },
    { 4, 0, 2, "|TInterface/ICONS/Achievement_Dungeon_Drak\'Tharon_Heroic:25|t Draktharon keep |cffff0000(74-76)|r", 0, 74, 0, 571, 4765.59f, -2038.24f, 229.363f, 0.887627f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_gundrak_heroic:25|t Gundrak |cffff0000(76-78)|r", 0, 71, 0, 571, 6722.44f, -4640.67f, 450.632f, 3.91123f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_boss_lichking:25|t Instances of IceCrown |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_pitofsaron:25|t Pit of saron |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_forgeofsouls:25|t The forge of souls |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_hallsofreflection:25|t Halls of reflection |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_nexus80_25man:25|t Instances of Nexus |cffff0000(71-80)|r", 0, 71, 0, 571, 3782.89f, 6965.23f, 105.088f, 6.14194f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_nexus70_heroic:25|t The nexus |cffff0000(71-73)|r", 0, 71, 0, 571, 3782.89f, 6965.23f, 105.088f, 6.14194f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_nexus80_heroic:25|t Okulus |cffff0000(80)|r", 0, 79, 0, 571, 3782.89f, 6965.23f, 105.088f, 6.14194f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_theviolethold_heroic:25|t The violet hold |cffff0000(75-77)|r", 0, 75, 0, 571, 5693.08f, 502.588f, 652.672f, 4.0229f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_ulduar80_heroic:25|t Halls of lightning |cffff0000(80)|r", 0, 79, 0, 571, 9136.52f, -1311.81f, 1066.29f, 5.19113f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_ulduar77_heroic:25|t Halls of stone |cffff0000(77-79)|r", 0, 77, 0, 571, 8922.12f, -1009.16f, 1039.56f, 1.57044f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_utgardekeep_heroic:25|t Utgarde keep |cffff0000(70-72)|r", 0, 69, 0, 571, 1203.41f, -4868.59f, 41.2486f, 0.283237f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_utgardepinnacle_heroic:25|t Utgarde pinnacle |cffff0000(80)|r", 0, 75, 0, 571, 1267.24f, -4857.3f, 215.764f, 3.22768f },
    { 4, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_illidan:25|t Black-temple", 0, 70, 0, 530, -3649.92f, 317.469f, 35.2827f, 2.94285f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_nefarion:25|t Blackwing lair", 0, 60, 0, 229, 152.451f, -474.881f, 116.84f, 0.001073f },
    { 5, 0, 2, "|TInterface/ICONS/Achievement_Boss_Archimonde:25|t Hyjal Summit", 0, 70, 0, 1, -8177.89f, -4181.23f, -167.552f, 0.913338f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_ladyvashj:25|t Serpentshrine cavern", 0, 70, 0, 530, 797.855f, 6865.77f, -65.4165f, 0.005938f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_reputation_argentchampion:25|t Trial of the crusader", 0, 80, 0, 571, 8515.61f, 714.153f, 558.248f, 1.57753f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_gruulthedragonkiller:25|t Gruuls lair", 0, 70, 0, 530, 3530.06f, 5104.08f, 3.50861f, 5.51117f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_magtheridon:25|t Magtheridons lair", 0, 70, 0, 530, -336.411f, 3130.46f, -102.928f, 5.20322f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_icecrownentrance:25|t IceCrown Citadel", 0, 80, 0, 571, 5855.22f, 2102.03f, 635.991f, 3.57899f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_princemalchezaar_02:25|t Karazhan", 0, 70, 0, 0, -11118.9f, -2010.33f, 47.0819f, 0.649895f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_ragnaros:25|t Molten core", 0, 60, 0, 230, 1126.64f, -459.94f, -102.535f, 3.46095f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_naxxramas_10man:25|t Naxxramas", 0, 80, 0, 571, 3668.72f, -1262.46f, 243.622f, 4.785f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_onyxia:25|t Onyxias lair", 0, 80, 0, 1, -4708.27f, -3727.64f, 54.5589f, 3.72786f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_ossiriantheunscarred:25|t Ruins of ahnqiraj", 0, 60, 0, 1, -8409.82f, 1499.06f, 27.7179f, 2.51868f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_kiljaedan:25|t Sunwell plateau", 0, 70, 0, 530, 12574.1f, -6774.81f, 15.0904f, 3.13788f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_character_bloodelf_male:25|t The Eye", 0, 67, 0, 530, 3088.49f, 1381.57f, 184.863f, 4.61973f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_cthun:25|t Ahn'Qiraj", 0, 60, 0, 1, -8240.09f, 1991.32f, 129.072f, 0.941603f },
    { 5, 0, 2, "|TInterface/ICONS/Achievement_Boss_Malygos_01:25|t The eye of eternity", 0, 80, 0, 571, 3784.17f, 7028.84f, 161.258f, 5.79993f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_coablackdragonflight_10man:25|t The obsidian sanctum", 0, 80, 0, 571, 3472.43f, 264.923f, -120.146f, 3.27923f },
    { 5, 0, 2, "|TInterface/ICONS/spell_shadow_twilight:25|t The ruby sanctum", 0, 80, 0, 571, 3472.43f, 264.923f, -120.146f, 3.27923f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_ulduarraid_misc_01:25|t Ulduar", 0, 80, 0, 571, 9222.88f, -1113.59f, 1216.12f, 6.27549f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_kologarn_01:25|t Vault of archavon", 0, 80, 0, 571, 5453.72f, 2840.79f, 421.28f, 0.0f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_hakkar:25|t Zul'Gurub", 0, 57, 0, 0, -11916.7f, -1215.72f, 92.289f, 4.72454f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_zuljin:25|t Zul'Aman", 0, 70, 1, 530, 6851.78f, -7972.57f, 179.242f, 4.64691f },
    { 5, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_elwynnforest:25|t Elwynn forest |cffff0000(1-10)|r", 0, 0, 2, 0, -9449.06f, 64.8392f, 56.3581f, 3.07047f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_eversongwoods:25|t Eversong woods |cffff0000(1-10)|r", 0, 0, 1, 530, 9024.37f, -6682.55f, 16.8973f, 3.14131f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_dunmorogh:25|t Dun morogh |cffff0000(1-10)|r", 0, 0, 2, 0, -5603.76f, -482.704f, 396.98f, 5.23499f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_tirisfalglades_01:25|t Tirisfal glades |cffff0000(1-10)|r", 0, 0, 1, 0, 2274.95f, 323.918f, 34.1137f, 4.24367f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_ghostlands:25|t Ghostlands |cffff0000(10-20)|r", 0, 10, 1, 530, 7595.73f, -6819.6f, 84.3718f, 2.56561f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_lochmodan:25|t Loch modan |cffff0000(10-20)|r", 0, 10, 2, 0, -5405.85f, -2894.15f, 341.972f, 5.48238f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_silverpine_01:25|t Silverpine forest |cffff0000(10-20)|r", 0, 10, 1, 0, 505.126f, 1504.63f, 124.808f, 1.77987f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_westfall_01:25|t Westfall |cffff0000(10-20)|r", 0, 10, 2, 0, -10684.9f, 1033.63f, 32.5389f, 6.07384f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_redridgemountains:25|t Redridge mountains |cffff0000(15-25)|r", 0, 15, 2, 0, -9447.8f, -2270.85f, 71.8224f, 0.283853f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_duskwood:25|t Duskwood |cffff0000(18-30)|r", 0, 18, 2, 0, -10531.7f, -1281.91f, 38.8647f, 1.56959f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_hillsbradfoothills:25|t Hillsbrad foothills |cffff0000(20-30)|r", 0, 20, 0, 0, -385.805f, -787.954f, 54.6655f, 1.03926f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_wetlands_01:25|t Wetlands |cffff0000(20-30)|r", 0, 20, 2, 0, -3517.75f, -913.401f, 8.86625f, 2.60705f },
    { 6, 0, 2, "|TInterface/ICONS/Achievement_Zone_AlteracMountains_01:25|t Alterac mountains |cffff0000(30-40)|r", 0, 30, 0, 0, 275.049f, -652.044f, 130.296f, 0.502032f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_arathihighlands_01:25|t Arathi highlands |cffff0000(30-40)|r", 0, 30, 0, 0, -1581.45f, -2704.06f, 35.4168f, 0.490373f },
    { 6, 0, 2, "|TInterface/ICONS/Achievement_Zone_Stranglethorn_01:25|t Stranglethorn vale |cffff0000(30-45)|r", 0, 30, 0, 0, -11921.7f, -59.544f, 39.7262f, 3.73574f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_badlands_01:25|t Dadlands |cffff0000(35-45)|r", 0, 35, 0, 0, -6782.56f, -3128.14f, 240.48f, 5.65912f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_swampsorrows_01:25|t Swamp of sorrows |cffff0000(35-45)|r", 0, 35, 0, 0, -10368.6f, -2731.3f, 21.6537f, 5.29238f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_hinterlands_01:25|t Hinterlands |cffff0000(40-50)|r", 0, 40, 0, 0, 112.406f, -3929.74f, 136.358f, 0.981903f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_searinggorge_01:25|t Searing gorge |cffff0000(43-50)|r", 0, 43, 0, 0, -6686.33f, -1198.55f, 240.027f, 0.916887f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_blastedlands_01:25|t Blastedlands |cffff0000(45-55)|r", 0, 45, 0, 0, -11184.7f, -3019.31f, 7.29238f, 3.20542f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_burningsteppes_01:25|t Burning steppes |cffff0000(50-58)|r", 0, 50, 0, 0, -7979.78f, -2105.72f, 127.919f, 5.10148f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_westernplaguelands_01:25|t Wester nplaguelands |cffff0000(51-58)|r", 0, 51, 0, 0, 1743.69f, -1723.86f, 59.6648f, 5.23722f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_easternplaguelands:25|t Easter nplaguelands |cffff0000(53-60)|r", 0, 53, 0, 0, 2280.64f, -5275.05f, 82.0166f, 4.7479f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_deadwindpass:25|t Deadwind pass |cffff0000(55-60)|r", 0, 55, 0, 0, -10438.8f, -1932.75f, 104.617f, 4.77402f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_isleofqueldanas:25|t Isle of quel danas |cffff0000(70)|r", 0, 70, 0, 530, 12806.5f, -6911.11f, 41.1156f, 2.22935f },
    { 6, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_azuremystisle_01:25|t Azuremyst isle |cffff0000(1-10)|r", 0, 0, 2, 530, -4192.62f, -12576.7f, 36.7598f, 1.62813f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_darnassus:25|t Teldrassil |cffff0000(1-10)|r", 0, 0, 2, 1, 9889.03f, 915.869f, 1307.43f, 1.9336f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_durotar:25|t Durotar |cffff0000(1-10)|r", 0, 0, 1, 1, 228.978f, -4741.87f, 10.1027f, 0.416883f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_mulgore_01:25|t Mulgore |cffff0000(1-10)|r", 0, 0, 1, 1, -2473.87f, -501.225f, -9.42465f, 0.6525f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_bloodmystisle_01:25|t Bloodmyst isle |cffff0000(10-20)|r", 0, 15, 2, 530, -2095.7f, -11841.1f, 51.1557f, 6.19288f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_darkshore_01:25|t Darkshore |cffff0000(10-20)|r", 0, 10, 2, 1, 6463.25f, 683.986f, 8.92792f, 4.33534f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_barrens_01:25|t The barrens |cffff0000(10-25)|r", 0, 10, 1, 1, -575.772f, -2652.45f, 95.6384f, 0.006469f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_stonetalon_01:25|t Stonetalon mountains |cffff0000(15-27)|r", 0, 15, 0, 1, 1574.89f, 1031.57f, 137.442f, 3.8013f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_ashenvale_01:25|t Ashenvale |cffff0000(18-30)|r", 0, 18, 0, 1, 1919.77f, -2169.68f, 94.6729f, 6.14177f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_thousandneedles_01:25|t The thousand needles |cffff0000(25-35)|r", 0, 25, 1, 1, -5375.53f, -2509.2f, -40.432f, 2.41885f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_desolace:25|t Desolace |cffff0000(30-40)|r", 0, 30, 0, 1, -656.056f, 1510.12f, 88.3746f, 3.29553f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_dustwallowmarsh:25|t Dustwallow marsh |cffff0000(35-45)|r", 0, 35, 0, 1, -3350.12f, -3064.85f, 33.0364f, 5.12666f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_feralas:25|t Feralas |cffff0000(40-50)|r", 0, 40, 0, 1, -4808.31f, 1040.51f, 103.769f, 2.90655f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_tanaris_01:25|t Tanaris |cffff0000(40-50)|r", 0, 40, 0, 1, -6940.91f, -3725.7f, 48.9381f, 3.11174f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_azshara_01:25|t Azshara |cffff0000(45-55)|r", 0, 45, 0, 1, 3117.12f, -4387.97f, 91.9059f, 5.49897f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_felwood:25|t Felwood |cffff0000(48-55)|r", 0, 48, 0, 1, 3898.8f, -1283.33f, 220.519f, 6.24307f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_ungorocrater_01:25|t Un'goro crater |cffff0000(45-55)|r", 0, 48, 0, 1, -6291.55f, -1158.62f, -258.138f, 0.457099f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_silithus_01:25|t Silithus |cffff0000(55-60)|r", 0, 55, 0, 1, -6815.25f, 730.015f, 40.9483f, 2.39066f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_winterspring:25|t Wintersprings |cffff0000(55-60)|r", 0, 55, 0, 1, 6658.57f, -4553.48f, 718.019f, 5.18088f },
    { 7, 0, 2, "|TInterface/ICONS/spell_arcane_teleportmoonglade:25|t Moon glade |cffff0000(1-80)|r", 0, 0, 0, 1, 7654.3f, -2232.87f, 462.107f, 5.96786f },
    { 7, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_hellfirepeninsula_01:25|t Hellfire peninsula |cffff0000(58-63)|r", 0, 58, 0, 530, -207.335f, 2035.92f, 96.464f, 1.59676f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_zangarmarsh:25|t Zangarmarsh |cffff0000(60-64)|r", 0, 60, 0, 530, -220.297f, 5378.58f, 23.3223f, 1.61718f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_terrokar:25|t Terrokar |cffff0000(62-65)|r", 0, 62, 0, 530, -2266.23f, 4244.73f, 1.47728f, 3.68426f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_nagrand_01:25|t Nagrand |cffff0000(64-67)|r", 0, 64, 0, 530, -1610.85f, 7733.62f, -17.2773f, 1.33522f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_bladesedgemtns_01:25|t Blades edge mountains |cffff0000(65-68)|r", 0, 65, 0, 530, 2029.75f, 6232.07f, 133.495f, 1.30395f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_netherstorm_01:25|t Netherstorm |cffff0000(67-70)|r", 0, 67, 0, 530, 3271.2f, 3811.61f, 143.153f, 3.44101f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_shadowmoon:25|t Shadowmoon valley |cffff0000(67-70)|r", 0, 67, 0, 530, -3681.01f, 2350.76f, 76.587f, 4.25995f },
    { 8, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_boreantundra_01:25|t Boreantundra |cffff0000(68-72)|r", 0, 68, 0, 571, 2954.24f, 5379.13f, 60.4538f, 2.55544f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_howlingfjord_01:25|t Howlingfjord |cffff0000(68-72)|r", 0, 68, 0, 571, 682.848f, -3978.3f, 230.161f, 1.54207f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_dragonblight_01:25|t Dragonblight |cffff0000(71-74)|r", 0, 71, 0, 571, 2678.17f, 891.826f, 4.37494f, 0.101121f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_grizzlyhills_01:25|t Grizzly hills |cffff0000(73-75)|r", 0, 73, 0, 571, 4017.35f, -3403.85f, 290.0f, 5.35431f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_zuldrak_03:25|t Zul'drak |cffff0000(74-77)|r", 0, 74, 0, 571, 5560.23f, -3211.66f, 371.709f, 5.55055f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_sholazar_01:25|t Sholazar |cffff0000(76-78)|r", 0, 76, 0, 571, 5614.67f, 5818.86f, -69.722f, 3.60807f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_crystalsong_01:25|t Crystal song |cffff0000(77-80)|r", 0, 77, 0, 571, 5411.17f, -966.37f, 167.082f, 1.57167f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_stormpeaks_01:25|t Stormpeaks |cffff0000(77-80)|r", 0, 77, 0, 571, 6120.46f, -1013.89f, 408.39f, 5.12322f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_icecrown_01:25|t Icecrown |cffff0000(77-80)|r", 0, 77, 0, 571, 8323.28f, 2763.5f, 655.093f, 2.87223f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_stormpeaks_03:25|t Wintergrasp |cffff0000(PvP)|r", 0, 77, 0, 571, 4522.23f, 2828.01f, 389.975f, 0.215009f },
    { 9, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [back]", 0, 0, 0, 0, 0, 0, 0, 0 },
};

// TextIDs from npc_text
enum eEnums
{
    TEXT_MAIN_H  = 300000,   //Horde main menu text
    TEXT_MAIN_A  = 300001,   //Alliance main menu text
    TEXT_DUNGEON = 300002,   //Dungeon teleport menu texts
    TEXT_RAID    = 300003,   //Raid teleport menu text
    TEXT_AREA    = 300004,   //Area teleport location menu texts

    TELEPORT_COUNT = sizeof Tele / sizeof(*Tele),
};

#define ARE_YOU_SURE "Are you sure you want appear in "
#define ERROR_COMBAT "|cffff0000in combat!|r"

bool Custom_FactCheck(uint32 Fact, unsigned char Key)
{
    bool Show = false;
    switch (Tele[Key].faction)
    {
    case 0:
        Show = true;
        break;
    case 1:
        if (Fact == HORDE)
        Show = true;
        break;
    case 2:
        if (Fact == ALLIANCE)
        Show = true;
        break;
    }
    return (Show);
}

uint32 Custom_GetText(unsigned int menu, Player* pPlayer)
{
    uint32 TEXT = TEXT_AREA;
    switch (menu)
    {
    case 0:
    case 1:
        switch (pPlayer->GetTeam())
        {
        case ALLIANCE:
            TEXT = TEXT_MAIN_A;
            break;
        case HORDE:
            TEXT = TEXT_MAIN_H;
            break;
        }
        break;
    case 2:
    case 3:
    case 4:
        TEXT = TEXT_DUNGEON;
        break;
    case 5:
        TEXT = TEXT_RAID;
        break;
    }
    return (TEXT);
}

void Custom_GetMenu(Player* player, Creature* pCreature, uint32 Key)
{
    bool ENDMENU = false;
    for (uint32 i = 0; i < TELEPORT_COUNT; i++)
    {
        if (ENDMENU && Tele[i].menu_id != Key)
        break;
        if (Tele[i].menu_id == Key && player->GetLevel() >= Tele[i].level && Custom_FactCheck(player->GetTeam(), i))
        {
            if (Tele[i].next_menu_id != 0)
            AddGossipItemFor(player, GossipOptionIcon(Tele[i].icon), Tele[i].name, GOSSIP_SENDER_MAIN, i, "", Tele[i].cost, false);
            else
            AddGossipItemFor(player, GossipOptionIcon(Tele[i].icon), Tele[i].name, GOSSIP_SENDER_MAIN, i, ARE_YOU_SURE + Tele[i].name + " |r?", Tele[i].cost, false);
            ENDMENU = true;
        }
    }
    player->PlayerTalkClass->SendGossipMenu(Custom_GetText(Key, player), pCreature->GetGUID());
}

class npc_teleportation : public CreatureScript
{
public:
    npc_teleportation()
    : CreatureScript("npc_teleportation") { }

    struct npc_teleportationAI : public ScriptedAI
    {
        npc_teleportationAI(Creature* creature) : ScriptedAI(creature)
        {
            me->CastSpell(me, 72523, true);
            SetEquipmentSlots(false, 40348);
        }

        bool OnGossipHello(Player* player) override
        {
            Custom_GetMenu(player, me, 1);
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 gossipListId) override
        {
            //uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);

            ClearGossipMenuFor(player);

            player->ModifyMoney(-1 * Tele[action].cost); // Take cash
            uint32 Key = Tele[action].next_menu_id;
            if (Key == 0) // If no next_menu_id, then teleport to coords
            {
                if (!player->IsInCombat())
                {
                    CloseGossipMenuFor(player);
                    player->TeleportTo(Tele[action].map, Tele[action].x, Tele[action].y, Tele[action].z, Tele[action].o);
                    return true;
                }
                player->GetSession()->SendAreaTriggerMessage(ERROR_COMBAT);
                Key = Tele[action].menu_id;
            }

            Custom_GetMenu(player, me, Key);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_teleportationAI(creature);
    }
};

class npc_reagent : public CreatureScript
{
public:
    npc_reagent() : CreatureScript("npc_reagent") {}

    struct npc_reagentAI : public ScriptedAI
    {
        npc_reagentAI(Creature* creature) : ScriptedAI(creature) {}

        bool OnGossipHello(Player* player) override
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_elemental_eternal_shadow:25:25:-15:0|tHousehold goods", GOSSIP_SENDER_MAIN, 1);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\trade_alchemy:25:25:-15:0|tReagents for Alchemy", GOSSIP_SENDER_MAIN, 2);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\trade_mining:25:25:-15:0|tBlacksmithing Reagents", GOSSIP_SENDER_MAIN, 3);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_misc_armorkit_17:25:25:-15:0|tLeather Reagents", GOSSIP_SENDER_MAIN, 4);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_hammer_20:25:25:-15:0|tDevices | Fixtures", GOSSIP_SENDER_MAIN, 5);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\trade_engineering:25:25:-15:0|tReagents for Engineering", GOSSIP_SENDER_MAIN, 6);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\trade_engraving:25:25:-15:0|tEnchanting Reagents", GOSSIP_SENDER_MAIN, 7);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_inscription_tradeskill01:25:25:-15:0|tReagents for inscription", GOSSIP_SENDER_MAIN, 8);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\trade_tailoring:25:25:-15:0|tReagents for Tailoring", GOSSIP_SENDER_MAIN, 9);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_misc_herb_icethorn:25:25:-15:0|tCooking Reagents", GOSSIP_SENDER_MAIN, 10);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_misc_gem_01:25:25:-15:0|tReagents for Jeweler", GOSSIP_SENDER_MAIN, 11);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\inv_elemental_eternal_shadow:25:25:-15:0|tMiscellaneous Goods", GOSSIP_SENDER_MAIN, 12);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tClose", GOSSIP_SENDER_MAIN, 100);
            player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 gossipListId) override
        {
            //uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            player->PlayerTalkClass->ClearMenus();

            switch (action)
            {
            case 100:
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1:
                player->GetSession()->SendListInventory(me->GetGUID(), 400050);
                break;
            case 2:
                player->GetSession()->SendListInventory(me->GetGUID(), 400051);
                break;
            case 3:
                player->GetSession()->SendListInventory(me->GetGUID(), 400052);
                break;
            case 4:
                player->GetSession()->SendListInventory(me->GetGUID(), 400053);
                break;
            case 5:
                player->GetSession()->SendListInventory(me->GetGUID(), 400054);
                break;
            case 6:
                player->GetSession()->SendListInventory(me->GetGUID(), 400055);
                break;
            case 7:
                player->GetSession()->SendListInventory(me->GetGUID(), 400056);
                break;
            case 8:
                player->GetSession()->SendListInventory(me->GetGUID(), 400057);
                break;
            case 9:
                player->GetSession()->SendListInventory(me->GetGUID(), 400058);
                break;
            case 10:
                player->GetSession()->SendListInventory(me->GetGUID(), 400059);
                break;
            case 11:
                player->GetSession()->SendListInventory(me->GetGUID(), 400060);
                break;
            case 12:
                player->GetSession()->SendListInventory(me->GetGUID(), 400061);
                break;
            }
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_reagentAI(creature);
    }
};

enum Enchants
{
    ENCHANT_WEP_BERSERKING = 3789,
    ENCHANT_WEP_BLADE_WARD = 3869,
    ENCHANT_WEP_BLOOD_DRAINING = 3870,
    ENCHANT_WEP_ACCURACY = 3788,
    ENCHANT_WEP_AGILITY_1H = 1103,
    ENCHANT_WEP_SPIRIT = 3844,
    ENCHANT_WEP_BATTLEMASTER = 2675,
    ENCHANT_WEP_BLACK_MAGIC = 3790,
    ENCHANT_WEP_ICEBREAKER = 3239,
    ENCHANT_WEP_LIFEWARD = 3247,
    ENCHANT_WEP_MIGHTY_SPELL_POWER = 3834, // One-hand
    ENCHANT_WEP_EXECUTIONER = 3225,
    ENCHANT_WEP_STRYJA = 1894,
    ENCHANT_WEP_POTENCY = 3833,
    ENCHANT_WEP_TITANGUARD = 3851,
    ENCHANT_2WEP_MASSACRE = 3827,
    ENCHANT_2WEP_SCOURGEBANE = 3247,
    ENCHANT_2WEP_GIANT_SLAYER = 3257,
    ENCHANT_2WEP_GREATER_SPELL_POWER = 3854,
    ENCHANT_2WEP_AGILITY = 2670,
    ENCHANT_2WEP_MONGOOSE = 2673,

    ENCHANT_SHIELD_DEFENSE = 1952,
    ENCHANT_SHIELD_INTELLECT = 1128,
    ENCHANT_SHIELD_RESILIENCE = 3229,
    ENCHANT_SHIELD_BLOCK = 2655,
    ENCHANT_SHIELD_STAMINA = 1077,
    ENCHANT_SHIELD_TOUGHSHIELD = 2653,
    ENCHANT_SHIELD_TITANIUM_PLATING = 3849,

    ENCHANT_HEAD_BLISSFUL_MENDING = 3819,
    ENCHANT_HEAD_BURNING_MYSTERIES = 3820,
    ENCHANT_HEAD_DOMINANCE = 3796,
    ENCHANT_HEAD_SAVAGE_GLADIATOR = 3842,
    ENCHANT_HEAD_STALWART_PROTECTOR = 3818,
    ENCHANT_HEAD_TORMENT = 3817,
    ENCHANT_HEAD_TRIUMPH = 3795,
    ENCHANT_HEAD_ECLIPSED_MOON = 3815,
    ENCHANT_HEAD_FLAME_SOUL = 3816,
    ENCHANT_HEAD_FLEEING_SHADOW = 3814,
    ENCHANT_HEAD_FROSTY_SOUL = 3812,
    ENCHANT_HEAD_TOXIC_WARDING = 3813,

    ENCHANT_SHOULDER_MASTERS_AXE = 3835,
    ENCHANT_SHOULDER_MASTERS_CRAG = 3836,
    ENCHANT_SHOULDER_KRIT = 3838,
    ENCHANT_SHOULDER_MASTERS_PINNACLE = 3837,
    ENCHANT_SHOULDER_MASTERS_STORM = 3813,
    ENCHANT_SHOULDER_GREATER_AXE = 3808,
    ENCHANT_SHOULDER_GREATER_CRAG = 3809,
    ENCHANT_SHOULDER_GREATER_GLADIATOR = 3852,
    ENCHANT_SHOULDER_GREATER_PINNACLE = 3811,
    ENCHANT_SHOULDER_GREATER_STORM = 3810,
    ENCHANT_SHOULDER_DOMINANCE = 3794,
    ENCHANT_SHOULDER_TRIUMPH = 3793,

    ENCHANT_CLOAK_DARKGLOW_EMBROIDERY = 3728,
    ENCHANT_CLOAK_SWORDGUARD_EMBROIDERY = 3730,
    ENCHANT_CLOAK_LIGHTWEAVE_EMBROIDERY = 3722,
    ENCHANT_CLOAK_SPRINGY_ARACHNOWEAVE = 3859,
    ENCHANT_CLOAK_WISDOM = 3296,
    ENCHANT_CLOAK_TITANWEAVE = 1957,
    ENCHANT_CLOAK_SPELL_PIERCING = 3243,
    ENCHANT_CLOAK_SHADOW_ARMOR = 3256,
    ENCHANT_CLOAK_MIGHTY_ARMOR = 3294,
    ENCHANT_CLOAK_MAJOR_AGILITY = 1099,
    ENCHANT_CLOAK_GREATER_SPEED = 3831,

    ENCHANT_LEG_EARTHEN = 3853,
    ENCHANT_LEG_FROSTHIDE = 3822,
    ENCHANT_LEG_ICESCALE = 3823,
    ENCHANT_LEG_BRILLIANT_SPELLTHREAD = 3719,
    ENCHANT_LEG_SAPPHIRE_SPELLTHREAD = 3721,
    ENCHANT_LEG_DRAGONSCALE = 3331,
    ENCHANT_LEG_WYRMSCALE = 3332,

    ENCHANT_GLOVES_GREATER_BLASTING = 3249,
    ENCHANT_GLOVES_ARMSMAN = 3253,
    ENCHANT_GLOVES_CRUSHER = 1603,
    ENCHANT_GLOVES_AGILITY = 3222,
    ENCHANT_GLOVES_PRECISION = 3234,
    ENCHANT_GLOVES_SPD = 3246,
    ENCHANT_GLOVES_EXPERTISE = 3237,
    ENCHANT_GLOVES_HAST = 3604,
    ENCHANT_GLOVES_RAKETA = 3603,

    ENCHANT_BRACERS_MAJOR_STAMINA = 3850,
    ENCHANT_BRACERS_SUPERIOR_SP = 2332,
    ENCHANT_BRACERS_GREATER_ASSUALT = 3845,
    ENCHANT_BRACERS_MAJOR_SPIRT = 1147,
    ENCHANT_BRACERS_EXPERTISE = 3231,
    ENCHANT_BRACERS_INTELLECT = 1119,
    ENCHANT_BRACERS_FURL_ARCANE = 3763,
    ENCHANT_BRACERS_FURL_FIRE = 3759,
    ENCHANT_BRACERS_FURL_FROST = 3760,
    ENCHANT_BRACERS_FURL_NATURE = 3762,
    ENCHANT_BRACERS_FURL_SHADOW = 3761,
    ENCHANT_BRACERS_FURL_ATTACK = 3756,
    ENCHANT_BRACERS_FURL_STAMINA = 3757,
    ENCHANT_BRACERS_FURL_SPELLPOWER = 3758,

    ENCHANT_CHEST_POWERFUL_STATS = 3832,
    ENCHANT_CHEST_SUPER_HEALTH = 3297,
    ENCHANT_CHEST_GREATER_MAINA_REST = 2387,
    ENCHANT_CHEST_EXCEPTIONAL_RESIL = 3245,
    ENCHANT_CHEST_GREATER_DEFENSE = 1953,

    ENCHANT_BOOTS_GREATER_ASSULT = 1597,
    ENCHANT_BOOTS_TUSKARS_VITLIATY = 3232,
    ENCHANT_BOOTS_SUPERIOR_AGILITY = 983,
    ENCHANT_BOOTS_GREATER_SPIRIT = 1147,
    ENCHANT_BOOTS_GREATER_VITALITY = 3244,
    ENCHANT_BOOTS_ICEWALKER = 3826,
    ENCHANT_BOOTS_GREATER_FORTITUDE = 1075,
    ENCHANT_BOOTS_NITRO_BOOTS = 3606,

    ENCHANT_RING_ASSULT = 3839,
    ENCHANT_RING_GREATER_SP = 3840,
    ENCHANT_RING_STAMINA = 3791,

    ENCHANT_VZRIV = 3599,
    ENCHANT_BOMBA = 3601,
    ENCHANT_WAIST_SOCKET = 3729,
    ENCHANT_KISTIRYK_SOCKET = 3723,
    ENCHANT_ZAPYASTIE_SOCKET = 3717,

    ENCHANT_SPINA = 3859,
    ENCHANT_SPINADVA = 3605

};

void AdvancedEnchant(Player* player, Item* item, EnchantmentSlot slot, uint32 socketGem)
{
    if (!item)
    {
        player->GetSession()->SendNotification("You should equip item for enchant!");
        return;
    }

    if (!socketGem)
    {
        player->GetSession()->SendNotification("Something went wrong in the code. It has been logged for developers and will be looked into, sorry for the inconvenience.");
        return;
    }

    player->ApplyEnchantment(item, slot, false);
    item->SetEnchantment(slot, socketGem, 0, 0);
    player->ApplyEnchantment(item, slot, true);
    player->GetSession()->SendNotification("|cff800080%s |cffFF0000 successfull", item->GetTemplate()->Name1.c_str());
}

void Enchant(Player* player, Item* item, uint32 enchantid)
{
    if (!item)
    {
        player->GetSession()->SendNotification("You should equip item for enchant!");
        return;
    }

    if (!enchantid)
    {
        player->GetSession()->SendNotification("Something went wrong in the code. It has been logged for developers and will be looked into, sorry for the inconvenience.");
        //sLog->("enchant_vendor::Enchant: Enchant NPC 'enchantid' is NULL, something went wrong here!");
        return;
    }

    player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
    item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);
    player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
    player->GetSession()->SendNotification("|cff800080%s |cffFF0000 successfull!", item->GetTemplate()->Name1.c_str());
}

void RemoveEnchant(Player* player, Item* item)
{
    if (!item)
    {
        player->GetSession()->SendNotification("You haven't item");
        return;
    }

    item->ClearEnchantment(PERM_ENCHANTMENT_SLOT);
    player->GetSession()->SendNotification("|cff800080%s |cffFF0000 successfull removed enchant!", item->GetTemplate()->Name1.c_str());
}

class npc_enchantment : public CreatureScript
{
public:
    npc_enchantment() : CreatureScript("npc_enchantment") { }

    struct npc_enchantmentAI : public ScriptedAI
    {
        npc_enchantmentAI(Creature* creature) : ScriptedAI(creature) { }

        bool OnGossipHello(Player* player) override
        {
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_misc_dust_infinite:20:20:-15:0|t[Enchanting]", GOSSIP_SENDER_MAIN, 1001);
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_starofelune_01:20:20:-15:0|t[Sockets]", GOSSIP_SENDER_MAIN, 1002);
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_inscription_armorscroll01:20:20:-15:0|t[Glyphs]", GOSSIP_SENDER_MAIN, 1003);
            player->PlayerTalkClass->SendGossipMenu(100007, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 uiAction) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(uiAction);
            player->PlayerTalkClass->ClearMenus();
            Item * item;

            switch (action)
            {
            case 1002:
                // ??????
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_shadowspirit_01:20:20:-15:0|t[Meta]", GOSSIP_SENDER_MAIN, 1101);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_gem_38:20:20:-15:0|t[Yellow]", GOSSIP_SENDER_MAIN, 1102);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_gem_37:20:20:-15:0|t[Red]", GOSSIP_SENDER_MAIN, 1103);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_gem_42:20:20:-15:0|t[Blue]", GOSSIP_SENDER_MAIN, 1104);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_gem_40:20:20:-15:0|t[Purple]", GOSSIP_SENDER_MAIN, 1105);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_gem_39:20:20:-15:0|t[Orange]", GOSSIP_SENDER_MAIN, 1106);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_gem_41:20:20:-15:0|t[Green]", GOSSIP_SENDER_MAIN, 1107);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Inv_jewelcrafting_dragonseye02:20:20:-15:0|t[Brilliant]", GOSSIP_SENDER_MAIN, 1108);
                player->PlayerTalkClass->SendGossipMenu(100007, me->GetGUID());
                return true;
                break;
            case 1101:
                player->GetSession()->SendListInventory(me->GetGUID(), 400000);
                break;
            case 1102:
                player->GetSession()->SendListInventory(me->GetGUID(), 400001);
                break;
            case 1103:
                player->GetSession()->SendListInventory(me->GetGUID(), 400002);
                break;
            case 1104:
                player->GetSession()->SendListInventory(me->GetGUID(), 400003);
                break;
            case 1105:
                player->GetSession()->SendListInventory(me->GetGUID(), 400004);
                break;
            case 1106:
                player->GetSession()->SendListInventory(me->GetGUID(), 400005);
                break;
            case 1107:
                player->GetSession()->SendListInventory(me->GetGUID(), 400006);
                break;
            case 1108:
                player->GetSession()->SendListInventory(me->GetGUID(), 400007);
                break;
            case 1003:
                // ???????
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_holy_avenginewrath:20:20:-15:0|t[Paladin]", GOSSIP_SENDER_MAIN, 1121);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_nature_thunderclap:20:20:-15:0|t[Warrior]", GOSSIP_SENDER_MAIN, 1122);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_shadow_painspike:20:20:-15:0|t[Hunter]", GOSSIP_SENDER_MAIN, 1123);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_shadow_nethercloak:20:20:-15:0|t[Rogue]", GOSSIP_SENDER_MAIN, 1124);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_holy_prayerofshadowprotection:20:20:-15:0|t[Priest]", GOSSIP_SENDER_MAIN, 1125);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_deathknight_icetouch:20:20:-15:0|t[Death knight]", GOSSIP_SENDER_MAIN, 1126);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_nature_earthshock:20:20:-15:0|t[Shaman]", GOSSIP_SENDER_MAIN, 1127);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_nature_polymorph:20:20:-15:0|t[Mage]", GOSSIP_SENDER_MAIN, 1128);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_shadow_demoniccircleteleport:20:20:-15:0|t[Warlock]", GOSSIP_SENDER_MAIN, 1129);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_nature_forceofnature:20:20:-15:0|t[Druid]", GOSSIP_SENDER_MAIN, 1130);
                player->PlayerTalkClass->SendGossipMenu(100007, me->GetGUID());
                return true;
                break;
            case 1121:
                player->GetSession()->SendListInventory(me->GetGUID(), 400021);
                break;
            case 1122:
                player->GetSession()->SendListInventory(me->GetGUID(), 400022);
                break;
            case 1123:
                player->GetSession()->SendListInventory(me->GetGUID(), 400023);
                break;
            case 1124:
                player->GetSession()->SendListInventory(me->GetGUID(), 400024);
                break;
            case 1125:
                player->GetSession()->SendListInventory(me->GetGUID(), 400025);
                break;
            case 1126:
                player->GetSession()->SendListInventory(me->GetGUID(), 400026);
                break;
            case 1127:
                player->GetSession()->SendListInventory(me->GetGUID(), 400027);
                break;
            case 1128:
                player->GetSession()->SendListInventory(me->GetGUID(), 400028);
                break;
            case 1129:
                player->GetSession()->SendListInventory(me->GetGUID(), 400029);
                break;
            case 1130:
                player->GetSession()->SendListInventory(me->GetGUID(), 400030);
                break;

            case 1001:
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_helmet_164:20:20:-15:0|t[Enchant: helm]", GOSSIP_SENDER_MAIN, 4);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_shoulder_126:20:20:-15:0|t[Enchant: shoulder]", GOSSIP_SENDER_MAIN, 5);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_chest_plate26:20:20:-15:0|t[Enchant: chest]", GOSSIP_SENDER_MAIN, 7);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_pants_plate_37:20:20:-15:0|t[Enchant: legs]", GOSSIP_SENDER_MAIN, 10);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_gauntlets_91:20:20:-15:0|t[Enchant: hands]", GOSSIP_SENDER_MAIN, 9);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_bracer_43:20:20:-15:0|t[Enchant: wrist]", GOSSIP_SENDER_MAIN, 8);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_belt_48c:20:20:-15:0|t[Enchant: waist]", GOSSIP_SENDER_MAIN, 15);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_boots_plate_13:20:20:-15:0|t[Enchant: feet]", GOSSIP_SENDER_MAIN, 11);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_misc_cape_18:20:20:-15:0|t[Enchant: cloak]", GOSSIP_SENDER_MAIN, 6);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_axe_113:20:20:-15:0|t[Enchant: weapon]", GOSSIP_SENDER_MAIN, 1);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_axe_116:20:20:-15:0|t[Enchant: off-hand weapon]", GOSSIP_SENDER_MAIN, 13);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_axe_115:20:20:-15:0|t[Enchant: two-hand weapon]", GOSSIP_SENDER_MAIN, 2);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_shield_75:20:20:-15:0|t[Enchant: shield]", GOSSIP_SENDER_MAIN, 3);

                if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_jewelry_ring_83:20:20:-15:0|t[Enchant: ring]", GOSSIP_SENDER_MAIN, 12);
                }

                //AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|t[remove enchant from item]", GOSSIP_SENDER_MAIN, 14);

                player->PlayerTalkClass->SendGossipMenu(100007, me->GetGUID());
                return true;

                break;

            case 1: // Enchant Weapon

                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBlade Ward", GOSSIP_SENDER_MAIN, 102);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBlood Draining", GOSSIP_SENDER_MAIN, 103);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+26 agility", GOSSIP_SENDER_MAIN, 100);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+45 spirit", GOSSIP_SENDER_MAIN, 101);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBerserking", GOSSIP_SENDER_MAIN, 104);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 hit, +25 crit", GOSSIP_SENDER_MAIN, 105);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBlack magic", GOSSIP_SENDER_MAIN, 106);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBattlemaster", GOSSIP_SENDER_MAIN, 107);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tIcebreaker", GOSSIP_SENDER_MAIN, 108);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tLifeward", GOSSIP_SENDER_MAIN, 109);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 stamina", GOSSIP_SENDER_MAIN, 110);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+65 attack power", GOSSIP_SENDER_MAIN, 111);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+63 spell power", GOSSIP_SENDER_MAIN, 112);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tMangoose", GOSSIP_SENDER_MAIN, 113);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tExecutor", GOSSIP_SENDER_MAIN, 114);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tIcy Chill", GOSSIP_SENDER_MAIN, 701);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100002, me->GetGUID());
                return true;
                break;
            case 2: // Enchant 2H Weapon
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (!item)
                    {
                        player->GetSession()->SendNotification("You should be equiped by two-hand weapon");
                        OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBerserking", GOSSIP_SENDER_MAIN, 104);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+81 spell power", GOSSIP_SENDER_MAIN, 115);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+35 agility", GOSSIP_SENDER_MAIN, 116);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+110 attack power", GOSSIP_SENDER_MAIN, 117);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tMangoose", GOSSIP_SENDER_MAIN, 113);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tExecutor", GOSSIP_SENDER_MAIN, 114);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tIcy Chill", GOSSIP_SENDER_MAIN, 701);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                        player->PlayerTalkClass->SendGossipMenu(100003, me->GetGUID());
                        return true;
                    }
                    else
                    {
                        player->GetSession()->SendNotification("You should be equiped by two-hand weapon");
                        OnGossipHello(player);
                    }
                }
                break;
            case 3: // Enchant Shield
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendNotification("You should be equiped by shield");
                        OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 defense", GOSSIP_SENDER_MAIN, 118);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 intellect", GOSSIP_SENDER_MAIN, 119);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+12 resilience", GOSSIP_SENDER_MAIN, 120);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tTitanium Plating", GOSSIP_SENDER_MAIN, 121);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+18 stamina", GOSSIP_SENDER_MAIN, 122);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+36 block", GOSSIP_SENDER_MAIN, 123);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                        player->PlayerTalkClass->SendGossipMenu(100004, me->GetGUID());
                        return true;
                    }
                    else
                    {
                        player->GetSession()->SendNotification("You should be equiped by shield");
                        OnGossipHello(player);
                    }
                }
                break;
            case 4: // Enchant Head
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 spell power + 10 mp5", GOSSIP_SENDER_MAIN, 124);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 spell power +20 crit rating", GOSSIP_SENDER_MAIN, 125);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+29 stamina +20 resilience", GOSSIP_SENDER_MAIN, 126);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 stamina + 25 resilience", GOSSIP_SENDER_MAIN, 127);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+37 stamina + 20 defense", GOSSIP_SENDER_MAIN, 128);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 attack power + 20 crit rating", GOSSIP_SENDER_MAIN, 129);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 attack power +20 resilience", GOSSIP_SENDER_MAIN, 130);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 arcana resistance +30 stamina", GOSSIP_SENDER_MAIN, 131);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 fire resistance +30 stamina", GOSSIP_SENDER_MAIN, 132);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 shadow resistance +30 stamina", GOSSIP_SENDER_MAIN, 133);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 ice resistance +30 stamina", GOSSIP_SENDER_MAIN, 134);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 nature resistance +30 stamina", GOSSIP_SENDER_MAIN, 135);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100005, me->GetGUID());
                return true;
                break;
            case 5: // Enchant Shoulders
                if (player->HasSkill(SKILL_INSCRIPTION) && player->GetSkillValue(SKILL_INSCRIPTION) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+120 attack power + 15 crit effect rating [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 136);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 spell power +8 mp5 [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 137);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 spell power +15 crit effect rating [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 411);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+60 dodge + 15 defense [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 138);
                    // AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 ? ????????????? ????? ??????? ? 30 ? ????????????", GOSSIP_SENDER_MAIN, 139);
                }
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 attack power +15 crit rating", GOSSIP_SENDER_MAIN, 140);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+24 attack power +8 mp5", GOSSIP_SENDER_MAIN, 141);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 stamina +15 resilience", GOSSIP_SENDER_MAIN, 142);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 dodge +15 defense", GOSSIP_SENDER_MAIN, 143);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+24 spell power +15 crit rating", GOSSIP_SENDER_MAIN, 144);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+23 spell power +15 resilience", GOSSIP_SENDER_MAIN, 145);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 attack power + 15 resilience", GOSSIP_SENDER_MAIN, 146);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100006, me->GetGUID());
                return true;
                break;
            case 6: // Enchant Cloak
                if (player->HasSkill(SKILL_TAILORING) && player->GetSkillValue(SKILL_TAILORING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tDarkglow Embroidery [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 149);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tLightweave Embroidery [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 150);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tSwordguard Embroidery [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 151);
                }
                if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tSpringy Arachnoweave +27 spell power + parachut [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 350);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tFlexweave Underlay +23 agility + parachut [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 351);
                }
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+10 spirit 2% Reduced Threat", GOSSIP_SENDER_MAIN, 152);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 defense", GOSSIP_SENDER_MAIN, 153);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+35 penetration", GOSSIP_SENDER_MAIN, 154);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+225 armor", GOSSIP_SENDER_MAIN, 155);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+22 agility", GOSSIP_SENDER_MAIN, 156);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+23 haste", GOSSIP_SENDER_MAIN, 157);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tIncreased Stealth and +10 Agility", GOSSIP_SENDER_MAIN, 148);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100007, me->GetGUID());
                return true;
                break;
            case 7: //Enchant chest
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+10 all stats", GOSSIP_SENDER_MAIN, 158);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+275 health", GOSSIP_SENDER_MAIN, 159);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+10 mp5", GOSSIP_SENDER_MAIN, 160);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 resilience", GOSSIP_SENDER_MAIN, 161);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+22 defense", GOSSIP_SENDER_MAIN, 162);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100008, me->GetGUID());
                return true;
                break;
            case 8: //Enchant Bracers
                if (player->HasSkill(SKILL_LEATHERWORKING) && player->GetSkillValue(SKILL_LEATHERWORKING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 arcan resistance [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 170);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 fire resistance [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 171);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 ice resistance [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 172);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 nature resistance [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 173);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 shadow resistance [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 174);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+130 attack power [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 175);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+102 stamina [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 176);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+76 spell power [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 177);
                }
                if (player->HasSkill(SKILL_BLACKSMITHING) && player->GetSkillValue(SKILL_BLACKSMITHING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tAdd slot for socket [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 601);
                }
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 stamina", GOSSIP_SENDER_MAIN, 163);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 spell power", GOSSIP_SENDER_MAIN, 164);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 attack power", GOSSIP_SENDER_MAIN, 165);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+18 spirit", GOSSIP_SENDER_MAIN, 166);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+15 Expertise", GOSSIP_SENDER_MAIN, 167);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 intellect", GOSSIP_SENDER_MAIN, 169);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100009, me->GetGUID());
                return true;
                break;
            case 9: //Enchant Gloves
                if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
                {

                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tHyperspeed Accelerators [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 222);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tHand-Mounted Pyro Rocket [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 223);

                }
                if (player->HasSkill(SKILL_BLACKSMITHING) && player->GetSkillValue(SKILL_BLACKSMITHING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tAdd slot for socket [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 600);
                }
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 crit rating", GOSSIP_SENDER_MAIN, 178);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+2% Threat and 10 Parry Rating", GOSSIP_SENDER_MAIN, 179);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+44 attack power", GOSSIP_SENDER_MAIN, 180);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 agility", GOSSIP_SENDER_MAIN, 181);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 hit", GOSSIP_SENDER_MAIN, 182);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+28 spell power", GOSSIP_SENDER_MAIN, 168);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100010, me->GetGUID());
                return true;
                break;
            case 10: //Enchant legs
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 resilience + 28 stamina", GOSSIP_SENDER_MAIN, 184);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+55 stamina +22 agility", GOSSIP_SENDER_MAIN, 185);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+75 attack power +22 crit rating", GOSSIP_SENDER_MAIN, 186);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 spell power +20 spirit", GOSSIP_SENDER_MAIN, 187);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 spell power +30 stamina", GOSSIP_SENDER_MAIN, 188);
                // AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+72 ? ???????????? ? +35 ? ????????", GOSSIP_SENDER_MAIN, 189);
                // AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+100 ? ???? ????? ? +36 ? ???????? ???????????? ?????", GOSSIP_SENDER_MAIN, 190);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100017, me->GetGUID());
                return true;
                break;
            case 11: //Enchant feet
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+32 attack power", GOSSIP_SENDER_MAIN, 191);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+15 Stamina and Minor Speed Increase", GOSSIP_SENDER_MAIN, 192);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 agility", GOSSIP_SENDER_MAIN, 193);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+18 spirit", GOSSIP_SENDER_MAIN, 194);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+7 health and mana each 5 sec", GOSSIP_SENDER_MAIN, 195);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+12 hit + 12 crit rating", GOSSIP_SENDER_MAIN, 196);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+22 stamina", GOSSIP_SENDER_MAIN, 197);
                if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tNitro boots [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 198);
                }
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100012, me->GetGUID());
                return true;
                break;
            case 12: //Enchant rings
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 attack power [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 202);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+23 spell power [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 203);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 stamina [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 204);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100013, me->GetGUID());
                return true;
                break;
            case 13: //Enchant Off-Hand weapons
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendNotification("You need equiped off-hand item");
                        OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBlade Ward", GOSSIP_SENDER_MAIN, 205);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBlood Draining", GOSSIP_SENDER_MAIN, 219);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+26 agility", GOSSIP_SENDER_MAIN, 206);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+45 spirit", GOSSIP_SENDER_MAIN, 207);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBerserking", GOSSIP_SENDER_MAIN, 208);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 hits +25 crit rating", GOSSIP_SENDER_MAIN, 209);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBlack magic", GOSSIP_SENDER_MAIN, 210);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tBattlemaster", GOSSIP_SENDER_MAIN, 211);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tIcebreaker Weapon", GOSSIP_SENDER_MAIN, 212);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tLifeward", GOSSIP_SENDER_MAIN, 213);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 stamina", GOSSIP_SENDER_MAIN, 214);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+65 attack power", GOSSIP_SENDER_MAIN, 215);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+63 spell power", GOSSIP_SENDER_MAIN, 216);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tMangoose", GOSSIP_SENDER_MAIN, 217);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tExecutor", GOSSIP_SENDER_MAIN, 218);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                        player->PlayerTalkClass->SendGossipMenu(100002, me->GetGUID());
                        return true;
                    }
                    else
                    {
                        player->GetSession()->SendNotification("You need equiped off-hand weapon");
                        OnGossipHello(player);
                    }
                }
                break;
            case 15: //Enchant poyas
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tAdd slot for socket", GOSSIP_SENDER_MAIN, 220);
                if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tFrag Belt [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 221);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tElectromagnetic Pulse Generator [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 299);
                }
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tBack", GOSSIP_SENDER_MAIN, 300);
                player->PlayerTalkClass->SendGossipMenu(100513, me->GetGUID());
                return true;
                break;
            case 100:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_AGILITY_1H);
                OnGossipHello(player);
                break;
            case 101:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_SPIRIT);
                OnGossipHello(player);
                break;
            case 102:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLADE_WARD);
                OnGossipHello(player);
                break;
            case 103:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLOOD_DRAINING);
                OnGossipHello(player);
                break;
            case 104:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BERSERKING);
                OnGossipHello(player);
                break;
            case 105:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_ACCURACY);
                OnGossipHello(player);
                break;
            case 106:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLACK_MAGIC);
                OnGossipHello(player);
                break;
            case 107:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BATTLEMASTER);
                OnGossipHello(player);
                break;
            case 108:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_ICEBREAKER);
                OnGossipHello(player);
                break;
            case 109:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_LIFEWARD);
                OnGossipHello(player);
                break;
            case 110:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_TITANGUARD);
                OnGossipHello(player);
                break;
            case 111:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_POTENCY);
                OnGossipHello(player);
                break;
            case 112:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_MIGHTY_SPELL_POWER);
                OnGossipHello(player);
                break;
            case 113:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_MONGOOSE);
                OnGossipHello(player);
                break;
            case 114:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_EXECUTIONER);
                OnGossipHello(player);
                break;
            case 701:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_STRYJA);
                OnGossipHello(player);
                break;
            case 115:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a 2H weapon equipped.");
                        OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_GREATER_SPELL_POWER);
                        OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a Two-Handed weapon equipped.");
                        OnGossipHello(player);
                    }
                }
                break;
            case 116:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a 2H weapon equipped.");
                        OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_AGILITY);
                        OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a Two-Handed weapon equipped.");
                        OnGossipHello(player);
                    }
                }
                break;
            case 117:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a 2H weapon equipped.");
                        OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_MASSACRE);
                        OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a Two-Handed weapon equipped.");
                        OnGossipHello(player);
                    }
                }
                break;
            case 118:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_DEFENSE);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 119:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_INTELLECT);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 120:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                       OnGossipHello(player);
                        return false;
                    }

                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_RESILIENCE);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 121:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                       OnGossipHello(player);
                        return false;
                    }

                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_TITANIUM_PLATING);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 122:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_STAMINA);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 123:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_TOUGHSHIELD);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 124:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_BLISSFUL_MENDING);
               OnGossipHello(player);
                break;
            case 125:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_BURNING_MYSTERIES);
               OnGossipHello(player);
                break;
            case 126:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_DOMINANCE);
               OnGossipHello(player);
                break;
            case 127:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_SAVAGE_GLADIATOR);
               OnGossipHello(player);
                break;
            case 128:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_STALWART_PROTECTOR);
               OnGossipHello(player);
                break;
            case 129:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TORMENT);
               OnGossipHello(player);
                break;
            case 130:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TRIUMPH);
               OnGossipHello(player);
                break;
            case 131:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_ECLIPSED_MOON);
               OnGossipHello(player);
                break;
            case 132:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FLAME_SOUL);
               OnGossipHello(player);
                break;
            case 133:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FLEEING_SHADOW);
               OnGossipHello(player);
                break;
            case 134:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FROSTY_SOUL);
               OnGossipHello(player);
                break;
            case 135:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TOXIC_WARDING);
               OnGossipHello(player);
                break;
            case 136:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_AXE);
               OnGossipHello(player);
                break;
            case 137:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_CRAG);
               OnGossipHello(player);
                break;
            case 411:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_KRIT);
               OnGossipHello(player);
                break;
            case 138:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_PINNACLE);
               OnGossipHello(player);
                break;
            case 139:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_STORM);
               OnGossipHello(player);
                break;
            case 140:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_AXE);
               OnGossipHello(player);
                break;
            case 141:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_CRAG);
               OnGossipHello(player);
                break;
            case 142:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_GLADIATOR);
               OnGossipHello(player);
                break;
            case 143:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_PINNACLE);
               OnGossipHello(player);
                break;
            case 144:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_STORM);
               OnGossipHello(player);
                break;
            case 145:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_DOMINANCE);
               OnGossipHello(player);
                break;
            case 146:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_TRIUMPH);
               OnGossipHello(player);
                break;
            case 147:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SPRINGY_ARACHNOWEAVE);
               OnGossipHello(player);
                break;
            case 148:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SHADOW_ARMOR);
               OnGossipHello(player);
                break;
            case 149:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_DARKGLOW_EMBROIDERY);
               OnGossipHello(player);
                break;
            case 150:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_LIGHTWEAVE_EMBROIDERY);
               OnGossipHello(player);
                break;
            case 151:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SWORDGUARD_EMBROIDERY);
               OnGossipHello(player);
                break;
            case 351:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_SPINADVA);
               OnGossipHello(player);
                break;
            case 350:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_SPINA);
               OnGossipHello(player);
                break;
            case 152:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_WISDOM);
               OnGossipHello(player);
                break;
            case 153:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_TITANWEAVE);
               OnGossipHello(player);
                break;
            case 154:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SPELL_PIERCING);
               OnGossipHello(player);
                break;
            case 155:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_MIGHTY_ARMOR);
               OnGossipHello(player);
                break;
            case 156:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_MAJOR_AGILITY);
               OnGossipHello(player);
                break;
            case 157:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_GREATER_SPEED);
               OnGossipHello(player);
                break;
            case 158:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_POWERFUL_STATS);
               OnGossipHello(player);
                break;
            case 159:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_SUPER_HEALTH);
               OnGossipHello(player);
                break;
            case 160:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_GREATER_MAINA_REST);
               OnGossipHello(player);
                break;
            case 161:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_EXCEPTIONAL_RESIL);
               OnGossipHello(player);
                break;
            case 162:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_GREATER_DEFENSE);
               OnGossipHello(player);
                break;
            case 163:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_MAJOR_STAMINA);
               OnGossipHello(player);
                break;
            case 601:
                AdvancedEnchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_ZAPYASTIE_SOCKET); // Socket Belt
               OnGossipHello(player);
                break;
            case 164:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_SUPERIOR_SP);
               OnGossipHello(player);
                break;
            case 165:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_GREATER_ASSUALT);
               OnGossipHello(player);
                break;
            case 166:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_MAJOR_SPIRT);
               OnGossipHello(player);
                break;
            case 167:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_EXPERTISE);
               OnGossipHello(player);
                break;
            case 169:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_INTELLECT);
               OnGossipHello(player);
                break;
            case 170:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_ARCANE);
               OnGossipHello(player);
                break;
            case 171:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_FIRE);
               OnGossipHello(player);
                break;
            case 172:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_FROST);
               OnGossipHello(player);
                break;
            case 173:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_NATURE);
               OnGossipHello(player);
                break;
            case 174:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_SHADOW);
               OnGossipHello(player);
                break;
            case 175:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_ATTACK);
               OnGossipHello(player);
                break;
            case 176:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_STAMINA);
               OnGossipHello(player);
                break;
            case 177:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_SPELLPOWER);
               OnGossipHello(player);
                break;
            case 178:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_GREATER_BLASTING);
               OnGossipHello(player);
                break;
            case 179:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_ARMSMAN);
               OnGossipHello(player);
                break;
            case 180:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_CRUSHER);
               OnGossipHello(player);
                break;
            case 181:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_AGILITY);
               OnGossipHello(player);
                break;
            case 182:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_PRECISION);
               OnGossipHello(player);
                break;
            case 600:
                AdvancedEnchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_KISTIRYK_SOCKET); // Socket Belt
               OnGossipHello(player);
                break;
            case 168:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_SPD);
               OnGossipHello(player);
                break;
            case 184:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_EARTHEN);
               OnGossipHello(player);
                break;
            case 185:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_FROSTHIDE);
               OnGossipHello(player);
                break;
            case 186:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_ICESCALE);
               OnGossipHello(player);
                break;
            case 187:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_BRILLIANT_SPELLTHREAD);
               OnGossipHello(player);
                break;
            case 188:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_SAPPHIRE_SPELLTHREAD);
               OnGossipHello(player);
                break;
            case 189:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_DRAGONSCALE);
               OnGossipHello(player);
                break;
            case 190:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_WYRMSCALE);
               OnGossipHello(player);
                break;
            case 191:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_ASSULT);
               OnGossipHello(player);
                break;
            case 192:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_TUSKARS_VITLIATY);
               OnGossipHello(player);
                break;
            case 193:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_SUPERIOR_AGILITY);
               OnGossipHello(player);
                break;
            case 194:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_SPIRIT);
               OnGossipHello(player);
                break;
            case 195:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_VITALITY);
               OnGossipHello(player);
                break;
            case 196:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_ICEWALKER);
               OnGossipHello(player);
                break;
            case 197:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_FORTITUDE);
               OnGossipHello(player);
                break;
            case 198:
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("You should equip feet");
                    player->PlayerTalkClass->SendCloseGossip();
                    return false;
                }
                if (item->GetTemplate()->ItemLevel>150)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_NITRO_BOOTS);
                   OnGossipHello(player);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You can't enchant this item");
                   OnGossipHello(player);
                }
                break;
            case 202:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_ASSULT);
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_ASSULT);
               OnGossipHello(player);
                break;
            case 203:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_GREATER_SP);
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_GREATER_SP);
               OnGossipHello(player);
                break;
            case 204:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_STAMINA);
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_STAMINA);
               OnGossipHello(player);
                break;
            case 220:
                AdvancedEnchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST), PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_WAIST_SOCKET);
               OnGossipHello(player);
                break;
            case 221:
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("You should equip waist");
                    player->PlayerTalkClass->SendCloseGossip();
                    return false;
                }
                if (item->GetTemplate()->ItemLevel>150)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST), ENCHANT_BOMBA);
                   OnGossipHello(player);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You can't enchant this item");
                   OnGossipHello(player);
                }
                break;
            case 299:
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("You should equip waist");
                    player->PlayerTalkClass->SendCloseGossip();
                    return false;
                }
                if (item->GetTemplate()->ItemLevel>150)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST), ENCHANT_VZRIV);
                   OnGossipHello(player);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You can't enchant this item");
                   OnGossipHello(player);
                }
                break;
            case 222:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_HAST);
               OnGossipHello(player);
                break;
            case 223:
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_RAKETA);
               OnGossipHello(player);
                break;
            case 205:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLADE_WARD);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 206:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_AGILITY_1H);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 207:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_SPIRIT);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 208:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BERSERKING);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 209:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_ACCURACY);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 210:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLACK_MAGIC);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 211:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BATTLEMASTER);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 212:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_ICEBREAKER);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 213:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_LIFEWARD);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 214:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_TITANGUARD);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 215:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_POTENCY);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 216:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_MIGHTY_SPELL_POWER);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 217:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_2WEP_MONGOOSE);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;

            case 218:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_EXECUTIONER);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 219:
                {
                    item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!item)
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                        return false;
                    }
                    if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                    {
                        Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLOOD_DRAINING);
                       OnGossipHello(player);
                    }
                    else
                    {
                        player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                       OnGossipHello(player);
                    }
                }
                break;
            case 300: //<-Back menu
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_helmet_164:20:20:-15:0|t[Enchant: helm]", GOSSIP_SENDER_MAIN, 4);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_shoulder_126:20:20:-15:0|t[Enchant: shoulder]", GOSSIP_SENDER_MAIN, 5);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_chest_plate26:20:20:-15:0|t[Enchant: chest]", GOSSIP_SENDER_MAIN, 7);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_pants_plate_37:20:20:-15:0|t[Enchant: legs]", GOSSIP_SENDER_MAIN, 10);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_gauntlets_91:20:20:-15:0|t[Enchant: hands]", GOSSIP_SENDER_MAIN, 9);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_bracer_43:20:20:-15:0|t[Enchant: wrist]", GOSSIP_SENDER_MAIN, 8);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_belt_48c:20:20:-15:0|t[Enchant: waist]", GOSSIP_SENDER_MAIN, 15);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_boots_plate_13:20:20:-15:0|t[Enchant: feet]", GOSSIP_SENDER_MAIN, 11);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_misc_cape_18:20:20:-15:0|t[Enchant: cloak]", GOSSIP_SENDER_MAIN, 6);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_axe_113:20:20:-15:0|t[Enchant: weapon]", GOSSIP_SENDER_MAIN, 1);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_axe_116:20:20:-15:0|t[Enchant: off-hand weapon]", GOSSIP_SENDER_MAIN, 13);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_axe_115:20:20:-15:0|t[Enchant: two-hand weapon]", GOSSIP_SENDER_MAIN, 2);
                    AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_shield_75:20:20:-15:0|t[Enchant: shield]", GOSSIP_SENDER_MAIN, 3);
                    if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
                    {
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\inv_jewelry_ring_83:20:20:-15:0|t[Enchant: ring]", GOSSIP_SENDER_MAIN, 12);
                    }
                    player->PlayerTalkClass->SendGossipMenu(100007, me->GetGUID());
                    return true;
                }
                break;

            }
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_enchantmentAI(creature);
    }
};

#define GOSSIP_PROFESSION_TRAIN "|TInterface\\icons\\Ability_Repair:25:25:-18:0|tProfessions|r"
#define GOSSIP_RIDING "|TInterface\\icons\\spell_nature_swiftness:25:25:-18:0|tRiding|r"
#define GOSSIP_WEAPON "|TInterface\\icons\\Ability_DualWield:25:25:-18:0|tWeapon skills|r"

class ultimate_trainer_npc : public CreatureScript
{
public:
    ultimate_trainer_npc() : CreatureScript("ultimate_trainer_npc") { }

    struct ultimate_trainer_npcAI : public ScriptedAI
    {
        ultimate_trainer_npcAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->CastSpell(creature, 43897, true);
        }

        bool OnGossipHello(Player* player) override
        {
            ClearGossipMenuFor(player);

            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, GOSSIP_PROFESSION_TRAIN, GOSSIP_SENDER_MAIN, 4);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, GOSSIP_RIDING, GOSSIP_SENDER_MAIN, 5);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, GOSSIP_WEAPON, GOSSIP_SENDER_MAIN, 6);
            SendGossipMenuFor(player, 68, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 gossipListId) override
        {
            uint32 sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            player->PlayerTalkClass->ClearMenus();

            if (sender == GOSSIP_SENDER_MAIN)
            {
                switch (action)
                {
                    case 4: // ?????????
                    {
                        uint32 freeProfs = player->GetFreePrimaryProfessionPoints();
                        if (freeProfs > 0)
                        {
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Alchemy:30|t alchemy.|r", GOSSIP_SENDER_MAIN, 100);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_BlackSmithing:30|t blacksmithing|r", GOSSIP_SENDER_MAIN, 101);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Misc_ArmorKit_17:30|t leatherworking|r", GOSSIP_SENDER_MAIN, 102);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Tailoring:30|t tailoring|r", GOSSIP_SENDER_MAIN, 103);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Engineering:30|t engineering|r", GOSSIP_SENDER_MAIN, 104);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Engraving:30|t enchanting|r", GOSSIP_SENDER_MAIN, 105);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Misc_Gem_01:30|t jewelcrafting|r", GOSSIP_SENDER_MAIN, 106);
                            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Inscription_Tradeskill01:30|t inscription|r", GOSSIP_SENDER_MAIN, 107);
                        }
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/ICONS/Spell_Nature_NatureTouchGrow:30|t herbalism|r", GOSSIP_SENDER_MAIN, 108);
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/ICONS/INV_Misc_Pelt_Wolf_01:30|t skinning|r", GOSSIP_SENDER_MAIN, 109);
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/ICONS/Trade_Mining:30|t mining|r", GOSSIP_SENDER_MAIN, 110);
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/ICONS/INV_Misc_Food_15:30|t cooking|r", GOSSIP_SENDER_MAIN, 111);
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/ICONS/Spell_Holy_SealOfSacrifice:30|t first aid|r", GOSSIP_SENDER_MAIN, 112);
                        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/ICONS/Trade_Fishing:30|t fishing|r", GOSSIP_SENDER_MAIN, 113);
                        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface/icons/Spell_ChargeNegative:30|t close!|r", GOSSIP_SENDER_MAIN, 114);
                        player->PlayerTalkClass->SendGossipMenu(68, me->GetGUID());
                        break;
                    }
                    case 5: // ???????? ????
                    {
                        CloseGossipMenuFor(player);
                        //creature->Whisper(MSG_RIDING_COMPLETE, LANG_UNIVERSAL, player, false);
                        player->CastSpell(player, 33389, false); // Apprentice Riding
                        player->CastSpell(player, 33392, false); // Journeyman Riding
                        player->CastSpell(player, 34092, false); // Expert Riding
                        player->CastSpell(player, 34093, false); // Artisan Riding
                        player->LearnSpell(54197, false); // Allows you to ride flying mounts in Northrend.
                        break;
                    }
                    case 6: // ????????? ??????
                    {
                        switch (player->GetClass())
                        {
                            case CLASS_WARRIOR:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(5011, false); // ????????
                                player->LearnSpell(200, false); // ????????? ??????
                                player->LearnSpell(15590, false); // ???????? ??????
                                player->LearnSpell(264, false); // ????
                                player->LearnSpell(266, false); // ?????
                                player->LearnSpell(227, false); // ??????
                                break;
                            }
                            case CLASS_PALADIN:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(197, false); // ????????? ??????
                                player->LearnSpell(200, false); // ????????? ??????
                                player->LearnSpell(196, false); // ?????????? ??????
                                break;
                            }
                            case CLASS_WARLOCK:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(201, false); // ?????????? ????
                                break;
                            }
                            case CLASS_PRIEST:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(1180, false); // ???????
                                break;
                            }
                            case CLASS_HUNTER:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(5011, false); // ????????
                                player->LearnSpell(202, false); // ????????? ????
                                player->LearnSpell(200, false); // ????????? ??????
                                player->LearnSpell(15590, false); // ???????? ??????
                                player->LearnSpell(264, false); // ????
                                player->LearnSpell(2567, false); // ??????????? ??????
                                player->LearnSpell(227, false); // ??????
                                break;
                            }
                            case CLASS_MAGE:
                            {
                                ////creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(1180, false); // ???????
                                player->LearnSpell(201, false); // ?????????? ????
                                break;
                            }
                            case CLASS_SHAMAN:
                            {
                                ////creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(199, false); // ????????? ???????? ??????
                                player->LearnSpell(197, false); // ????????? ??????
                                player->LearnSpell(1180, false); // ???????
                                player->LearnSpell(15590, false); // ???????? ??????
                                player->LearnSpell(196, false); // ?????????? ??????
                                break;
                            }
                            case CLASS_ROGUE:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(5011, false); // ????????
                                player->LearnSpell(198, false); // ?????????? ???????? ??????
                                player->LearnSpell(15590, false); // ???????? ??????
                                player->LearnSpell(264, false); // ????
                                player->LearnSpell(201, false); // ?????????? ????
                                player->LearnSpell(266, false); // ?????
                                player->LearnSpell(196, false); // ?????????? ??????
                                break;
                            }
                            case CLASS_DEATH_KNIGHT:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(199, false); // ????????? ???????? ??????
                                player->LearnSpell(198, false); // ?????????? ???????? ??????
                                break;
                            }
                            case CLASS_DRUID:
                            {
                                //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                                player->LearnSpell(199, false); // ????????? ???????? ??????
                                player->LearnSpell(200, false); // ????????? ??????
                                player->LearnSpell(15590, false); // ???????? ??????
                                break;
                            }
                        }
                        CloseGossipMenuFor(player);
                        break;
                    }
                    case 100:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_ALCHEMY))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_ALCHEMY);
                        break;
                    }
                    case 101:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_BLACKSMITHING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_BLACKSMITHING);
                        break;
                    }
                    case 102:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_LEATHERWORKING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_LEATHERWORKING);
                        break;
                    }
                    case 103:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_TAILORING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_TAILORING);
                        break;
                    }
                    case 104:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_ENGINEERING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_ENGINEERING);
                        break;
                    }
                    case 105:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_ENCHANTING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_ENCHANTING);
                        break;
                    }
                    case 106:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_JEWELCRAFTING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_JEWELCRAFTING);
                        break;
                    }
                    case 107:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_INSCRIPTION))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }
                        CompleteLearnProfession(player, me, SKILL_INSCRIPTION);
                        break;
                    }
                    case 108:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_HERBALISM))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_HERBALISM);
                        break;
                    }
                    case 109:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_SKINNING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_SKINNING);
                        break;
                    }
                    case 110:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_MINING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_MINING);
                        break;
                    }
                    case 111:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_COOKING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_COOKING);
                        break;
                    }
                    case 112:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_FIRST_AID))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_FIRST_AID);
                        break;
                    }
                    case 113:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                        if (player->HasSkill(SKILL_FISHING))
                        {
                            player->GetSession()->SendNotification("You already know this profession");
                            break;
                        }

                        CompleteLearnProfession(player, me, SKILL_FISHING);
                        break;
                    }
                }
            }
            return true;
        }

        bool PlayerAlreadyHasTwoProfessions(Player *pPlayer)
        {
            uint32 skillCount = 0;

            if (pPlayer->HasSkill(SKILL_MINING))
                skillCount++;
            if (pPlayer->HasSkill(SKILL_SKINNING))
                skillCount++;
            if (pPlayer->HasSkill(SKILL_HERBALISM))
                skillCount++;
            if (skillCount >= 2)
                return true;

            SkillLineDBCMap const& skilllinemap = sDBCStoresMgr->GetSkillLineDBCMap();
            for (SkillLineDBCMap::const_iterator itr = skilllinemap.begin(); itr != skilllinemap.end(); ++itr)
            {
                if (SkillLineDBC const* SkillInfo = &itr->second)
                {
                    if (SkillInfo->CategoryID == SKILL_CATEGORY_SECONDARY)
                        continue;

                    if ((SkillInfo->CategoryID != SKILL_CATEGORY_PROFESSION) || !SkillInfo->CanLink)
                        continue;

                    const uint32 skillID = SkillInfo->ID;
                    if (pPlayer->HasSkill(skillID))
                        skillCount++;

                    if (skillCount >= 4)
                        return true;
                }
            }
            return false;
        }

        bool LearnAllRecipesInProfession(Player *pPlayer, SkillType skill)
        {
            ChatHandler handler(pPlayer->GetSession());
            std::string skill_name;

            SkillLineDBC const *SkillInfo = sDBCStoresMgr->GetSkillLineDBC(skill);
            skill_name = SkillInfo->DisplayName[handler.GetSessionDbcLocale()];

            if (!SkillInfo)
            {
                FMT_LOG_ERROR("server", "Profession NPC: received non-valid SkillType (LearnAllRecipesInProfession)");
                return false;
            }

            LearnSkillRecipesHelper(pPlayer, SkillInfo->ID);

            pPlayer->SetSkill(SkillInfo->ID, pPlayer->GetSkillStep(SkillInfo->ID), 450, 450);
            handler.PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, skill_name);
            return true;
        }

        void LearnSkillRecipesHelper(Player *player, uint32 skill_id)
        {
            uint32 classmask = player->GetClassMask();
            SkillLineAbilityDBCMap const& entryMap = sDBCStoresMgr->GetSkillLineAbilityDBCMap();
            for (const auto& indexID : entryMap)
            {
                if (SkillLineAbilityDBC const* skillLine = &indexID.second)
                {
                    // wrong skill
                    if (skillLine->SkillLine != skill_id)
                        continue;

                    // not high rank
                    if (skillLine->SupercededBySpell)
                        continue;

                    // skip racial skills
                    if (skillLine->RaceMask != 0)
                        continue;

                    // skip wrong class skills
                    if (skillLine->ClassMask && (skillLine->ClassMask & classmask) == 0)
                        continue;

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->Spell);
                    if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
                        continue;

                    player->LearnSpell(skillLine->Spell, false);
                }
            }
        }

        bool IsSecondarySkill(SkillType skill)
        {
            return skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
        }

        void CompleteLearnProfession(Player* pPlayer, Creature* /*pCreature*/, SkillType skill)
        {
            if (PlayerAlreadyHasTwoProfessions(pPlayer) && !IsSecondarySkill(skill))
                me->Whisper("You already know two professions!", LANG_UNIVERSAL, pPlayer, false);

            else
            {
                if (!LearnAllRecipesInProfession(pPlayer, skill))
                    me->Whisper("Ooops, error", LANG_UNIVERSAL, pPlayer, false);

            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new ultimate_trainer_npcAI(creature);
    }
};

void AddSC_custom_npc()
{
    new npc_teleportation();
    new npc_reagent();
    new npc_enchantment();
    new ultimate_trainer_npc();
}
