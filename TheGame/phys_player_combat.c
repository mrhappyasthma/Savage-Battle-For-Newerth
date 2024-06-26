// (C) 2001 S2 Games

// phys_player_combat.c


#include "game.h"

extern physicsParams_t p;
extern phys_out_t *p_out;
extern playerState_t *ps;
extern inputState_t *in;
extern objectData_t *unit;
extern cvar_t p_lunge;
extern cvar_t p_cooldownRate;
extern cvar_t p_maxBlockTime;

//these are named the same as the functions in object_config.c intentionally
//for ease of use, and as a safeguard so that the function headers never get exposed to this module
#define GetObjectByType(type) (p.objectTypeFunc(type))
#define IsCharacterType(type) (GetObjectByType(type)->objclass == OBJCLASS_UNIT && !GetObjectByType(type)->isVehicle)
#define IsUnitType(type) (GetObjectByType(type)->objclass == OBJCLASS_UNIT)
#define IsBuildingType(type) (GetObjectByType(type)->objclass == OBJCLASS_BUILDING)
#define IsWorkerType(type) (GetObjectByType(type)->isWorker && IsUnitType(type))
#define IsMobileType(type) IsUnitType(type)
#define IsItemType(type) (GetObjectByType(type)->objclass == OBJCLASS_ITEM)
#define IsWeaponType(type) (GetObjectByType(type)->objclass == OBJCLASS_WEAPON)
#define IsMeleeType(type) (GetObjectByType(type)->objclass == OBJCLASS_MELEE)
#define IsUpgradeType(type) (GetObjectByType(type)->objclass == OBJCLASS_UPGRADE)



/*==========================

  WeaponStateChange

 ==========================*/

void	WeaponStateChange(int wpstate)
{
	if (wpstate == AS_WPSTATE_FIRE)
	{
		if (ps->weaponState != AS_WPSTATE_FIRE || !GetObjectByType(ps->inventory[ps->item])->continuous)
		{
			//send a fire weapon event
			Phys_AddEvent(ps, EVENT_WEAPON_FIRE, 0, ps->inventory[ps->item]);
		}
	}

	if (!(ps->flags & PS_ATTACK_LOCKED) || !GetObjectByType(ps->inventory[ps->item])->continuous)
		ps->wpAnimStartTime = in->gametime;

	ps->wpStateStartTime = in->gametime;
	
	ps->attackFlags = 0;
	ps->weaponState = wpstate;
	ps->attackFlags |= PS_ATK_CLEAR_MELEE_HITS;
	if (wpstate <= AS_BLOCK)
		p_out->doAttack = 0;
}



/*==========================

  CheckAttackImpact

  either the time when we fire a weapon, or the time when a melee attack does a damage trace
  a weapon sets "doAttack" whenever this function is called, while a melee attack may not
  want to perform the actual damage trace until the "impact time" has been reached

 ==========================*/


void	CheckAttackImpact()
{
	int				impactTime;
	int				impactStopTime;
	bool			isweapon = false;
	bool			isitem = false;
	objectData_t	*wpn = GetObjectByType(ps->inventory[ps->item]);

	if (IS_MELEE_ATTACK(ps->weaponState))
	{
		impactTime = unit->attacks[ps->weaponState].impact.min + ps->wpStateStartTime;
		impactStopTime = unit->attacks[ps->weaponState].impact.max + ps->wpStateStartTime;
	}
	else if (IsWeaponType(ps->inventory[ps->item]) || IsItemType(ps->inventory[ps->item]))
	{
		if (ps->weaponState == AS_WPSTATE_FIRE)
			impactTime = ps->wpStateStartTime + MIN(wpn->fireDelay, wpn->refreshTime - 1);
		else
			impactTime = 0;
		impactStopTime = 0;
		isweapon = true;
	}
	else
	{
		return;
	}

	if (in->gametime >= impactTime && !(ps->attackFlags & PS_ATK_FINISHED_IMPACT))
	{
		if (isweapon)
		{
			if (wpn->useMana)
			{
				if (ps->mana < wpn->manaCost)
					return;
				else if (ps->mana >= wpn->manaCost)
					ps->mana -= wpn->manaCost;
			}
			else
			{
				//don't fire the weapon if we don't have any ammo
				if (ps->ammo[ps->item] == 0)
					return;
				else if (ps->ammo[ps->item] > 0)
					ps->ammo[ps->item]--;
			}
		}
		else if (isitem)
		{
			//if it has an ammo coumt, decrement it
			if (ps->ammo[ps->item] > 0)
				ps->ammo[ps->item]--;
		}

		if (!impactStopTime || in->gametime <= impactStopTime)
			p_out->doAttack = ps->weaponState;
		if (in->gametime >= impactStopTime)
			ps->attackFlags |= PS_ATK_FINISHED_IMPACT;
	}
}




/*==========================

  CheckAttackLunge

  an attack lunge is when we get a forward momentum from doing a melee attack

 ==========================*/

void	CheckAttackLunge()
{
	int lungeStartTime;

	if (!p_lunge.integer)
		return;

	if (!IS_MELEE_ATTACK(ps->weaponState))
		return;

	lungeStartTime = unit->attacks[ps->weaponState].lunge + ps->wpStateStartTime;

	if (!unit->attacks[ps->weaponState].lungeTime)
		return;

	//begin the lunge momentum?
	if (in->gametime >= lungeStartTime && !(ps->attackFlags & PS_ATK_STARTED_LUNGE) && ps->flags & PS_ON_GROUND)
	{
		ps->stunFlags	|= STUN_MOVEMENT;
		ps->stunTime	= in->gametime + unit->attacks[ps->weaponState].lungeTime;		
		ps->velocity[0]	= p.forward[0] * unit->attacks[ps->weaponState].xLunge;
		ps->velocity[1] = p.forward[1] * unit->attacks[ps->weaponState].yLunge;
		ps->attackFlags |= PS_ATK_STARTED_LUNGE;
	}
}



/*==========================

  CheckAttackFinished

  if the attack has finished, switch the weapon state to nextWeaponState

 ==========================*/

void	CheckAttackFinished(int nextWeaponState)
{
	int	attackEndTime;

	if (!IS_MELEE_ATTACK(ps->weaponState) && 
		ps->weaponState != AS_WPSTATE_FIRE &&
		ps->weaponState != AS_WPSTATE_BACKFIRE)
		return;

	if (ps->weaponState == AS_WPSTATE_FIRE)
		attackEndTime = GetObjectByType(ps->inventory[ps->item])->refreshTime + ps->wpStateStartTime;
	else if (ps->weaponState == AS_WPSTATE_BACKFIRE)
		attackEndTime = GetObjectByType(ps->inventory[ps->item])->backfireTime + ps->wpStateStartTime;
	else
		attackEndTime = unit->attacks[ps->weaponState].time + ps->wpStateStartTime;

	if (in->gametime >= attackEndTime)
	{
		WeaponStateChange(nextWeaponState);

		//remove items that are out of ammo from the inventory here, so
		//that players aren't jolted immediately into third person
		if (IsItemType(ps->inventory[ps->item]) && ps->ammo[ps->item] <= 0)
		{
			int start = ps->item;

			ps->ammo[ps->item] = 0;
			ps->inventory[ps->item] = 0;

			do
			{
				if (--ps->item < 0)
					ps->item = MAX_INVENTORY - 1;
			}
			while (!IsWeaponType(ps->inventory[ps->item]) &&
					!IsMeleeType(ps->inventory[ps->item]) &&
					ps->item != start);
		}
	}
}


/*==========================

  Phys_DoCombat

  the main combat processing function

 ==========================*/

void	Phys_DoCombat()
{
	bool				stunned = ps->stunFlags & STUN_COMBAT;
	objectData_t		*obj = GetObjectByType(ps->inventory[ps->item]);
	int					attackEndTime;
	int					newWpState = 0;
	objectData_t		*unit = GetObjectByType(ps->unittype);

	//skip all combat if stunned
	if (stunned && ps->stunTime > in->gametime)
		return;
	else
		ps->stunFlags &= ~STUN_COMBAT;

	//no fighting while you are revived
	if (ps->animState == AS_RESURRECTED)
		return;

	//process current attack state
	switch (ps->weaponState)
	{

	//=========================================================================
	case AS_IDLE:	//no weapon state, so we're free to initiate an attack
		if (unit->isVehicle)
		{
			if (in->buttons & B_ATTACK)
			{
				ps->charge = 0.0;

				if (IsWeaponType(ps->inventory[ps->item]))
				{				
					//only 'repeat' style weapons will fire when this is not an initial press
					if ((ps->flags & PS_ATTACK_LOCKED) && (obj->refreshTime > 0))
						break;

					if ((GetObjectByType(ps->inventory[ps->item])->useMana && ps->mana < GetObjectByType(ps->inventory[ps->item])->manaCost) ||
						(!GetObjectByType(ps->inventory[ps->item])->useMana && !ps->ammo[ps->item]))
						break;

					//proceed with a new attack
					if (obj->spinupTime)
						WeaponStateChange(AS_WPSTATE_SPINUP);
					else
					{
						if (obj->chargeTime > 0)
							WeaponStateChange(AS_WPSTATE_CHARGE);
						else
							WeaponStateChange(AS_WPSTATE_FIRE);
					}
				}
			}
			break;
		}

		if (in->buttons & B_BLOCK && obj->canBlock)
		{
			if (!(ps->flags & PS_BLOCK_LOCKED))
				WeaponStateChange(AS_BLOCK);
		}
		else if (in->buttons & B_ATTACK)
		{
			//can't fire while blocking
			if (ps->weaponState == AS_BLOCK)
				break;

			ps->charge = 0.0;

			if (IsWeaponType(ps->inventory[ps->item]) || IsItemType(ps->inventory[ps->item]))
			{				
				if ((obj->useMana && ps->mana < obj->manaCost) ||
					(!obj->useMana && !ps->ammo[ps->item]))
					break;

				//proceed with a new attack
				if (obj->spinupTime)
				{
					//the weapon needs to spin up before it can be fired
					WeaponStateChange(AS_WPSTATE_SPINUP);
				}
				else
				{
					if (obj->chargeTime > 0)
					{
						//this weapon uses the 'charge' field as a multiplier for some projectile property
						WeaponStateChange(AS_WPSTATE_CHARGE);
					}
					else
					{
						//the weapon fires as soon as it is triggered
						WeaponStateChange(AS_WPSTATE_FIRE);
					}
				}
			}
			else if (IsMeleeType(ps->inventory[ps->item]))
			{
				int	attack = obj->useAltMelee ? AS_ALT_MELEE_1 : AS_MELEE_1;
				
				if (obj->manaCost > ps->mana)
					break;
				
				if (obj->targetCorpse)
				{
					traceinfo_t	trace;
					vec3_t		end, bmin, bmax;
					vec3_t		forward, right;
					float		scale;

					HeadingFromAngles(0, ps->angle[YAW], forward, right);
					scale = (ps->angle[PITCH] - 90.0) / 90.0;

					//set the end point of the trace
					M_PointOnLine(ps->pos, forward, unit->attacks[attack].range, end);

					//and the bounds for the trace
					M_CopyVec3(unit->bmin, bmin);
					M_CopyVec3(unit->bmax, bmax);
					if (scale < 0)
						bmin[2] += bmax[2] * scale;
					else
						bmax[2] += bmax[2] * scale;

					p.tracefunc(&trace, ps->pos, end, bmin, bmax, SURF_TERRAIN);
					if (trace.index < 0 || !(trace.flags & SURF_REVIVABLE))
						break;
				}

				if (!(ps->flags & PS_ATTACK_LOCKED))
				{
					WeaponStateChange(attack);
					ps->mana -= obj->manaCost;
				}
			}
		}
		break;

	//=========================================================================
	case AS_BLOCK:
		if (!(in->buttons & B_BLOCK) || in->gametime - ps->wpStateStartTime > p_maxBlockTime.integer)
			WeaponStateChange(AS_IDLE);
		break;

	//=========================================================================
	case AS_WPSTATE_BACKFIRE:
		CheckAttackImpact();
		CheckAttackFinished(AS_IDLE);
		break;

	//=========================================================================
	case AS_WPSTATE_CHARGE:
		//calculate charge amount, used as a multiplier for projectile fields
		//the longer the trigger is held down, the higher the charge gets
		ps->charge = (in->gametime - ps->wpStateStartTime) / (float)obj->chargeTime;
		if (ps->charge < 0)
			ps->charge = 0;

		if (obj->startFuseAtCharge && 
			(in->gametime - ps->wpStateStartTime) >= GETRANGE(obj->fuseTime))
			WeaponStateChange(AS_WPSTATE_BACKFIRE);

		//check for trigger release
		if (!(in->buttons & B_ATTACK))
			WeaponStateChange(AS_WPSTATE_FIRE);

		break;

	//=========================================================================
	case AS_WPSTATE_SPINUP:
		//check for trigger release
		if (!(in->buttons & B_ATTACK))
		{
			if (obj->spindownTime)
				WeaponStateChange(AS_WPSTATE_SPINDOWN);
			else
				WeaponStateChange(AS_IDLE);
		}
		else
		{
			if (in->gametime - ps->wpStateStartTime >= obj->spinupTime)
			{
				if (obj->chargeTime > 0)
					WeaponStateChange(AS_WPSTATE_CHARGE);
				else
					WeaponStateChange(AS_WPSTATE_FIRE);
			}
		}

		break;

	//=========================================================================
	case AS_WPSTATE_FIRE:
	{
		CheckAttackImpact();

		if (obj->repeat && (in->buttons & B_ATTACK))
		{
			if (obj->overheatTime)
			{
				ps->overheatCounter += p.frametime * 1000.0;

				if (ps->overheatCounter >= obj->overheatTime)
				{
					ps->overheatCounter = obj->overheatTime;
					if (obj->cooldownTime)
						newWpState = AS_WPSTATE_OVERHEAT;
					else
						newWpState = AS_WPSTATE_FIRE;
				}
				else
					newWpState = AS_WPSTATE_FIRE;
			}
			else
			{
				newWpState = AS_WPSTATE_FIRE;
			}

			if (newWpState == AS_WPSTATE_FIRE && !obj->repeat && (in->buttons & B_ATTACK))
			{
				if (obj->spindownTime)
					newWpState = AS_WPSTATE_SPINDOWN;
				else
					newWpState = AS_IDLE;
			}
		}
		else
		{
			if (obj->spindownTime)
				newWpState = AS_WPSTATE_SPINDOWN;
			else
				newWpState = AS_IDLE;
		}		

		if ((obj->useMana && ps->mana < obj->manaCost) ||
			(!obj->useMana && !ps->ammo[ps->item]))
		{
			if (obj->spindownTime)
				newWpState = AS_WPSTATE_SPINDOWN;
			else
				newWpState = AS_IDLE;
		}

		CheckAttackFinished(newWpState);
		break;
	}

	//=========================================================================
	case AS_WPSTATE_OVERHEAT:
		if ((in->gametime - ps->wpStateStartTime) >= obj->cooldownTime)
		{
			WeaponStateChange(AS_IDLE);
			ps->overheatCounter = 0;
		}
		break;

	//=========================================================================
	case AS_WPSTATE_SPINDOWN:
		if ((in->gametime - ps->wpStateStartTime) >= obj->spindownTime)
		{			
			WeaponStateChange(AS_IDLE);
		}
		break;

	//=========================================================================
	case AS_ALT_MELEE_1:
	case AS_ALT_MELEE_2:
	case AS_ALT_MELEE_3:
	case AS_ALT_MELEE_4:
	case AS_MELEE_1:
	case AS_MELEE_2:
	case AS_MELEE_3:
	case AS_MELEE_4:
	{
		int flurryWindowStart, flurryWindowEnd;

		attackEndTime = unit->attacks[ps->weaponState].time + ps->wpStateStartTime;
		flurryWindowStart = unit->attacks[ps->weaponState].flurry + ps->wpStateStartTime;
		flurryWindowEnd = unit->attacks[ps->weaponState].flurryTime + flurryWindowStart + ps->wpStateStartTime;

		CheckAttackImpact();
		CheckAttackLunge();

		if (unit->attacks[ps->weaponState].flurryTime > 0)
		{
			if ((in->buttons & B_BLOCK) && obj->canBlock)
			{
				if (in->gametime >= flurryWindowStart)
				{
					WeaponStateChange(AS_BLOCK);
					break;
				}
			}

			if (ps->attackFlags & PS_ATK_QUEUE_FLURRY)
			{
				if (in->gametime >= flurryWindowStart)
				{
					//next attack should begin exactly at the beginning of the flurry window
					WeaponStateChange(ps->weaponState+1);
					break;
				}
			}
			else if (in->buttons & B_ATTACK && !(ps->flags & PS_ATTACK_LOCKED))
			{
				if (in->gametime >= flurryWindowStart && in->gametime <= flurryWindowEnd)
				{
					if (in->buttons & B_BLOCK && obj->canBlock)
					{
						WeaponStateChange(AS_BLOCK);
					}
					else
					{
						WeaponStateChange(ps->weaponState + 1);
					}
					break;
				}
				else if (in->gametime > flurryWindowEnd)
				{
					//start the melee chain over again
					WeaponStateChange(obj->useAltMelee ? AS_ALT_MELEE_1 : AS_MELEE_1);
					break;
				}
				else
				{
					//ps->attackFlags |= PS_ATK_QUEUE_FLURRY;
					break;
				}
			}
		}
		/*
//		if (!(ps->attackFlags & PS_ATK_CANCEL_FLURRY))
		{
			if (in->buttons & B_ATTACK && !(ps->flags & PS_ATTACK_LOCKED))
			{
				if (ps->currentAttack < (ATTACK_JAB + unitdata->flurryCount))
				{
					//queue a flurry attack if possible
					if (in->gametime >= flurryWindowStart && in->gametime <= flurryWindowEnd)
					{
						AttackStateChange(ps->currentAttack+1);
						break;
					}
					else if (in->gametime > flurryWindowEnd)
					{
						//if we missed the window, go back to the first attack
						AttackStateChange(ATTACK_JAB);
						break;
					}
					else
					{
						//missed the flurry window, cancel the flurry
			//			ps->attackFlags |= PS_ATK_CANCEL_FLURRY;
					}
				}
			}
		}
*/
		CheckAttackFinished(0);
		break;
	}

	//=========================================================================
	case AS_MELEE_CHARGE:
		ps->charge = (in->gametime - ps->wpStateStartTime) / (float)MELEE_CHARGE_TIME;
		if (ps->charge > 1.0)
			ps->charge = 1.0;

		attackEndTime = in->gametime - ps->wpStateStartTime;

		if (!(in->buttons & B_ATTACK))
			WeaponStateChange(AS_MELEE_RELEASE);
		break;

	//=========================================================================
	case AS_MELEE_RELEASE:
		CheckAttackImpact();
		CheckAttackFinished(0);
		break;

	//=========================================================================
	default:
		core.Console_DPrintf("Unhandled wpState #%i in Phys_DoCombat()\n", ps->weaponState);
		WeaponStateChange(AS_IDLE);
		break;
	}

	//decrease overheat counter
	if (ps->weaponState != AS_WPSTATE_OVERHEAT && ps->weaponState != AS_WPSTATE_FIRE)
	{
		ps->overheatCounter -= p.frametime * (1000.0 * p_cooldownRate.value);
		if (ps->overheatCounter < 0)
			ps->overheatCounter = 0;
	}

	//set button flags
	if (!(in->buttons & B_ATTACK))
		ps->flags &= ~PS_ATTACK_LOCKED;
	else
		ps->flags |= PS_ATTACK_LOCKED;

	if (obj->canBlock)
	{
		if (!(in->buttons & B_BLOCK))
			ps->flags &= ~PS_BLOCK_LOCKED;
		else
			ps->flags |= PS_BLOCK_LOCKED;
	}
}
