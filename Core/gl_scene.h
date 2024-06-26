// (C) 2003 S2 Games

// gl_scene.h


void	GL_InitScene(void);
void	GL_LightMesh(mesh_t *mesh, sceneobj_t *obj, const vec3_t *normalarray, bvec4_t *colorarray);
void	GL_DrawSprite(vec3_t pos, mdlsprite_t *spr);
void	GL_DrawGroundPlane(sceneobj_t *obj, mdlsprite_t *spr);
void    GL_DrawBillboard(vec3_t pos, float width, float height, float angle, float s1, float t1, float s2, float t2, bool all_axes);
void	GL_DrawPoly(scenefacelist_t *f);
void	GL_DrawBox(vec3_t bmin, vec3_t bmax);
void	GL_SelectObjectShader(sceneobj_t *obj, shader_t *shader);


//model
void	GL_InitModel();
bool	GL_AddModelToLists(sceneobj_t *obj);
void	GL_DrawMesh(sceneobj_t *obj, mesh_t *mesh);
void	GL_DrawSkeleton(int boneidx, skeleton_t *skel, model_t *model);
void	GL_RedrawMesh();

//scene builder

void	GL_InitSceneBuilder();

void	GL_AddMesh(residx_t shader, mesh_t *mesh, sceneobj_t *obj);
void	GL_AddSprite(residx_t shader, mdlsprite_t *sprite, sceneobj_t *obj);

void	GL_RenderLists();
void	GL_ClearLists();

void	GL_LoadObjectMatrix(sceneobj_t *obj);


extern camera_t *cam;
extern cvar_t terrain_chunksize;
extern cvar_t gfx_renderDataRefCount;