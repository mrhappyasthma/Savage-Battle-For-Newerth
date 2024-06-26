#include "core.h"
#include "nvtristrip.h"

extern "C"
{
	

	bool	Stripify(model_t *model)
	{
		int n;

		for (n=0; n<model->num_meshes; n++)
		{			
			int count = 0;
			mesh_t *mesh = &model->meshes[n];			
			PrimitiveGroup *outPrims;
			unsigned short outNum;
			unsigned short *indices = new unsigned short[mesh->num_faces * 3];			
			
			if (!mesh->num_faces)
				continue;

			//copy the face list into an unsigned short array
			for (int face=0; face<mesh->num_faces; face++)
			{
				for (int v=0; v<3; v++)
				{					
					indices[count++] = (unsigned short)mesh->facelist[face][v];
				}
			}

			//make sure we output a triangle list
			SetListsOnly(true);
			GenerateStrips(indices, count, &outPrims, &outNum);

			//should only ouput 1 list
			if (outNum != 1)
			{
				delete [] indices;
				return false;
			}

			//map the faces back into the mesh
			count = 0;
			for (face=0; face<mesh->num_faces; face++)
			{
				for (int v=0; v<3; v++)
				{
					mesh->facelist[face][v] = indices[count++];
				}				
			}

			delete [] indices;
		}

		return true;
	}
}
