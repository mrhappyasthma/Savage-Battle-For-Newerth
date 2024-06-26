// (C) 2002 S2 Games
// techtree.c
// techtree info needed by server and client

#include "game.h"

static int			techtree_count = 0;
static techEntry_t	techtree[MAX_TECHTREE_ITEMS];
objectData_t *(*objTypeFunc)(int type);

//these are named the same as the functions in object_config.c intentionally
//for ease of use, and as a safeguard so that the function headers never get exposed to this module
#define GetObjectByType(type) (objTypeFunc(type))
#define IsCharacterType(type) (GetObjectByType(type)->objclass == OBJCLASS_UNIT && !GetObjectByType(type)->isVehicle)
#define IsUnitType(type) (GetObjectByType(type)->objclass == OBJCLASS_UNIT)
#define IsBuildingType(type) (GetObjectByType(type)->objclass == OBJCLASS_BUILDING)
#define IsWorkerType(type) (GetObjectByType(type)->isWorker && IsUnitType(type))
#define IsMobileType(type) IsUnitType(type)
#define IsItemType(type) (GetObjectByType(type)->objclass == OBJCLASS_ITEM)
#define IsWeaponType(type) (GetObjectByType(type)->objclass == OBJCLASS_WEAPON)
#define IsMeleeType(type) (GetObjectByType(type)->objclass == OBJCLASS_MELEE)
#define IsUpgradeType(type) (GetObjectByType(type)->objclass == OBJCLASS_UPGRADE)
#define GetObjectTypeByName Tech_GetObjectTypeByName
#define GetObjectByName Tech_GetObjectByName

objectData_t *Tech_GetObjectByName(const char *name)
{
	int n;

	for (n=0; n<MAX_OBJECT_TYPES; n++)
	{
		if (stricmp(name, GetObjectByType(n)->name)==0)
			return GetObjectByType(n);
	}

	return GetObjectByType(0);
}

int	Tech_GetObjectTypeByName(const char *name)
{
	int n;

	for (n=0; n<MAX_OBJECT_TYPES; n++)
	{
		if (stricmp(name, GetObjectByType(n)->name)==0)
			return n;
	}

	return 0;
}


/*==========================

  Tech_GetEntry

  Returns the techtree entry of a give object type

 ==========================*/

techEntry_t	*Tech_GetEntry(int objtype)
{
	int n;

	if (!GetObjectByType(objtype)->objclass)
		return NULL;

	for (n = 0; n < techtree_count; n++)
	{
		if(techtree[n].objnum == objtype)
			return &techtree[n];
	}

	//fixme: return an empty techitem instead
	return NULL;
}


/*==========================

  Tech_HasEntry

 ==========================*/

bool	Tech_HasEntry(int objtype)
{
	int n;

	if (!GetObjectByType(objtype)->objclass)
		return false;

	for (n = 0; n < techtree_count; n++)
	{
		if(techtree[n].objnum == objtype)
			return true;
	}

	return false;
}


/*==========================

  Tech_CheckRequirements

  Returs true if an items depenencies are met

 ==========================*/

bool	Tech_CheckRequirements(techEntry_t *tech, researchData_t research[])
{
	int n, techLevel, baseLevel;

	//tally up the tech and base points for the team
	techLevel = baseLevel = 0;
	for (n = 0; n < MAX_OBJECT_TYPES; n++)
	{
		if (research[n].count)
		{
			techLevel += GetObjectByType(n)->techPointValue;
			baseLevel += GetObjectByType(n)->basePointValue;
		}
	}

	//test the maxpopulation cap
	if (tech->maxPopulation > 0 && research[tech->objnum].count >= tech->maxPopulation)
		return false;

	//test the point values
	if (baseLevel < tech->minBaseLevel  || techLevel < tech->minTechLevel)
		return false;

	//check for specific tech requirements
	for (n = 0; n < tech->num_requirements; n++)
	{
		if (!Tech_IsResearched(tech->requirements[n], research))
			return false;			
	}

	return true;
}


/*==========================

  Tech_IsAvailable

  Returns true if item is available for purchase

 ==========================*/

bool	Tech_IsAvailable(int objtype, researchData_t research[])
{
	techEntry_t *tech;

	tech = Tech_GetEntry(objtype);
	if (!tech)
		return false;

	if (GetObjectByType(objtype)->alwaysAvailable)
		return true;

	//now check to see if the required items are built
	if (!Tech_CheckRequirements(tech, research))
		return false;

	return true;
}


/*==========================

  Tech_IsResearchable

  Returns true if an item is available to be researched

 ==========================*/

bool	Tech_IsResearchable(int objtype, researchData_t research[])
{
	techEntry_t *tech;

	tech = Tech_GetEntry(objtype);
	if (!tech)
		return false;	

	//if this is a research item, we may already have the item and can't buy it again
	if (Tech_IsResearchType(objtype))
	{
		if (Tech_IsResearched(objtype, research))
			return false;
	}

	//now check to see if the required items are built
	if (!Tech_CheckRequirements(tech, research))
		return false;

	return true;
}


/*==========================

  Tech_IsResearchType

 ==========================*/

bool	Tech_IsResearchType(int objtype)
{
	if ((IsUnitType(objtype) && !IsWorkerType(objtype)) ||
		IsWeaponType(objtype) || IsUpgradeType(objtype) ||
		IsItemType(objtype) || IsMeleeType(objtype))
		return true;

	return false;
}


/*==========================

  Tech_IsResearched

 ==========================*/

int	Tech_IsResearched(int objtype, researchData_t research[])
{
	techEntry_t *tech;

	tech = Tech_GetEntry(objtype);

	if (!tech)
		return false;

	if (GetObjectByType(objtype)->alwaysAvailable)		//this item doesn't need to be in the researchedItems array because it's always available
		return true;

	//check for upgraded versions of a building
	//the base type for a building that has been upgraded still counts
	if (IsBuildingType(objtype))
	{
		int index;

		for (index = 0; index < tech->numItemsToBuild; index++)
		{
			if (IsBuildingType(tech->itemsToBuild[index]))
				if (Tech_IsResearched(tech->itemsToBuild[index], research))
					return true;
		}
	}

	return research[objtype].count;
}


/*==========================

  Tech_IsBuilder

  Returns true if objtype can be built by buildertype

 ==========================*/

bool	Tech_IsBuilder(int objtype, int buildertype)
{
	int			index;
	techEntry_t	*obj, *builder;

	builder = Tech_GetEntry(buildertype);
	obj = Tech_GetEntry(objtype);
	if (!obj)
		return false;

	//buildings that are built by workers don't need their buildertype to be placed
	if (IsBuildingType(objtype) && 
		(IsWorkerType(obj->builder[0]) || IsWorkerType(obj->builder[1]) || GetObjectByType(objtype)->selfBuild))
		return true;

	if (!builder)
		return false;

	for (index = 0; index < builder->numItemsToBuild; index++)
	{
		if (builder->itemsToBuild[index] == objtype)
			return true;
	}

	return false;
}


/*==========================

  Tech_FillTree

 ==========================*/

void	Tech_FillTree()
{
	int n, numObjects = CountActiveObjects();
	int i;

	for (n = 0; n <= numObjects; n++)
	{
		int r;
		objectData_t	*obj = GetObjectByType(n);

		if (!obj->objclass || !obj->race)
			continue;

		techtree[techtree_count].objnum = n;
		techtree[techtree_count].builder[0] = GetObjectTypeByName(obj->builder[0]);
		techtree[techtree_count].builder[1] = GetObjectTypeByName(obj->builder[1]);
	
		//fill in requirements
		for (r = 0; r < MAX_REQUIREMENTS; r++)
		{
			if (obj->requirements[r][0])
			{
				techtree[techtree_count].requirements[r] = GetObjectTypeByName(obj->requirements[r]);
				if (!techtree[techtree_count].requirements[r])
					continue;
				techtree[techtree_count].num_requirements++;
			}
			else
				break;
		}

		//fill in tech and base point requirements
		techtree[techtree_count].minBaseLevel = obj->needBasePoints;
		techtree[techtree_count].minTechLevel = obj->needTechPoints;
		techtree[techtree_count].maxPopulation = obj->maxPopulation;

		techtree_count++;

		if (techtree_count >= MAX_TECHTREE_ITEMS)
		{
			core.Console_Printf("WARNING: Techtree contains too many items!\n");
			break;
		}
	}

	//do a pass over it to fill in the itemsToBuild array
	for (n = 0; n < techtree_count; n++)
	{
		techtree[n].numItemsToBuild = 0;

		for (i = 0; i < techtree_count; i++)
		{
			bool added = false;
			int builder;

			if (i == n)
				continue;

			builder = techtree[n].objnum;

			while (builder)
			{
				if (builder == techtree[i].builder[0])
				{
					techtree[n].itemsToBuild[techtree[n].numItemsToBuild] = techtree[i].objnum;
					techtree[n].numItemsToBuild++;
					added = true;
					break;
				}

				//if the builder is a building and it's builder is a building and buildee is not a building
				//then try the builder's builder...
				//
				//in other words, buildings that transform inherit their parents build lists, minus any
				//transformations
				if (IsBuildingType(builder) && 
					GetObjectByName(GetObjectByType(builder)->builder[0])->objclass == OBJCLASS_BUILDING &&
					!IsBuildingType(techtree[i].objnum))
				{
					techEntry_t	*tech;

					tech = Tech_GetEntry(builder);

					if (tech)
						builder = tech->builder[0];
					else
						break;
				}
				else
				{
					break;
				}
			}
			
			//builder2 doesn't inherit, that would be a mess
			if (techtree[i].builder[1] == techtree[n].objnum && !added)
			{
				techtree[n].itemsToBuild[techtree[n].numItemsToBuild] = techtree[i].objnum;
				techtree[n].numItemsToBuild++;
			}
		}
	}
}


/*==========================

  Tech_Generate

  ==========================*/

void	Tech_Generate(objectData_t *(*objectTypeFunc)(int type))
{
	techtree_count = 0;	
	objTypeFunc = objectTypeFunc;

	memset(techtree, 0, sizeof(techtree));

	//fill techtree with buildings, units and weapons and upgrades and items and(?)...
	Tech_FillTree();
	core.Console_Printf("Generated techtree with %i entries\n", techtree_count);
}

