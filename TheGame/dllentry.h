
int 	InitGameDLL(coreAPI_shared_t *core_api_shared);
void 	ShutdownGameDLL();
void	SV_InitAPIs(coreAPI_server_t *core_api, serverAPI_t *server_api);
void	INT_InitAPIs(coreAPI_interface_t *core_int_api, interfaceAPI_t *game_interface_api);
void	CL_InitAPIs(coreAPI_client_t *core_api, clientAPI_t *client_api);
