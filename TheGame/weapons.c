// (C) 2001 S2 Games

// weapons.c

// all data specific to player weapons

#include "game.h"


char *chargeFlagNames[] =
{
	"velocity",
	"damage",
	"range",
	"accuracy",
	"fusetime",
	"blastradius",
	"blastdamage",
	"count",
	"inverse",
	""
};

char *dmgFlagNames[] =
{
	"no_knockback",
	"no_air",
	"no_ground",
	"no_block",
	"self_none",
	"self_half",
	"no_falloff",
	"no_structures",
	"quake_event",
	"no_reaction",
	"strip_states",
	"explosive",
	"burning",
	"electrical",

	""
};

char *projReactionNames[] =
{
	"die",
	"die_noeffect",
	"stop",
	"ignore",
	"pierce",
	"bounce",
	"bounce_damage",
	""
};

char *projActionNames[] =
{
	"damageRadius",
	"findTarget",
	"attack",
	"attackRandom",
	""
};
