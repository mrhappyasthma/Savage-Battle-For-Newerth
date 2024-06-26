// (C) 2001 S2 Games

// gui_track.h

// track control


#define TRACK_CLASS	"trackcontrol"

typedef struct
{
	gui_element_t			*parent;

	float					laxness; // the amount of "give" before deciding the current 
									 // controller value is on a keyframe (and setting it to be exactly on it)

	int						selection;
	residx_t				keyShader;
	residx_t				keySelectedShader;
	bool					dragging;
	ivec2_t					mousedown_pos;

	ivec2_t					icon_size;

	gui_element_t			*element;
} gui_track_t;

void	GUI_Track_Init();
