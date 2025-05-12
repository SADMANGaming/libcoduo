#ifndef _SL_FUNCTIONS_HPP_
#define _SL_FUNCTIONS_HPP_

// Scr
typedef void (*Scr_AddIString_t)(const char *string);
extern Scr_AddIString_t Scr_AddIString;

typedef void (*Scr_GetIString_t)(const char *istring);
extern Scr_GetIString_t Scr_GetIString;

//typedef void(*Scr_ConstructMessageString_t)(unsigned int firstParmIndex, unsigned int lastParmIndex, const char *errorContext, char *string, unsigned int stringLimit);
//extern Scr_ConstructMessageString_t Scr_ConstructMessageString;

typedef xfunction_t (*Scr_GetFunction_t)(const char** v_functionName, qboolean *v_developer);
extern Scr_GetFunction_t Scr_GetFunction;

typedef xmethod_t (*Scr_GetMethod_t)(const char** v_methodName, qboolean *v_developer);
extern Scr_GetMethod_t Scr_GetMethod;

typedef void (*Scr_Error_t)(const char *string);
extern Scr_Error_t Scr_Error;

typedef short (*Scr_ExecThread_t)(int callbackHook, unsigned int numArgs);
extern Scr_ExecThread_t Scr_ExecThread;

typedef short (*Scr_ExecEntThread_t)(gentity_t* ent, int callbackHook, unsigned int numArgs);
extern Scr_ExecEntThread_t Scr_ExecEntThread;

typedef unsigned short (*Scr_ExecEntThreadNum_t)(int entnum, unsigned int classnum, int handle, unsigned int paramcount);
extern Scr_ExecEntThreadNum_t Scr_ExecEntThreadNum;

typedef short (*Scr_FreeThread_t)(short thread_id);
extern Scr_FreeThread_t Scr_FreeThread;

typedef int (*Scr_GetFunctionHandle_t)(const char* scriptName, const char* labelName);

typedef int (*Scr_GetNumParam_t)(void);
extern Scr_GetNumParam_t Scr_GetNumParam;

typedef int (*Scr_IsSystemActive_t)();
extern Scr_IsSystemActive_t Scr_IsSystemActive;

typedef int (*Scr_GetInt_t)(unsigned int param);
extern Scr_GetInt_t Scr_GetInt;

typedef const char * (*Scr_GetString_t)(unsigned int param);
extern Scr_GetString_t Scr_GetString;

typedef int (*Scr_GetType_t)(unsigned int param);
extern Scr_GetType_t Scr_GetType;

typedef gentity_t * (*Scr_GetEntity_t)(unsigned int index);
extern Scr_GetEntity_t Scr_GetEntity;

typedef void (*Scr_AddBool_t)(qboolean value);
extern Scr_AddBool_t Scr_AddBool;

typedef void (*Scr_AddInt_t)(int value);
extern Scr_AddInt_t Scr_AddInt;

typedef void (*Scr_AddFloat_t)(float value);
extern Scr_AddFloat_t Scr_AddFloat;

typedef void (*Scr_AddString_t)(const char *string);
extern Scr_AddString_t Scr_AddString;

typedef void (*Scr_AddUndefined_t)(void);
extern Scr_AddUndefined_t Scr_AddUndefined;

typedef void (*Scr_AddVector_t)(vec3_t vec);
extern Scr_AddVector_t Scr_AddVector;

typedef void (*Scr_MakeArray_t)(void);
extern Scr_MakeArray_t Scr_MakeArray;

typedef void (*Scr_AddArray_t)(void);
extern Scr_AddArray_t Scr_AddArray;

typedef void (*Scr_AddObject_t)(unsigned int object);
extern Scr_AddObject_t Scr_AddObject;

typedef unsigned int (*Scr_LoadScript_t)(const char *filename);

typedef void (*Scr_ObjectError_t)(const char *error);
extern Scr_ObjectError_t Scr_ObjectError;

typedef unsigned short (*Scr_GetConstString_t)(unsigned int param);
extern Scr_GetConstString_t Scr_GetConstString;

typedef void (*Scr_ParamError_t)(int paramNum, const char *error);
extern Scr_ParamError_t Scr_ParamError;


// Client
typedef void (*ClientCommand_t)(int clientNum);
extern ClientCommand_t ClientCommand;

// Trap
typedef void (*trap_Argv_t)(int arg, char *buffer, int bufferLength);

typedef void (*trap_SendServerCommand_t)(int clientnum, svscmd_type type, const char *text);
extern trap_SendServerCommand_t trap_SendServerCommand;

#endif