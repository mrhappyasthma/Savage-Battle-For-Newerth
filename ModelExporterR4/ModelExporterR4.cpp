// (C) 2002 S2 Games

// ModelExporterR4.cpp

// 3dsmax .model exporter

#define TEMP_GEOM_FILE

static float one = 1.0;
static float *pOne = &one;



#include "ModelExporterR4.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

extern "C"
{
	#include "../core/savage_types.h"
	#include "../core/savage_mathlib.h"
	#include "../core/geom.h"
	#include "../core/mem.h"
}


typedef matrix44_t transform_t;
#define M_MultiplyTransforms M_MultiplyMatrix44

#define OBJECT_NAME_LENGTH 64
#define OBJECT_CATEGORY_LENGTH 64
#define OUTPUT_MESSAGE_LENGTH 128000
#define ERROR_LOG_LENGTH	8192

//#define	MEM_SCENE	1			//memory that is valid for the entire scene traversal

//exporter mode
typedef enum
{
	MODE_SCENE_INFO,
	MODE_WRITE_FILE,
	MODE_WRITE_CLIP
} exporterModes_enum;

typedef enum
{
	NODETYPE_SPRITE,
	NODETYPE_MESH,
	NODETYPE_REFERENCE_BONE,
	NODETYPE_SURFACE
} nodeTypes_enum;

char *nodeTypeNames[] = 
{
	"Sprite",
	"Mesh",
	"Reference Bone",
	"Collision Surface"
};

char *editorCategories[] = 
{
	"Nomad_Chars",
	"Nomad_Structs",
	"Nomad_Props",
	"Nomad_Weapons",
	"Nomad_Artillery",
	"Beast_Chars",
	"Beast_Structs",
	"Beast_Props",	"Beast_Weapons",
	"Beast_Artillery",	"NPC_Chars",
	"NPC_Structs",
	"NPC_Props",
	"Trees",
	"Foliage",
	"General_Nature",
	"Rocks",
	""
};
/*
char	*objectTypes[] =
{
	"blocker",
	"nonblocker"
}
*/
typedef struct
{
	DWORD	vertidx;
	UVVert	uv;
	Point3	col;
	Point3	nml;
} s2vert_t;

#define MAX_TEXREFS	128

typedef struct
{
	BitmapTex	*bmptex;
	int			meshidx;
} texref_t;

#define MODELEXPORTERR4_CLASS_ID	Class_ID(0x5a5dc836, 0x267fdcb1)




class ModelExporterR4 : public UtilityObj {
	public:
		HWND			hPanel;
		HWND			hStatus;
		IUtil			*iu;
		Interface		*ip;

		model_t			model;
		modelClip_t		modelClip;

		INode			*node;		
		Tab<s2vert_t>	new_verts;
		Tab<Face>		new_faces;
		int				original_num_verts;

		//Tab<s2vert_t>	s2verts;
	//	Tab<Face>		s2faces;

		//objdef settings
		char			object_name[OBJECT_NAME_LENGTH];
		char			object_category[OBJECT_CATEGORY_LENGTH];
		float			scale_range_low;
		float			scale_range_high;

		//file options
		BOOL			export_textures;
		BOOL			export_objdef;
		BOOL			export_clip;
		char			filename[_MAX_PATH];

		char			output_message[OUTPUT_MESSAGE_LENGTH];		
		char			error_message[ERROR_LOG_LENGTH];
		char			msg_indent[32];

		int				nodecount;
		int				physique_count;
		int				spritecount;
		int				keyframed_mesh_count;
		int				num_meshes;

		BOOL			ignore_physique;		
		BOOL			link_to_root;
		BOOL			selected_deform_check;		
		BOOL			model_error;
		BOOL			debug_info;

		int				num_scene_nodes;

		texref_t		texrefs[MAX_TEXREFS];
		int				num_texrefs;

		INode			*boneNodes[MAX_BONES];

		//file writing
		int				seekback;		//position in the file to fill in the block length		

		exporterModes_enum mode;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void Export();								//do the export
		void ExportAnimationOnly();					//export only animation data
		BOOL ObjectSettings();						//display object settings dialog
		void SceneInfo();
		void TraverseNode(INode *node);
		void CountNodes(INode *node);
		Modifier *IsNodePhysiqued(INode *node);		
		BOOL AddPhysiqueBones(IPhyContextExport *mcExport, int *num_blended_verts, int *num_nonblended_verts);
		void StoreBoneMotions();
		void AddMessage(const char *msg);			//add a message to the scene info window
		void Indent();
		void Unindent();
		void ClearIndent();
		void Error(const char *msg);				//output an error message		
		void AddSprite(INode *node, int spritetype);
		void BeginNode(INode *node, nodeTypes_enum nodetype);
		void EndNode();
		void AddSurf(INode *node);
		BOOL AddSurfToModel(INode *node, convexPolyhedron_t *surf);
		void AddMesh(INode *node);
		INode *GetS2BoneParentNode(INode *node);
		BOOL AddBaseMeshData(mesh_t *s2mesh);
		BOOL AddKeyframedMesh(mesh_t *s2mesh);
		BOOL AddNonPhysiquedMesh(mesh_t *s2mesh);
		BOOL AddPhysiquedMesh(mesh_t *s2mesh, Modifier *mod);		
		BOOL ShouldCheckDeform(INode *node);
		BOOL CreateReferenceMesh();
		void DeleteReferenceMesh();
		int	GetStartFrame();
		int GetEndFrame();
		int GetNumFrames();
		void ClearSceneInfo();
		void Status(const char *msg, int progress);
		void SecondaryStatusText(const char *msg);
		void SecondaryStatusProgress(int progress);
		void OpenStatusBar();
		void CloseStatusBar();
		BOOL CopyMeshVerts(mesh_t *s2mesh, int frame, int boneindex);		
		BOOL CopyNodeTransformToBoneFrame(INode *node, bone_t *s2bone, boneMotion_t *bone, int frame);
		BOOL CreateExtraTopology();
		BOOL FinishMesh(mesh_t *s2mesh);
		BOOL ReAllocVerts(mesh_t *s2mesh, int count);
		void DuplicateVertex(mesh_t *s2mesh, DWORD from, DWORD to);
		void AddReferenceBone(INode *node);
		void Help();
		void WriteClipFile();
		void WriteFloatKeys(int boneindex, floatKeys_t *keys, int keytype, FILE *stream);
		void WriteByteKeys(int boneindex, byteKeys_t *keys, int keytype, FILE *stream);
		BOOL CopyKeys(INode *node, bone_t *s2bone, boneMotion_t *motion, int frame);
		BOOL GetFilename(char *fileTypeDesc, char *wildcard);
		void AddTextureReference(int meshidx);
		void CopyTextures();
		
		BOOL GetExportOptions();
		void ShowSceneInfo();
		void ShowErrorInfo();

		//disk writing functions
		BOOL WriteModelToDisk();
		void BeginBlock(const char *blockname, FILE *stream);
		void WriteBlockData(void *data, int size, FILE *stream);
		void EndBlock(FILE *stream);


		//functions that manipulate the 'model' structure
		void	AddBaseModelData();
		bone_t *AddBone(INode *refnode, const char *name, int *index);
		bone_t *GetBone(const char *name, int *index);
		BOOL	AddMeshToModel(mesh_t *s2mesh);
		int		CountMeshType(int meshtype);
		void	FinishModel();
		void	Cleanup();

		void DeleteThis() { }		
		//Constructor/Destructor

		ModelExporterR4();
		~ModelExporterR4();		

};

static ModelExporterR4 theModelExporterR4;

class ModelExporterR4ClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theModelExporterR4; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return MODELEXPORTERR4_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("ModelExporterR4"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static ModelExporterR4ClassDesc ModelExporterR4Desc;
ClassDesc2* GetModelExporterR4Desc() { return &ModelExporterR4Desc; }


static BOOL CALLBACK ModelExporterR4DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theModelExporterR4.Init(hWnd);
			
			break;

		case WM_DESTROY:
			theModelExporterR4.Destroy(hWnd);
			break;

		case WM_COMMAND:
			//control messages
			switch (LOWORD(wParam))
			{
				case IDC_EXPORT:
					theModelExporterR4.Export();
					break;
				case IDC_OBJECT_SETTINGS:
					theModelExporterR4.ObjectSettings();
					break;
				case IDC_CLOSE:
					theModelExporterR4.iu->CloseUtility();
					break;
				case IDC_SCENE_INFO:
					theModelExporterR4.SceneInfo();
					break;
				case IDC_EXPORT_ANIMATION:
					theModelExporterR4.ExportAnimationOnly();
					break;
				case IDC_HELPINFO:
					theModelExporterR4.Help();
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theModelExporterR4.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}




BOOL IsNodeBone(INode *pNode)
{
	if(pNode == NULL)
		return FALSE;
	if (pNode->IsRootNode())
		return FALSE;

	ObjectState os = pNode->EvalWorldState(0);
	if(os.obj->ClassID() == Class_ID(BONE_CLASS_ID, 0))
		return TRUE;
	// dct 10/18/01, r4 creates bones as objects rather than helpers
	if(os.obj->ClassID() == BONE_OBJ_CLASSID)
		return TRUE;

	// we don't want biped end effectors
	if(os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0))
		return FALSE;

	Control *cont = pNode->GetTMController();
	if(cont->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ||
		cont->ClassID() == BIPBODY_CONTROL_CLASS_ID
//		cont->ClassID() == FOOTPRINT_CLASS_ID
	) return TRUE;

	return FALSE;
}


//if it's a true bone, return the node tm with scale removed
//otherwise return the object TM (to take into account object offsets)
Matrix3 GetBoneTM(INode *pNode, TimeValue t)
{
	Matrix3 tm;

	if (IsNodeBone(pNode))
	{
		tm = pNode->GetNodeTM(t);
		tm.NoScale();
	}
	else
	{
		tm = pNode->GetObjectTM(t);
		tm.NoScale();
	}

	return tm;
}





//--- ModelExporterR4 -------------------------------------------------------
ModelExporterR4::ModelExporterR4()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	hStatus = NULL;

	scale_range_low = 1.0;
	scale_range_high = 1.0;
	object_name[0] = '\0';
	object_category[0] = '\0';
}

ModelExporterR4::~ModelExporterR4()
{

}

void ModelExporterR4::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		ModelExporterR4DlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void ModelExporterR4::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void ModelExporterR4::Init(HWND hWnd)
{
	//initialize memory tags
	Mem_Init();
}

void ModelExporterR4::Destroy(HWND hWnd)
{
	Mem_ShutDown();
}


ISpinnerControl *scaleRangeLow = NULL;
ISpinnerControl *scaleRangeHigh = NULL;

static BOOL CALLBACK ObjectSettingsDlgProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int n = 0;

	switch(msg)
	{
		case WM_INITDIALOG:

			scaleRangeLow = SetupFloatSpinner(hwndDlg, IDC_SCALE_RANGE_LOW_SPIN, IDC_SCALE_RANGE_LOW, 0.1f, 10, theModelExporterR4.scale_range_low, 0.01f);
			scaleRangeHigh = SetupFloatSpinner(hwndDlg, IDC_SCALE_RANGE_HIGH_SPIN, IDC_SCALE_RANGE_HIGH, 0.1f, 10, theModelExporterR4.scale_range_high, 0.01f);
			SetDlgItemText(hwndDlg, IDC_OBJECT_NAME, theModelExporterR4.object_name);
			SetDlgItemText(hwndDlg, IDC_CATEGORY, theModelExporterR4.object_category);
			//set up editor category dropdown
			while(editorCategories[n][0])
			{
				SendDlgItemMessage(hwndDlg, IDC_CATEGORY, CB_ADDSTRING, 0, ((LPARAM)(LPCSTR)editorCategories[n]));
				n++;
			}

			CenterWindow(hwndDlg,GetParent(hwndDlg));
			break;

		case WM_COMMAND:
			switch LOWORD(wParam)
			{
				case IDOK:
					theModelExporterR4.scale_range_low = scaleRangeLow->GetFVal();
					theModelExporterR4.scale_range_high = scaleRangeHigh->GetFVal();
					GetDlgItemText(hwndDlg, IDC_OBJECT_NAME, theModelExporterR4.object_name, OBJECT_NAME_LENGTH-1);
					GetDlgItemText(hwndDlg, IDC_CATEGORY, theModelExporterR4.object_category, OBJECT_CATEGORY_LENGTH-1);
					EndDialog(hwndDlg, TRUE);
					break;
	        	case IDCANCEL:
					EndDialog(hwndDlg, FALSE);
					break;
				default:
					break;
			}
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL ModelExporterR4::ObjectSettings()
{
	return DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_OBJECT_SETTINGS),
		hPanel,
		ObjectSettingsDlgProc,(LPARAM)this);
}

static BOOL CALLBACK ExportOptionsDlgProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int n = 0;

	switch(msg)
	{
		case WM_INITDIALOG:
			
			CheckDlgButton(hwndDlg, IDC_EXPORT_ANIMATION, 0);
			CheckDlgButton(hwndDlg, IDC_EXPORT_TEXTURES, 1);
			CheckDlgButton(hwndDlg, IDC_SAVE_OBJDEF, 0);
			//set up editor category dropdown
			while(editorCategories[n][0])
			{
				SendDlgItemMessage(hwndDlg, IDC_CATEGORY, CB_ADDSTRING, 0, ((LPARAM)(LPCSTR)editorCategories[n]));
				n++;
			}

			CenterWindow(hwndDlg,GetParent(hwndDlg));
			break;

		case WM_COMMAND:
			switch LOWORD(wParam)
			{
				case IDC_SAVE_OBJDEF:
					if (IsDlgButtonChecked(hwndDlg, IDC_SAVE_OBJDEF))
					{
						//bring up the object settings dialog
						if (!DialogBoxParam(
									hInstance,
									MAKEINTRESOURCE(IDD_OBJECT_SETTINGS),
									hwndDlg,
									ObjectSettingsDlgProc,0))
						{
							//if they cancelled, uncheck the button
							CheckDlgButton(hwndDlg, IDC_SAVE_OBJDEF, 0);
						}
					}
					break;
				case IDOK:
					theModelExporterR4.export_clip = IsDlgButtonChecked(hwndDlg, IDC_EXPORT_ANIMATION);
					theModelExporterR4.export_textures = IsDlgButtonChecked(hwndDlg, IDC_EXPORT_TEXTURES);
					theModelExporterR4.export_objdef = IsDlgButtonChecked(hwndDlg, IDC_SAVE_OBJDEF);
					
					EndDialog(hwndDlg, TRUE);
					break;
	        	case IDCANCEL:
					EndDialog(hwndDlg, FALSE);
					break;
				default:
					break;
			}
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL ModelExporterR4::GetExportOptions()
{
	return DialogBoxParam(hInstance,
		MAKEINTRESOURCE(IDD_EXPORT_OPTIONS),
		hPanel,
		ExportOptionsDlgProc, (LPARAM)this);
}

INode *ModelExporterR4::GetS2BoneParentNode(INode *node)
{
	if (node->GetParentNode())
	{
		if (link_to_root)
			return ip->GetRootNode();
		else
			return node->GetParentNode();
	}

	return NULL;
}

int		GetNumModifiers(INode *nodePtr)
{
	int numMods = 0;

	if (!nodePtr)
		return 0;

	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr)
		return NULL;

	while (ObjectPtr && ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);

		numMods += DerivedObjectPtr->NumModifiers();

		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	return numMods;
}

Modifier *FindModifier(INode *nodePtr, Class_ID &class_id)
{
	// Get object from node. Abort if no object.
	if (!nodePtr) return NULL;

	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr ) return NULL;
	
	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		// Yes -> Cast.
		IDerivedObject* DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);				
			
		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;

		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			// Get current modifier.
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			// Is this the correct modifier?
			if (ModifierPtr->ClassID() == class_id)
			{
				//ModContext *mc = DerivedObjectPtr->GetModContext(ModStackIndex);
				return ModifierPtr;
			}

			ModStackIndex++;
		}

		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	// Not found.
	return NULL;

}


Modifier *ModelExporterR4::IsNodePhysiqued(INode *nodePtr)
{	
	return FindModifier(nodePtr, Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));
}

void ModelExporterR4::Indent()
{
	if (strlen(msg_indent) + 4 >= 32)
		return;

	strcat(msg_indent, "    ");
}

void ModelExporterR4::Unindent()
{
	int pos = strlen(msg_indent) - 4;
	if (pos < 0)
		pos = 0;

	msg_indent[pos] = '\0';
}

void ModelExporterR4::ClearIndent()
{
	msg_indent[0] = '\0';
}

void ModelExporterR4::AddMessage(const char *message)
{
	_snprintf(output_message, OUTPUT_MESSAGE_LENGTH-1, "%s%s%s\r\n", output_message, msg_indent, message);
}

void ModelExporterR4::Error(const char *message)
{	
	if (!node)
	{
		_snprintf(error_message, ERROR_LOG_LENGTH-1, "%s%s\r\n", error_message, message);
	}
	else
	{
		_snprintf(error_message, ERROR_LOG_LENGTH-1, "%s%s: %s\r\n", error_message, node->GetName(), message);
	}

	model_error = TRUE;
}

void ModelExporterR4::BeginNode(INode *node, nodeTypes_enum nodetype)
{	
	AddMessage(fmt("-------======= %s ======-------\r\n\r\n", node->GetName()));
	
	AddMessage(fmt("Type: %s", nodeTypeNames[nodetype]));
	AddMessage("Details:");
	Indent();

	this->node = node;
}

void ModelExporterR4::EndNode()
{
	AddMessage("\r\n\r\n");
	Unindent();

	this->node = NULL;
}

//from PhyExportSample.cpp:

// This function can be used to set the non-uniform scale of a biped.
// The node argument should be a biped node.
// If the scale argument is non-zero the non-uniform scale will be removed from the biped.
// Remove the non-uniform scale before exporting biped nodes and animation data
// If the scale argument is zero the non-uniform scaling will be reapplied to the biped.
// Add the non-uniform scaling back on the biped before exporting skin data
//***********************************************************************************
void ScaleBiped(INode* node, int scale)
{
	if (node->IsRootNode()) return;

	// Use the class ID to check to see if we have a biped node
	Control* c = node->GetTMController();
    if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
         (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
         (c->ClassID() == FOOTPRINT_CLASS_ID))
    {

        // Get the Biped Export Interface from the controller 
        IBipedExport *BipIface = (IBipedExport *) c->GetInterface(I_BIPINTERFACE);

		if (BipIface)
		{
			// Either remove the non-uniform scale from the biped, 
			// or add it back in depending on the BOOLean scale value
			BipIface->RemoveNonUniformScale(scale);
			Control* iMaster = (Control *) c->GetInterface(I_MASTER);
			iMaster->NotifyDependents(FOREVER, PART_TM, REFMSG_CHANGE);
		
			// Release the interfaces
			c->ReleaseInterface(I_MASTER,iMaster);
			c->ReleaseInterface(I_BIPINTERFACE,BipIface);
		}
	}
}



BOOL ModelExporterR4::AddPhysiqueBones(IPhyContextExport *mcExport, int *num_blended_verts, int *num_nonblended_verts)
{
	int i = 0, x = 0;
	INodeTab bones;
	INode* bone;

	SecondaryStatusText("Adding physique-referenced bones");
	SecondaryStatusProgress(0);	

	//These are the different types of vertex classes 
	IPhyBlendedRigidVertex *rb_vtx;
	IPhyRigidVertex *r_vtx;
	//IPhyFloatingVertex *f_vtx;

	//get the vertex count from the export interface
	int numverts = mcExport->GetNumberVertices();

	*num_blended_verts = 0;
	*num_nonblended_verts = 0;
	
	//iterate through all vertices and gather the bone list
	for (i = 0; i<numverts; i++) 
	{
		BOOL exists = FALSE;
		
		//get the hierarchial vertex interface
		IPhyVertexExport* vi = mcExport->GetVertexInterface(i);
		if (vi) {

			//check the vertex type and process accordingly
			int type = vi->GetVertexType();
			switch (type) 
			{
				//The vertex is rigid, blended vertex.  It's assigned to multiple links
				case RIGID_BLENDED_TYPE:
					//type-cast the node to the proper class		
					rb_vtx = (IPhyBlendedRigidVertex*)vi;
					
					//iterate through the bones assigned to this vertex
					for (x = 0; x<rb_vtx->GetNumberNodes(); x++) 
					{
						exists = FALSE;
						//get the node by index
						bone = rb_vtx->GetNode(x);
						
						//If the bone is a biped bone, scale needs to be
						//restored before exporting skin data
						//ScaleBiped(bone, 0);
						
						//check to see if we already have this bone
						for (int z=0;z<bones.Count();z++)
							if (bone == bones[z]) exists = TRUE;

						//if you didn't find a match add it to the list
						if (!exists) bones.Append(1, &bone);
					}

					(*num_blended_verts)++;

					break;
				//The vertex is a rigid vertex and only assigned to one link
				case RIGID_TYPE:
					//type-cast the node to the proper class
					r_vtx = (IPhyRigidVertex*)vi;
					
					//get the node
					bone = r_vtx->GetNode();

					//If the bone is a biped bone, scale needs to be
					//restored before exporting skin data
					//ScaleBiped(bone, 0);

					//check to see if the bone is already in the list
					for (x = 0;x<bones.Count();x++)
					{
						if (bone == bones[x]) exists = TRUE;
					}
					//if you didn't find a match add it to the list
					if (!exists) bones.Append(1, &bone);

					(*num_nonblended_verts)++;
					break;

				// Shouldn't make it here because we converted to rigid earlier.  
				// It should be one of the above two types
				default:
					Error("Vertex type was not RIGID_TYPE or RIGID_BLENDED_TYPE!");
					return FALSE;  
			}

			SecondaryStatusProgress(((float)i / numverts)*100.0);
		}
		/* 
		// After gathering the bones from the rigid vertex interface
		// gather all floating bones if there are any 
		f_vtx = (IPhyFloatingVertex*)mcExport->GetVertexInterface(i, FLOATING_VERTEX);
		if (f_vtx) {	//We have a vertex assigned to a floating bone
			// iterate through the links assigned to this vertex
			for (x = 0; x<f_vtx->GetNumberNodes(); x++)
			{
				exists = FALSE;

				 //get the node by index
				bone = f_vtx->GetNode(x); 
				
				//If the bone is a biped bone, scale needs to be
				//restored before exporting skin data
				ScaleBiped(bone, 0);
				
				//check to see if we already have this bone
				for (int z=0;z<bones.Count();z++)
					if (bone == bones[z]) exists = TRUE;

				//if you didn't find a match add it to the list
				if (!exists) bones.Append(1, &bone);
			}
		}*/
	}

	for (i=0;i<bones.Count();i++)
	{
		int index;

		bone_t *s2bone = AddBone(bones[i], bones[i]->GetName(), &index);			
	}

	return TRUE;
}



BOOL ModelExporterR4::AddPhysiquedMesh(mesh_t *s2mesh, Modifier *mod)
{
	IPhysiqueExport *phyExport = (IPhysiqueExport *)mod->GetInterface(I_PHYEXPORT);

	if (!phyExport)
	{
		Error(fmt("Error with physiqued mesh %s", node->GetName()));
		return FALSE;
	}

	IPhyContextExport *mcExport = (IPhyContextExport *)phyExport->GetContextInterface(node);

	if (!mcExport)
	{
		Error(fmt("Error with physiqued mesh %s (couldn't get context interface)", node->GetName()));
		return FALSE;
	}

	int num_blended_verts = 0;
	int num_nonblended_verts = 0;

	mcExport->ConvertToRigid(TRUE);

	if (!AddPhysiqueBones(mcExport, &num_blended_verts, &num_nonblended_verts))
	{
		Error(fmt("Failed to add physique-referenced bones for mesh %s", s2mesh->name));
		return FALSE;
	}

	if (mode == MODE_WRITE_CLIP)
		return TRUE;				//if we're only exporting animation we don't need vertex info

	int numverts = mcExport->GetNumberVertices();
	int max_links = 1;

	//allocate space for vertex data
	if (num_blended_verts)
	{
		s2mesh->mode = MESH_SKINNED_BLENDED;
		s2mesh->blendedLinks = (blendedLink_t *)Tag_Malloc(numverts * sizeof(blendedLink_t), MEM_SCENE);
		if (!s2mesh->blendedLinks)
		{
			Error(fmt("Failed to allocate %i blended verts", numverts));
			return FALSE;
		}
	}
	else	//all verts are nonblended
	{
		s2mesh->mode = MESH_SKINNED_NONBLENDED;
		s2mesh->singleLinks = (singleLink_t *)Tag_Malloc(numverts * sizeof(singleLink_t), MEM_SCENE);
		if (!s2mesh->singleLinks)
		{
			Error(fmt("Failed to allocate %i nonblended verts", numverts));
			return FALSE;
		}
	}

	//copy vertex coordinates
	CopyMeshVerts(s2mesh, GetStartFrame(), -1);

	//copy physique weights
	SecondaryStatusText("Copying physique weights");
	SecondaryStatusProgress(0);

	for (int v=0; v<numverts; v++)
	{
	   IPhyVertexExport *vtxExport = mcExport->GetVertexInterface(v);

	   if (vtxExport)
	   {
		   if (num_blended_verts)
		   {
			   if (vtxExport->GetVertexType() == RIGID_BLENDED_TYPE)
			   {
				   //vertex is attached to multiple bones with different weights

				   int numnodes;

					IPhyBlendedRigidVertex *vtxBlend = (IPhyBlendedRigidVertex *)vtxExport;

					numnodes = vtxBlend->GetNumberNodes();
					if (numnodes > max_links)
						max_links = numnodes;

					//allocate space for this vertex
					s2mesh->blendedLinks[v].indexes = (int *)Tag_Malloc(sizeof(int) * numnodes, MEM_SCENE);
					s2mesh->blendedLinks[v].weights = (float *)Tag_Malloc(sizeof(float) * numnodes, MEM_SCENE);
					s2mesh->blendedLinks[v].num_weights = numnodes;			

					Point3 BlendP = Point3(0,0,0);

					//for each bone...
					for (int n = 0; n < numnodes; n++)
					{
						int index;
						INode *Bone = vtxBlend->GetNode(n);
						bone_t *s2bone = GetBone(Bone->GetName(), &index);
						if (!s2bone)
						{
							//this shouldn't happen
							Error("ERROR: Found a vertex linked to a bone that's not in the bones list!");
							break;
						}
						
		  				Point3 Offset = vtxBlend->GetOffsetVector(n);
						blendedLink_t *link = &s2mesh->blendedLinks[v];
						s2mesh->blendedLinks[v].indexes[n] = index;						
						s2mesh->blendedLinks[v].weights[n] = vtxBlend->GetWeight(n);
												
/*
						if (v % 100 == 0)
						{
							Matrix3 bonetm = Bone->GetObjectTM(FRAME(GetStartFrame()));
							bonetm.Invert();
							BlendP = bonetm * 
							AddMessage(fmt("vtx %i: node %i: offset=(%f,%f,%f)  weight=%f",v,n,Offset[0],Offset[1],Offset[2],link->weight));
	       					BlendP += (Bone->GetNodeTM(GetStartFrame()) * Offset) * link->weight;
							AddMessage(fmt("final: %f %f %f"), BlendP.x, BlendP.y, BlendP.z);
							BlendP = 
						}
*/				
					}

					//contribute to bounding box (this only takes into account the first frame of the animation)
					//M_AddPointToBounds((float *)(BlendP), model.bmin, model.bmax);				
			   }
			   else
			   {
				   //vertex is attached to only one bone

					IPhyRigidVertex *vtx = (IPhyRigidVertex *)vtxExport;
					int index;					
					
					bone_t *s2bone = GetBone(vtx->GetNode()->GetName(), &index);
					if (!s2bone)
					{
						//this shouldn't happen
						Error("Found a vertex linked to a bone that's not in the bones list!");
						break;
					}

					//Point3 Offset = vtx->GetOffsetVector();										
					
					//allocate space for this vertex					
					s2mesh->blendedLinks[v].num_weights = 1;
					s2mesh->blendedLinks[v].weights = pOne;
					s2mesh->blendedLinks[v].indexes = (int *)Tag_Malloc(sizeof(int), MEM_SCENE);
					s2mesh->blendedLinks[v].indexes[0] = index;
			   } 
		   }
		   else
		   {			   
				IPhyRigidVertex *vtx = (IPhyRigidVertex *)vtxExport;
				INode *node = vtx->GetNode();
				int index;
				
				if (!node)
				{
					Error(fmt("Unknown problem exporting physique data for %s", s2mesh->name));
					break;
				}

				bone_t *s2bone = GetBone(node->GetName(), &index);
				if (!s2bone)
				{
					//this shouldn't happen
					Error("Found a vertex linked to a bone that's not in the bones list!");
					break;
				}

				//Point3 Offset = vtx->GetOffsetVector();										
				
				//allocate space for this vertex				
				s2mesh->singleLinks[v] = index;
		   }
	   }

	   mcExport->ReleaseVertexInterface(vtxExport);

	   SecondaryStatusProgress(((float)v / numverts)*100.0);
	}

	AddMessage("Physique info:");
	Indent();
	AddMessage(fmt("Blended vertices: %i", num_blended_verts));
	AddMessage(fmt("Nonblended vertices: %i", num_nonblended_verts));
	AddMessage(fmt("Max links: %i", max_links));
	Unindent();

	phyExport->ReleaseContextInterface(mcExport);
	mod->ReleaseInterface(I_PHYINTERFACE, phyExport);

	physique_count++;

	return TRUE;
}

void ModelExporterR4::AddSprite(INode *node, int spritetype)
{
	Object *obj;

	obj = node->GetObjectRef();
	if (!obj)
		return;
	if (obj->ClassID() != Class_ID(BOXOBJ_CLASS_ID, 0))
	{		
		Error(fmt("%s must be a BOX object to export (use length and width)", node->GetName()));
		return;
	}

	IParamArray *pa = obj->GetParamBlock();

	if (!pa)
		return;

	Interval valid;
	float width, height;

	pa->GetValue(obj->GetParamBlockIndex(BOXOBJ_WIDTH), 0, width, valid);
	pa->GetValue(obj->GetParamBlockIndex(BOXOBJ_LENGTH), 0, height, valid);

	BeginNode(node, NODETYPE_SPRITE);
	AddMessage(fmt("Width = %f, Height = %f, Type = %i", width, height, spritetype));
	EndNode();
	spritecount++;
}

BOOL IsMesh(INode *node, Interface *ip)
{
	Object *obj = node->EvalWorldState(ip->GetTime()).obj;
	if (!obj)
		return FALSE;

	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		return TRUE;

	return FALSE;
}

//modified from ascii exporter
Point3 GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	// Is normal specified
	// SPECIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL) {
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1) {
			vertexNormal = rv->rn.getNormal();
		}
		else {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++) {
				if (rv->ern[i].getSmGroup() & smGroup) {
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else 
	{		
		vertexNormal = mesh->getFaceNormal(faceNo);
	}
	
	return vertexNormal;
}


int CreateFaceAndVertexData(Mesh *mesh, Tab<Face>& newFaces, Tab<s2vert_t>& newVerts, Matrix3& tm)
{
    Face tmpFace;
    Point3 tmpVert;
    int numVerts = mesh->getNumVerts();
	int numFaces = mesh->getNumFaces();
	int ret = 0;	
	BOOL negparity = tm.Parity();
 
	BitArray vWritten;
    vWritten.SetSize(numVerts);
    vWritten.ClearAll();

    newVerts.SetCount(numVerts);
    newFaces.SetCount(numFaces);

	tm.NoScale();
	tm.NoTrans();	

	mesh->buildNormals();			//make sure normal data is up to date

	for (int fc=0; fc<numFaces; fc++)
	{			
		DWORD idx;	
		Face tmpFace;

		tmpFace = mesh->faces[fc];

		for (int fvcount=0; fvcount<3; fvcount++)
		{
			s2vert_t tmpVert;			

			/*if (negparity)
				fv = 2 - fvcount;
			else
				fv = fvcount;*/

			idx = mesh->faces[fc].v[fvcount];
			
			tmpVert.vertidx = idx;
			if (!mesh->getNumTVerts())
			{
				tmpVert.uv.x = 0;
				tmpVert.uv.y = 0;
			}
			else
			{
				tmpVert.uv = mesh->tVerts[mesh->tvFace[fc].t[fvcount]];
			}
			if (!mesh->getNumVertCol())
			{
				tmpVert.col.x = 1;
				tmpVert.col.y = 1;
				tmpVert.col.z = 1;
			}
			else
			{
				tmpVert.col = mesh->vertCol[mesh->vcFace[fc].t[fvcount]];
			}
			tmpVert.nml = GetVertexNormal(mesh, fc, mesh->getRVertPtr(idx));
			tmpVert.nml.Normalize();

			tmpVert.nml = tm * tmpVert.nml;

			if (vWritten[idx] && (tmpVert.uv.x != newVerts[idx].uv.x || tmpVert.uv.y != newVerts[idx].uv.y))
			{
				//create a duplicate vertex to hold this UV info
				ret++;
				
				tmpFace.v[fvcount] = newVerts.Count();		//make sure this face points to the new vert				
				newVerts.Insert(newVerts.Count(), 1, &tmpVert);				
			}
			else
			{
				newVerts[idx] = tmpVert;
                vWritten.Set(idx);				
			}			
		}

		newFaces[fc] = tmpFace;

		if (negparity)
		{
			newFaces[fc].v[0] = tmpFace.v[2];
			newFaces[fc].v[2] = tmpFace.v[0];
		}
	}

	return ret;
}

//often meshes will have more than one UV coordinate assigned to a vertex
//in this case, we need to create extra vertex/face data to account for this
BOOL ModelExporterR4::CreateExtraTopology()
{
	int deleteIt;
	int extra;

	TriObject *triobj = GetTriObject(ip, FRAME(GetStartFrame()), node, deleteIt);
	if (!triobj)
	{
		Error("CreateExtraTopology: Failed to retrieve TriObject");
		return FALSE;
	}

	original_num_verts = triobj->mesh.getNumVerts();
	
	Matrix3 tm = node->GetObjectTM(FRAME(GetStartFrame()));
	BOOL negParity = tm.Parity();
	extra = CreateFaceAndVertexData(&triobj->mesh, new_faces, new_verts, tm/*, negParity*/);
	if (extra)
	{
		AddMessage(fmt("%i extra vertices created to account for multiple UVs", extra));
	}
	if (negParity)
	{
		AddMessage(fmt("Node transform has a negative parity"));
	}
	
	if (deleteIt)
		triobj->DeleteMe();

	return TRUE;
}

BOOL ModelExporterR4::ReAllocVerts(mesh_t *s2mesh, int count)
{
	//realloc vertex coordinates
	vec3_t *verts = (vec3_t *)Tag_Realloc(s2mesh->verts, sizeof(vec3_t) * count, MEM_SCENE);
	if (!verts)
	{
		Error("ReAllocVerts failed");
		return FALSE;
	}
	s2mesh->verts = verts;

	//realloc weights
	switch(s2mesh->mode)
	{/*
		case MESH_KEYFRAMED:
		{
			int n;
			int numframes = GetNumFrames();
			for (n=0; n<numframes; n++)
			{
				keyVert_t *key_verts = (keyVert_t *)TagRealloc(s2mesh->keyframes[n].key_verts, sizeof(keyVert_t) * count);
				if (!key_verts)
					return FALSE;
				s2mesh->keyframes[n].key_verts = key_verts;
			}
			break;
		}
		*/
		case MESH_SKINNED_BLENDED:
		{
			blendedLink_t *blendedLinks = (blendedLink_t *)Tag_Realloc(s2mesh->blendedLinks, sizeof(blendedLink_t) * count, MEM_SCENE);		
			if (!blendedLinks)
			{
				Error("ReAllocVerts failed");
				return FALSE;
			}
			s2mesh->blendedLinks = blendedLinks;
			break;
		}
		case MESH_SKINNED_NONBLENDED:
		{
			singleLink_t *singleLinks = (singleLink_t *)Tag_Realloc(s2mesh->singleLinks, sizeof(singleLink_t) * count, MEM_SCENE);
			if (!singleLinks)
			{
				Error("ReAllocVerts failed");
				return FALSE;
			}
			s2mesh->singleLinks = singleLinks;
			break;
		}
		default:
		{
			Error("ReAllocVerts: invalid mesh mode");
			return FALSE;						
		}
	}

	s2mesh->num_verts = count;

	return TRUE;
}

void ModelExporterR4::DuplicateVertex(mesh_t *s2mesh, DWORD from, DWORD to)
{
	//copy vertex coord
	s2mesh->verts[to][0] = s2mesh->verts[from][0];
	s2mesh->verts[to][1] = s2mesh->verts[from][1];
	s2mesh->verts[to][2] = s2mesh->verts[from][2];

	//copy weights
	switch(s2mesh->mode)
	{
		/*
		case MESH_KEYFRAMED:
		{
			int numframes = GetNumFrames();
			for (int n=0; n<numframes; n++)
			{				
				s2mesh->keyframes[n].key_verts[to][0] = s2mesh->keyframes[n].key_verts[from][0];
				s2mesh->keyframes[n].key_verts[to][1] = s2mesh->keyframes[n].key_verts[from][1];
				s2mesh->keyframes[n].key_verts[to][2] = s2mesh->keyframes[n].key_verts[from][2];
			}
			break;
		}
		*/
		case MESH_SKINNED_BLENDED:
		{
			int num_weights, n;
			num_weights = s2mesh->blendedLinks[to].num_weights = s2mesh->blendedLinks[from].num_weights;
			s2mesh->blendedLinks[to].indexes = (int *)Tag_Malloc(sizeof(int) * num_weights, MEM_SCENE);
			s2mesh->blendedLinks[to].weights = (float *)Tag_Malloc(sizeof(float) * num_weights, MEM_SCENE);
			for (n=0; n<num_weights; n++)
			{
				s2mesh->blendedLinks[to].indexes[n] = s2mesh->blendedLinks[from].indexes[n];
				s2mesh->blendedLinks[to].weights[n] = s2mesh->blendedLinks[from].weights[n];				
			}			
			break;
		}
		case MESH_SKINNED_NONBLENDED:
		{
			s2mesh->singleLinks[to] = s2mesh->singleLinks[from];			
			break;
		}
		default:
		{
			return;			
		}
	}
}

void ModelExporterR4::AddTextureReference(int meshidx)
{
	int n;
	mesh_t *s2mesh = &model.meshes[meshidx];

	if (num_texrefs >= MAX_TEXREFS)
	{
		AddMessage("Exceeded max texture references for model!");
		return;
	}

	//find the texture associated with this mesh

	Mtl *mtl = node->GetMtl();

	if (!mtl)
		return;

	if (mtl->ClassID() != Class_ID(DMTL_CLASS_ID, 0))
		return;	

	Texmap *tmap = mtl->GetSubTexmap(ID_DI);
	if (!tmap)
		return;

	if (tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
	{
		BitmapTex *bmt = texrefs[num_texrefs].bmptex = (BitmapTex *)tmap;
		strncpySafe(s2mesh->defaultShader, Filename_GetFilename(bmt->GetMapName()), SKIN_SHADERNAME_LENGTH);

		for (n=0; n<num_texrefs; n++)
		{
			if (bmt == texrefs[n].bmptex)
				return;			
		}

		texrefs[num_texrefs].meshidx = meshidx;
		
		num_texrefs++;
	}	
}

BOOL ModelExporterR4::FinishMesh(mesh_t *s2mesh)
{
	int old_num_verts = s2mesh->num_verts;

	if (mode == MODE_WRITE_CLIP)
		return TRUE;

	if (!CreateExtraTopology())
	{		
		Error("FinishMesh: failed to create extra topology");
		return FALSE;
	}

	s2mesh->tverts = (vec2_t *)Tag_Malloc(sizeof(vec2_t) * new_verts.Count(), MEM_SCENE);
	if (!s2mesh->tverts)
	{
		Error("FinishMesh: failed to allocate tverts");
		return FALSE;
	}
	s2mesh->colors = (bvec4_t *)Tag_Malloc(sizeof(bvec4_t) * new_verts.Count(), MEM_SCENE);
	if (!s2mesh->colors)
	{
		Error("FinishMesh: failed to allocate colors");
		return FALSE;
	}
	s2mesh->normals = (vec3_t *)Tag_Malloc(sizeof(vec3_t) * new_verts.Count(), MEM_SCENE);
	if (!s2mesh->normals)
	{
		Error("FinishMesh: failed to allocate normals");
		return FALSE;
	}

	if (new_verts.Count() > old_num_verts)
	{
		//allocate extra space for vertices
		if (!ReAllocVerts(s2mesh, new_verts.Count()))
		{
			Error("FinishMesh: ReAllocVerts failed");
			return FALSE;
		}
	}

	for (int v=0; v<s2mesh->num_verts; v++)
	{
		s2mesh->tverts[v][0] = new_verts[v].uv.x;
		s2mesh->tverts[v][1] = 1 - new_verts[v].uv.y;		//silverback wants V coord flipped

		s2mesh->colors[v][0] = new_verts[v].col.x * 255;
		s2mesh->colors[v][1] = new_verts[v].col.y * 255;
		s2mesh->colors[v][2] = new_verts[v].col.z * 255;
		s2mesh->colors[v][3] = 255;

		s2mesh->normals[v][0] = new_verts[v].nml.x;// * 127.0;
		s2mesh->normals[v][1] = new_verts[v].nml.y;// * 127.0;
		s2mesh->normals[v][2] = new_verts[v].nml.z;// * 127.0;

		if (v >= old_num_verts)
		{
			DuplicateVertex(s2mesh, new_verts[v].vertidx, v);
		}
	}

	//copy face connectivity data

	s2mesh->num_faces = new_faces.Count();

	s2mesh->facelist = (uivec3_t *)Tag_Malloc(sizeof(uivec3_t) * s2mesh->num_faces, MEM_SCENE);

	for (int n=0; n<s2mesh->num_faces; n++)
	{
		s2mesh->facelist[n][0] = new_faces[n].v[0];
		s2mesh->facelist[n][1] = new_faces[n].v[1];
		s2mesh->facelist[n][2] = new_faces[n].v[2];
	
		//sanity check (probably not necessary, but max is so buggy...)
		if (s2mesh->facelist[n][0] > new_verts.Count() || s2mesh->facelist[n][1] > new_verts.Count() || s2mesh->facelist[n][2] > new_verts.Count())
		{
			Error(fmt("Node %s contains invalid face data!", node->GetName()));

			return FALSE;
		}
	}

	//de-allocated topology arrays
	new_verts.SetCount(0);
	new_faces.SetCount(0);

	AddMessage("");
	AddMessage(fmt("Mesh has %i vertices and %i faces", s2mesh->num_verts, s2mesh->num_faces));	

	return TRUE;
}

/*
//create a reference mesh with extra vertices/face data to account for multiple UVs
BOOL ModelExporterR4::CreateReferenceMesh()
{
	int deleteIt;
	Tab<s2vert_t> newVerts;
	Tab<Face> newFaces;

	TriObject *triobj = GetTriObject(ip, FRAME(GetStartFrame()), node, deleteIt);
	if (!triobj)
		return FALSE;

	original_num_verts = triobj->mesh.getNumVerts();

	CreateFaceAndVertexData(&triobj->mesh, newFaces, newVerts, node->GetObjectTM(FRAME(GetStartFrame())).Parity());

	mesh = new Mesh(triobj->mesh);
	if (!mesh)
		return FALSE;

	mesh->setNumVerts(newVerts.Count());
	mesh->setNumVertCol(newVerts.Count());
	mesh->setNumTVerts(newVerts.Count());
	mesh->setNumFaces(newFaces.Count());

	for (int n = 0; n < newVerts.Count(); n++)
	{
		mesh->setVert(n, triobj->mesh.verts[newVerts[n].vertidx]);
		mesh->setTVert(n, newVerts[n].uv);
		mesh->vertCol[n] = newVerts[n].col;
	}
	for (n = 0; n < newFaces.Count(); n++)
	{
		mesh->faces[n] = newFaces[n];
	}

	if (deleteIt)
		triobj->DeleteMe();

	return TRUE;
}
*/

void ModelExporterR4::DeleteReferenceMesh()
{
}


//compute bounding box in MESH SPACE
void	ComputeBounds(Mesh *mesh, /*Matrix3 tm,*/ vec3_t bmin, vec3_t bmax)
{
	Point3 p;

	M_ClearBounds(bmin, bmax);

	for (int n=0; n<mesh->numVerts; n++)
	{
		p = mesh->verts[n];// * tm;

		if (p.x < bmin[0])
			bmin[0] = p.x;
		if (p.y < bmin[1])
			bmin[1] = p.y;
		if (p.z < bmin[2])
			bmin[2] = p.z;
	
		if (p.x > bmax[0])
			bmax[0] = p.x;
		if (p.y > bmax[1])
			bmax[1] = p.y;
		if (p.z > bmax[2])
			bmax[2] = p.z;
	}

	//expand the bounds by an epsilon to prevent divide by 0 errors
	bmin[0] -= EPSILON;
	bmin[1] -= EPSILON;
	bmin[2] -= EPSILON;
	bmax[0] += EPSILON;
	bmax[1] += EPSILON;
	bmax[2] += EPSILON;
}


//add mesh face connectivity data, mesh name, etc and store in 's2mesh'
BOOL ModelExporterR4::AddBaseMeshData(mesh_t *s2mesh)
{
	int deleteIt;

	if (!s2mesh)
	{
		Error("NULL passed into AddBaseMeshData");
		return FALSE;
	}

	if (strlen(node->GetName()) >= MESH_NAME_LENGTH-1)
	{
		Error(fmt("Name too long.  Shorten to less than %i characters", MESH_NAME_LENGTH));
	}

	memset(s2mesh, 0, sizeof(mesh_t));

	TriObject *triobj = GetTriObject(ip, FRAME(GetStartFrame()), node, deleteIt);
	if (!triobj)
	{
		Error("AddBaseMeshData: failed to retrieve TriObject");
		return FALSE;
	}
	Mesh *mesh = &triobj->mesh;
	
	strncpySafe(s2mesh->name, node->GetName(), MESH_NAME_LENGTH);

	//compute bounding box
	//ComputeBounds(&triobj->mesh, s2mesh->bmin, s2mesh->bmax);

	if (deleteIt)
		triobj->DeleteMe();

	return TRUE;
}


//copy over mesh vertices.
//if boneindex is -1, vertices are stored in world coords (physiqued meshes should do this),
//otherwise vertices are transformed into the space of the bone
BOOL ModelExporterR4::CopyMeshVerts(mesh_t *s2mesh, int frame, int boneindex)
{
	int n;
	int deleteIt;
	Matrix3 worldTM, tm;

	//copy over the vertices from max to the s2mesh

	TriObject *triobj = GetTriObject(ip, frame, node, deleteIt);
	if (!triobj)
	{
		Error("CopyMeshVerts: failed to retrieve TriObject");
		return FALSE;
	}
	Mesh *mesh = &triobj->mesh;

	worldTM = node->GetObjectTM(FRAME(GetStartFrame()));

	if (boneindex > -1)
	{
		//get the inverse of the bone matrix for the world->bone space conversion
		/*Matrix3 invBoneTM = boneNodes[boneindex]->GetObjectTM(FRAME(GetStartFrame()));
		invBoneTM.Invert();
		tm = worldTM * invBoneTM;		//this matrix will transform the verts from mesh->bone space*/
		//tm.IdentityMatrix();
		tm = worldTM;
	}
	else
		tm = worldTM;
	
	s2mesh->verts = (vec3_t *)Tag_Malloc(sizeof(vec3_t) * mesh->numVerts, MEM_SCENE);
	if (!s2mesh->verts)
	{
		Error("Failed to allocate mesh verts");
		return FALSE;
	}
	s2mesh->num_verts = mesh->numVerts;

	M_ClearBounds(s2mesh->bmin, s2mesh->bmax);

	for (n=0; n<s2mesh->num_verts; n++)
	{	
		Point3 vert = mesh->verts[n] * tm;
		s2mesh->verts[n][0] = vert.x;
		s2mesh->verts[n][1] = vert.y;
		s2mesh->verts[n][2] = vert.z;

		//contribute to bounding box
		M_AddPointToBounds(s2mesh->verts[n], s2mesh->bmin, s2mesh->bmax);
		M_AddPointToBounds(s2mesh->verts[n], model.bmin, model.bmax);
	}

	s2mesh->bonelink = boneindex;

	if (deleteIt)
		triobj->DeleteMe();

	AddMessage(fmt("Copied %i vertices", s2mesh->num_verts));
	
	return TRUE;
}

void MatrixToTransform(Matrix3 &tm, transform_t *transform)
{
	Point3 axis[3];
	Point3 pos;

	axis[0] = tm.GetRow(0);
	axis[1] = tm.GetRow(1);
	axis[2] = tm.GetRow(2);
	pos = tm.GetRow(3);

	transform->t.axis[0][0] = axis[0].x;
	transform->t.axis[0][1] = axis[0].y;
	transform->t.axis[0][2] = axis[0].z;
	transform->t.axis[0][3] = 0;
	transform->t.axis[1][0] = axis[1].x;
	transform->t.axis[1][1] = axis[1].y;
	transform->t.axis[1][2] = axis[1].z;
	transform->t.axis[1][3] = 0;
	transform->t.axis[2][0] = axis[2].x;
	transform->t.axis[2][1] = axis[2].y;
	transform->t.axis[2][2] = axis[2].z;
	transform->t.axis[2][3] = 0;
	transform->t.pos[0] = pos.x;
	transform->t.pos[1] = pos.y;
	transform->t.pos[2] = pos.z;
	transform->t.pos[3] = 1;
}


/*
BOOL ModelExporterR4::CopyMeshVerts(mesh_t *s2mesh, int frame, BOOL transformToWorld)
{
	int n;
	int deleteIt;

	//copy over the vertices from max to the s2mesh

	TriObject *triobj = GetTriObject(ip, frame, node, deleteIt);
	if (!triobj)
		return FALSE;
	Mesh *mesh = &triobj->mesh;

	s2mesh->num_verts = mesh->numVerts;
	s2mesh->verts = (vec3_t *)TagMalloc(sizeof(vec3_t) * s2mesh->num_verts, MEM_SCENE);

	//convert from void * to vec3_t *
	vec3_t *verts = (vec3_t *)s2mesh->verts;

	if (transformToWorld)
	{
		Matrix3 tm = node->GetObjectTM(FRAME(frame));
		for (n=0; n<s2mesh->num_verts; n++)
		{
			Point3 vert = Point3(mesh->verts[n].x, mesh->verts[n].y, mesh->verts[n].z);
			vert = vert * tm;
			verts[n][0] = vert.x;
			verts[n][1] = vert.y;
			verts[n][2] = vert.z;
		}
	}
	else
	{
		for (n=0; n<s2mesh->num_verts; n++)
		{	
			verts[n][0] = mesh->verts[n].x;
			verts[n][1] = mesh->verts[n].y;
			verts[n][2] = mesh->verts[n].z;
		}
	}
	

	if (deleteIt)
		triobj->DeleteMe();

	AddMessage(fmt("Copied %i vertices", s2mesh->num_verts));
	
	return TRUE;
}
*/







bone_t *ModelExporterR4::AddBone(INode *refnode, const char *name, int *index)
{
	if (strlen(name) >= BONE_NAME_LENGTH-1)
	{
		Error(fmt("Bone name %s is too long.  Needs to be < %i characters.", name, BONE_NAME_LENGTH));
		return NULL;
	}
	if (model.num_bones >= MAX_BONES)
	{
		Error(fmt("Tried to add more than %i bones to the model", MAX_BONES));
		return NULL;
	}
	//do a name check to make sure this bone doesn't already exist
	for (int n=0; n<model.num_bones; n++)
	{
		if (strncmp(model.bones[n].name, name, BONE_NAME_LENGTH-1)==0)
		{
			*index = n;
			return &model.bones[n];
		}
	}

	//allocate a new bone
	bone_t *newbones = (bone_t *)Tag_Realloc(model.bones, sizeof(bone_t) * (model.num_bones + 1), MEM_SCENE);
	if (!newbones)
		return NULL;

	model.bones = newbones;

	bone_t *bone = &model.bones[model.num_bones];
	strncpySafe(bone->name, name, BONE_NAME_LENGTH);

	Matrix3 baseTM = GetBoneTM(refnode, FRAME(GetStartFrame()));		
	MatrixToTransform(baseTM, &bone->base);
	Matrix3 invBaseTM = baseTM;
	invBaseTM.Invert();
	MatrixToTransform(invBaseTM, &bone->invBase);
	//keep track of the INode we got the bone from

	if (debug_info)
	{
		Matrix3 ident = baseTM * invBaseTM;
		AddMessage(fmt("ident:  %f %f %f  %f %f %f  %f %f %f  %f %f %f",
			ident.GetRow(0).x, ident.GetRow(0).y, ident.GetRow(0).z,
			ident.GetRow(1).x, ident.GetRow(1).y, ident.GetRow(1).z,
			ident.GetRow(2).x, ident.GetRow(2).y, ident.GetRow(2).z,
			ident.GetRow(3).x, ident.GetRow(3).y, ident.GetRow(3).z));

		transform_t ident2;
		M_MultiplyTransforms(&bone->base, &bone->invBase, &ident2);
		AddMessage(fmt("ident2: %f %f %f  %f %f %f  %f %f %f  %f %f %f",
			ident2.matrix[0], ident2.matrix[1], ident2.matrix[2],
			ident2.matrix[4], ident2.matrix[5], ident2.matrix[6],
			ident2.matrix[8], ident2.matrix[9], ident2.matrix[10],
			ident2.matrix[12], ident2.matrix[13], ident2.matrix[14]));		
	}

	boneNodes[model.num_bones] = refnode;

	//add the bone to the modelClip structure in case we're exporting animation	
	modelClip.motions = (boneMotion_t *)Tag_Realloc(modelClip.motions, sizeof(boneMotion_t) * (model.num_bones + 1), MEM_SCENE);
	modelClip.motions[model.num_bones].bone = model.num_bones;


//	bone->frames = (transform_t *)malloc(sizeof(transform_t) * GetNumFrames());
//	if (!bone->frames)
//		return NULL;
	
	*index = model.num_bones;

	model.num_bones++;

	//get parent information
	INode *parentNode = GetS2BoneParentNode(refnode);
	if (parentNode)
	{
		int idx;
		bone_t *parentBone;		
		parentBone = AddBone(parentNode, parentNode->GetName(), &idx);
		bone = &model.bones[*index];			//bone may have become invalid after the Realloc, so reintiialize the pointer
		bone->parentIndex = idx;

		if (debug_info)
		{
			Matrix3 invParentTM = GetBoneTM(parentNode, FRAME(GetStartFrame()));
			invParentTM.Invert();

			Matrix3 localTM = invParentTM * baseTM;

			AddMessage(fmt("localTM:  %f %f %f  %f %f %f  %f %f %f  %f %f %f",
				localTM.GetRow(0).x, localTM.GetRow(0).y, localTM.GetRow(0).z,
				localTM.GetRow(1).x, localTM.GetRow(1).y, localTM.GetRow(1).z,
				localTM.GetRow(2).x, localTM.GetRow(2).y, localTM.GetRow(2).z,
				localTM.GetRow(3).x, localTM.GetRow(3).y, localTM.GetRow(3).z));
		}
	}
	else
	{
		bone->parentIndex = -1;
	}

	if (bone->parentIndex > -1)
	{
		AddMessage(fmt("Added bone \"%s\" (index %i) to model (parent: %s)", name, *index, model.bones[bone->parentIndex].name));
	}
	else
	{
		AddMessage(fmt("Added bone \"%s\" (index %i) to model (no parent)", name, *index));
	}

	return bone;
}

bone_t *ModelExporterR4::GetBone(const char *name, int *index)
{	
	for (int n=0; n<model.num_bones; n++)
	{
		if (strncmp(model.bones[n].name, name, BONE_NAME_LENGTH-1)==0)
		{
			*index = n;
			return &model.bones[n];
		}
	}

	return NULL;
}


//add a rigid mesh (a mesh that may transform but the vertices remain static)
BOOL ModelExporterR4::AddNonPhysiquedMesh(mesh_t *s2mesh)
{
	bone_t *bone;
	int index;

	//make a bone which represents this mesh's transform
	bone = AddBone(node, node->GetName(), &index);
	if (!bone)
	{
		Error("AddNonPhysiquedMesh: failed to add bone");
		return FALSE;
	}

	if (mode == MODE_WRITE_CLIP)
		return TRUE;					//we don't need any info beyond the animation data

	//copy the mesh vertices relative to the bone we just made
	if (!CopyMeshVerts(s2mesh, GetStartFrame(), index))
	{
		Error("AddNonPhysiquedMesh: failed to copy mesh verts");
		return FALSE;
	}

	/*
	//transform the verts back into world space to calculate the bounding box
	Matrix3 tm = boneNodes[index]->GetObjectTM(FRAME(GetStartFrame()));

	for (int n=0; n<s2mesh->num_verts; n++)
	{
		//add to the model bounds
		Point3 p = s2mesh->verts[n] * tm;			
		M_AddPointToBounds((float *)(p), model.bmin, model.bmax);
	}
	*/

	s2mesh->mode = MESH_SKINNED_NONBLENDED;

	return TRUE;
	
}

BOOL	ModelExporterR4::AddKeyframedMesh(mesh_t *s2mesh)
{
	Error("Keyframes not currently supported");
	return FALSE;
}

/*
//add keyframed vertex info to a mesh
BOOL ModelExporterR4::AddKeyframedMesh(mesh_t *s2mesh)
{	
	int numframes = GetNumFrames();
	BOOL firstloop = TRUE;

	s2mesh->keyframes = (keyframe_t *)malloc(sizeof(keyframe_t) * numframes);
	if (!s2mesh->keyframes)
		return FALSE;

	SecondaryStatusText("Adding keyframed mesh");

	int endframe = GetEndFrame();
	int startframe = GetStartFrame();
	for (int frame = startframe; frame < endframe; frame++)
	{
		int deleteIt;
		keyframe_t *keyframe = &s2mesh->keyframes[frame-startframe];
		TriObject *triobj = GetTriObject(ip, FRAME(frame), node, deleteIt);

		if (!triobj)
			return FALSE;

		SecondaryStatusProgress(((float)(frame - startframe) / GetNumFrames()) * 100.0);

		Mesh *mesh = &triobj->mesh;

		if (firstloop)
		{
			s2mesh->num_verts = mesh->numVerts;
			firstloop = FALSE;
		}

		keyframe->key_verts = (keyVert_t *)malloc(sizeof(keyVert_t) * s2mesh->num_verts);
		if (!keyframe->key_verts)
			return FALSE;
	
		vec3_t bmin, bmax;
		ComputeBounds(mesh, bmin, bmax);
		M_CalcBoxExtents(bmin, bmax, keyframe->bpos, keyframe->bext);

		for (int v=0; v<s2mesh->num_verts; v++)
		{
			Point3 vert;
			int compressed[3];

			//compress to -0.5 to 0.5 range
			vert.x = (mesh->verts[v].x - keyframe->bpos[0]) / keyframe->bext[0];
			vert.y = (mesh->verts[v].y - keyframe->bpos[1]) / keyframe->bext[1];
			vert.z = (mesh->verts[v].z - keyframe->bpos[2]) / keyframe->bext[2];

			//transform to model space
			vert = vert * node->GetObjectTM(FRAME(frame));

			//while we're at it, add to the model bounds
			M_AddPointToBounds((float *)(vert), model.bmin, model.bmax);


			compressed[0] = (char)(vert.x * 255);
			compressed[1] = (char)(vert.y * 255);
			compressed[2] = (char)(vert.z * 255);
			for (int i=0; i<3; i++)
			{
				if (compressed[i] > 127)
					compressed[i] = 127;
				else if (compressed[i] < -128)
					compressed[i] = -128;
			}
			
			keyframe->key_verts[v][0] = (char)(compressed[0]);
			keyframe->key_verts[v][1] = (char)(compressed[1]);
			keyframe->key_verts[v][2] = (char)(compressed[2]);
		}

		if (deleteIt)
			triobj->DeleteMe();
	}

	AddMessage(fmt("Copied %i keyframes", numframes));

	return TRUE;
}
*/

//this can be expanded later to export a user specified animation range if need be
int	ModelExporterR4::GetStartFrame()
{
	return (ip->GetAnimRange().Start() / GetTicksPerFrame());
}

int ModelExporterR4::GetEndFrame()
{
	return (ip->GetAnimRange().End() / GetTicksPerFrame());
}

int ModelExporterR4::GetNumFrames()
{
	return (GetEndFrame() - GetStartFrame()) + 1;
}




#if 0
//this is the general animated but non-physiqued mesh function
//first we must determine if this mesh has deforming vertices or not
//then we call the appropriate function
BOOL ModelExporterR4::AddAnimatingMesh(mesh_t *s2mesh, BOOL checkDeform)
{
	int frame;
	int deleteIt;
	int numframes = GetNumFrames();
	BOOL deformed = FALSE;
	TriObject *tri1, *tri2;
	Mesh copy;
	int endframe = GetEndFrame();

	//figured i could test the validity interval to determine if the object was deforming or not
	//but this doesn't always seem to work (what a surprise)...right after a file open the validity does not
	//seem to get set to FOREVER, even if it should be...if i move the frame slider a little bit
	//in max and test it again, i get the correct validity interval of FOREVER.  screw it.

	SecondaryStatusText("Determining if mesh deforms");

	if (GetNumModifiers(node) && checkDeform)
	{
		for (frame = GetStartFrame()+1; frame <= endframe; frame++)
		{
			int v;

			SecondaryStatusProgress(((float)(frame - GetStartFrame()) / GetNumFrames()) * 100.0);

			tri1 = GetTriObject(ip, FRAME(frame-1), node, deleteIt);
			if (!tri1)
				return FALSE;

			copy = tri1->mesh;

			if (deleteIt)
				tri1->DeleteMe();

			tri2 = GetTriObject(ip, FRAME(frame), node, deleteIt);
			if (!tri2)
				return FALSE;

			/*if (tri1 == tri2)
			{
				if (delete1)
					tri1->DeleteMe();
				continue;
			}*/

			//compare the vertices of the two meshes
			if (tri2->mesh.numVerts != copy.numVerts)
			{
				if (deleteIt)
					tri2->DeleteMe();
				Error("Mesh did not keep the same number of verts");
				return FALSE;
			}

			for (v=0; v<tri2->mesh.numVerts; v++)
			{
				if (copy.verts[v] != tri2->mesh.verts[v])
				{
					deformed = TRUE;
					break;
				}			
			}

			if (deleteIt)
				tri2->DeleteMe();

			if (deformed)
				break;
		}
	}

	if (deformed)
	{
		AddMessage("Mesh vertices are animated, mesh mode is KEYFRAMED");		
		return AddKeyframedMesh(s2mesh);		
	}
	else
	{
		AddMessage("Mesh vertices are not animated, mesh mode is RIGID");
		return AddRigidMesh(s2mesh);
	}
}

#endif

BOOL ModelExporterR4::AddMeshToModel(mesh_t *s2mesh)
{
	mesh_t *meshes = (mesh_t *)Tag_Realloc(model.meshes, sizeof(mesh_t) * (model.num_meshes+1), MEM_SCENE);
	if (!meshes)
	{
		Error("AddMeshToModel failed on realloc");
		return FALSE;
	}
	model.meshes = meshes;
	
	memcpy(&meshes[model.num_meshes], s2mesh, sizeof(mesh_t));		//this will copy the pointer values, which will remain valid until the end of the export

	AddTextureReference(model.num_meshes);
	
	model.num_meshes++;

	return TRUE;
}

BOOL IsNodeSelected(INode *node)
{
	int n;

	for (n=0; n<GetCOREInterface()->GetSelNodeCount(); n++)
	{
		if (GetCOREInterface()->GetSelNode(n) == node)
			return TRUE;
	}

	return FALSE;
}

BOOL ModelExporterR4::ShouldCheckDeform(INode *node)
{
	if (!selected_deform_check)
		return TRUE;

	if (IsNodeSelected(node))
		return TRUE;
	else
		return FALSE;
}

void ModelExporterR4::AddMesh(INode *node)
{
	mesh_t *s2mesh = (mesh_t *)Tag_Malloc(sizeof(mesh_t), MEM_SCENE);

	BeginNode(node, NODETYPE_MESH);

	if (!AddBaseMeshData(s2mesh))				//add data common to all mesh types
		goto meshError;
	
	if (ignore_physique)
	{
		if (!AddNonPhysiquedMesh(s2mesh))
			goto meshError;
	}	
	else
	{
		Modifier *phyMod = IsNodePhysiqued(node);
		if (phyMod)
		{						
			if (!AddPhysiquedMesh(s2mesh, phyMod))
				goto meshError;
		}
		else
		{
			if (!AddNonPhysiquedMesh(s2mesh))
				goto meshError;
		}		
	}

	if (!FinishMesh(s2mesh))
		goto meshError;

	//mesh data stored successfully, add this mesh to the model
	AddMeshToModel(s2mesh);	

	EndNode();

	return;

meshError:
	//free any mesh data that was allocated and return
	AddMessage("There was an error with this mesh");
	EndNode();	
}

void ModelExporterR4::AddReferenceBone(INode *node)
{
	int index;
	bone_t *bone;

	BeginNode(node, NODETYPE_REFERENCE_BONE);

	bone = AddBone(node, node->GetName(), &index);
	if (!bone)
		return;
	
	EndNode();
}

#define	FACEVERTEX(mesh, face, idx) mesh->verts[mesh->faces[face].v[idx]]



BOOL ModelExporterR4::AddSurfToModel(INode *node, convexPolyhedron_t *surf)
{
	convexPolyhedron_t *surfs = (convexPolyhedron_t *)Tag_Realloc(model.surfs, sizeof(convexPolyhedron_t) * (model.num_surfs+1), MEM_SCENE);
	if (!surfs)
	{
		Error("AddSurfToModel failed on realloc");
		return FALSE;
	}
	model.surfs = surfs;
	
	model.surfs[model.num_surfs] = *surf;
		
	model.num_surfs++;

	return TRUE;
}

//add a collision surface
void ModelExporterR4::AddSurf(INode *node)
{
	int deleteIt;
	TriObject *triobj;	
	int *ignoreFaces;
	Mesh *mesh;
	Matrix3 tm;
	convexPolyhedron_t surf;	
	int i;
	Point3 min,max;

	BeginNode(node, NODETYPE_SURFACE);

	memset(&surf, 0, sizeof(surf));

	triobj = GetTriObject(ip, FRAME(GetStartFrame()), node, deleteIt);	

	if (!triobj)
	{
		Error("Couldn't get TriObject");
		return;
	}

	if (!TestMeshConvexity(triobj->mesh))
	{
		Error("The surface is not convex");
		return;
	}
	
	BOOL negParity = FALSE;

	tm = node->GetObjectTM(FRAME(GetStartFrame()));
	if (tm.Parity())
	{
		negParity = TRUE;
		AddMessage("Using reversed winding");
	}

	mesh = &triobj->mesh;

	//allocate ignorefaces array (tag malloc memsets everything to 0 for us)
	ignoreFaces = (int *)Tag_Malloc(sizeof(int) * mesh->numFaces, MEM_SCENE);

	AddMessage(fmt("Generating surface from a mesh with %i faces", mesh->numFaces));

	for (int face = 0; face < mesh->numFaces; face++)
	{
		if (ignoreFaces[face])
			continue;

		plane_t plane;

		//get the face plane
		if (negParity)
		{
			M_CalcPlane((float*)(FACEVERTEX(mesh, face, 2)) * tm, (float*)(FACEVERTEX(mesh, face, 1)) * tm, (float*)(FACEVERTEX(mesh, face, 0)) * tm, &plane);
		}
		else
		{
			M_CalcPlane((float*)(FACEVERTEX(mesh, face, 0)) * tm, (float*)(FACEVERTEX(mesh, face, 1)) * tm, (float*)(FACEVERTEX(mesh, face, 2)) * tm, &plane);
		}

		//see if we can merge other faces into this plane
		for (int i = face + 1; i < mesh->numFaces; i++)
		{
			plane_t nextPlane;

			if (negParity)
			{
				M_CalcPlane((float*)(FACEVERTEX(mesh, i, 2)) * tm, (float*)(FACEVERTEX(mesh, i, 1)) * tm, (float*)(FACEVERTEX(mesh, i, 0)) * tm, &nextPlane);
			}
			else
			{
				M_CalcPlane((float*)(FACEVERTEX(mesh, i, 0)) * tm, (float*)(FACEVERTEX(mesh, i, 1)) * tm, (float*)(FACEVERTEX(mesh, i, 2)) * tm, &nextPlane);
			}

			if (M_CompareVec3(nextPlane.normal, plane.normal))
			{
				if (ABS(nextPlane.dist - plane.dist) > 0.01)
				{
					Error(fmt("Collision surface appears to be invalid (ABS(nextPlane.dist - plane.dist) == %f)", ABS(nextPlane.dist - plane.dist)));
					goto finishAddSurf;
				}

				//this face is on the same plane as mesh->faces[face], so we can safely ignore the face
				ignoreFaces[i] = 1;
			}
		}
				

		//add this plane to the surface
		surf.planes[surf.numPlanes] = plane;

		surf.numPlanes++;
		if (surf.numPlanes > MAX_POLYHEDRON_PLANES)		//the > instead of >= is intentional
		{
			Error("Surface contains too many planes...try cutting down the detail a bit");
			goto finishAddSurf;
		}
	}

	if (surf.numPlanes < 4)
	{
		Error(fmt("Not enough planes in surface (numplanes == %i)", surf.numPlanes));
		goto finishAddSurf;
	}

	//work out bounding box	
	ComputeBounds(&triobj->mesh, surf.bmin, surf.bmax);
	//make sure to convert to world space!
	min = Point3(surf.bmin) * tm;
	max = Point3(surf.bmax) * tm;
	
	for (i=0; i<3; i++)
	{
		//paranoia: expand the bounds a little in case it's not quite accurate
		surf.bmin[i] = min[i] - 1;
		surf.bmax[i] = max[i] + 1;
	}

	surf.num_faces = 0;		//not yet supported
	surf.num_verts = 0;		//not yet supported	

	//add the surface to the model
	if (AddSurfToModel(node, &surf))
	{
		AddMessage(fmt("Surface contains %i planes", surf.numPlanes));
		AddMessage(fmt("Bounds: (%f, %f, %f)  to  (%f, %f, %f)", surf.bmin[0], surf.bmin[1], surf.bmin[2], surf.bmax[0], surf.bmax[1], surf.bmax[2]));
	}

finishAddSurf:

	if (deleteIt)
		triobj->DeleteMe();

	Tag_Free(ignoreFaces);

	EndNode();
}

void ModelExporterR4::TraverseNode(INode *node)
{	
	int n, numChildren;

	//if (model_error)
	//	return;

	if (!node->IsHidden())		//ignore this node if hidden
	{
		Status(fmt("Processing %s...", node->GetName()), ((float)nodecount / num_scene_nodes) * 100);

		//check for special nodes
		if (strncmp(node->GetName(), "_billboard", 10)==0)
		{
			AddSprite(node, S2SPRITE_BILLBOARD);
		}
		else if (strncmp(node->GetName(), "_groundplane", 11)==0)
		{
			AddSprite(node, S2SPRITE_GROUNDPLANE);
		}
		else if (strncmp(node->GetName(), "_surf", 5)==0)
		{
			AddSurf(node);
		}		
		else if (strncmp(node->GetName(), "_bone", 5)==0)
		{
			//this is simply a reference bone we'll use for things like weapons which can be inserted in the model
			AddReferenceBone(node);
		}
		else
		{
			if (IsMesh(node, ip))
			{
				AddMesh(node);
			}
		}

		nodecount++;
	}	

	numChildren = node->NumberOfChildren();
	for (n=0; n<numChildren; n++)
		TraverseNode(node->GetChildNode(n));
}

void ModelExporterR4::CountNodes(INode *node)
{
	if (!node->IsHidden())
	{
		num_scene_nodes++;
	}

	int numChildren = node->NumberOfChildren();

	for (int n=0; n<numChildren; n++)
	{
		CountNodes(node->GetChildNode(n));
	}
}

static BOOL CALLBACK SceneInfoDlgProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int n = 0;

	switch(msg)
	{
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg, IDC_DETAILS, theModelExporterR4.output_message);			
			break;
		case WM_COMMAND:
			switch LOWORD(wParam)
			{
				case IDOK:
					EndDialog(hwndDlg, TRUE);
					break;
	        	default:
					break;
			}
		default:
			return FALSE;
	}

	return TRUE;
}

static BOOL CALLBACK ErrorInfoDlgProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int n = 0;

	switch(msg)
	{
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg, IDC_DETAILS, theModelExporterR4.error_message);			
			break;
		case WM_COMMAND:
			switch LOWORD(wParam)
			{
				case IDOK:
					EndDialog(hwndDlg, TRUE);
					break;
	        	default:
					break;
			}
		default:
			return FALSE;
	}

	return TRUE;
}


void ModelExporterR4::ClearSceneInfo()
{
	bone_t *bone;
	int idx;

	ClearIndent();

	nodecount = 0;
	physique_count = 0;
	spritecount = 0;
	keyframed_mesh_count = 0;
	num_meshes = 0;
	output_message[0] = '\0';
	error_message[0] = '\0';
	num_texrefs = 0;
	model_error = FALSE;

	ignore_physique = IsDlgButtonChecked(hPanel, IDC_IGNORE_PHYSIQUE);
	link_to_root = IsDlgButtonChecked(hPanel, IDC_LINK_TO_ROOT);
	debug_info = IsDlgButtonChecked(hPanel, IDC_DEBUG_INFO);
	selected_deform_check = IsDlgButtonChecked(hPanel, IDC_SELECTED_DEFORM_CHECK);
	export_clip = 0;
	export_textures = 0;
	export_objdef = 0;

	num_scene_nodes = 0;

	memset(&model, 0, sizeof(model_t));
	memset(&modelClip, 0, sizeof(modelClip_t));
	M_ClearBounds(model.bmin, model.bmax);

	CountNodes(ip->GetRootNode());

	//add origin bone
	bone = AddBone(ip->GetRootNode(), ip->GetRootNode()->GetName(), &idx);
}

static BOOL CALLBACK StatusBarDlgProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void ModelExporterR4::OpenStatusBar()
{
	if (hStatus)
		return;

	hStatus = CreateDialog(
		hInstance,
		MAKEINTRESOURCE(IDD_STATUS_BAR),
		hPanel,
		StatusBarDlgProc);
	ShowWindow(hStatus, SW_SHOWNORMAL);
}

void ModelExporterR4::Status(const char *msg, int progress)
{
	if (!hStatus)
		return;

	SetDlgItemText(hStatus, IDC_STATUS_MESSAGE, (LPCSTR)msg);
	SendDlgItemMessage(hStatus, IDC_PROGRESS, PBM_SETPOS, (WPARAM)progress, 0);
	SetDlgItemText(hStatus, IDC_SECONDARY_STATUS_MESSAGE, "");
	SendDlgItemMessage(hStatus, IDC_PROGRESS2, PBM_SETPOS, 0, 0);
}

void ModelExporterR4::SecondaryStatusText(const char *msg)
{
	if (!hStatus)
		return;

	SetDlgItemText(hStatus, IDC_SECONDARY_STATUS_MESSAGE, (LPCSTR)msg);	
}

void ModelExporterR4::SecondaryStatusProgress(int progress)
{
	if (!hStatus)
		return;
	
	SendDlgItemMessage(hStatus, IDC_PROGRESS2, PBM_SETPOS, (WPARAM)progress, 0);
}

void ModelExporterR4::CloseStatusBar()
{
	if (hStatus)
	{
		DestroyWindow(hStatus);
		hStatus = NULL;
	}
}

void ModelExporterR4::AddBaseModelData()
{

}

int ModelExporterR4::CountMeshType(int meshtype)
{
	int count = 0;
	for (int n=0; n<model.num_meshes; n++)
	{
		if (model.meshes[n].mode == meshtype)
			count++;
	}

	return count;
}

void ModelExporterR4::WriteFloatKeys(int boneindex, floatKeys_t *keys, int keytype, FILE *stream)
{
	bone_t *bone = &model.bones[boneindex];

	BeginBlock("bmtn", stream);

	keyBlock_t keyblock;

	keyblock.boneIndex = boneindex;
	strncpySafe(keyblock.boneName, bone->name, BONE_NAME_LENGTH);
	keyblock.key_type = keytype;
	keyblock.num_keys = keys->num_keys;

	WriteBlockData(&keyblock, sizeof(keyblock), stream);

	//write keys
	for (int n=0; n<keys->num_keys; n++)
	{
		WriteBlockData(&keys->keys[n], sizeof(float), stream);
	}

	EndBlock(stream);
}

void ModelExporterR4::WriteByteKeys(int boneindex, byteKeys_t *keys, int keytype, FILE *stream)
{
	bone_t *bone = &model.bones[boneindex];

	BeginBlock("bmtn", stream);

	keyBlock_t keyblock;

	keyblock.boneIndex = boneindex;
	strncpySafe(keyblock.boneName, bone->name, BONE_NAME_LENGTH);
	keyblock.key_type = keytype;
	keyblock.num_keys = keys->num_keys;

	WriteBlockData(&keyblock, sizeof(keyblock), stream);

	//write keys
	for (int n=0; n<keys->num_keys; n++)
	{
		WriteBlockData(&keys->keys[n], 1, stream);
	}

	EndBlock(stream);
}


void ModelExporterR4::WriteClipFile()
{
/*	FILE *f = fopen(filename, "wb");

	if (!f)
	{
		MessageBox(hPanel, "Couldn't create .clip file", "File Error", MB_OK);
		return;
	}

	fwrite(&model.num_frames, sizeof(int), 1, f);

	for (int b=0; b<model.num_bones; b++)
	{
		fwrite(model.bones[b].name, 1, strlen(model.bones[b].name)+1, f);

		for (int n=0; n<model.num_frames; n++)
		{			
			fwrite(&model.bones[b].frames[n], sizeof(transform_t), 1, f);
		}
	}

	fclose(f);*/

#ifdef WRITE_CLIP_AS_TEXT

	FILE *stream = fopen(filename, "wt");
	int numframes = GetNumFrames();
	int n,f,i;

	if (!stream)
	{
		MessageBox(hPanel, "Couldn't create .clip file", "File Error", MB_OK);
		return;
	}

	fprintf(stream, "%i frames\n\n", numframes);

	for (n=0; n<model.num_bones; n++)
	{
		fprintf(stream, "%s\n", model.bones[n].name);
		for (f=0; f<numframes; f++)
		{
			fprintf(stream, "[");
			for (i=0; i<3; i++)
			{
				fprintf(stream, " %f", modelClip.motions[n].transforms[f].axis[0][i]);
			}
			fprintf(stream, "  ");
			for (i=0; i<3; i++)
			{
				fprintf(stream, " %f", modelClip.motions[n].transforms[f].axis[1][i]);
			}
			fprintf(stream, "  ");
			for (i=0; i<3; i++)
			{
				fprintf(stream, " %f", modelClip.motions[n].transforms[f].axis[2][i]);
			}
			fprintf(stream, "  ");
			for (i=0; i<3; i++)
			{
				fprintf(stream, " %f", modelClip.motions[n].transforms[f].pos[i]);
			}
			fprintf(stream, " ]\n");
		}
		fprintf(stream, "\n\n");
	}

	fclose(stream);
#else

	FILE *stream = fopen(filename, "wb");
	int numframes = GetNumFrames();
	int n;

	if (!stream)
	{
		MessageBox(hPanel, "Couldn't create .clip file", "File Error", MB_OK);
		return;
	}

	fwrite("CLIP", 4, 1, stream);

	BeginBlock("head", stream);

	clipHeader_t clipheader;

	clipheader.version = 1;
	clipheader.num_bones = model.num_bones;
	clipheader.num_frames = numframes;
	
	WriteBlockData(&clipheader, sizeof(clipheader), stream);

	EndBlock(stream);

	for (n=0; n<model.num_bones; n++)
	{		
		//write keys
		WriteFloatKeys(n, &modelClip.motions[n].keys_x, MKEY_X, stream);
		WriteFloatKeys(n, &modelClip.motions[n].keys_y, MKEY_Y, stream);
		WriteFloatKeys(n, &modelClip.motions[n].keys_z, MKEY_Z, stream);
		WriteFloatKeys(n, &modelClip.motions[n].keys_pitch, MKEY_PITCH, stream);
		WriteFloatKeys(n, &modelClip.motions[n].keys_roll, MKEY_ROLL, stream);
		WriteFloatKeys(n, &modelClip.motions[n].keys_yaw, MKEY_YAW, stream);		
		WriteFloatKeys(n, &modelClip.motions[n].keys_scale, MKEY_SCALE, stream);
		WriteByteKeys(n, &modelClip.motions[n].keys_visibility, MKEY_VISIBILITY, stream);
	}

	fclose(stream);	

	MessageBox(hPanel, fmt("Successfully wrote clip file to %s", filename), "Clip write complete", MB_OK);

#endif
}


BOOL AllocateFloatKeys(floatKeys_t *keys, int numframes)
{
	keys->keys = (float *)Tag_Malloc(sizeof(float) * numframes, MEM_SCENE);
	keys->num_keys = 0;

	return TRUE;
}

BOOL AllocateByteKeys(byteKeys_t *keys, int numframes)
{
	keys->keys = (byte *)Tag_Malloc(numframes, MEM_SCENE);
	keys->num_keys = 0;

	return TRUE;
}

void	CompressFloatKeys(floatKeys_t *keys)
{
	for (int n=1; n<keys->num_keys; n++)
	{
		if (ABS(keys->keys[n] - keys->keys[0]) > 0.0001)
			return;
	}

	//all keys are the same, so only store one key
	keys->num_keys = 1;
}

void	CompressByteKeys(byteKeys_t *keys)
{
	for (int n=1; n<keys->num_keys; n++)
	{
		if (keys->keys[n] != keys->keys[0])
			return;
	}

	//all keys are the same, so only store one key
	keys->num_keys = 1;
}


BOOL ModelExporterR4::CopyKeys(INode *node, bone_t *s2bone, boneMotion_t *motion, int frame)
{	
	float eulers[3];
	Point3 axis[3];
	Point3 pos;
	Matrix3 tm;
	INode *parent;
	int baseframe = frame - GetStartFrame();

	//store the transform info for this frame
	tm = GetBoneTM(node, FRAME(frame));	
	parent = GetS2BoneParentNode(node);

	if (parent)
	{
		//store the transform relative to the parent bone
		Matrix3 invParentTM;
		invParentTM = GetBoneTM(parent, FRAME(frame));
		invParentTM.Invert();
		tm = tm * invParentTM;
	}

	motion->keys_x.keys[baseframe] = tm.GetTrans().x;
	motion->keys_x.num_keys++;
	motion->keys_y.keys[baseframe] = tm.GetTrans().y;
	motion->keys_y.num_keys++;
	motion->keys_z.keys[baseframe] = tm.GetTrans().z;
	motion->keys_z.num_keys++;

	MatrixToEuler(tm, eulers, EULERTYPE_YXZ);		//YXZ is the euler order used in M_GetAxis()

	motion->keys_roll.keys[baseframe] = RAD2DEG(eulers[0]);
	motion->keys_roll.num_keys++;
	motion->keys_pitch.keys[baseframe] = RAD2DEG(eulers[1]);
	motion->keys_pitch.num_keys++;
	motion->keys_yaw.keys[baseframe] = RAD2DEG(eulers[2]);
	motion->keys_yaw.num_keys++;

	motion->keys_scale.keys[baseframe] = 1;		//we don't support scale yet
	motion->keys_scale.num_keys++;

	motion->keys_visibility.keys[baseframe] = node->GetVisibility(FRAME(frame)) * 255;
	motion->keys_visibility.num_keys++;	
	
	return TRUE;
}


//store animation data
void ModelExporterR4::StoreBoneMotions()
{
	if (!export_clip)
		return;

	int i;
	int numframes = GetNumFrames();

	for (i=0; i<model.num_bones; i++)
	{
		boneMotion_t *motion = &modelClip.motions[i];
		//make sure we have enough memory allocated to store all keys

		AllocateFloatKeys(&motion->keys_x, numframes);
		AllocateFloatKeys(&motion->keys_y, numframes);
		AllocateFloatKeys(&motion->keys_z, numframes);
		AllocateFloatKeys(&motion->keys_pitch, numframes);
		AllocateFloatKeys(&motion->keys_roll, numframes);
		AllocateFloatKeys(&motion->keys_yaw, numframes);
		AllocateFloatKeys(&motion->keys_scale, numframes);
		AllocateByteKeys(&motion->keys_visibility, numframes);
	}	

	for (int frame=GetStartFrame(); frame<=GetEndFrame(); frame++)
	{
		for (i=0;i<model.num_bones;i++)
		{						
			modelClip.motions[i].bone = i;
			
			CopyKeys(boneNodes[i], &model.bones[i], &modelClip.motions[i], frame);
		}
		//SecondaryStatusProgress(((float)(frame - GetStartFrame()) / GetNumFrames()) * 100.0);
	}

	//do a compare on all keys to see which ones we can compress down

	for (i=0; i<model.num_bones; i++)
	{
		boneMotion_t *motion = &modelClip.motions[i];

		CompressFloatKeys(&motion->keys_x);
		CompressFloatKeys(&motion->keys_y);
		CompressFloatKeys(&motion->keys_z);
		CompressFloatKeys(&motion->keys_pitch);
		CompressFloatKeys(&motion->keys_roll);
		CompressFloatKeys(&motion->keys_yaw);
		CompressFloatKeys(&motion->keys_scale);
		CompressByteKeys(&motion->keys_visibility);
	}
}

void ModelExporterR4::FinishModel()
{
	int n,i;
	int totalverts = 0;
	int totalfaces = 0;

	

	if (mode == MODE_WRITE_CLIP)
		return;

	AddMessage("******************************************");
	AddMessage("**********     MODEL SUMMARY    **********");
	AddMessage("******************************************");
	Indent();

	for (n=0; n<model.num_meshes; n++)
	{
		totalverts += model.meshes[n].num_verts;
		totalfaces += model.meshes[n].num_faces;
		
		for (i=n+1; i<model.num_meshes; i++)
		{
			if (strcmp(model.meshes[n].name, model.meshes[i].name)==0)
			{
				Error(fmt("Duplicate mesh name: \"%s\"", model.meshes[i].name));
			}
		}
	}

	for (n=0; n<model.num_bones; n++)
	{
		for (i=n+1; i<model.num_bones; i++)
		{
			if (strcmp(model.bones[n].name, model.bones[i].name)==0)
			{
				Error(fmt("Duplicate bone name: \"%s\"", model.bones[i].name));
			}
		}		
	}

	AddMessage("");
	AddMessage("");
	AddMessage(fmt("%i vertices", totalverts));
	AddMessage(fmt("%i faces", totalfaces));
	AddMessage("");
	AddMessage(fmt("%i meshes", model.num_meshes));
	AddMessage(fmt("%i bones", model.num_bones));
	AddMessage(fmt("%i sprites", model.num_sprites));
	AddMessage(fmt("%i collision surfaces", model.num_surfs));
	AddMessage(fmt("Bounding box: (%f, %f, %f) to (%f, %f, %f)", model.bmin[0], model.bmin[1], model.bmin[2], model.bmax[0], model.bmax[1], model.bmax[2]));
	AddMessage("");
	AddMessage("Mesh breakdown:");
	AddMessage(fmt("%i non-blended / rigid meshes", CountMeshType(MESH_SKINNED_NONBLENDED)));
	AddMessage(fmt("%i blended meshes", CountMeshType(MESH_SKINNED_BLENDED)));	
	
	if (model_error)
	{
		if (mode == MODE_WRITE_FILE)
		{
			MessageBox(hPanel, "There were one or more errors with this model.  Check error log for details.  The model will not be written to disk until the errors have been resolved.", "Error(s) with model", MB_OK);
		}
		else
		{
			MessageBox(hPanel, "There were one or more errors with this model.  Check error log for details.", "Error(s) with model", MB_OK);
		}
	}
}

void ModelExporterR4::Cleanup()
{
	Tag_FreeAll(MEM_SCENE);		//free any memory allocated during scene traversal
}

void ModelExporterR4::SceneInfo()
{
	//print out some information about the scene
	mode = MODE_SCENE_INFO;

	ClearSceneInfo();
	AddBaseModelData();
	
	OpenStatusBar();

	TraverseNode(ip->GetRootNode());
	
	FinishModel();

	CloseStatusBar();

	if (model_error)
		ShowErrorInfo();
	ShowSceneInfo();

	Cleanup();
}

void ModelExporterR4::ShowSceneInfo()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_SCENE_INFO),
		hPanel,
		SceneInfoDlgProc,(LPARAM)this);
}

void ModelExporterR4::ShowErrorInfo()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_MODEL_ERRORS),
		hPanel,
		ErrorInfoDlgProc,(LPARAM)this);
}

void ModelExporterR4::Export()
{
	mode = MODE_WRITE_FILE;

	ClearSceneInfo();

	//get the export options
	if (!GetExportOptions())
		return;

	AddBaseModelData();

	OpenStatusBar();

	TraverseNode(ip->GetRootNode());

	StoreBoneMotions();

	FinishModel();

	CloseStatusBar();

	if (!model_error)
	{
		//get the filename
		if (!GetFilename("Silverback Model Files (*.model)", "*.model"))
			return;

		WriteModelToDisk();

		if (export_clip)
		{
			if (GetFilename("Silverback Animation Files (*.clip)", "*.clip"))
			{
				WriteClipFile();
			}
		}

		if (model_error)
			ShowErrorInfo();

		ShowSceneInfo();
	}
	else
	{
		ShowErrorInfo();
	}

	Cleanup();
}

void ModelExporterR4::ExportAnimationOnly()
{
	mode = MODE_WRITE_CLIP;
	
	ClearSceneInfo();
	export_clip = TRUE;

	if (!GetFilename("Silverback Animation Files (*.clip)", "*.clip"))
		return;

	AddBaseModelData();

	OpenStatusBar();

	TraverseNode(ip->GetRootNode());

	StoreBoneMotions();

	FinishModel();
	
	CloseStatusBar();

	if (!model_error)
	{		
		WriteClipFile();	
	}
	else
	{
		ShowErrorInfo();
	}

	Cleanup();
}

static BOOL CALLBACK HelpDlgProc(
		HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
			switch LOWORD(wParam)
			{
				case IDOK:
					EndDialog(hwndDlg, TRUE);
					break;
	        	default:
					break;
			}
		default:
			return FALSE;
	}

	return TRUE;
}

void ModelExporterR4::Help()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_HELP),
		hPanel,
		HelpDlgProc,(LPARAM)this);
}

BOOL ModelExporterR4::GetFilename(char *fileTypeDesc, char *wildcard)
{
	OPENFILENAME ofn;
	FilterList fl;

	fl.Append(fileTypeDesc);
	fl.Append(wildcard);

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hPanel;
	ofn.hInstance = (HINSTANCE)GetWindowLongPtr(ofn.hwndOwner, GWLP_HINSTANCE);
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = fl;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}

void	ModelExporterR4::BeginBlock(const char *blockName, FILE *stream)
{
	int templength = 0;

	fwrite(blockName, 4, 1, stream);
	//we'll fill in the length here on the EndBlock call
	seekback = ftell(stream);
	fwrite(&templength, 4, 1, stream);
}

void	ModelExporterR4::EndBlock(FILE *stream)
{
	fpos_t curpos;
	int length;
	
	curpos = ftell(stream);
	length = curpos - seekback - 4;
	fseek(stream, seekback, SEEK_SET);
	fwrite(&length, 4, 1, stream);
	fseek(stream, curpos, SEEK_SET);
}

void	ModelExporterR4::WriteBlockData(void *data, int size, FILE *stream)
{
	fwrite(data, size, 1, stream);
}

void	ModelExporterR4::CopyTextures()
{
	int n;

	for (n=0; n<num_texrefs; n++)
	{
		char *name = texrefs[n].bmptex->GetMapName();

		if (CopyFile(name, fmt("%s%s", Filename_GetDir(filename), model.meshes[texrefs[n].meshidx].defaultShader), FALSE))
		{
			AddMessage(fmt("Copied texture %s to %s", name, Filename_GetDir(filename)));
		}
		else
		{
			AddMessage(fmt("WARNING: Unable to copy texture %s to %s", name, Filename_GetDir(filename)));
		}
	}
}

BOOL	ModelExporterR4::WriteModelToDisk()
{
	FILE *stream;
	int version = 2;
	int n, f, v;
	int i;
	
	stream = fopen(filename, "wb");
	if (!stream)
	{
		Error(fmt("Failed to open %s for writing", filename));
		return FALSE;
	}

	//write identifier
	fwrite("SMDL", 4, 1, stream);

	//HEADER BLOCK
	
	BeginBlock("head", stream);

	modelHeader_t header;

	header.version = 2;
	header.num_meshes = model.num_meshes;
	header.num_sprites = model.num_sprites;
	header.num_surfs = model.num_surfs;
	header.num_bones = model.num_bones;
	M_CopyVec3(model.bmin, header.bmin);
	M_CopyVec3(model.bmax, header.bmax);

	WriteBlockData(&header, sizeof(header), stream);
	
	EndBlock(stream);


	BeginBlock("bone", stream);

	for (n=0; n<model.num_bones; n++)
	{
		boneBlock_t boneblock;

		boneblock.parentIndex = model.bones[n].parentIndex;
		strncpySafe(boneblock.name, model.bones[n].name, BONE_NAME_LENGTH);
		memcpy(&boneblock.invBase, &model.bones[n].invBase, sizeof(transform_t));
		memcpy(&boneblock.base, &model.bones[n].base, sizeof(transform_t));
		
		WriteBlockData(&boneblock, sizeof(boneblock), stream);
	}

	EndBlock(stream);

	//write all the mesh data
	for (n=0; n<model.num_meshes; n++)
	{
		mesh_t *mesh = &model.meshes[n];

		BeginBlock("mesh", stream);
		
		meshBlock_t meshblock;

		meshblock.mesh_index	= n;
		strncpySafe(meshblock.name, mesh->name, MESH_NAME_LENGTH);
		strncpySafe(meshblock.defaultShader, mesh->defaultShader, SKIN_SHADERNAME_LENGTH);
		meshblock.mode			= mesh->mode;
		meshblock.num_verts		= mesh->num_verts;
		M_CopyVec3(mesh->bmin, meshblock.bmin);
		M_CopyVec3(mesh->bmax, meshblock.bmax);
		meshblock.bonelink =	mesh->bonelink;

		WriteBlockData(&meshblock, sizeof(meshblock), stream);

		EndBlock(stream);

		/**************
		    vertex data
	 	 **************/

		BeginBlock("vrts", stream);

		vertexBlock_t vertblock;

		vertblock.mesh_index = n;

		WriteBlockData(&vertblock, sizeof(vertblock), stream);
		for (v=0; v<mesh->num_verts; v++)
		{
			WriteBlockData(mesh->verts[v], sizeof(vec3_t), stream);		
		}

		EndBlock(stream);

		if (mesh->mode == MESH_SKINNED_BLENDED)
		{
			BeginBlock("lnk1", stream);

			blendedLinksBlock_t blendblock;

			blendblock.mesh_index = n;
			blendblock.num_verts = mesh->num_verts;

			WriteBlockData(&blendblock, sizeof(blendblock), stream);

			for (v=0; v<mesh->num_verts; v++)
			{								
				fwrite(&mesh->blendedLinks[v].num_weights, 4, 1, stream);				
				fwrite(mesh->blendedLinks[v].weights, 4 * mesh->blendedLinks[v].num_weights, 1, stream);
				fwrite(mesh->blendedLinks[v].indexes, 4 * mesh->blendedLinks[v].num_weights, 1, stream);				
			}

			EndBlock(stream);
		}
		else
		{
			if (mesh->bonelink == -1)
			{
				BeginBlock("lnk2", stream);

				singleLinksBlock_t singleblock;

				singleblock.mesh_index = n;
				singleblock.num_verts = mesh->num_verts;
				
				WriteBlockData(&singleblock, sizeof(singleblock), stream);

				for (v=0; v<mesh->num_verts; v++)
				{
					int bone = mesh->singleLinks[v];
					fwrite(&mesh->singleLinks[v], 4, 1, stream);
					fwrite(&bone, 4, 1, stream);
				}

				EndBlock(stream);
			}
		}

		/**************
		    face list
		 **************/

		BeginBlock("face", stream);

		faceBlock_t faceblock;

		faceblock.mesh_index = n;
		faceblock.num_faces = mesh->num_faces;

		WriteBlockData(&faceblock, sizeof(faceblock), stream);
		
		for (f=0; f<mesh->num_faces; f++)
		{
			fwrite(mesh->facelist[f], sizeof(uivec3_t), 1, stream);
		}

		EndBlock(stream);

		/**************
		    texture coords
		 **************/

		BeginBlock("texc", stream);

		textureCoordsBlock_t texblock;

		texblock.mesh_index = n;

		WriteBlockData(&texblock, sizeof(texblock), stream);

		for (f=0; f<mesh->num_verts; f++)
		{
			fwrite(mesh->tverts[f], sizeof(vec2_t), 1, stream);
		}

		EndBlock(stream);

		/******************
		    vertex colors
		 ******************/

		BeginBlock("colr", stream);

		colorBlock_t colblock;

		colblock.mesh_index = n;

		WriteBlockData(&colblock, sizeof(colblock), stream);
				

		for (f=0; f<mesh->num_verts; f++)
		{
			fwrite(mesh->colors[f], sizeof(bvec4_t), 1, stream);
		}

		EndBlock(stream);

		/**************
		    normals
		 **************/

		BeginBlock("nrml", stream);

		normalBlock_t nrmlblock;

		nrmlblock.mesh_index = n;

		WriteBlockData(&nrmlblock, sizeof(nrmlblock), stream);

		for (f=0; f<mesh->num_verts; f++)
		{
			fwrite(mesh->normals[f], sizeof(vec3_t), 1, stream);
		}

		EndBlock(stream);
	}

	//write all the surface data
	for (n=0; n<model.num_surfs; n++)
	{
		convexPolyhedron_t *surf = &model.surfs[n];

		BeginBlock("surf", stream);

		surfBlock_t surfblock;

		surfblock.surf_index = n;
		M_CopyVec3(surf->bmin, surfblock.bmin);
		M_CopyVec3(surf->bmax, surfblock.bmax);
		surfblock.flags = surf->flags;
		surfblock.num_planes = surf->numPlanes;
		
		WriteBlockData(&surfblock, sizeof(surfblock), stream);

		//write planes
		for (int p=0; p<surf->numPlanes; p++)
		{
			fwrite(&surf->planes[p], sizeof(plane_t), 1, stream);
		}

		EndBlock(stream);
	}

	fclose(stream);

	CopyTextures();

	MessageBox(hPanel, (fmt("Model written successfully to %s", filename)), "Model write complete", MB_OK);

	return TRUE;
}