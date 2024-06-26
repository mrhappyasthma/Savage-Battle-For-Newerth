// (C) 2003 S2 Games

// world_api.h

// World query functions



typedef struct
{
	float	fraction;	//amount box/ray traveled before hitting something
	vec3_t	endpos;		//point where the box/ray stopped


} traceinfo_t;

typedef struct
{
	int		flags;		//type of terrain at this point
	float	z;			//height of terrain at this point
	vec3_t	nml;		//surface normal of terrain at this point
} pointinfo_t;


void	World_TraceRay(traceinfo_t *result, const vec3_t start, const vec3_t end);
void	World_TraceBox(traceinfo_t *result, const vec3_t start, const vec3_t end,
						const vec3_t minb, const vec3_t maxb);
void	World_SamplePoint(pointinfo_t *result, const vec3_t point);