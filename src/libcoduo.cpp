#include <signal.h>
#include <arpa/inet.h>

#include <memory>
#include <tuple>
#include <array>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>    // fabs, sqrt


#include "gsc.hpp"




// Stock cvars
cvar_t *com_cl_running;
cvar_t *com_dedicated;
cvar_t *com_logfile;
cvar_t *com_sv_running;
cvar_t *fs_game;
cvar_t *sv_allowDownload;
cvar_t *sv_floodProtect;
cvar_t *sv_fps;
cvar_t *sv_gametype;
cvar_t *sv_mapRotation;
cvar_t *sv_mapRotationCurrent;
cvar_t *sv_master[MAX_MASTER_SERVERS];
cvar_t *sv_maxclients;
cvar_t *sv_maxRate;
cvar_t *sv_pure;
cvar_t *sv_rconPassword;
cvar_t *sv_serverid;
cvar_t *sv_showAverageBPS;
cvar_t *sv_showCommands;

// Custom cvars
cvar_t *sv_cracked;


cHook *hook_com_init;
//cHook *hook_gametype_scripts;
//cHook *hook_sv_addoperatorcommands;
cHook *hook_sys_loaddll;

// Stock callbacks
int codecallback_startgametype = 0;
int codecallback_playerconnect = 0;
int codecallback_playerdisconnect = 0;
int codecallback_playerdamage = 0;
int codecallback_playerkilled = 0;

// Custom callbacks
int codecallback_client_spam = 0;
int codecallback_playercommand = 0;



/*callback_t callbacks[] =
{
    { &codecallback_startgametype, "CodeCallback_StartGameType" }, // g_scr_data.gametype.startupgametype
    { &codecallback_playerconnect, "CodeCallback_PlayerConnect" }, // g_scr_data.gametype.playerconnect
    { &codecallback_playerdisconnect, "CodeCallback_PlayerDisconnect" }, // g_scr_data.gametype.playerdisconnect
    { &codecallback_playerdamage, "CodeCallback_PlayerDamage" }, // g_scr_data.gametype.playerdamage
    { &codecallback_playerkilled, "CodeCallback_PlayerKilled" }, // g_scr_data.gametype.playerkilled

    { &codecallback_client_spam, "CodeCallback_CLSpam"},
    { &codecallback_playercommand, "CodeCallback_PlayerCommand"}
};*/

//customPlayerState_t customPlayerState[MAX_CLIENTS];
// Game lib objects
gentity_t* g_entities;
gclient_t* g_clients;
//level_locals_t* level;


// Game lib functions
Scr_GetFunctionHandle_t Scr_GetFunctionHandle;
Scr_GetNumParam_t Scr_GetNumParam;
Scr_IsSystemActive_t Scr_IsSystemActive;
Scr_GetInt_t Scr_GetInt;
Scr_GetString_t Scr_GetString;
Scr_GetType_t Scr_GetType;
Scr_GetEntity_t Scr_GetEntity;
Scr_AddBool_t Scr_AddBool;
Scr_AddInt_t Scr_AddInt;
Scr_AddFloat_t Scr_AddFloat;
Scr_AddString_t Scr_AddString;
Scr_AddUndefined_t Scr_AddUndefined;
Scr_AddVector_t Scr_AddVector;
Scr_MakeArray_t Scr_MakeArray;
Scr_AddArray_t Scr_AddArray;
Scr_AddObject_t Scr_AddObject;
Scr_LoadScript_t Scr_LoadScript;
Scr_ExecThread_t Scr_ExecThread;
Scr_ExecEntThread_t Scr_ExecEntThread;
Scr_ExecEntThreadNum_t Scr_ExecEntThreadNum;
Scr_FreeThread_t Scr_FreeThread;
Scr_GetFunction_t Scr_GetFunction;
Scr_GetMethod_t Scr_GetMethod;
Scr_Error_t Scr_Error;
Scr_ObjectError_t Scr_ObjectError;
Scr_GetConstString_t Scr_GetConstString;
Scr_ParamError_t Scr_ParamError;

ClientCommand_t ClientCommand;


void custom_Com_Init(char *commandLine)
{
    hook_com_init->unhook();
    void (*Com_Init)(char *commandLine);
    *(int *)&Com_Init = hook_com_init->from;
    Com_Init(commandLine);
    hook_com_init->hook();
    
    // Get references to stock cvars
    com_cl_running = Cvar_FindVar("cl_running");
    com_dedicated = Cvar_FindVar("dedicated");
    com_logfile = Cvar_FindVar("logfile");
    com_sv_running = Cvar_FindVar("sv_running");
    sv_allowDownload = Cvar_FindVar("sv_allowDownload");
    sv_floodProtect = Cvar_FindVar("sv_floodProtect");
    sv_gametype = Cvar_FindVar("g_gametype");
    sv_maxclients = Cvar_FindVar("sv_maxclients");
    sv_mapRotation = Cvar_FindVar("sv_mapRotation");
    sv_mapRotationCurrent = Cvar_FindVar("sv_mapRotationCurrent");
    sv_pure = Cvar_FindVar("sv_pure");
    sv_rconPassword = Cvar_FindVar("rconpassword");
    sv_serverid = Cvar_FindVar("sv_serverid");
    sv_showCommands = Cvar_FindVar("sv_showCommands");
    sv_fps = Cvar_FindVar("sv_fps");
    sv_maxRate = Cvar_FindVar("sv_maxRate");
    sv_showAverageBPS = Cvar_FindVar("sv_showAverageBPS");
    sv_showCommands = Cvar_FindVar("sv_showCommands");

    // Register custom cvars
    Cvar_Get("libcoduo", "1", CVAR_SERVERINFO);
    Cvar_Get("sv_wwwDownload", "0", CVAR_SYSTEMINFO | CVAR_ARCHIVE);
    Cvar_Get("sv_wwwBaseURL", "", CVAR_SYSTEMINFO | CVAR_ARCHIVE);
    

    sv_cracked = Cvar_Get("sv_cracked", "0", CVAR_ARCHIVE);


}

/*int custom_GScr_LoadGameTypeScript()
{
    unsigned int i;
    char path_for_cb[512] = "maps/mp/gametypes/_callbacksetup";

    hook_gametype_scripts->unhook();
    int (*GScr_LoadGameTypeScript)();
    *(int *)&GScr_LoadGameTypeScript = hook_gametype_scripts->from;
    int ret = GScr_LoadGameTypeScript();
    hook_gametype_scripts->hook();

    if(*fs_callbacks_additional->string)
    {
        if(!Scr_LoadScript(fs_callbacks_additional->string))
            Com_DPrintf("custom_GScr_LoadGameTypeScript: Scr_LoadScript for fs_callbacks_additional cvar failed.\n");
    }
    else
    {
        Com_DPrintf("custom_GScr_LoadGameTypeScript: No custom callback file specified in fs_callbacks_additional cvar.\n");
    }

    if(*fs_callbacks->string)
        strncpy(path_for_cb, fs_callbacks->string, sizeof(path_for_cb));
        
    for (i = 0; i < sizeof(callbacks)/sizeof(callbacks[0]); i++)
    {
        if(!strcmp(callbacks[i].name, "CodeCallback_PlayerCommand")) // Custom callback: PlayerCommand
            *callbacks[i].pos = Scr_GetFunctionHandle(fs_callbacks_additional->string, callbacks[i].name);
        else
            *callbacks[i].pos = Scr_GetFunctionHandle(path_for_cb, callbacks[i].name);
        
    }

    return ret;
}*/



/*void hook_ClientCommand(int clientNum)
{
    if(!Scr_IsSystemActive())
        return;

    char* cmd = Cmd_Argv(0);
    if(!strcmp(cmd, "gc"))
        return; // Prevent server crash
      
    if(!codecallback_playercommand)
    {
        ClientCommand(clientNum);
        return;
    }

    stackPushArray();
    int args = Cmd_Argc();
    for(int i = 0; i < args; i++)
    {
        char tmp[MAX_STRINGLENGTH];
        trap_Argv(i, tmp, sizeof(tmp));
        if(i == 1 && tmp[0] >= 20 && tmp[0] <= 22)
        {
            char *part = strtok(tmp + 1, " ");
            while(part != NULL)
            {
                stackPushString(part);
                stackPushArrayLast();
                part = strtok(NULL, " ");
            }
        }
        else
        {
            stackPushString(tmp);
            stackPushArrayLast();
        }
    }
    
    short ret = Scr_ExecEntThread(&g_entities[clientNum], codecallback_playercommand, 1);
    Scr_FreeThread(ret);
}*/

const char* hook_AuthorizeState(int arg)
{
    const char* s = Cmd_Argv(arg);
    if(sv_cracked->integer && !strcmp(s, "deny"))
        return "accept";
    return s;
}




/*void custom_SV_AddOperatorCommands()
{
    hook_sv_addoperatorcommands->unhook();
    void (*SV_AddOperatorCommands)();
    *(int *)&SV_AddOperatorCommands = hook_sv_addoperatorcommands->from;
    SV_AddOperatorCommands();


    hook_sv_addoperatorcommands->hook();
}*/



// See https://nachtimwald.com/2017/04/02/constant-time-string-comparison-in-c/
bool str_iseq(const char *s1, const char *s2)
{
    int             m = 0;
    volatile size_t i = 0;
    volatile size_t j = 0;
    volatile size_t k = 0;

    if (s1 == NULL || s2 == NULL)
        return false;

    while (1) {
        m |= s1[i]^s2[j];

        if (s1[i] == '\0')
            break;
        i++;

        if (s2[j] != '\0')
            j++;
        if (s2[j] == '\0')
            k++;
    }

    return m == 0;
}


void ServerCrash(int sig)
{
    int fd;
    FILE *fp;
    void *array[20];
    size_t size = backtrace(array, 20);

    // Write to crash log
    fp = fopen("./crash.log", "a");
    if (fp)
    {
        fd = fileno(fp);
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "Error: Server crashed with signal 0x%x {%d}\n", sig, sig);
        fflush(fp);
        backtrace_symbols_fd(array, size, fd);
    }
    
    // Write to stderr
    fprintf(stderr, "Error: Server crashed with signal 0x%x {%d}\n", sig, sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    
    system("stty sane");
    exit(1);
}


void* custom_Sys_LoadDll(const char *name, char *fqpath, int (**entryPoint)(int, ...), int (*systemcalls)(int, ...))
{
    hook_sys_loaddll->unhook();
    void* (*Sys_LoadDll)(const char *name, char *fqpath, int (**entryPoint)(int, ...), int (*systemcalls)(int, ...));
    *(int *)&Sys_LoadDll = hook_sys_loaddll->from;
    void* ret = Sys_LoadDll(name, fqpath, entryPoint, systemcalls);
    hook_sys_loaddll->hook();

    // Unprotect the game lib
    char libPath[512];
    cvar_t* fs_game = Cvar_FindVar("fs_game");
    if(*fs_game->string)
        sprintf(libPath, "%s/game.mp.uo.i386.so", fs_game->string);
    else
        sprintf(libPath, "main/game.mp.uo.i386.so");
    char buf[512];
    char flags[4];
    void *low, *high;
    FILE *fp;
    fp = fopen("/proc/self/maps", "r");
    if(!fp)
        return 0;
    while (fgets(buf, sizeof(buf), fp))
    {
        if(!strstr(buf, libPath))
            continue;
        if(sscanf (buf, "%p-%p %4c", &low, &high, flags) != 3)
            continue;
        mprotect((void *)low, (int)high-(int)low, PROT_READ | PROT_WRITE | PROT_EXEC);
    }
    fclose(fp);
    
    g_entities = (gentity_t*)dlsym(ret, "g_entities");
    g_clients = (gclient_t*)dlsym(ret, "g_clients");
//    level = (level_locals_t*)dlsym(ret, "level");


    Scr_IsSystemActive = (Scr_IsSystemActive_t)dlsym(ret, "Scr_IsSystemActive");
    Scr_GetInt = (Scr_GetInt_t)dlsym(ret, "Scr_GetInt");
    Scr_GetString = (Scr_GetString_t)dlsym(ret, "Scr_GetString");
    Scr_GetType = (Scr_GetType_t)dlsym(ret, "Scr_GetType");
    Scr_GetEntity = (Scr_GetEntity_t)dlsym(ret, "Scr_GetEntity");
    Scr_AddBool = (Scr_AddBool_t)dlsym(ret, "Scr_AddBool");
    Scr_AddInt = (Scr_AddInt_t)dlsym(ret, "Scr_AddInt");
    Scr_AddFloat = (Scr_AddFloat_t)dlsym(ret, "Scr_AddFloat");
    Scr_AddString = (Scr_AddString_t)dlsym(ret, "Scr_AddString");
    Scr_AddUndefined = (Scr_AddUndefined_t)dlsym(ret, "Scr_AddUndefined");
    Scr_AddVector = (Scr_AddVector_t)dlsym(ret, "Scr_AddVector");
    Scr_MakeArray = (Scr_MakeArray_t)dlsym(ret, "Scr_MakeArray");
    Scr_AddArray = (Scr_AddArray_t)dlsym(ret, "Scr_AddArray");
    Scr_AddObject = (Scr_AddObject_t)dlsym(ret, "Scr_AddObject");
    Scr_LoadScript = (Scr_LoadScript_t)dlsym(ret, "Scr_LoadScript");

    Scr_ExecThread = (Scr_ExecThread_t)dlsym(ret, "Scr_ExecThread");
    Scr_ExecEntThread = (Scr_ExecEntThread_t)dlsym(ret, "Scr_ExecEntThread");
    Scr_ExecEntThreadNum = (Scr_ExecEntThreadNum_t)dlsym(ret, "Scr_ExecEntThreadNum");
    Scr_FreeThread = (Scr_FreeThread_t)dlsym(ret, "Scr_FreeThread");
    Scr_GetFunction = (Scr_GetFunction_t)dlsym(ret, "Scr_GetFunction");
    Scr_GetMethod = (Scr_GetMethod_t)dlsym(ret, "Scr_GetMethod");
    Scr_Error = (Scr_Error_t)dlsym(ret, "Scr_Error");
    Scr_ObjectError = (Scr_ObjectError_t)dlsym(ret, "Scr_ObjectError");
    Scr_GetConstString = (Scr_GetConstString_t)dlsym(ret, "Scr_GetConstString");
    Scr_ParamError = (Scr_ParamError_t)dlsym(ret, "Scr_ParamError");
    


    ClientCommand = (ClientCommand_t)dlsym(ret, "ClientCommand");



//    hook_call((int)dlsym(ret, "vmMain") + 0xF0, (int)hook_ClientCommand); // CALL clientcommand address - vmMain address



//    hook_gametype_scripts = new cHook((int)dlsym(ret, "GScr_LoadGameTypeScript"), (int)custom_GScr_LoadGameTypeScript);
//    hook_gametype_scripts->hook();


    
    return ret;
}

class libcoduo
{
public:
    libcoduo()
    {

        printf("========Initializing LIBCODUO========\n");
        // Don't inherit lib of parent
        unsetenv("LD_PRELOAD");

        // Crash handlers for debugging
        signal(SIGSEGV, ServerCrash);
        signal(SIGABRT, ServerCrash);
        
        // Otherwise the printf()'s are printed at crash/end on older os/compiler versions
        setbuf(stdout, NULL);

        printf("> [LIBCODUO] Compiled for: CoDUO: 1.51\n");
        printf("> [LIBCODUO] Special thanks to raphael and his libcod1\n");
        printf("> [LIBCODUO] Compiled %s %s using GCC %s\n", __DATE__, __TIME__, __VERSION__);


        mprotect((void *)0x08048000, 0x135000, PROT_READ | PROT_WRITE | PROT_EXEC);
        
        hook_call(0x0809e8ed, (int)Scr_GetCustomFunction);
        hook_call(0x0809eb29, (int)Scr_GetCustomMethod);
        hook_call(0x0808a2c9, (int)hook_AuthorizeState);
        printf("End  hook\n");


        hook_sys_loaddll = new cHook(0x080c9071, (int)custom_Sys_LoadDll);
        hook_sys_loaddll->hook();
        hook_com_init = new cHook(0x0807154e, (int)custom_Com_Init);
        hook_com_init->hook();
        

        //hook_sv_addoperatorcommands = new cHook(0x8084A3C, (int)custom_SV_AddOperatorCommands);
        //hook_sv_addoperatorcommands->hook();
        //



        printf("> [PLUGIN LOADED]\n");
    }

    ~libcoduo()
    {
        printf("> [PLUGIN UNLOADED]\n");
    }
};

libcoduo *pro;
void __attribute__ ((constructor)) lib_load(void)
{
    pro = new libcoduo;
}
void __attribute__ ((destructor)) lib_unload(void)
{
    delete pro;
}