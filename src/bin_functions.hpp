#ifndef _BIN_FUNCTIONS_HPP_
#define _BIN_FUNCTIONS_HPP_

// Cvar
typedef cvar_t * (*Cvar_Set_t)(const char *var_name, const char *value);
static const Cvar_Set_t Cvar_Set = (Cvar_Set_t)0x8073798;

typedef cvar_t * (*Cvar_Get_t)(const char *var_name, const char *var_value, unsigned short flags);
static const Cvar_Get_t Cvar_Get = (Cvar_Get_t)0x8073114;

typedef cvar_t * (*Cvar_FindVar_t)(const char *var_name);
static const Cvar_FindVar_t Cvar_FindVar = (Cvar_FindVar_t)0x8072FAE;

// Com
typedef void (*Com_Printf_t)(const char *format, ...);
static const Com_Printf_t Com_Printf = (Com_Printf_t)0x8070248;

typedef void (*Com_DPrintf_t)(const char *format, ...);
static const Com_DPrintf_t Com_DPrintf = (Com_DPrintf_t)0x8070297;

typedef int (*Com_sprintf_t)(char *dest, int size, const char *format, ...);
static const Com_sprintf_t Com_sprintf = (Com_sprintf_t)0x8086C8E;

typedef void (*Com_PrintMessage_t)(int channel, const char *message);
static const Com_PrintMessage_t Com_PrintMessage = (Com_PrintMessage_t)0x806FE5F;

typedef void (*Com_Error_t)(errorParm_t code, const char *format, ...);
static const Com_Error_t Com_Error = (Com_Error_t)0x80704AC;

// SL
typedef char* (*SL_ConvertToString_t)(unsigned int index);
static const SL_ConvertToString_t SL_ConvertToString = (SL_ConvertToString_t)0x080a4458;

// SV
typedef playerState_t* (*SV_GameClientNum_t)(int num);
static const SV_GameClientNum_t SV_GameClientNum = (SV_GameClientNum_t)0x0808e105; // i searched cmdCount > MAX_PACKET_USERCMDS, thanks cod2rev

typedef void (*SV_DropClient_t)(client_t *drop, const char *reason);
static const SV_DropClient_t SV_DropClient = (SV_DropClient_t)0x0808ba15; // search CS_ZOMBIE duh?

// Cmd
typedef char* (*Cmd_Argv_t)(int arg);
static const Cmd_Argv_t Cmd_Argv = (Cmd_Argv_t)0x0805ff28;

typedef int (*Cmd_Argc_t)();
static const Cmd_Argc_t Cmd_Argc = (Cmd_Argc_t)0x0805ff1e;

typedef void (*Cmd_AddCommand_t)(const char *cmd_name, xcommand_t function);
static const Cmd_AddCommand_t Cmd_AddCommand = (Cmd_AddCommand_t)0x08060272; //txt str

// NET
typedef const char* (*NET_AdrToString_t)(netadr_t a);
static const NET_AdrToString_t NET_AdrToString = (NET_AdrToString_t)0x080848f8;

typedef void (*NET_OutOfBandPrint_t)(netsrc_t net_socket, netadr_t adr, const char *format, ...);
static const NET_OutOfBandPrint_t NET_OutOfBandPrint = (NET_OutOfBandPrint_t)0x08084d76;

typedef int (*NET_CompareAdrSigned_t)(netadr_t *a, netadr_t *b);
static const NET_CompareAdrSigned_t NET_CompareAdrSigned = (NET_CompareAdrSigned_t)0x080849fe;

#endif