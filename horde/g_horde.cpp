// Copyright (c) ZeniMax Media Inc.
// Licensed under the GNU General Public License 2.0.
#include "../g_local.h"
#include <sstream>

static const int MAX_MONSTERS_BIG_MAP = 40;
static const int MAX_MONSTERS_PER_WAVE = 22;
static const int MAX_MONSTERS_SMALL_MAP = 17;

int remainingMonsters = 0;
int current_wave_number = 1;
const int BOSS_TO_SPAWN = 1;
cvar_t* g_horde;
enum class horde_state_t
{
	warmup,
	spawning,
	cleanup,
	rest
};

static struct {
	gtime_t			warm_time = 5_sec;
	horde_state_t	state = horde_state_t::warmup;

	gtime_t			monster_spawn_time = 0.5_sec;
	int32_t			num_to_spawn;
	int32_t			level;
} g_horde_local;

bool next_wave_message_sent = false;

static void Horde_InitLevel(int32_t lvl)
{
	current_wave_number++;
	g_horde_local.level = lvl;
	g_horde_local.monster_spawn_time = level.time + random_time(0.5_sec, 0.9_sec);

	bool isSmallMap = false;
	bool isBigMap = false;

	if (g_horde_local.level == 5) {
		gi.cvar_set("g_vampire", "1");
		gi.sound(world, CHAN_VOICE, gi.soundindex("makron/roar1.wav"), 1, ATTN_NONE, 0);
		gi.cvar_set("g_damage_scale", "1.5");
		gi.LocBroadcast_Print(PRINT_CENTER, "You're covered in blood!\n\n\nVampire Ability\nENABLED!\n");

		next_wave_message_sent = false;
	}

	if (g_horde_local.level == 10) {
		gi.cvar_set("g_damage_scale", "1.8");
		gi.cvar_set("ai_damage_scale", "1.5");
		gi.cvar_set("g_ammoregen", "1");
		//	gi.sound(world, CHAN_VOICE, gi.soundindex("misc/ir_start.wav"), 1, ATTN_NONE, 0);
		gi.sound(world, CHAN_VOICE, gi.soundindex("misc/keyuse.wav"), 1, ATTN_NONE, 0);
		gi.LocBroadcast_Print(PRINT_CENTER, "\n\nAMMO REGEN\n\nENABLED!\n");
	}

	if (g_horde_local.level == 15) {
		gi.cvar_set("g_damage_scale", "2.0");
	}

	if (g_horde_local.level == 20) {
		gi.cvar_set("g_damage_scale", "2.3");
	}

	if (!Q_strcasecmp(level.mapname, "base1")) {
		g_horde_local.num_to_spawn = 30 + (lvl * 5);
	}
	else if (!Q_strcasecmp(level.mapname, "base2")) {
		g_horde_local.num_to_spawn = 38 + (lvl * 5);
	}
	else if (!Q_strcasecmp(level.mapname, "base3")) {
		g_horde_local.num_to_spawn = 38 + (lvl * 5);
	}
	else if (!Q_strcasecmp(level.mapname, "bunk1")) {
		g_horde_local.num_to_spawn = 38 + (lvl * 5);
	}
	else if (!Q_strcasecmp(level.mapname, "ware1")) {
		g_horde_local.num_to_spawn = 38 + (lvl * 5);
	}
	else  if (!Q_strcasecmp(level.mapname, "q2ctf5")) {
		isBigMap = true;
	}
	else  if (!Q_strcasecmp(level.mapname, "xdm2")) {
		isBigMap = true;
	}
	else  if (!Q_strcasecmp(level.mapname, "xdm6")) {
		isBigMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q2dm8")) {
		g_horde_local.num_to_spawn = 12 + (lvl * 3);
	}
	else if (!Q_strcasecmp(level.mapname, "q2dm3")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q2dm2")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "dm10")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64/dm10")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64\\dm10")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "dm9")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64/dm9")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64\\dm9")) {
		isSmallMap = true;
	}
	else	if (!Q_strcasecmp(level.mapname, "dm7")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64/dm7")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64\\dm7")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "dm2")) {
		isSmallMap = true;
	}
	else 	if (!Q_strcasecmp(level.mapname, "q64/dm2")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "q64\\dm2")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "fact3")) {
		isSmallMap = true;
	}
	else if (!Q_strcasecmp(level.mapname, "mgdm1")) {
		isSmallMap = true;
	}
	if (isSmallMap && !isBigMap) {
		g_horde_local.num_to_spawn = 9 + (lvl * 1);
		if (g_horde_local.num_to_spawn > MAX_MONSTERS_SMALL_MAP) {
			g_horde_local.num_to_spawn = MAX_MONSTERS_SMALL_MAP;
		}
	}

	if (isBigMap && !isSmallMap) {
		g_horde_local.num_to_spawn = 24 + (lvl * 4);
			if (g_horde_local.num_to_spawn > MAX_MONSTERS_BIG_MAP) {
				g_horde_local.num_to_spawn = MAX_MONSTERS_BIG_MAP;
			};
	}


	if (!isBigMap && !isSmallMap) {
		g_horde_local.num_to_spawn = 10 + (lvl * 1.3);
	}
	// limiting max monsters
	if (!isBigMap && !isSmallMap && g_horde_local.num_to_spawn > MAX_MONSTERS_PER_WAVE)
	{
		g_horde_local.num_to_spawn = MAX_MONSTERS_PER_WAVE;
	}
}





bool G_IsDeathmatch()
{
	return deathmatch->integer && !g_horde->integer;
}

bool G_IsCooperative()
{
	return coop->integer || g_horde->integer;
}

struct weighted_item_t;

using weight_adjust_func_t = void(*)(const weighted_item_t& item, float& weight);

void adjust_weight_health(const weighted_item_t& item, float& weight);
void adjust_weight_weapon(const weighted_item_t& item, float& weight);
void adjust_weight_ammo(const weighted_item_t& item, float& weight);
void adjust_weight_armor(const weighted_item_t& item, float& weight);
void adjust_weight_powerup(const weighted_item_t& item, float& weight);

constexpr struct weighted_item_t {
	const char* classname;
	int32_t					min_level = -1, max_level = -1;
	float					weight = 1.0f;
	weight_adjust_func_t	adjust_weight = nullptr;
} items[] = {
	{ "item_health_small", -1, 9, 0.26f, adjust_weight_health },
	{ "item_health", -1, -1, 0.20f, adjust_weight_health },
	{ "item_health_large", -1, -1, 0.25f, adjust_weight_health },
	{ "item_health_mega", -1, -1, 0.09f, adjust_weight_health },
	{ "item_adrenaline", -1, -1, 0.15f, adjust_weight_health },

	{ "item_armor_shard", -1, 9, 0.26f, adjust_weight_armor },
	{ "item_armor_jacket", -1, 4, 0.35f, adjust_weight_armor },
	{ "item_armor_combat", 6, -1, 0.12f, adjust_weight_armor },
	{ "item_armor_body", 8, -1, 0.1f, adjust_weight_armor },
	//{ "item_power_screen", 4, -1, 0.07f, adjust_weight_armor },
	{ "item_power_shield", 4, -1, 0.07f, adjust_weight_armor },

	{ "item_quad", 6, 19, 0.073f, adjust_weight_powerup },
	{ "item_double", 5, -1, 0.078f, adjust_weight_powerup },
	{ "item_quadfire", 4, -1, 0.085f, adjust_weight_powerup },
	{ "item_invulnerability", 4, -1, 0.051f, adjust_weight_powerup },
	{ "item_sphere_defender", -1, -1, 0.1f, adjust_weight_powerup },
	{ "item_invisibility", 4, -1, 0.06f, adjust_weight_powerup },

	{ "weapon_chainfist", -1, 3, 0.23f, adjust_weight_weapon },
	{ "weapon_shotgun", -1, -1, 0.27f, adjust_weight_weapon },
	{ "weapon_supershotgun", 4, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_machinegun", -1, -1, 0.29f, adjust_weight_weapon },
	{ "weapon_etf_rifle", 3, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_boomer", 4, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_chaingun", 5, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_grenadelauncher", 6, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_proxlauncher", 8, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_hyperblaster", 5, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_phalanx", 10, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_rocketlauncher", 5, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_railgun", 6, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_plasmabeam", 7, -1, 0.19f, adjust_weight_weapon },
	{ "weapon_disintegrator", 12, -1, 0.15f, adjust_weight_weapon },
	{ "weapon_bfg", 17, -1, 0.15f, adjust_weight_weapon },


	{ "ammo_shells", -1, -1, 0.25f, adjust_weight_ammo },
	{ "ammo_bullets", -1, -1, 0.35f, adjust_weight_ammo },
	{ "ammo_flechettes", 5, -1, 0.35f, adjust_weight_ammo },
	{ "ammo_grenades", -1, -1, 0.35f, adjust_weight_ammo },
	{ "ammo_prox", 7, -1, 0.25f, adjust_weight_ammo },
	{ "ammo_tesla", 4, -1, 0.25f, adjust_weight_ammo },
	{ "ammo_cells", 5, -1, 0.30f, adjust_weight_ammo },
	{ "ammo_magslug", 9, -1, 0.25f, adjust_weight_ammo },
	{ "ammo_slugs", 7, -1, 0.25f, adjust_weight_ammo },
	{ "ammo_disruptor", 7, -1, 0.24f, adjust_weight_ammo },
	{ "ammo_rockets", 7, -1, 0.30f, adjust_weight_ammo },
	{ "item_bandolier", 4, -1, 0.37f, adjust_weight_ammo },
	{ "item_pack", 8, -1, 0.34f, adjust_weight_ammo },
};

void adjust_weight_health(const weighted_item_t& item, float& weight)
{
}

void adjust_weight_weapon(const weighted_item_t& item, float& weight)
{
}

void adjust_weight_ammo(const weighted_item_t& item, float& weight)
{
}

void adjust_weight_armor(const weighted_item_t& item, float& weight)
{
}

void adjust_weight_powerup(const weighted_item_t& item, float& weight)
{
}

constexpr weighted_item_t monsters[] = {
	{ "monster_soldier_light", -1, 6, 0.55f },
	{ "monster_soldier_ss", -1, 21, 0.55f },
	{ "monster_fixbot", 6, 19, 0.12f },
	{ "monster_soldier", -1, 4, 0.45f },
	{ "monster_soldier_hypergun", 2, 7, 0.55f },
	{ "monster_stalker", 4, 8, 0.13f },
	{ "monster_gekk", 3, 12, 0.22f },
	{ "monster_parasite", 4, 14, 0.2f },
	{ "monster_soldier_lasergun", 3, 10, 0.45f },
	{ "monster_soldier_ripper", 3, 7, 0.45f },
	{ "monster_infantry2", 2, 13, 0.36f },
	{ "monster_infantry", 8, -1, 0.36f },
	{ "monster_gunner", 10, -1, 0.54f },
	{ "monster_gunner2", 4, 9, 0.3f },
	{ "monster_brain", 5, -1, 0.3f },
	{ "monster_guncmdr2", 8, 15, 0.3f },
	{ "monster_guncmdr", 16, 22, 0.3f },
//	{ "monster_chick_heat", 7, -1, 0.73f },
	{ "monster_hover2", 5, 10, 0.17f },
	{ "monster_medic", 5, 8, 0.12f },
	{ "monster_hover", 13, -1, 0.27f },
	{ "monster_flyer", 3, 19, 0.18f },
	{ "monster_floater", 9, 16, 0.13f },
	{ "monster_daedalus", 13, -1, 0.12f },
//	{ "monster_daedalus2", 6, 8, 0.22f },
	{ "monster_gladiator", 5, -1, 0.24f },
	{ "monster_gladc", 8, 19, 0.24f },
	{ "monster_gladb", 14, -1, 0.75f},
	{ "monster_mutant", 7, 18, 0.55f },
	{ "monster_chick", 7, 19, 0.7f },
	{ "monster_berserk", 9, -1, 0.45f },
	{ "monster_boss2_64", 14, -1, 0.13f },
	{ "monster_floater2", 17, -1, 0.35f},
	{ "monster_carrier2", 17, -1, 0.23f },
	{ "monster_spider", 12, -1, 0.24f },
	{ "monster_tank", 7, -1, 0.3f },
	{ "monster_tank_commander", 11, 18, 0.15f },
	{ "monster_janitor", 16, -1, 0.18f },
    { "monster_medic_commander", 13, -1, 0.18f },
	{ "monster_shambler", 17, -1, 0.1f },
	{ "monster_tank_64", 16, -1, 0.13f },
	{ "monster_janitor2", 19, -1, 0.1f },
	{ "monster_makron", 18, 19, 0.2f },
	{ "monster_perrokl", 21, -1, 0.27f },
	{ "monster_guncmdrkl", 25, -1, 0.34f },
	{ "monster_shamblerkl", 27, -1, 0.34f },
};

struct boss_t {
	const char* classname;
	int32_t min_level;
	int32_t max_level;
	float weight;
};

constexpr boss_t BOSS[] = {
	{ "monster_jorg", 8, 3, 0.75f },
	{ "monster_shambler", 5, 3, 0.45f },
	{ "monster_guncmdr", -1, -1, 0.22f },
	{ "monster_berserk", -1, 7, 0.30f },
	{ "monster_brain", -1, 11, 0.30f },
	{ "monster_soldier_lasergun", -1, 8, 0.90f },
	{ "monster_soldier_ripper", -1, 9, 0.85f },
	{ "monster_chick", -1, 9, 0.92f },
	{ "monster_chick_heat", -1, -1, 0.63f },
	{ "monster_mutant", -1, -1, 0.75f },
	{ "monster_tank_commander", -1, 8, 0.45f },
	{ "monster_gladb", -1, -1, 0.5f },
	{ "monster_boss5", -1, -1, 0.18f },
	{ "monster_hover", -1, -1, 0.85f },
	{ "monster_flyer", -1, 6, 0.75f },
	{ "monster_floater", -1, 9, 0.85f },
	{ "monster_guardian", 11, 9, 0.85f },
	{ "monster_carrier", -1, -1, 0.07f },
	{ "monster_stalker", -1, -1, 0.07f },
	{ "monster_makronkl", -1, -1, 0.07f },
	{ "monster_perrokl", -1, -1, 0.07f },
	{ "monster_shamblerkl", -1, -1, 0.07f },
	{ "monster_guncmdrkl", -1, -1, 0.07f },
	{ "monster_boss2kl", -1, -1, 0.07f },
	{ "monster_supertankkl", -1, -1, 0.07f },
	{ "monster_turretkl", -1, -1, 0.07f },
};

struct picked_item_t {
	const weighted_item_t* item;
	float weight;
};

gitem_t* G_HordePickItem()
{
	// collect valid items
	static std::array<picked_item_t, q_countof(items)> picked_items;
	static size_t num_picked_items;

	num_picked_items = 0;

	float total_weight = 0;

	for (auto& item : items)
	{
		if (item.min_level != -1 && g_horde_local.level < item.min_level)
			continue;
		if (item.max_level != -1 && g_horde_local.level > item.max_level)
			continue;

		float weight = item.weight;

		if (item.adjust_weight)
			item.adjust_weight(item, weight);

		if (weight <= 0)
			continue;

		total_weight += weight;
		picked_items[num_picked_items++] = { &item, total_weight };
	}

	if (!total_weight)
		return nullptr;

	float r = frandom() * total_weight;

	for (size_t i = 0; i < num_picked_items; i++)
		if (r < picked_items[i].weight)
			return FindItemByClassname(picked_items[i].item->classname);

	return nullptr;
}

const char* G_HordePickMonster()
{
	// collect valid monsters
	static std::array<picked_item_t, q_countof(items)> picked_monsters;
	static size_t num_picked_monsters;

	num_picked_monsters = 0;

	float total_weight = 0;

	for (auto& item : monsters)
	{

		if (item.min_level != -1 && g_horde_local.level < item.min_level)
			continue;
		if (item.max_level != -1 && g_horde_local.level > item.max_level)
			continue;

		float weight = item.weight;

		if (item.adjust_weight)
			item.adjust_weight(item, weight);

		if (weight <= 0)
			continue;

		total_weight += weight;
		picked_monsters[num_picked_monsters++] = { &item, total_weight };
	}

	if (!total_weight)
		return nullptr;

	float r = frandom() * total_weight;

	for (size_t i = 0; i < num_picked_monsters; i++)
		if (r < picked_monsters[i].weight)
			return picked_monsters[i].item->classname;

	return nullptr;
}
const char* G_HordePickBOSS()
{
	const char* desired_boss = nullptr;

	if (!Q_strcasecmp(level.mapname, "q2dm1")) {
		desired_boss = "monster_guncmdr";
	}
	if (!Q_strcasecmp(level.mapname, "rdm14")) {
		desired_boss = "monster_makronkl";
	}
	else if (!Q_strcasecmp(level.mapname, "q2dm2")) {
		desired_boss = "monster_boss2kl";
	}
	else if (!Q_strcasecmp(level.mapname, "q2dm8")) {
		desired_boss = "monster_shamblerkl";
	}
	else if (!Q_strcasecmp(level.mapname, "xdm2")) {
		desired_boss = "monster_boss2kl";
	}
	else if (!Q_strcasecmp(level.mapname, "dm7") || !Q_strcasecmp(level.mapname, "q64/dm7")|| !Q_strcasecmp(level.mapname, "q64\\dm7")) {
		desired_boss = "monster_perrokl";
	}
	else if (!Q_strcasecmp(level.mapname, "dm10") || !Q_strcasecmp(level.mapname, "q64/dm10") || !Q_strcasecmp(level.mapname, "q64\\dm10")) {
		desired_boss = "monster_guncmdrkl";
	}
	else if (!Q_strcasecmp(level.mapname, "dm2") || !Q_strcasecmp(level.mapname, "q64/dm2") || !Q_strcasecmp(level.mapname, "q64\\dm2")) {
		desired_boss = "monster_guncmdrkl";
	}
	else if (!Q_strcasecmp(level.mapname, "q2ctf5")) {
		desired_boss = "monster_supertankkl";
	}

	else {
		return nullptr;
	}

	for (const auto& item : BOSS)
	{
		if (strcmp(item.classname, desired_boss) == 0)
		{
			return item.classname;
		}
	}
	return nullptr;
}

void Horde_PreInit()
{
	g_horde = gi.cvar("horde", "0", CVAR_LATCH);

	if (!g_horde->integer)
		return;

	if (!deathmatch->integer || ctf->integer || teamplay->integer || coop->integer)

	gi.Com_Print("Horde mode must be DM.\n");
	gi.cvar_set("deathmatch", "1");
	gi.cvar_set("ctf", "0");
	gi.cvar_set("teamplay", "0");
	gi.cvar_set("coop", "0");
	gi.cvar_set("timelimit", "20");
	gi.cvar_set("fraglimit", "0");

		g_horde = gi.cvar("horde", "0", CVAR_LATCH);

	if (g_horde->integer)
	{
		gi.Com_Print("Horde mode must be DM.\n");
		gi.cvar_set("deathmatch", "1");
		gi.cvar_set("ctf", "0");
		gi.cvar_set("teamplay", "0");
		gi.cvar_set("coop", "0");
		gi.cvar_set("timelimit", "25");
		gi.cvar_set("fraglimit", "0");
		gi.cvar_set("sv_target_id", "1");
		gi.cvar_set("g_dm_instant_items", "1");
//		gi.cvar_set("g_instagib", "1");
//		gi.cvar_set("g_dm_no_self_damage", "1");
//		gi.cvar_set("g_allow_techs", "1");
//		gi.cvar_set("g_use_hook", "1");
//		gi.cvar_set("hook_pullspeed", "1200");
//		gi.cvar_set("hook_speed", "3000");
//		gi.cvar_set("hook_sky", "1");
//		gi.cvar_set("g_no_nukes", "1");
//		gi.cvar_set("g_allow_grapple 1", "1");
//		gi.cvar_set("g_grapple_fly_speed", "3000");
//		gi.cvar_set("g_grapple_pull_speed", "1200");
//		gi.cvar_set("g_dm_no_fall_damage", "1");
//		gi.cvar_set("g_instant_weapon_switch", "1");
//		gi.cvar_set("g_start_items", "item_bandolier 1");

	}
}

void Horde_Init()
{

	// precache all items
	for (auto& item : itemlist)
		PrecacheItem(&item);

	// all monsters too
	for (auto& monster : monsters)
	{
		edict_t* e = G_Spawn();
		e->classname = monster.classname;
		e->monsterinfo.aiflags |= AI_DO_NOT_COUNT; // FIX BUG COUNT MONSTER IN COOP BEING +8

		ED_CallSpawn(e);
		G_FreeEdict(e);
	}

	g_horde_local.warm_time = level.time + 4_sec;
}

static bool Horde_AllMonstersDead()
{
	for (size_t i = 0; i < globals.max_edicts; i++)
	{
		if (!g_edicts[i].inuse)
			continue;
		else if (g_edicts[i].svflags & SVF_MONSTER)
		{
			if (!g_edicts[i].deadflag && g_edicts[i].health > 0)
				return false;
		}
	}
	return true;
}

static void Horde_CleanBodies()
{
	for (size_t i = 0; i < globals.max_edicts; i++)
	{
		if (!g_edicts[i].inuse)
			continue;
		else if (g_edicts[i].svflags & SVF_DEADMONSTER)
		{
			G_FreeEdict(&g_edicts[i]);			
		}
	}
}


void  SpawnBossAutomatically()
{
	if ((Q_strcasecmp(level.mapname, "q2dm1") == 0 && current_wave_number % 5 == 0 && current_wave_number != 0) ||
		(Q_strcasecmp(level.mapname, "rdm14") == 0 && current_wave_number % 5 == 0 && current_wave_number != 0) ||
		(Q_strcasecmp(level.mapname, "q2dm2") == 0 && current_wave_number % 5 == 0 && current_wave_number != 0) ||
		(Q_strcasecmp(level.mapname, "q2dm8") == 0 && current_wave_number % 5 == 0 && current_wave_number != 0) ||
		(Q_strcasecmp(level.mapname, "xdm2") == 0 && current_wave_number % 6 == 0 && current_wave_number != 0) ||
		(Q_strcasecmp(level.mapname, "q2ctf5") == 0 && current_wave_number % 7 == 0 && current_wave_number != 0) ||
		((!Q_strcasecmp(level.mapname, "dm10") || !Q_strcasecmp(level.mapname, "q64/dm10") || !Q_strcasecmp(level.mapname, "q64\\dm10")) && current_wave_number % 3 == 0 && current_wave_number != 0) ||
		((!Q_strcasecmp(level.mapname, "dm7") || !Q_strcasecmp(level.mapname, "q64/dm7") || !Q_strcasecmp(level.mapname, "q64\\dm7")) && current_wave_number % 3 == 0 && current_wave_number != 0) ||
		((!Q_strcasecmp(level.mapname, "dm2") || !Q_strcasecmp(level.mapname, "q64/dm2") || !Q_strcasecmp(level.mapname, "q64\\dm2")) && current_wave_number % 3 == 0 && current_wave_number != 0))
	{
	
	

		// Solo necesitas un bucle aqu� para generar un jefe
		edict_t* boss = G_Spawn(); // Creas un nuevo edict_t solo si es necesario
		if (!boss)
			return;

		// Aqu� selecciona el jefe deseado
		const char* desired_boss = G_HordePickBOSS();
		if (!desired_boss) {
			return; // No se pudo encontrar un jefe v�lido
		}
		boss->classname = desired_boss;
		// origin for monsters
		if (!Q_strcasecmp(level.mapname, "q2dm1")) {
			boss->s.origin[0] = 1280;
			boss->s.origin[1] = 336;
			boss->s.origin[2] = 664;
		}
		if (!Q_strcasecmp(level.mapname, "rdm14")) {
			boss->s.origin[0] = 1280;
			boss->s.origin[1] = 336;
			boss->s.origin[2] = 664;
		}
		if (!Q_strcasecmp(level.mapname, "q2dm2")) {
			boss->s.origin[0] = 128;
			boss->s.origin[1] = -960;
			boss->s.origin[2] = 704;
		}
		else if (!Q_strcasecmp(level.mapname, "q2dm8")) {
			boss->s.origin[0] = 112;
			boss->s.origin[1] = 1216;
			boss->s.origin[2] = 88;
		}
		else if (!Q_strcasecmp(level.mapname, "q2ctf5")) {
			boss->s.origin[0] = 2432;
			boss->s.origin[1] = -960;
			boss->s.origin[2] = 168;
		}
		else if (!Q_strcasecmp(level.mapname, "xdm2")) {
			boss->s.origin[0] = -232;
			boss->s.origin[1] = 472;
			boss->s.origin[2] = 424;
		}
		else if (!Q_strcasecmp(level.mapname, "dm7") || !Q_strcasecmp(level.mapname, "q64/dm7") || !Q_strcasecmp(level.mapname, "q64\\dm7")) {
			boss->s.origin[0] = 64;
			boss->s.origin[1] = 224;
			boss->s.origin[2] = 120;
		}
		else if (!Q_strcasecmp(level.mapname, "dm10") || !Q_strcasecmp(level.mapname, "q64/dm10") || !Q_strcasecmp(level.mapname, "q64\\dm10")) {
			boss->s.origin[0] = -304;
			boss->s.origin[1] = 512;
			boss->s.origin[2] = -92;
		}
		else if (!Q_strcasecmp(level.mapname, "dm2") || !Q_strcasecmp(level.mapname, "q64/dm2") || !Q_strcasecmp(level.mapname, "q64\\dm2")) {
			boss->s.origin[0] = 1328;
			boss->s.origin[1] = -256;
			boss->s.origin[2] = 272;
		}

		else {
			return; 
		}

		boss->s.angles[0] = 0;
		boss->s.angles[1] = -45;
		boss->s.angles[2] = 0;


		boss->maxs *= 1.4;
		boss->mins *= 1.4;
		boss->s.scale = 1.4;
		boss->health *= pow(1.28, current_wave_number);
	//	boss->s.renderfx = RF_TRANSLUCENT;
	//	boss->s.effects = EF_FLAG1 | EF_QUAD;

		vec3_t effectPosition = boss->s.origin;
		effectPosition[0] += (boss->s.origin[0] - effectPosition[0]) * (boss->s.scale - 3);
		effectPosition[1] += (boss->s.origin[1] - effectPosition[1]) * (boss->s.scale - 3);
		effectPosition[2] += (boss->s.origin[2] - effectPosition[2]) * (boss->s.scale - 3);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BOSSTPORT);
		gi.WritePosition(effectPosition);
		gi.multicast(effectPosition, MULTICAST_PHS, false);
		ED_CallSpawn(boss);
	}
}

void ResetGame() {
	g_horde_local.state = horde_state_t::warmup;
	next_wave_message_sent = false; // Reinicia la bandera de mensaje de pr�xima oleada
	gi.cvar_set("g_chaotic", "0");
	gi.cvar_set("g_insane", "0");
	gi.cvar_set("g_vampire", "0");
	gi.cvar_set("ai_damage_scale", "1");
	gi.cvar_set("g_damage_scale", "1");
	gi.cvar_set("g_ammoregen", "0");
	gi.cvar_set("g_hardcoop", "0");

	}

#include <chrono>

// Define una variable global para almacenar el tiempo de referencia cuando se cumple la condici�n
std::chrono::steady_clock::time_point condition_start_time;

// Variable para almacenar el valor anterior de remainingMonsters
int previous_remainingMonsters = 0;

// Funci�n para verificar si la condici�n de remainingMonsters se cumple durante m�s de x segundos
bool CheckRemainingMonstersCondition() {
	if (remainingMonsters <= 3 && current_wave_number <= 7) {
		// Si la condici�n se cumple por primera vez, actualiza el tiempo de referencia
		if (condition_start_time == std::chrono::steady_clock::time_point()) {
			condition_start_time = std::chrono::steady_clock::now();
		}
		// Verifica si la condici�n ha estado activa durante m�s de x segundos
		auto current_time = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - condition_start_time) ;

		if (duration.count() >= 12) {
			return true;
		}
	}
	else if (remainingMonsters <= 6 && current_wave_number >= 8) {
		// Si la condici�n se cumple por primera vez, actualiza el tiempo de referencia
		if (condition_start_time == std::chrono::steady_clock::time_point()) {
			condition_start_time = std::chrono::steady_clock::now();
		}

		// Verifica si la condici�n ha estado activa durante m�s de x segundos
		auto current_time = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - condition_start_time);

		if (duration.count() >= 14 ) {
			return true;
		}
	}
	else {
		// Si la condici�n no se cumple, reinicia el tiempo de referencia si remainingMonsters est� aumentando
		if (remainingMonsters > previous_remainingMonsters) {
			condition_start_time = std::chrono::steady_clock::time_point();
		}
		// Actualiza el valor anterior de remainingMonsters
		previous_remainingMonsters = remainingMonsters;
	}
	return false;
}

void Horde_RunFrame()
{
	switch (g_horde_local.state)
	{
	case horde_state_t::warmup:
		if (g_horde_local.warm_time < level.time + 1_sec)
		{
			remainingMonsters = 0;
			g_horde_local.state = horde_state_t::spawning;
			Horde_InitLevel(1);
			current_wave_number = 2;

			// init random
			std::srand(static_cast<unsigned int>(std::time(nullptr)));

			//generate random number
			float r = frandom();

			if (r < 0.333f) // less than 0.333
			{
				gi.sound(world, CHAN_VOICE, gi.soundindex("misc/r_tele3.wav"), 1, ATTN_NONE, 0);
			}
			else if (r < 0.666f) // if less than 0.666
			{
				gi.sound(world, CHAN_VOICE, gi.soundindex("world/redforce.wav"), 1, ATTN_NONE, 0);
			}
			else // same or more than 0.666
			{
				gi.sound(world, CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
			}
			// Print message according difficult
			if (!g_chaotic->integer)
				gi.LocBroadcast_Print(PRINT_CENTER, "????");
			else
				gi.LocBroadcast_Print(PRINT_CENTER, "CHAOS IMMINENT");

			// random sound
			if (!g_chaotic->integer || (g_chaotic->integer && r > 0.666f))
				gi.sound(world, CHAN_VOICE, gi.soundindex("misc/r_tele3.wav"), 1, ATTN_NONE, 0);
			else if (r > 0.333f)
				gi.sound(world, CHAN_VOICE, gi.soundindex("world/redforce.wav"), 1, ATTN_NONE, 0);
			else
				gi.sound(world, CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
		}
		break;

	case horde_state_t::spawning:
		if (!next_wave_message_sent)
		{
			next_wave_message_sent = true;
		}
		if (g_horde_local.monster_spawn_time <= level.time)
		{

			// Llama a la funci�n para spawnear el jefe solo si es el momento adecuado
			if (g_horde_local.num_to_spawn == BOSS_TO_SPAWN)
			{
				SpawnBossAutomatically();
			}

			edict_t* e = G_Spawn();
			e->classname = G_HordePickMonster();
			select_spawn_result_t result = SelectDeathmatchSpawnPoint(false, true, false);

			if (result.any_valid)
			{
				e->s.origin = result.spot->s.origin;
				e->s.angles = result.spot->s.angles;
				e->item = G_HordePickItem();
				// e->s.renderfx = RF_TRANSLUCENT;
				ED_CallSpawn(e);
				remainingMonsters = level.total_monsters - level.killed_monsters; // Calcula la cantidad de monstruos restantes


				{
					// spawn animation
					vec3_t spawngrow_pos = result.spot->s.origin;
					float start_size = (sqrt(spawngrow_pos[0] * spawngrow_pos[0] + spawngrow_pos[1] * spawngrow_pos[1] + spawngrow_pos[1] * spawngrow_pos[1])) * 0.04f; // scaling
					float end_size = start_size;

					// Generar el Spawngrow con los tama�os calculados
					SpawnGrow_Spawn(spawngrow_pos, start_size, end_size);
				}

				e->enemy = &g_edicts[1]; 
				e->gib_health = -280;
				e->health *= pow(1.045, current_wave_number);
				FoundTarget(e);

				if (current_wave_number >= 14) {
					g_horde_local.monster_spawn_time = level.time + random_time(1.4_sec, 1.9_sec);
				}
				g_horde_local.monster_spawn_time = level.time + random_time(0.3_sec, 0.9_sec);

				--g_horde_local.num_to_spawn;

				if (!g_horde_local.num_to_spawn)
				{
					{
						std::ostringstream message_stream;
						message_stream << "New Wave Is Here.\n Current Level: " << g_horde_local.level << "\n";
						gi.LocBroadcast_Print(PRINT_CENTER, message_stream.str().c_str());
					}
					g_horde_local.state = horde_state_t::cleanup;
					g_horde_local.monster_spawn_time = level.time + 1_sec;
				}
			}
			else
			{
				remainingMonsters = level.total_monsters - level.killed_monsters;; // Calcula la cantidad de monstruos restantes
				g_horde_local.monster_spawn_time = level.time + 0.5_sec;
			}
		}
		break;

	case horde_state_t::cleanup:

		if (CheckRemainingMonstersCondition()) {
			if (current_wave_number >= 15) {
				gi.cvar_set("g_insane", "1");

				// Si se cumple la condici�n durante m�s de x segundos, avanza al estado 'rest'
				g_horde_local.state = horde_state_t::rest;
				break;
			}
			else if ((current_wave_number <= 14))
			gi.cvar_set("g_chaotic", "1");


			// Si se cumple la condici�n durante m�s de x segundos, avanza al estado 'rest'
			g_horde_local.state = horde_state_t::rest;
			break;
		}

		if (g_horde_local.monster_spawn_time < level.time)
		{
			if (Horde_AllMonstersDead())
			{
				g_horde_local.warm_time = level.time + 5_sec;
				g_horde_local.state = horde_state_t::rest;
				remainingMonsters = 0;

				if (g_chaotic->integer || g_insane->integer) {
					gi.LocBroadcast_Print(PRINT_CENTER, "\n\n\n\n\n\nHarder Wave Controlled, GG\n");
					gi.sound(world, CHAN_VOICE, gi.soundindex("world/x_light.wav"), 1, ATTN_NONE, 0);
					gi.cvar_set("g_chaotic", "0");
					gi.cvar_set("g_insane", "0");
					gi.cvar_set("ai_damage_scale", "1.2");
				}
				else if (!g_chaotic->integer) {
					gi.LocBroadcast_Print(PRINT_CENTER, "\n\n\n\n\n\nWave Defeated, GG !\n");
				}
			}
			else
			remainingMonsters = level.total_monsters + 1 - level.killed_monsters;
			g_horde_local.monster_spawn_time = level.time + 3_sec;
		}
		break;


	case horde_state_t::rest:
		if (g_horde_local.warm_time < level.time)
		{
			if (g_chaotic->integer || g_insane->integer) {
				gi.LocBroadcast_Print(PRINT_CENTER, "**************\n\n\n--STRONGER WAVE COMING--\n\n\n STROGGS STARTING TO PUSH !\n\n\n **************");
				float r = frandom();

				if (r < 0.167f) // Aproximadamente 16.7% de probabilidad para cada sonido
					gi.sound(world, CHAN_VOICE, gi.soundindex("nav_editor/action_fail.wav"), 1, ATTN_NONE, 0);
				else if (r < 0.333f)
					gi.sound(world, CHAN_VOICE, gi.soundindex("makron/roar1.wav"), 1, ATTN_NONE, 0);
				else if (r < 0.5f)
					gi.sound(world, CHAN_VOICE, gi.soundindex("world/battle5.wav"), 1, ATTN_NONE, 0);
				else if (r < 0.667f)
					gi.sound(world, CHAN_VOICE, gi.soundindex("misc/spawn1.wav"), 1, ATTN_NONE, 0);
				else if (r < 0.833f)
					gi.sound(world, CHAN_VOICE, gi.soundindex("makron/voice3.wav"), 1, ATTN_NONE, 0);
				else
					gi.sound(world, CHAN_VOICE, gi.soundindex("world/v_fac3.wav"), 1, ATTN_NONE, 0);
			}
			else if (!g_chaotic->integer || !g_insane->integer) {
				gi.LocBroadcast_Print(PRINT_CENTER, "Loading Next Wave");
				gi.sound(world, CHAN_VOICE, gi.soundindex("world/lite_on1.wav"), 1, ATTN_NONE, 0);
			}
			g_horde_local.state = horde_state_t::spawning;
			Horde_InitLevel(g_horde_local.level + 1);
			Horde_CleanBodies();
		}
		break;
	}
}

void HandleResetEvent() {
	ResetGame();

}

