// (C) 2001 S2 Games

// gl_terrain.h

#define	TERRAIN_REBUILD_VERTICES	0x0001
#define	TERRAIN_REBUILD_COLORS		0x0002
#define	TERRAIN_REBUILD_TEXCOORDS	0x0004
#define	TERRAIN_REBUILD_SHADERS		0x0008
#define TERRAIN_REBUILD_NORMALS		0x0010

void	GL_InitTerrain();
void	GL_RebuildTerrain(int chunkSize);
void	GL_DestroyTerrain();
void	GL_InvalidateTerrainVertex(int layer, int tx, int ty, int flags);
void	GL_FlagTerrainChunkForRendering(int chunkx, int chunky);
void	GL_RenderTerrain();
void	GL_InvalidateTerrainLayer(int layer, int flags);
void	GL_BeginCloudLayer(int texunit);
void	GL_EndCloudLayer(int texunit);
