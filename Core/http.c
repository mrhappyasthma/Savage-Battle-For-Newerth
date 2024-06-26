/*
 * http.c (C) 2003 S2 Games
 *
 * Library to handle http data transfers
 */

//=============================================================================

#include "core.h"
#include <curl/curl.h>

//=============================================================================

#define MAX_HTTP_REQUESTS 20

typedef enum
{
	HTTP_REQUESTED,
	HTTP_FAILED,
	HTTP_IN_PROGRESS,
	HTTP_SUCCESS
}
httpResult_e;

typedef struct
{
	char *buf;
	int max_size;
	int size;
}
textBuffer_t;

typedef struct
{
	bool			active;
	httpResult_e	result;
	CURL			*handle;
	char			*url;
	textBuffer_t	buf;
	char			*filename;
	float			progress;
	int				size;
	int				bytesTransferred;
	int				startTime;
	bool			bufIsFileHandle;
}
http_request_t;

//=============================================================================

//when the proxy address is actually passed into libcurl, it is important that
//the string not be altered, so it will be coppied to proxystr each time it is
//set, which is the address that curl_easy_setopt() will receive
cvar_t	http_proxy = { "http_proxy", "" };
char	*proxystr = NULL;

CURLM *multi;

http_request_t current_requests[MAX_HTTP_REQUESTS];

int num_current_requests = 0;

//=============================================================================


/*==========================

  WriteFileCallback

 ==========================*/

size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written;
	file_t *f = (file_t *)stream;
	written = f->write(ptr, size, nmemb, f);
	return written;
}


/*==========================

  WriteMemoryCallback

 ==========================*/

size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	register int realsize = size * nmemb;
	int oursize;
	textBuffer_t *buf = (textBuffer_t *)data;
  
	oursize = realsize;
	if (realsize > buf->max_size - buf->size)
	{
		Console_DPrintf("Warning!  Truncating http packet (%i bytes) to fit into buffer (%i bytes)\n", realsize, buf->max_size - buf->size);
		oursize = buf->max_size - buf->size;
	}

	if (buf->buf) 
	{
		memcpy(&(buf->buf[buf->size]), ptr, oursize);
		buf->size += oursize;
	}
	return realsize;
}


/*==========================

  WriteMemoryCallback_Alloc

 ==========================*/

size_t WriteMemoryCallback_Alloc(void *ptr, size_t size, size_t nmemb, void *data)
{
	register int realsize = size * nmemb;
	int oursize;
	textBuffer_t *buf = (textBuffer_t *)data;

	oursize = realsize;
	if (realsize > buf->max_size - buf->size)
	{
		oursize = realsize - (buf->max_size - buf->size);
		buf->buf = Tag_Realloc(buf->buf, buf->max_size + oursize, MEM_HTTP);
		buf->max_size += oursize;
		Console_DPrintf("reallocing buffer to %i bytes (ptr %p)\n", buf->max_size, buf->buf);
	}

	if (buf->buf) 
	{
		memcpy(&(buf->buf[buf->size]), ptr, realsize);
		buf->size += realsize;
	}
	return realsize;
}


/*==========================

  HTTP_SetOptions

  Sets general options for curl

 ==========================*/

void	HTTP_SetOptions(CURL *curl)
{
	if (!curl)
		return;

	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);

	curl_easy_setopt(curl, CURLOPT_MUTE, true);

	if (http_proxy.string[0])
	{
		proxystr = Tag_Strdup(http_proxy.string, MEM_HTTP);
		curl_easy_setopt(curl, CURLOPT_PROXY, proxystr);
	}
	
	return;
}


/*==========================

  HTTP_GetText

 ==========================*/

int		HTTP_GetText(char *addr, char *textbuf, int buf_size)
{
	CURLcode res;
	textBuffer_t buf;
	CURL *curl = NULL;
	float start = System_Milliseconds();

	buf.buf = textbuf;
	buf.max_size = buf_size;
	buf.size = 0;

	curl = curl_easy_init();
	if (!curl)
	{
		Console_Errorf("Curl initialization failed!\n");
		return -1;
	}
	
	HTTP_SetOptions(curl);
	
	Console_DPrintf("Issuing GET request on `%s\'\n", addr);
	curl_easy_setopt(curl, CURLOPT_URL, addr);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)&buf);

	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	Console_DPrintf("HTTP Request took %fms\n", System_Milliseconds() - start);
	
	return res;
}


/*==========================

  HTTP_GetFile

 ==========================*/

int		HTTP_GetFile(char *url, char *filename)
{
	file_t *file;
	CURL *curl;
	CURLcode res;
	float start = System_Milliseconds();

	curl = curl_easy_init();
	if (!curl)
	{
		Console_Errorf("Error!  curl has not been initialized yet!\n");
		return -1;
	}
	
	file = File_Open(filename, "wb");
	if (!file)
	{
		Console_DPrintf("ERROR!  Couldn't open %f for writing\n", filename);
		return -1;
	}

	HTTP_SetOptions(curl);
	
	curl_easy_setopt(curl, CURLOPT_URL, url);

	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	//send all data to this function 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)file);

	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	File_Close(file);

	Console_DPrintf("HTTP Request took %fms\n", System_Milliseconds() - start);

	return res;
}


/*==========================

  HTTP_OpenFile

 ==========================*/

file_t	*HTTP_OpenFile(char *url)
{
	file_t *file = NULL;
	CURLcode res;
	textBuffer_t buf;
	CURL *curl = NULL;
	float start = System_Milliseconds();

	file = File_NewBufferFile();
	File_FileTrackingRename(file, url);

	buf.buf = NULL;
	buf.max_size = 0;
	buf.size = 0;

	curl = curl_easy_init();
	if (!curl)
	{
		Console_Errorf("Error!  curl has not been initialized yet!\n");
		return NULL;
	}
	
	HTTP_SetOptions(curl);
	
	
	Console_DPrintf("Issuing GET request on `%s\'\n", url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback_Alloc);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)&buf);

	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

	Console_DPrintf("getting http file %s\n", url);
	res = curl_easy_perform(curl);
	Console_DPrintf("done getting http file %s\n", url);

	curl_easy_cleanup(curl);

	File_Buffered_Import(file, buf.buf, buf.size);
	Console_DPrintf("HTTP Request took %fms\n", System_Milliseconds() - start);

	return file;
}


/*==========================

  HTTP_FileProgress

 ==========================*/

int		HTTP_FileProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int i = (int)clientp;

	if (i < 0 || i > MAX_HTTP_REQUESTS || !current_requests[i].active)
		return 0;

	if (dltotal > 0)
	{
		current_requests[i].progress = dlnow / dltotal;
		current_requests[i].size = dltotal;
		current_requests[i].bytesTransferred = dlnow;
	}
	else if (ultotal > 0)
	{
		current_requests[i].progress = ulnow / ultotal;
		current_requests[i].size = ultotal;
		current_requests[i].bytesTransferred = ulnow;
	}
	return 0;
}


/*==========================

  HTTP_Frame

 ==========================*/

void	HTTP_Frame()
{
	CURLMcode res;
	CURLMsg *msg;
	int handles, i, queueSize;
	
	if (!num_current_requests)
		return;
	
	res = curl_multi_perform(multi, &handles);
	while (res == CURLM_CALL_MULTI_PERFORM)
		res = curl_multi_perform(multi, &handles);

	while ((msg = curl_multi_info_read(multi, &queueSize)))
	{
		for (i = 0; i < MAX_HTTP_REQUESTS; i++)
		{
			if (!current_requests[i].active)
				continue;

			if (current_requests[i].handle == msg->easy_handle)
			{
				if (msg->msg == CURLMSG_DONE)
				{
					if (msg->data.result == CURLE_OK)
					{
						Console_DPrintf("finished loading handle (%i) %s\n", i, current_requests[i].url);
						current_requests[i].result = HTTP_SUCCESS;
					}
					else
					{
						switch (msg->data.result)
						{	
							case CURLE_COULDNT_RESOLVE_HOST:
								Console_Printf("HTTP error: Couldn't resolve the hostname for url: %s\n", current_requests[i].url);
								break;

							case CURLE_COULDNT_CONNECT:
								Console_Printf("HTTP error: Couldn't connect to url: %s\n", current_requests[i].url);
								break;

							default:
								Console_Printf("Error trying to get %s - error %i\n", current_requests[i].url, msg->data.result);
						}	
						current_requests[i].result = HTTP_FAILED;
					}

					curl_multi_remove_handle(multi, current_requests[i].handle);
				}
			}
			else if (current_requests[i].result == HTTP_REQUESTED)
			{
				Console_DPrintf("still loading handle %i (%s)\n", i, current_requests[i].url);
				current_requests[i].result = HTTP_IN_PROGRESS;
			}
		}
		if (i >= MAX_HTTP_REQUESTS)
		{
			Console_DPrintf("HTTP Warning: Couldn't find handle matching message (handle is %p)!\n", msg->easy_handle);
		}
	}
}


/*==========================

  HTTP_CancelAllRequests

 ==========================*/

void	HTTP_CancelAllRequests()
{
	int i;

	if (!num_current_requests)
		return;
	
	for (i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		char url[1024];
		char filename[1024];
		if (!current_requests[i].active)
			continue;

		current_requests[i].result = HTTP_FAILED;

		curl_multi_remove_handle(multi, current_requests[i].handle);

		strcpy(url, current_requests[i].url);

		//call the appropriate function to do the final cleanup
		//this will call curl_easy_cleanup, free mem, etc
		if (current_requests[i].bufIsFileHandle)
		{
			strcpy(filename, current_requests[i].filename);

			HTTP_GetFileNonBlocking(url, filename);
		}
		else
		{
			file_t *file = HTTP_OpenFileNonBlocking(url);
			if (file)
				File_Close(file);
		}
	}

	memset(&current_requests, 0, sizeof(current_requests));
	num_current_requests = 0;
}


/*==========================

  HTTP_OpenFileNonBlocking

 ==========================*/

file_t *HTTP_OpenFileNonBlocking(char *url)
{
	file_t *file = NULL;
	CURLcode res;
	CURL *curl = NULL;
	int i;

	for (i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		if (current_requests[i].active && strcmp(current_requests[i].url, url) == 0)
		{
			if (current_requests[i].result == HTTP_SUCCESS)
			{
				file = File_NewBufferFile();
				File_FileTrackingRename(file, url);
			}
			else if (current_requests[i].result == HTTP_FAILED)
			{
				//rather than return NULL, which might make them think they need to keep asking for this file
				//give them an empty file
				Console_DPrintf("HTTP_OpenFileNonBlocking(%s) failed\n", url);
				file = File_NewBufferFile();
				File_FileTrackingRename(file, url);
			}
			else //still getting it
			{
				return NULL; //not here yet
			}
			
			Console_DPrintf("wrapping up request for handle %i (%s)\n", i, current_requests[i].url);
			
			current_requests[i].active = false;

			Tag_Free(current_requests[i].url);
			File_Buffered_Import(file, current_requests[i].buf.buf, current_requests[i].buf.size);
			
			current_requests[i].buf.buf = NULL;
			current_requests[i].buf.size = 0;

			curl_easy_cleanup(current_requests[i].handle);

			num_current_requests--;
			return file;
		}
	}

	//this is a new request - find a free slot
	for (i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		if (!current_requests[i].active)
			break;
	}
	if (i >= MAX_HTTP_REQUESTS)
		return NULL; //they should re-request it, maybe we'll have a free slot then

	Console_DPrintf("This is a new request, putting it in slot %i\n", i);
	
	current_requests[i].buf.buf = NULL;
	current_requests[i].buf.max_size = 0;
	current_requests[i].buf.size = 0;

	curl = curl_easy_init();
	if (!curl)
	{
		Console_Errorf("Error!  curl has not been initialized yet!\n");
		return NULL;
	}
	
	current_requests[i].result = HTTP_REQUESTED;
	current_requests[i].url = Tag_Strdup(url, MEM_HTTP);
	current_requests[i].active = true;
	current_requests[i].handle = curl;
	current_requests[i].progress = 0;
	current_requests[i].size = 0;
	current_requests[i].bytesTransferred = 0;
	current_requests[i].startTime = System_Milliseconds();
	current_requests[i].bufIsFileHandle = false;

	HTTP_SetOptions(curl);
	
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, i);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, HTTP_FileProgress);

	Console_DPrintf("Issuing GET request on `%s\'\n", url);
	curl_easy_setopt(curl, CURLOPT_URL, current_requests[i].url);
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback_Alloc);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)&current_requests[i].buf);

	Console_DPrintf("getting http file %s\n", url);
	res = curl_multi_add_handle(multi, curl);
	num_current_requests++;

	return NULL;
}


/*==========================

  HTTP_GetFileNonBlocking

  ==========================*/

int	HTTP_GetFileNonBlocking(char *url, char *filename)
{
	CURLcode res;
	CURL *curl = NULL;
	file_t *file;
	int i, result;

	for (i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		if (current_requests[i].active && strcmp(current_requests[i].url, url) == 0)
		{
			if (current_requests[i].result == HTTP_SUCCESS)
			{
				result = 1;
			}
			else if (current_requests[i].result == HTTP_FAILED)
			{
				result = 0;
			}
			else //still getting it
			{
				return -1; //not here yet
			}
			
			Console_DPrintf("wrapping up request for handle %i (%s)\n", i, current_requests[i].url);
			
			current_requests[i].active = false;

			file = (file_t *)current_requests[i].buf.buf;

			File_Close(file);
				
			if (current_requests[i].result != HTTP_SUCCESS)
			{
				//delete it from the file system
				File_Delete(current_requests[i].filename);
			}			

			Tag_Free(current_requests[i].url);
			/*
			else
			{
				//don't allow 0 byte files
				if (File_Size(current_requests[i].filename) == 0)
				{
					File_Delete(current_requests[i].filename);
					result = 0;
				}
			}*/
	
			Tag_Free(current_requests[i].filename);

			current_requests[i].buf.buf = NULL;
			current_requests[i].buf.size = 0;

			curl_easy_cleanup(current_requests[i].handle);

			num_current_requests--;
			return result;
		}
	}

	//this is a new request - find a free slot
	for (i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		if (!current_requests[i].active)
			break;
	}
	if (i >= MAX_HTTP_REQUESTS)
		return -1; //they should re-request it, maybe we'll have a free slot then

	Console_DPrintf("This is a new request, putting it in slot %i\n", i);
	
	file = File_Open(filename, "wb");
	if (!file)
	{
		Console_DPrintf("ERROR!  Couldn't open %f for writing\n", filename);
		return -1;
	}
	
	current_requests[i].buf.buf = (char *)file;
	current_requests[i].buf.max_size = 0;
	current_requests[i].buf.size = 0;

	curl = curl_easy_init();
	if (!curl)
	{
		Console_Errorf("Error!  curl has not been initialized yet!\n");
		return -1;
	}
	
	HTTP_SetOptions(curl);
	
	current_requests[i].result = HTTP_REQUESTED;
	current_requests[i].url = Tag_Strdup(url, MEM_HTTP);
	current_requests[i].filename = Tag_Strdup(filename, MEM_HTTP);
	current_requests[i].active = true;
	current_requests[i].handle = curl;
	current_requests[i].progress = 0;
	current_requests[i].size = 0;
	current_requests[i].bytesTransferred = 0;
	current_requests[i].startTime = System_Milliseconds();
	current_requests[i].bufIsFileHandle = true;

	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, i);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, HTTP_FileProgress);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

	Console_DPrintf("Issuing GET request on `%s\'\n", current_requests[i].url);
	curl_easy_setopt(curl, CURLOPT_URL, current_requests[i].url);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
	
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)file);

	Console_DPrintf("getting http file %s\n", url);
	res = curl_multi_add_handle(multi, curl);
	num_current_requests++;

	return -1;
}


/*==========================

  HTTP_GetProgress

  ==========================*/

float   HTTP_GetProgress(char *url, int *size, int *transferred, int *time)
{
	int i;

	for (i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		if (current_requests[i].active && strcmp(current_requests[i].url, url) == 0)
		{
			if (size)
				*size = current_requests[i].size;
			if (transferred)
				*transferred = current_requests[i].bytesTransferred;
			if (time)
				*time = System_Milliseconds() - current_requests[i].startTime;
			return current_requests[i].progress;
		}
	}
	return 1; //let's just tell them we're done
}


/*==========================

  HTTP_Get_Cmd

  Command to manually grab a url

 ==========================*/

void	HTTP_Get_Cmd(int argc, char *argv[])
{
	char buf[1];
	if (!argc)
	{
		Console_Printf("usage: httpget <url>\n");
		return;
	}
	
	HTTP_GetText(argv[0], buf, 0);
}


/*==========================

  HTTP_Reset_Cmd

  Flush out and reload the curl multi handle

 ==========================*/

void	HTTP_Reset_Cmd(int arc, char *argv[])
{
	HTTP_CancelAllRequests();

	curl_multi_cleanup(multi);
	curl_global_cleanup();

	multi = curl_multi_init();
}


/*==========================

  HTTP_Init

 ==========================*/

int 	HTTP_Init()
{
	Cmd_Register("httpget",		HTTP_Get_Cmd);
	Cmd_Register("httpreset",	HTTP_Reset_Cmd);
	Cvar_Register(&http_proxy);

	memset(&current_requests, 0, sizeof(current_requests));

	multi = curl_multi_init();
	return 0;
}


/*==========================

  HTTP_Shutdown

 ==========================*/

void	HTTP_Shutdown()
{
	Tag_Free(proxystr);

	curl_multi_cleanup(multi);
	curl_global_cleanup();
}

