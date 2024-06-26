// (C) 2003 S2 Games

// keyboard.h

// Functions for handling keyboard input


void		Key_SendEvent(int key, bool down);
void		Key_Init();
void		Key_BindCmd(int key, char *cmd);
