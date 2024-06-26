// (C) 2003 S2 Games

// le_brushfuncs.c

// brush functions


float	LE_AddModel(float source, float strength)
{
	return source + strength;
}

float	LE_FlattenModel(float source, float strength)
{
	float ret;

	ret = source - strength;

	if (ret < 0) ret=0;

	return ret;
}

float	LE_SubtractModel(float source, float strength)
{
	return source - strength;
}

void	LE_InitBrushFuncs()
{

}
