int     HTTP_Init();
void    HTTP_Shutdown();
int    	HTTP_GetText(char *addr, char *buf, int buf_size);
//int    	HTTP_GetData(char *url, char **buf, int *buf_size);

int    	HTTP_GetFile(char *url, char *filename);

file_t  *HTTP_OpenFile(char *url);
file_t *HTTP_OpenFileNonBlocking(char *url);
int 	HTTP_GetFileNonBlocking(char *url, char *filename);
float   HTTP_GetProgress(char *url, int *size, int *transferred, int *time);

void	HTTP_Frame();
void	HTTP_CancelAllRequests();