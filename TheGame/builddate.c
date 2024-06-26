#ifdef SAVAGE_DEMO
char *__builddate = "<DEMO> " __DATE__ " " __TIME__;
#else	//SAVAGE_DEMO
char *__builddate = __DATE__ " " __TIME__;
#endif	//SAVAGE_DEMO
