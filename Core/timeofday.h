#define MINUTESPERDAY (60*24)

void    TOD_Load(archive_t *archive, const char *worldname);
void    TOD_SetTime(float time);
int     TOD_GetNumKeyframes();
int     TOD_GetKeyframeTime(int keyframe);
int     TOD_MoveKeyframe(int keyframe, int newtime);
void    TOD_Init();

void    TOD_SaveToZip(void *zipfile, char *name);
