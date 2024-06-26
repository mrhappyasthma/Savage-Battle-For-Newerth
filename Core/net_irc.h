
bool    IRC_Init();
void	IRC_Shutdown();

bool    IRC_RegisterNick(char *nick);
void	IRC_Frame();
bool    IRC_Connect(char *servername);
bool    IRC_Disconnect(char *message);
bool	IRC_Join(char *channel);
bool	IRC_Say(char *msg);
bool	IRC_Msg(char *user, char *msg, bool printmsg);

void	IRC_OutputCallback(void (*outputcallback)(char *string));
void    IRC_OutputMsgCallback(void (*outputcallback)(char *user, char *string, bool incoming));
char 	**IRC_GetUserList();
char 	**IRC_GetBuddyList();
