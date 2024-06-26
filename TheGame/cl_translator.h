//  this .h file property of uncle upi_
//  you cannot use it without permission.
//  ask uncle upi_  before use it. he is 
//  very kind and will  let you use this 
//  peace of code.
//
//  (c) for all times yomoma inc.


#pragma once

//-----------------------------------------------------------------------------
//
#ifdef __cplusplus
extern "C" 
{
#endif

typedef const char* (__stdcall SGetText)(const char* psz);

extern SGetText* gettext;
extern SGetText* getstring;

#ifdef __cplusplus
}
#endif

