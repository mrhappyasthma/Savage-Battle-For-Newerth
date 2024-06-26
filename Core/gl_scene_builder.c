// (C) 2001 S2 Games

// gl_scene_builder.c

// builds arrays for passing off to gl_main.c in order of shader

#include "core.h"

//cvar_t	gfx_alphaDepthHack = { "gfx_alphaDepthHack", "1" };
cvar_t	gfx_alphaTestRef =	{ "gfx_alphaTestRef", "0.35", CVAR_VALUERANGE | CVAR_SAVECONFIG, 0.35, 0.65 };
cvar_t	gfx_noBlending = { "gfx_noBlending", "1", CVAR_SAVECONFIG };
cvar_t	gfx_doubleSided = { "gfx_doubleSided", "1", CVAR_CHEAT };

extern shader_t *current_shader;

enum
{
	GL_LIST_MESH,
	GL_LIST_SPRITE
};

//cvar_t	gl_doublesided = { "gl_doublesided", "0" };


//one render list is stored for each shader bucket
typedef struct gl_render_list_s
{
	bool		drawn;

	char		type;	//0 = mesh, 1 = sprite

	residx_t	shader;	//we store shader here because we might want to have > 1 shader per bucket

	sceneobj_t	*obj;	//used for generating transform matrix and setting rendering flags
	
	union
	{
		mesh_t		*mesh;  //pointer to geometry data
		mdlsprite_t *sprite; //pointer to sprite data if we're rendering a sprite
	} data;

	struct gl_render_list_s *next;
} gl_render_list_t;

typedef struct
{
	gl_render_list_t	*renderlist;
	gl_render_list_t	*renderlist_tail;	//store this so we can link in items in proper order
} gl_shader_bucket_t;


gl_shader_bucket_t		shader_buckets[MAX_SHADER_BUCKETS];



void	GL_LoadObjectMatrix(sceneobj_t *obj)
{
	float tm[16];

	if (obj)
	{
 		if (obj->flags & SCENEOBJ_BILLBOARD_ORIENTATION)
 		{
 			glTranslatef(obj->pos[0], obj->pos[1], obj->pos[2]);
 
 			tm[0] = cam->viewaxis[0][0];
 			tm[1] = cam->viewaxis[0][1];
 			tm[2] = cam->viewaxis[0][2];
 			tm[3] = 0.0;
 			tm[4] = cam->viewaxis[1][0];
 			tm[5] = cam->viewaxis[1][1];
 			tm[6] = cam->viewaxis[1][2];
 			tm[7] = 0.0;
 			tm[8] = cam->viewaxis[2][0];
 			tm[9] = cam->viewaxis[2][1];
 			tm[10] = cam->viewaxis[2][2];
 			tm[11] = 0.0;
			tm[12] = 0.0;
 			tm[13] = 0.0;
 			tm[14] = 0.0;
 			tm[15] = 1.0;
 			
 			glMultMatrixf(tm);
 
 			glRotatef(-90, 1, 0, 0);
 
 			return;
 		}
		else
		{	
			GL_AxisToMatrix(obj->axis, tm);
				
			if (obj->flags & SCENEOBJ_TRANSLATE_ROTATE)
			{
				//glRotatef(obj->angle[2], 0, 0, 1);
				//glRotatef(obj->angle[0], 1, 0, 0);
				//glRotatef(obj->angle[1], 0, 1, 0);
				glMultMatrixf(tm);
				glTranslatef(obj->pos[0], obj->pos[1], obj->pos[2]);
			}
			else
			{
				glTranslatef(obj->pos[0], obj->pos[1], obj->pos[2]);
				glMultMatrixf(tm);
			}
 		}
 	}
 	else
 	{
 		glLoadIdentity();
	}

	if (obj->scale != 1)
		glScalef(obj->scale, obj->scale, obj->scale);

}
 




/****************     Render List Management    *******************/

void	GL_AppendRenderList(int driveridx, gl_render_list_t *newarray)
{	
	if (shader_buckets[driveridx].renderlist == NULL)
	{
		shader_buckets[driveridx].renderlist = newarray;
	}
	else
	{
		shader_buckets[driveridx].renderlist_tail->next = newarray;
	}
	shader_buckets[driveridx].renderlist_tail = newarray;
	newarray->next = NULL;
}


void	GL_AddMesh(residx_t shader, mesh_t *mesh, sceneobj_t *obj)
{
	gl_render_list_t *newarray;
	
	newarray = Scene_FramePool_Alloc(sizeof(gl_render_list_t));
	if (!newarray)
		return;

	memset(newarray, 0, sizeof(gl_render_list_t));

	newarray->drawn = false;
	newarray->type = GL_LIST_MESH;
	newarray->obj = obj;
	newarray->data.mesh = mesh;
	newarray->shader = shader;

	GL_AppendRenderList(Res_GetShader(shader)->sortedidx, newarray);	
}

void	GL_AddSprite(residx_t shader, mdlsprite_t *sprite, sceneobj_t *obj)
{
	gl_render_list_t *newarray;

	newarray = Scene_FramePool_Alloc(sizeof(gl_render_list_t));
	if (!newarray)
		return;	

	memset(newarray, 0, sizeof(gl_render_list_t));

	newarray->drawn = false;
	newarray->type = GL_LIST_SPRITE;
	newarray->data.sprite = sprite;
	newarray->obj = obj;
	newarray->shader = shader;

	GL_AppendRenderList(Res_GetShader(shader)->sortedidx, newarray);	
}



void	GL_SetRenderStates(sceneobj_t *obj, shader_t *shader)
{
	int flags = obj->flags;

	if (flags & SCENEOBJ_SOLID_COLOR)
	{		
		glColor4f(obj->color[0], obj->color[1], obj->color[2], flags & SCENEOBJ_USE_ALPHA ? obj->alpha : obj->color[3]);
	}
	else
	{				
		glColor4fv(vec4(1,1,1, flags & SCENEOBJ_USE_ALPHA ? obj->alpha : 1));
	}
/*
	if (shader->flags & SHD_NO_LIGHTING)
	{
		GL_Disable(GL_LIGHTING);
	}
	else
	{
		if (lighting)
			GL_Enable(GL_LIGHTING);
	}
*/

}


void GL_TexModulate(float amt);
extern cvar_t	vid_overbright;

//bool solid = true;
static GLboolean depthWrite;
static GLboolean lighting;
static GLboolean texture2d;
static GLboolean fog;

void	GL_RenderBucket(gl_render_list_t *renderlist)
{
	gl_render_list_t *list;

	for (list = renderlist; list != NULL; list = list->next)
	{
		sceneobj_t *obj = list->obj;
		shader_t *shader;

		OVERHEAD_INIT;

		//bind the texture and set up blending parameters and texture matrix
		shader = Res_GetShader(list->shader);
		GL_SelectShader(shader, cam->time);

		GL_SetRenderStates(obj, shader);

		//disable fog and depth write for additive textures
		if (shader->srcblend == BLEND_ONE &&
			shader->dstblend == BLEND_ONE)
		{			
			GL_Disable(GL_FOG);
			if (depthWrite)
				GL_DepthMask(GL_FALSE);
			GL_Enable(GL_BLEND);
		}
		else
		{
			if (fog)
				GL_Enable(GL_FOG);
			if (obj->flags & SCENEOBJ_NO_ZWRITE)
			{
				GL_DepthMask(GL_FALSE);
			}
			else
			{
				if (depthWrite)
					GL_DepthMask(GL_TRUE);
			}

			if (list->obj->flags & SCENEOBJ_ALWAYS_BLEND)
				GL_Enable(GL_BLEND);
			else
				GL_Disable(GL_BLEND);			
		}

		if (shader->flags & SHD_NO_LIGHTING)
		{
			GL_TexModulate(1.0);
			GL_Disable(GL_LIGHTING);
		}
		else
		{
			if (vid_overbright.integer)
				GL_TexModulate(2.0);
			else
				GL_TexModulate(1.0);
			if (lighting)
			{
				GL_Enable(GL_LIGHTING);
			}			
		}

		if (shader->doubleSided || gfx_doubleSided.integer)
		{
			GL_Disable(GL_CULL_FACE);
		}
		else
		{
			GL_Enable(GL_CULL_FACE);
		}

		if (obj->flags & SCENEOBJ_NO_ZTEST)
		{
			GL_Disable(GL_DEPTH_TEST);
		}
		else
		{
			GL_Enable(GL_DEPTH_TEST);
		}

		glPushMatrix();		
		GL_LoadObjectMatrix(obj);

		OVERHEAD_COUNT(OVERHEAD_SCENE_BUCKETSETUP);

		

		switch (list->type)
		{
			case GL_LIST_MESH:
				/*
				 *   special hack for translucent polygons
				 */
				if (shader->translucent && depthWrite)
				{						
					GL_Enable(GL_ALPHA_TEST);
					GL_Disable(GL_BLEND);
					glAlphaFunc(GL_GREATER, gfx_alphaTestRef.value);
					GL_DrawMesh(obj, list->data.mesh);

					if (!gfx_noBlending.integer || (list->obj->flags & SCENEOBJ_ALWAYS_BLEND))
					{
						GL_Enable(GL_BLEND);
						if (depthWrite)
							GL_DepthMask(GL_FALSE);						
						glAlphaFunc(GL_LEQUAL, gfx_alphaTestRef.value);
						GL_RedrawMesh();												
						GL_Disable(GL_ALPHA_TEST);
						if (depthWrite)
							GL_DepthMask(GL_TRUE);
					}
					/*
						 //
						 // draw unordered alpha polys
						 //
						if (!gfx_noBlending.integer)
							GL_DepthMask(GL_FALSE);
						else
							GL_DepthMask(GL_TRUE);
						GL_DrawMesh(obj, list->data.mesh);						
						if (depthWrite)
							GL_DepthMask(GL_TRUE);
					*/
				}
				else
				{
					GL_Disable(GL_ALPHA_TEST);
					GL_DrawMesh(obj, list->data.mesh);
				}
				break;
			case GL_LIST_SPRITE:				
				break;
			default:
				Console_DPrintf("Unknown list type\n");
				break;
		}

		glPopMatrix();

		list->drawn = true;
	}

	//re-enable blending
	GL_Enable(GL_BLEND);

	GL_Disable(GL_ALPHA_TEST);
}


/*==========================

  GL_GetInitialRenderStates

  check the value of write mask, lighting, texturing, and fog

  this determines the appropriate times to enable/disable any
  of these states during RenderBucket()

 ==========================*/

void	GL_GetInitialRenderStates()
{
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
	glGetBooleanv(GL_LIGHTING, &lighting);
	glGetBooleanv(GL_TEXTURE_2D, &texture2d);
	glGetBooleanv(GL_FOG, &fog);
}

//render all renderlists in shader buckets
void	GL_RenderLists()
{
	int n;

	//glPushAttrib(GL_ENABLE_BIT);

	GL_GetInitialRenderStates();
  	

	for (n=0; n<MAX_SHADER_BUCKETS; n++)
	{
		gl_render_list_t *renderlist;

		if (!shader_buckets[n].renderlist)
			continue;

		renderlist = shader_buckets[n].renderlist;

		GL_RenderBucket(renderlist);
	}

	//glPopAttrib();
}

void	GL_ClearLists()
{
	int n;
	
	for (n=0; n<MAX_SHADERS; n++)
	{
		shader_buckets[n].renderlist = NULL;
		shader_buckets[n].renderlist_tail = NULL;
	}
}

void	GL_RegisterShaderCallback(shader_t *shader)
{
}


void	GL_InitSceneBuilder()
{
	GL_SetRegisterShaderCallback(GL_RegisterShaderCallback);

	memset(shader_buckets, 0, sizeof(shader_buckets));

	Cvar_Register(&gfx_alphaTestRef);	
	Cvar_Register(&gfx_noBlending);
	Cvar_Register(&gfx_doubleSided);

//	GL_Enable(GL_ALPHA_TEST);
//	glAlphaFunc(GL_GREATER, 0.6);
}
