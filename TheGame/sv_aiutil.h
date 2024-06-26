#ifndef SV_AI_UTIL_H
#define SV_AI_UTIL_H

struct ai_s;
struct aiGoal_s;
struct aiNav_s;
struct aiState_s;

extern cvar_t sv_debug;
extern int numAttackableUnits;
extern int attackableUnits[];

// specific random time function because we assume lo and hi to be in milliseconds (> 0.99f)
int SV_AI_RandomTime(int lo, int hi);

#define AI_PRINTF if (sv_debug.integer) cores.Console_DPrintf
#define repeat int repeatvar = 0; while ( repeatvar++ < 1000 )
#define repeatn(n) int repeatvar = 0; while ( repeatvar++ < n )

void SV_AISetSecondaryAnim(serverObject_t *obj, int anim);

#define SV_Building_IsValid(building) (building && building->base.active && (building->base.health > 0) && (GetObjectByType(building->base.type)->objclass == OBJCLASS_BUILDING))
#define SV_Unit_IsValid(unit) (unit && unit->base.active && (unit->base.health > 0) && (GetObjectByType(unit->base.type)->objclass == OBJCLASS_UNIT))
#define SV_Object_IsValid(object) (object && object->base.active && (object->base.health > 0))

#endif

