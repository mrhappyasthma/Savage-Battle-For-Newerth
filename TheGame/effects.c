
// (C) 2001 S2 Games

// effects.c

//=============================================================================
// Client side sounds/particles/etc.
//=============================================================================
#include "game.h"


//=============================================================================
//=============================================================================
char *vfclassnames[] =
{
	"null",
	"particles",
	"beam",

	""
};

char *sfxClassnames[] =
{
	"null",
	"sound",
	"emitter",
	"attractor",
	"tracer",

	""
};

#define NUL	0x01
#define PRT	0x02
#define	BEM	0x04

#define NULX	0x01
#define SNDX	0x02
#define	EMTX	0x04
#define	ATRX	0x08
#define	TRAX	0x10

#define ALL 0xff

char *particleStyleNames[] =
{
	"point",
	"trail",
	"directional",
	
	""
};

effectData_t	effectData[MAX_EFFECTS];

char			*exEffectNames[MAX_EFFECTS + MAX_EXEFFECTS];
exEffectData_t	exEffectData[MAX_EXEFFECTS];

configVar_t	effectData_desc[] =
{
	{"name",			0,			MAX_NAME_LEN,	"",	NULL,	T_STRING,		offsetof(effectData_t, name),			ALL },

	{"sound1",			0,			0,				"",			NULL,	T_DYNSTRING,	offsetof(effectData_t, sound[0]),		ALL },
	{"sound2",			0,			0,				"",			NULL,	T_DYNSTRING,	offsetof(effectData_t, sound[1]),		ALL },
	{"sound3",			0,			0,				"",			NULL,	T_DYNSTRING,	offsetof(effectData_t, sound[2]),		ALL },
	{"sound4",			0,			0,				"",			NULL,	T_DYNSTRING,	offsetof(effectData_t, sound[3]),		ALL },
	{"loopsound",		0,			0,				"",			NULL,	T_DYNSTRING,	offsetof(effectData_t, loopSound),		ALL },
	{"broadcastSound",	0,			0,				"",			NULL,	T_DYNSTRING,	offsetof(effectData_t, broadcastSound),	ALL },
	{"stopLoopSound",	0,			1,				"1",		NULL,	T_INT,			offsetof(effectData_t, stopLoopSound),	ALL },
	{"soundVolume",		0,			1,				"1.0",		NULL,	T_FLOATRANGE,	offsetof(effectData_t, soundVolume),	ALL },
	{"loopsoundVolume",	0,			1,				"1.0",		NULL,	T_FLOATRANGE,	offsetof(effectData_t, loopsoundVolume),	ALL },
	{"shakeScale",		0,			NO_LIMIT,		"0.0",		NULL,	T_FLOAT,		offsetof(effectData_t, shakeScale),	ALL },
	{"shakeRadius",		0,			NO_LIMIT,		"700",		NULL,	T_FLOAT,		offsetof(effectData_t, shakeRadius), ALL },
	{"shakeDuration",	0,			NO_LIMIT,		"4.0",		NULL,	T_FLOAT,		offsetof(effectData_t, shakeDuration), ALL },
	{"shakeFrequency",	0,			NO_LIMIT,		"250",		NULL,	T_FLOAT,		offsetof(effectData_t, shakeFrequency), ALL },

	{""}
};

configVar_t	exEffectData_desc[] =
{
	{ "name",			0,			MAX_NAME_LEN,	"",		NULL,	T_STRING,		offsetof(exEffectData_t, name),			ALL },

	{ "bounce",			0.0,		NO_LIMIT,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(exEffectData_t, bounce),		ALL },
	{ "gravity",		-NO_LIMIT,	NO_LIMIT,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(exEffectData_t, gravity),		ALL },
	{ "vForward",		-NO_LIMIT,	NO_LIMIT,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(exEffectData_t, vForward),		ALL },
	{ "vVertical",		-NO_LIMIT,	NO_LIMIT,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(exEffectData_t, vVertical),	ALL },
	{ "vHorizontal",	-NO_LIMIT,	NO_LIMIT,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(exEffectData_t, vHorizontal),	ALL },
	{ "vRandom",		-NO_LIMIT,	NO_LIMIT,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(exEffectData_t, vRandom),		ALL },

	{ "bone",			0,			0,				"",		NULL,	T_DYNSTRING,	offsetof(exEffectData_t, bone),			ALL },
	{""}
};

//=============================================================================
char *particleFlagNames[] =
{
	"parent_pos",
	"parent_vel",
	"param_location",
	"param_scale",
	"param_count",
	"param_groundz",
	"param_sinewave",
	"param_nearparent",
	"param_billboard2d",
	"",
};

configVar_t	viseffect_desc[] =
{
	{"visclass",		0,			NUM_VFCLASSES,			"0",		vfclassnames,		T_INT,			offsetof(viseffectData_t, vfclass),		ALL },

	{"flags",			0,			16,						"clear",	particleFlagNames,	T_FLAGS,		offsetof(viseffectData_t, flags),		PRT|BEM },
	{"shader",			0,			0,						"",			NULL,				T_DYNSTRING,	offsetof(viseffectData_t, shader),		PRT|BEM },
	{"size",			0,			NO_LIMIT,				"10",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, size),		PRT|BEM },
	{"sizeHeight",		0,			NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, sizeHeight),	PRT|BEM },
	{"growth",			-NO_LIMIT,	NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, growth),		PRT|BEM },
	{"alpha",			0,			1.0,					"1",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, alpha),		PRT|BEM },
	{"fade",			-1.0,		1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, fade),		PRT|BEM },
	{"white",			0,			1.0,					"1",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, white),		PRT|BEM },
	{"whitefade",		-1.0,		1.0,					"-1",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, whitefade),	PRT|BEM },
	{"red",				0,			1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, red),			PRT|BEM },
	{"green",			0,			1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, green),		PRT|BEM },
	{"blue",			0,			1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, blue),		PRT|BEM },
	{"redfade",			-1.0,		1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, redfade),		PRT|BEM },
	{"greenfade",		-1.0,		1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, greenfade),	PRT|BEM },
	{"bluefade",		-1.0,		1.0,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, bluefade),	PRT|BEM },
	{"delay",			0,			NO_LIMIT,				"0",		NULL,				T_INTRANGE,		offsetof(viseffectData_t, delay),		PRT|BEM },
	{"lifetime",		0,			NO_LIMIT,				"1000",		NULL,				T_INTRANGE,		offsetof(viseffectData_t, lifetime),	PRT|BEM },
	
	{"tilelength",		0,			NO_LIMIT,				"0",		NULL,				T_FLOAT,		offsetof(viseffectData_t, tilelength),	PRT|BEM },

	{"style",			0,			NUM_PARTICLE_STYLES,	"point",	particleStyleNames,	T_INT,			offsetof(viseffectData_t, style),		PRT },
	{"model",			0,			0,						"",			NULL,				T_DYNSTRING,	offsetof(viseffectData_t, model),		PRT },
	{"skin",			0,			0,						"",			NULL,				T_DYNSTRING,	offsetof(viseffectData_t, skin),		PRT },
	{"count",			0,			NO_LIMIT,				"20",		NULL,				T_INTRANGE,		offsetof(viseffectData_t, count),		PRT },
	{"velocity",		-NO_LIMIT,	NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, velocity),	PRT },
	{"acceleration",	-NO_LIMIT,	NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, acceleration),PRT },
	{"fps",				0,			NO_LIMIT,				"30",		NULL,				T_INTRANGE,		offsetof(viseffectData_t, fps),			PRT },
	{"offset",			-NO_LIMIT,	NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, offset),		PRT },
	{"gravity",			-NO_LIMIT,	NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, gravity),		PRT },
	{"angle",			0,			360,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, angle),		PRT },
	{"spin",			-360,		360,					"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, spin),		PRT },
	{"bone",			0,			0,						"",			NULL,				T_DYNSTRING,	offsetof(viseffectData_t, bone),		PRT|BEM },
	{"alt_offset",		-NO_LIMIT,	NO_LIMIT,				"0",		NULL,				T_FLOATRANGE,	offsetof(viseffectData_t, altOffset),	PRT },
	{"max_range",		0,			NO_LIMIT,				"0",		NULL,				T_FLOAT,		offsetof(viseffectData_t, maxRange),	PRT },
	{"minZ",			-NO_LIMIT,	NO_LIMIT,				"-999999",	NULL,			T_FLOAT,		offsetof(viseffectData_t, minZ),		PRT },
	{"maxZ",			-NO_LIMIT,	NO_LIMIT,				"999999",	NULL,			T_FLOAT,		offsetof(viseffectData_t, maxZ),		PRT },
	{""}
};

char	*sfxClassNames[] =
{
	"null",
	"sound",
	"emitter",
	"attractor",
	"tracer",

	""
};

char *emitterStyleNames[] =
{
	"point",
	"forward",
	"up",
	"right",
	"line",

	"",
};

configVar_t	subEffect_desc[] =
{
	{ "type",		0,	NUM_SUB_EFX_TYPES,	"0",		sfxClassNames,		T_INT,			offsetof(subEffectData_t, type),		ALL },
	
	{ "delay",		0,	NO_LIMIT,			"0",		NULL,				T_INTRANGE,		offsetof(subEffectData_t, delay),		ALL },
	{ "frequency",	0,	NO_LIMIT,			"0",		NULL,				T_INTRANGE,		offsetof(subEffectData_t, frequency),	ALL },
	{ "replays",	0,	NO_LIMIT,			"1",		NULL,				T_INTRANGE,		offsetof(subEffectData_t, replays),		ALL },
	{ "duration",	0,	NO_LIMIT,			"1",		NULL,				T_INTRANGE,		offsetof(subEffectData_t, duration),	ALL },

	{ "sound1",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, sound[0]),	SNDX },
	{ "sound2",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, sound[1]),	SNDX },
	{ "sound3",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, sound[2]),	SNDX },
	{ "sound4",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, sound[3]),	SNDX },
	{ "sound5",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, sound[4]),	SNDX },
	{ "sound6",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, sound[5]),	SNDX },

	{ "loop",		0,	1,					"0",		NULL,				T_INT,			offsetof(subEffectData_t, loop),		SNDX },
	{ "play2d",		0,	1,					"0",		NULL,				T_INT,			offsetof(subEffectData_t, play2d),		SNDX },
	{ "volume",		0,	1,					"1.0",		NULL,				T_FLOAT,		offsetof(subEffectData_t, volume),		SNDX },
	{ "radius",		0,	1000,				"50",		NULL,				T_FLOAT,		offsetof(subEffectData_t, radius),		SNDX },

	{ "style",		0,	NUM_EMITTER_STYLES,	"point",	emitterStyleNames,	T_INT,			offsetof(subEffectData_t, style),		EMTX },

	{ "velocityStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocity[0]),	EMTX },
	{ "velocityKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocity[1]),	EMTX },
	{ "velocityKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocity[2]),	EMTX },
	{ "velocityKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocity[3]),		EMTX },
	{ "velocityBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocityBias[0]),	EMTX },
	{ "velocityBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocityBias[1]),	EMTX },
	{ "velocityBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, velocityBias[2]),	EMTX },
	{ "velocityNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, velocityNumKeys),	EMTX },

	{ "angleStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angle[0]),	EMTX },
	{ "angleKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angle[1]),	EMTX },
	{ "angleKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angle[2]),	EMTX },
	{ "angleKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angle[3]),		EMTX },
	{ "angleBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angleBias[0]),	EMTX },
	{ "angleBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angleBias[1]),	EMTX },
	{ "angleBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, angleBias[2]),	EMTX },
	{ "angleNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, angleNumKeys),	EMTX },

	{ "offset",		-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, offset),	EMTX },
	{ "gravity",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, gravity),	EMTX },
	{ "inertia",	0,			NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, inertia),	EMTX },

	{ "count",		0,			NO_LIMIT,	"1",	NULL,	T_INTRANGE,		offsetof(subEffectData_t, count),	EMTX },
	{ "collide",	0,			1,			"0",	NULL,	T_INT,			offsetof(subEffectData_t, collide),	EMTX },
	{ "bounce",		0.0,		NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, bounce),	EMTX },
	{ "sticky",		0.0,		1.0,		"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, sticky),	EMTX },

	{ "lifetime",	0,	NO_LIMIT,	"1000",	NULL,	T_INTRANGE,	offsetof(subEffectData_t, lifetime),	EMTX },
	
	{ "pull",		-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, pull),	ATRX },
	{ "area",		0.0,		NO_LIMIT,	"10.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, area),	ATRX },

	{ "spacing",	0.0,		NO_LIMIT,	"50.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, spacing),	EMTX|TRAX },

	{ "shader",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, shader),	EMTX|TRAX },
	{ "model",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, model),	EMTX },
	{ "skin",	0,	0,	"",	NULL,	T_DYNSTRING,	offsetof(subEffectData_t, skin),	EMTX },

	{ "redStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, red[0]),	EMTX },
	{ "redKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, red[1]),	EMTX },
	{ "redKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, red[2]),	EMTX },
	{ "redKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, red[3]),		EMTX },
	{ "redBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, redBias[0]),	EMTX },
	{ "redBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, redBias[1]),	EMTX },
	{ "redBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, redBias[2]),	EMTX },
	{ "redNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, redNumKeys),	EMTX },

	{ "greenStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, green[0]),	EMTX },
	{ "greenKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, green[1]),	EMTX },
	{ "greenKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, green[2]),	EMTX },
	{ "greenKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, green[3]),		EMTX },
	{ "greenBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, greenBias[0]),	EMTX },
	{ "greenBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, greenBias[1]),	EMTX },
	{ "greenBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, greenBias[2]),	EMTX },
	{ "greenNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, greenNumKeys),	EMTX },

	{ "blueStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blue[0]),	EMTX },
	{ "blueKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blue[1]),	EMTX },
	{ "blueKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blue[2]),	EMTX },
	{ "blueKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blue[3]),		EMTX },
	{ "blueBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blueBias[0]),	EMTX },
	{ "blueBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blueBias[1]),	EMTX },
	{ "blueBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, blueBias[2]),	EMTX },
	{ "blueNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, blueNumKeys),	EMTX },

	{ "alphaStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alpha[0]),	EMTX },
	{ "alphaKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alpha[1]),	EMTX },
	{ "alphaKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alpha[2]),	EMTX },
	{ "alphaKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alpha[3]),		EMTX },
	{ "alphaBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alphaBias[0]),	EMTX },
	{ "alphaBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alphaBias[1]),	EMTX },
	{ "alphaBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, alphaBias[2]),	EMTX },
	{ "alphaNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, alphaNumKeys),	EMTX },

	{ "sizeStart",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, size[0]),	EMTX },
	{ "sizeKey1",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, size[1]),	EMTX },
	{ "sizeKey2",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, size[2]),	EMTX },
	{ "sizeKey3",	-NO_LIMIT,	NO_LIMIT,	"0.0",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, size[3]),		EMTX },
	{ "sizeBias1",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, sizeBias[0]),	EMTX },
	{ "sizeBias2",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, sizeBias[1]),	EMTX },
	{ "sizeBias3",	-NO_LIMIT,	NO_LIMIT,	"0.5",	NULL,	T_FLOATRANGE,	offsetof(subEffectData_t, sizeBias[2]),	EMTX },
	{ "sizeNumKeys",	1,	3,	"1",	NULL,	T_INT,	offsetof(subEffectData_t, sizeNumKeys),	EMTX },

	{""}

};
//=============================================================================


//=============================================================================
//ef* console commands
//=============================================================================
int	vfEditCurrent = 0;		//stores index of current particle system
int	efEditCurrent = 0;

int	sfxEditCurrent = 0;		//stores index of current particle system
int	efxEditCurrent = 0;

int		efLookup(char *name)
{
	int idx = 0;

	for (idx = 1; idx < MAX_EFFECTS; idx++)
	{
		if (stricmp(effectData[idx].name, name) == 0)
		{
			return idx + MAX_EXEFFECTS;
		}
	}

	SetInt(&idx, 1, MAX_EFFECTS - 1, &exEffectNames[MAX_EXEFFECTS], name);
	if (stricmp(exEffectNames[idx], name) != 0)
		idx = 0;

	return idx;
}

//=============================================================================
void	efEdit_Cmd(int argc, char *argv[])
{
	int	index = 0;

	if (!stricmp(argv[0], "new"))
	{
		for (index = 1; index < MAX_EFFECTS; index++)
		{
			if (!effectData[index].name[0])
			{
				efEditCurrent = index;
				break;
			}
		}

		if (index == MAX_EFFECTS)
		{
			core.Console_DPrintf("*** efEdit could not find a new effect! ***\n");
			return;
		}
	}
	else
	{
		SetInt(&efEditCurrent, 1, MAX_EFFECTS - 1, &exEffectNames[MAX_EXEFFECTS], argv[0]);
	}

	if (!effectData[efEditCurrent].touched)
	{
		xInit(effectData_desc, countConfigVars(effectData_desc), &effectData[efEditCurrent], efEditCurrent, false);
		for (index = 0; index < VISUALS_PER_EFFECT; index++)
			xInit(viseffect_desc, countConfigVars(viseffect_desc), &(effectData[efEditCurrent].visuals[index]), index, false);
		effectData[efEditCurrent].touched = true;
	}	
}

void	efxEdit_Cmd(int argc, char *argv[])
{
	int	index = 0;

	if (!stricmp(argv[0], "new"))
	{
		for (index = 1; index < MAX_EFFECTS; index++)
		{
			if (!exEffectData[index].name[0])
			{
				efxEditCurrent = index;
				break;
			}
		}

		if (index == MAX_EFFECTS)
		{
			core.Console_DPrintf("*** efxEdit could not find a new effect! ***\n");
			return;
		}
	}
	else
	{
		SetInt(&efxEditCurrent, 1, MAX_EXEFFECTS - 1, exEffectNames, argv[0]);
	}

	if (!exEffectData[efxEditCurrent].touched)
	{
		xInit(exEffectData_desc, countConfigVars(exEffectData_desc), &exEffectData[efxEditCurrent], efxEditCurrent, false);
		for (index = 0; index < MAX_SUB_EFFECTS; index++)
			xInit(subEffect_desc, countConfigVars(subEffect_desc), &(exEffectData[efxEditCurrent].subEffect[index]), index, false);
		exEffectData[efxEditCurrent].touched = true;
	}
}

//=============================================================================
void	efList_Cmd(int argc, char *argv[])
{
	//int n;

	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for effect #%i: %s\n", efEditCurrent, exEffectNames[MAX_EXEFFECTS + efEditCurrent]);
	core.Console_Printf("=========================================\n");
	xList(effectData_desc, countConfigVars(effectData_desc), &effectData[efEditCurrent], ALL);

	//for (n = 0; n < PARTICLES_PER_EFFECT; n++)
	{
		core.Console_Printf("\nVisual effect %i:\n", vfEditCurrent);
		core.Console_Printf("===================\n");
		xList(viseffect_desc, countConfigVars(viseffect_desc), &effectData[efEditCurrent].visuals[vfEditCurrent], ALL);
	}
}

void	efxList_Cmd(int argc, char *argv[])
{
	//int n;

	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for exEffect #%i: %s\n", efxEditCurrent, exEffectNames[efxEditCurrent]);
	core.Console_Printf("=========================================\n");
	xList(exEffectData_desc, countConfigVars(exEffectData_desc), &exEffectData[efxEditCurrent], ALL);
}

//=============================================================================
void	efSave_Cmd(int argc, char *argv[])
{
	char	fname[_MAX_PATH];
	file_t *f;
	int n;
	int	index = efEditCurrent;
	bool all = false;

	if (argc && !stricmp(argv[0], "all"))
	{
		index = 1;
		all = true;
	}

	while (effectData[index].name[0])
	{
		strcpy(fname, fmt("effects/%s.effect", effectData[index].name));

		//open it
		f = core.File_Open(fname, "w");
		if (!f)
		{
			core.Console_Printf("Couldn't open %s for writing\n", fname);
			return;
		}

		//write the config
		xSave(f, effectData_desc, countConfigVars(effectData_desc), &effectData[index], "ef", ALL);
		
		
		core.File_Printf(f, "\n//Particles:\n");
		for (n = 0; n < VISUALS_PER_EFFECT; n++)
		{
			core.File_Printf(f, "vfEdit %i\n", n);
			xSave(f, viseffect_desc, countConfigVars(viseffect_desc), &effectData[index].visuals[n], "vf", ALL);
			core.File_Printf(f, "\n");
		}

		core.File_Close(f);
		core.Console_Printf("Wrote %s\n", fname);
		
		if (!all)
			break;

		index++;
	}
}

void	efxSave_Cmd(int argc, char *argv[])
{
	char	fname[_MAX_PATH];
	file_t *f;
	int n;
	int	index = efxEditCurrent;
	bool all = false;

	if (argc && !stricmp(argv[0], "all"))
	{
		index = 1;
		all = true;
	}

	while (exEffectData[index].name[0])
	{
		strcpy(fname, fmt("effects/%s.efx", exEffectData[index].name));

		//open it
		f = core.File_Open(fname, "w");
		if (!f)
		{
			core.Console_Printf("Couldn't open %s for writing\n", fname);
			return;
		}

		//write the config
		xSave(f, exEffectData_desc, countConfigVars(exEffectData_desc), &exEffectData[index], "efx", ALL);
		
		
		core.File_Printf(f, "\n//Sub Effects:\n");
		for (n = 0; n < MAX_SUB_EFFECTS; n++)
		{
			core.File_Printf(f, "sfxEdit %i\n", n);
			xSave(f, subEffect_desc, countConfigVars(subEffect_desc), &exEffectData[index].subEffect[n], "sfx", (1 << exEffectData[index].subEffect[n].type));
			core.File_Printf(f, "\n");
		}

		core.File_Close(f);
		core.Console_Printf("Wrote %s\n", fname);
		
		if (!all)
			break;

		index++;
	}
}

//=============================================================================
void	efSet_Cmd(int argc, char *argv[])
{
	xSet(effectData_desc, countConfigVars(effectData_desc), &effectData[efEditCurrent], argc, argv);
}

void	efxSet_Cmd(int argc, char *argv[])
{
	xSet(exEffectData_desc, countConfigVars(exEffectData_desc), &exEffectData[efxEditCurrent], argc, argv);
}
//=============================================================================
//=============================================================================

//=============================================================================
//vf* console commands
//=============================================================================

//=============================================================================
void	vfEdit_Cmd(int argc, char *argv[])
{
	SetInt(&vfEditCurrent, 0, VISUALS_PER_EFFECT - 1, NULL, argv[0]);
}

void	sfxEdit_Cmd(int argc, char *argv[])
{
	SetInt(&sfxEditCurrent, 0, MAX_SUB_EFFECTS - 1, NULL, argv[0]);
}

//=============================================================================
void	vfList_Cmd(int argc, char *argv[])
{
	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for %s #%i of effect #%i: %s\n", vfclassnames[effectData[efEditCurrent].visuals[vfEditCurrent].vfclass], vfEditCurrent, efEditCurrent, exEffectNames[MAX_EXEFFECTS + efEditCurrent]);
	core.Console_Printf("=========================================\n");
	xList(viseffect_desc, countConfigVars(viseffect_desc), &(effectData[efEditCurrent].visuals[vfEditCurrent]), (1<<effectData[efEditCurrent].visuals[vfEditCurrent].vfclass));
}

void	sfxList_Cmd(int argc, char *argv[])
{
	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for %s #%i of exEffect #%i: %s\n", sfxClassnames[exEffectData[efxEditCurrent].subEffect[sfxEditCurrent].type], sfxEditCurrent, efxEditCurrent, exEffectNames[efxEditCurrent]);
	core.Console_Printf("=========================================\n");
	xList(subEffect_desc, countConfigVars(subEffect_desc), &(exEffectData[efxEditCurrent].subEffect[sfxEditCurrent]), (1<<exEffectData[efxEditCurrent].subEffect[sfxEditCurrent].type));
}

//=============================================================================
void	vfSet_Cmd(int argc, char *argv[])
{
	xSet(viseffect_desc, countConfigVars(viseffect_desc), &(effectData[efEditCurrent].visuals[vfEditCurrent]), argc, argv);
}

void	sfxSet_Cmd(int argc, char *argv[])
{
	xSet(subEffect_desc, countConfigVars(subEffect_desc), &(exEffectData[efxEditCurrent].subEffect[sfxEditCurrent]), argc, argv);
}
//=============================================================================
//=============================================================================


/*==========================

  efFileCallback

  Initializes an effect file

 ==========================*/

void	efFileCallback(const char *filename, void *userdata)
{
	char fname[_MAX_PATH];

	strcpy(fname, Filename_GetFilename((char*)filename));
	Filename_StripExtension(fname, fname);

	core.Cmd_Exec("efedit new");
	core.Cmd_Exec(fmt("efset name %s", fname));
	core.Cmd_Exec(fmt("exec /effects/%s", filename));
}


/*==========================

  efxFileCallback

  Initializes an effect file

 ==========================*/

void	efxFileCallback(const char *filename, void *userdata)
{
	char fname[_MAX_PATH];

	strcpy(fname, Filename_GetFilename((char*)filename));
	Filename_StripExtension(fname, fname);

	core.Cmd_Exec("efxedit new");
	core.Cmd_Exec(fmt("efxset name %s", fname));
	core.Cmd_Exec(fmt("exec /effects/%s", filename));
}


/*==========================

  GetEffectNumber

  Translates an effect name into it's index

 ==========================*/

int	GetEffectNumber(char *name)
{
	int	n;

	for (n = 0; n < MAX_EFFECTS + MAX_EXEFFECTS; n++)
	{
		if (strcmp(exEffectNames[n], name))
			continue;
		return n;
	}
	return 0; 
}


/*==========================

  InitEffects

 ==========================*/

void	InitEffects()
{
	int index, n;

	core.Console_Printf(" * Initializing effects...\n");

	core.Cmd_Register("efEdit",	efEdit_Cmd);
	core.Cmd_Register("efSet",	efSet_Cmd);
	core.Cmd_Register("efList",	efList_Cmd);
	core.Cmd_Register("efSave",	efSave_Cmd);
	core.Cmd_Register("vfEdit",	vfEdit_Cmd);
	core.Cmd_Register("vfSet",	vfSet_Cmd);
	core.Cmd_Register("vfList",	vfList_Cmd);

	core.Cmd_Register("efxEdit",	efxEdit_Cmd);
	core.Cmd_Register("efxSet",		efxSet_Cmd);
	core.Cmd_Register("efxList",	efxList_Cmd);
	core.Cmd_Register("efxSave",	efxSave_Cmd);
	core.Cmd_Register("sfxEdit",	sfxEdit_Cmd);
	core.Cmd_Register("sfxSet",		sfxSet_Cmd);
	core.Cmd_Register("sfxList",	sfxList_Cmd);

	memset(effectData, 0, sizeof(effectData_t) * MAX_EFFECTS);
	for (n = 0; n < MAX_EFFECTS; n++)
	{
		xInit(effectData_desc, countConfigVars(effectData_desc), &effectData[n], n, true);
			for (index = 0; index < VISUALS_PER_EFFECT; index++)
				xInit(viseffect_desc, countConfigVars(viseffect_desc), &(effectData[n].visuals[index]), index, true);
	}
	for (n = 0; n < MAX_EFFECTS; n++)
		exEffectNames[MAX_EXEFFECTS + n] = effectData[n].name;

	memset(exEffectData, 0, sizeof(exEffectData_t) * MAX_EXEFFECTS);
	for (n = 0; n < MAX_EXEFFECTS; n++)
	{
		xInit(exEffectData_desc, countConfigVars(exEffectData_desc), &exEffectData[n], n, true);
		for (index = 0; index < MAX_SUB_EFFECTS; index++)
			xInit(subEffect_desc, countConfigVars(subEffect_desc), &(exEffectData[n].subEffect[index]), index, true);
	}
	for (n = 0; n < MAX_EXEFFECTS; n++)
		exEffectNames[n] = exEffectData[n].name;

	core.System_Dir("effects", "*.effect", false, NULL, efFileCallback, NULL);	
	core.System_Dir("effects", "*.efx", false, NULL, efxFileCallback, NULL);	
}
