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
cvar_t *fs_callbacks_additional;
cvar_t *fs_callbacks;
cvar_t *sv_spectator_noclip;
cvar_t *jump_slowdownEnable;
cvar_t *jump_height;
//cvar_t* g_legacyStyle;
cvar_t *g_playerEject;
cvar_t *sv_fixq3fill;
cvar_t *fs_svrPaks;

cHook *hook_com_init;
cHook *hook_gametype_scripts;
//cHook *hook_sv_addoperatorcommands;
cHook *hook_Sys_LoadDll;
//cHook* hook_cvar_set2;
cHook *hook_PM_FlyMove;
cHook *hook_sv_begindownload_f;

// Stock callbacks
int codecallback_startgametype = 0;
int codecallback_playerconnect = 0;
int codecallback_playerdisconnect = 0;
int codecallback_playerdamage = 0;
int codecallback_playerkilled = 0;

// Custom callbacks
int codecallback_client_spam = 0;
int codecallback_playercommand = 0;

// Resume addresses
uintptr_t resume_addr_PM_WalkMove;
uintptr_t resume_addr_PM_SlideMove;

callback_t callbacks[] =
{
    { &codecallback_startgametype, "CodeCallback_StartGameType" }, // g_scr_data.gametype.startupgametype
    { &codecallback_playerconnect, "CodeCallback_PlayerConnect" }, // g_scr_data.gametype.playerconnect
    { &codecallback_playerdisconnect, "CodeCallback_PlayerDisconnect" }, // g_scr_data.gametype.playerdisconnect
    { &codecallback_playerdamage, "CodeCallback_PlayerDamage" }, // g_scr_data.gametype.playerdamage
    { &codecallback_playerkilled, "CodeCallback_PlayerKilled" }, // g_scr_data.gametype.playerkilled

    { &codecallback_client_spam, "CodeCallback_CLSpam"},
    { &codecallback_playercommand, "CodeCallback_PlayerCommand"}
};

//// Game lib
void* libHandle;
// Objects
gentity_t *g_entities;
//level_locals_t *level;
gclient_t* g_clients;
pmove_t **pm;

// Functions
ClientCommand_t ClientCommand;
Scr_IsSystemActive_t Scr_IsSystemActive;
Scr_GetNumParam_t Scr_GetNumParam;
Scr_GetInt_t Scr_GetInt;
Scr_GetString_t Scr_GetString;
Scr_AddBool_t Scr_AddBool;
Scr_AddInt_t Scr_AddInt;
Scr_AddFloat_t Scr_AddFloat;
Scr_AddString_t Scr_AddString;
Scr_AddUndefined_t Scr_AddUndefined;
Scr_AddVector_t Scr_AddVector;
Scr_MakeArray_t Scr_MakeArray;
Scr_AddArray_t Scr_AddArray;
Scr_GetFunction_t Scr_GetFunction;
Scr_GetMethod_t Scr_GetMethod;
Scr_Error_t Scr_Error;
Scr_LoadScript_t Scr_LoadScript;
Scr_GetFunctionHandle_t Scr_GetFunctionHandle;
Scr_ExecEntThread_t Scr_ExecEntThread;
Scr_FreeThread_t Scr_FreeThread;
trap_Argv_t trap_Argv;
va_t va;
VectorNormalize_t VectorNormalize;
BG_AddPredictableEventToPlayerstate_t BG_AddPredictableEventToPlayerstate;
BG_CheckProne_t BG_CheckProne;
BG_PlayAnim_t BG_PlayAnim;
PitchForYawOnNormal_t PitchForYawOnNormal;
AngleDelta_t AngleDelta;
G_ClientStopUsingTurret_t G_ClientStopUsingTurret;
G_EntUnlink_t G_EntUnlink;
trap_UnlinkEntity_t trap_UnlinkEntity;
Scr_SetString_t Scr_SetString;
G_SetClientContents_t G_SetClientContents;
player_die_t player_die;
trap_GetUsercmd_t trap_GetUsercmd;
G_SetOrigin_t G_SetOrigin;
SetClientViewAngle_t SetClientViewAngle;
ClientEndFrame_t ClientEndFrame;
ClientThink_real_t ClientThink_real;
G_ClientCanSpectateTeam_t G_ClientCanSpectateTeam;
Cmd_FollowCycle_f_t Cmd_FollowCycle_f;
StopFollowing_t StopFollowing;
HudElem_UpdateClient_t HudElem_UpdateClient;
AddLeanToPosition_t AddLeanToPosition;
UnGetLeanFraction_t UnGetLeanFraction;
AngleNormalize180Accurate_t AngleNormalize180Accurate;
AngleNormalize180_t AngleNormalize180;
BG_CheckProneValid_t BG_CheckProneValid;
trap_SendServerCommand_t trap_SendServerCommand;
BG_GetNumWeapons_t BG_GetNumWeapons;
BG_GetInfoForWeapon_t BG_GetInfoForWeapon;
BG_GetWeaponIndexForName_t BG_GetWeaponIndexForName;
StuckInClient_t StuckInClient;
PM_NoclipMove_t PM_NoclipMove;
Jump_Set_f_t Jump_Set_f;

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
    fs_game = Cvar_FindVar("fs_game");

    // Register custom cvars
    Cvar_Get("libcoduo", "1", CVAR_SERVERINFO);
    Cvar_Get("sv_wwwDownload", "0", CVAR_SYSTEMINFO | CVAR_ARCHIVE);
    Cvar_Get("sv_wwwBaseURL", "", CVAR_SYSTEMINFO | CVAR_ARCHIVE);
    

    sv_cracked = Cvar_Get("sv_cracked", "0", CVAR_ARCHIVE);
    fs_callbacks = Cvar_Get("fs_callbacks", "", CVAR_ARCHIVE);
    fs_callbacks_additional = Cvar_Get("fs_callbacks_additional", "", CVAR_ARCHIVE);
    sv_spectator_noclip = Cvar_Get("sv_spectator_noclip", "0", CVAR_ARCHIVE);
    jump_slowdownEnable =  Cvar_Get("jump_slowdownEnable", "1", CVAR_SYSTEMINFO | CVAR_ARCHIVE);
    jump_height =  Cvar_Get("jump_height", "39.0", CVAR_ARCHIVE);
    g_playerEject = Cvar_Get("g_playerEject", "1", CVAR_ARCHIVE);
    fs_svrPaks = Cvar_Get("fs_svrPaks", "", CVAR_ARCHIVE);
    sv_fixq3fill = Cvar_Get("sv_fixq3fill", "0", CVAR_ARCHIVE);

//    g_legacyStyle = Cvar_Get("g_legacyStyle", "1", CVAR_SYSTEMINFO | CVAR_ARCHIVE);
}

/*
std::map<std::string, std::map<std::string, WeaponProperties>> weapons_properties;

void toggleLegacyStyle(bool enable)
{
    if(enable)
        Cvar_Set2("jump_slowdownEnable", "0", qfalse);
    else
        Cvar_Set2("jump_slowdownEnable", "1", qfalse);
    if (enable)
    {
    int id_kar98k_sniper = BG_GetWeaponIndexForName("kar98k_sniper_mp");
    weaponinfo_t* weapon_kar98k_sniper = BG_GetInfoForWeapon(id_kar98k_sniper);
    int id_springfield = BG_GetWeaponIndexForName("springfield_mp");
    weaponinfo_t* weapon_springfield = BG_GetInfoForWeapon(id_springfield);
    int id_mosin_nagant_sniper = BG_GetWeaponIndexForName("mosin_nagant_sniper_mp");
    weaponinfo_t* weapon_mosin_nagant_sniper = BG_GetInfoForWeapon(id_mosin_nagant_sniper);

    if (weapon_kar98k_sniper)
    {
        const WeaponProperties* properties_kar98k_sniper = nullptr;
        if(enable)
            properties_kar98k_sniper = &weapons_properties[weapon_kar98k_sniper->name]["legacy"];
        else
            properties_kar98k_sniper = &weapons_properties[weapon_kar98k_sniper->name]["default"];
        weapon_kar98k_sniper->adsTransInTime = properties_kar98k_sniper->adsTransInTime;
        weapon_kar98k_sniper->OOPosAnimLength[0] = 1.0 / (float)weapon_kar98k_sniper->adsTransInTime;
        weapon_kar98k_sniper->adsZoomInFrac = properties_kar98k_sniper->adsZoomInFrac;
        weapon_kar98k_sniper->idleCrouchFactor = properties_kar98k_sniper->idleCrouchFactor;
        weapon_kar98k_sniper->idleProneFactor = properties_kar98k_sniper->idleProneFactor;
        weapon_kar98k_sniper->rechamberWhileAds = properties_kar98k_sniper->rechamberWhileAds;
        weapon_kar98k_sniper->adsViewErrorMin = properties_kar98k_sniper->adsViewErrorMin;
        weapon_kar98k_sniper->adsViewErrorMax = properties_kar98k_sniper->adsViewErrorMax;
    }

    if (weapon_mosin_nagant_sniper)
    {
        const WeaponProperties* properties_mosin_nagant_sniper = nullptr;
        if(enable)
            properties_mosin_nagant_sniper = &weapons_properties[weapon_mosin_nagant_sniper->name]["legacy"];
        else
            properties_mosin_nagant_sniper = &weapons_properties[weapon_mosin_nagant_sniper->name]["default"];
        weapon_mosin_nagant_sniper->reloadAddTime = properties_mosin_nagant_sniper->reloadAddTime;
        weapon_mosin_nagant_sniper->adsTransInTime = properties_mosin_nagant_sniper->adsTransInTime;
        weapon_mosin_nagant_sniper->OOPosAnimLength[0] = 1.0 / (float)weapon_mosin_nagant_sniper->adsTransInTime;
        weapon_mosin_nagant_sniper->adsZoomInFrac = properties_mosin_nagant_sniper->adsZoomInFrac;
        weapon_mosin_nagant_sniper->idleCrouchFactor = properties_mosin_nagant_sniper->idleCrouchFactor;
        weapon_mosin_nagant_sniper->idleProneFactor = properties_mosin_nagant_sniper->idleProneFactor;
        weapon_mosin_nagant_sniper->rechamberWhileAds = properties_mosin_nagant_sniper->rechamberWhileAds;
        weapon_mosin_nagant_sniper->adsViewErrorMin = properties_mosin_nagant_sniper->adsViewErrorMin;
        weapon_mosin_nagant_sniper->adsViewErrorMax = properties_mosin_nagant_sniper->adsViewErrorMax;
    }

    if (weapon_springfield)
    {
        const WeaponProperties* properties_springfield = nullptr;
        if(enable)
            properties_springfield = &weapons_properties[weapon_springfield->name]["legacy"];
        else
            properties_springfield = &weapons_properties[weapon_springfield->name]["default"];
        weapon_springfield->adsTransInTime = properties_springfield->adsTransInTime;
        weapon_springfield->OOPosAnimLength[0] = 1.0 / (float)weapon_springfield->adsTransInTime;
        weapon_springfield->adsZoomInFrac = properties_springfield->adsZoomInFrac;
        weapon_springfield->idleCrouchFactor = properties_springfield->idleCrouchFactor;
        weapon_springfield->idleProneFactor = properties_springfield->idleProneFactor;
        weapon_springfield->rechamberWhileAds = properties_springfield->rechamberWhileAds;
        weapon_springfield->adsViewErrorMin = properties_springfield->adsViewErrorMin;
        weapon_springfield->adsViewErrorMax = properties_springfield->adsViewErrorMax;
    }
    }

}

void custom_Cvar_Set2(const char *var_name, const char *value, qboolean force)
{
    bool check_g_legacyStyle = false;
    bool g_legacyStyle_before;
    bool g_legacyStyle_after;
    //printf("##### custom_Cvar_Set2 called: var_name: %s, value: %s\n", var_name, value);

    if(com_sv_running != NULL && com_sv_running->integer)
    {
        if(!strcasecmp(var_name, g_legacyStyle->name))
        {
            check_g_legacyStyle = true;
            g_legacyStyle_before = g_legacyStyle->integer ? true : false;
        }
    }
    
    hook_cvar_set2->unhook();
    cvar_t* (*Cvar_Set2)(const char *var_name, const char *value, qboolean force);
    *(int *)&Cvar_Set2 = hook_cvar_set2->from;

    if(check_g_legacyStyle)
    {
        cvar_t* var = Cvar_Set2(var_name, value, force);
        if(var)
        {
            g_legacyStyle_after = var->integer ? true : false;
            if(g_legacyStyle_before != g_legacyStyle_after)
                toggleLegacyStyle(var->integer);//printf("check_g_legacyStyle\n");//
        }
    }
    else
        Cvar_Set2(var_name, value, force);

    hook_cvar_set2->hook();
}
*/
int custom_GScr_LoadGameTypeScript()
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
        
        /*if ( *callbacks[i].pos && g_debugCallbacks->integer )
            Com_Printf("%s found @ %p\n", callbacks[i].name, scrVarPub.programBuffer + *callbacks[i].pos);*/ //TODO: verify scrVarPub_t
    }

    return ret;
}



void hook_ClientCommand(int clientNum)
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
}


const char* hook_AuthorizeState(int arg)
{
    const char* s = Cmd_Argv(arg);
    if(sv_cracked->integer && !strcmp(s, "deny"))
        return "accept";
    return s;
}

qboolean hook_StuckInClient(gentity_s *self)
{
    if(!g_playerEject->integer)
        return qfalse;
    return StuckInClient(self);
}



qboolean FS_svrPak(const char *base)
{
    if (strstr(base, "_svr_"))
        return qtrue;

    if (*fs_svrPaks->string)
    {
        bool isSvrPak = false;
        size_t lenString = strlen(fs_svrPaks->string) +1;
        char* stringCopy = (char*)malloc(lenString);
        strcpy(stringCopy, fs_svrPaks->string);

        const char* separator = ";";
        char* strToken = strtok(stringCopy, separator);

        while (strToken != NULL)
        {
            if (!strcmp(base, strToken))
            {
                isSvrPak = true;
                break;
            }
            strToken = strtok(NULL, separator);
        }

        free(stringCopy);
        if (isSvrPak)
            return qtrue;
    }

    return qfalse;
}

bool shouldServeFile(const char *requestedFilePath)
{
    static char localFilePath[MAX_OSPATH*2+5];
    searchpath_t* search;

    localFilePath[0] = 0;

    for(search = fs_searchpaths; search; search = search->next)
    {
        if(search->pak)
        {
            snprintf(localFilePath, sizeof(localFilePath), "%s/%s.pk3", search->pak->pakGamename, search->pak->pakBasename);
            if(!strcmp(localFilePath, requestedFilePath))
                if(!FS_svrPak(search->pak->pakBasename))
                    return true;
        }
    }
    return false;
}

void custom_SV_BeginDownload_f(client_t *cl)
{
    //// [exploit patch] q3dirtrav
    // See:
    //- https://aluigi.altervista.org/video/q3dirtrav.avi
    //- https://aluigi.altervista.org/poc/q3dirtrav.zip
    //- https://oldforum.aluigi.org/post3479.html#p3479
    
    int args = Cmd_Argc();
    if (args > 1)
    {
        const char* arg1 = Cmd_Argv(1);
        if (!shouldServeFile(arg1))
        {
            char ip[16];
            snprintf(ip, sizeof(ip), "%d.%d.%d.%d",
                cl->netchan.remoteAddress.ip[0],
                cl->netchan.remoteAddress.ip[1],
                cl->netchan.remoteAddress.ip[2],
                cl->netchan.remoteAddress.ip[3]);
            Com_Printf("WARNING: %s (%s) tried to download %s.\n", cl->name, ip, arg1);
            return;
        }
    }
    ////

    hook_sv_begindownload_f->unhook();
    void (*SV_BeginDownload_f)(client_t *cl);
    *(int*)&SV_BeginDownload_f = hook_sv_begindownload_f->from;
    SV_BeginDownload_f(cl);
    hook_sv_begindownload_f->hook();
}


int NET_CompareBaseAdr2(netadr_t from, netadr_t cmp)
{
    if (from.type != cmp.type)
        return qfalse;

    if (from.type == NA_LOOPBACK)
        return qtrue;

    if (from.type == NA_IP || from.type == NA_IPX) {
        if (from.ip[0] == cmp.ip[0] &&
            from.ip[1] == cmp.ip[1] &&
            from.ip[2] == cmp.ip[2] &&
            from.ip[3] == cmp.ip[3]) {
            return qtrue;
        }
    }

    return qfalse;
}


void custom_SV_DirectConnect(netadr_t from)
{
    int maxconnecttime = 10000; 

    if (sv_fixq3fill->integer == 1) {
        int connectingCount = 0;
        client_t *duplicateCl = NULL;

        for (int i = 0; i < sv_maxclients->integer; i++) {
            client_t *cl = &svs.clients[i];

            // fuck NET_CompareBaseAdr, dosent works here :C
            if (cl->state == CS_CONNECTED && NET_CompareBaseAdr2(from, cl->netchan.remoteAddress)) {
                int delta = svs.time - cl->lastPacketTime;

                printf("custom_SV_DirectConnect(): Fixq3fill if state 2\n");

                if (delta < maxconnecttime) {
                    connectingCount++;
                    duplicateCl = cl;  // save the reference
                }
            }
        }



        if (connectingCount >= 1 && duplicateCl != NULL) {
            Com_Printf("Rejected duplicate CONNECTING client from IP: %s\n", NET_AdrToString(from));
            NET_OutOfBandPrint(NS_SERVER, from, "Only one CONNECTING client allowed per IP (for a short time).\n");
            SV_DropClient(duplicateCl, NULL);
            return;
        }
    }
    printf("custom_SV_DirectConnect()\n");

    SV_DirectConnect(from);
}


/*void custom_SV_AddOperatorCommands()
{
    hook_sv_addoperatorcommands->unhook();
    void (*SV_AddOperatorCommands)();
    *(int *)&SV_AddOperatorCommands = hook_sv_addoperatorcommands->from;
    SV_AddOperatorCommands();


    hook_sv_addoperatorcommands->hook();
}*/

void custom_PM_FlyMove()
{
    if (sv_spectator_noclip->integer)
    {
        PM_NoclipMove();
        return;
    }

    hook_PM_FlyMove->unhook();
    void (*PM_FlyMove)();
    *(int*)&PM_FlyMove = hook_PM_FlyMove->from;
    PM_FlyMove();
    hook_PM_FlyMove->hook();
}

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


void *custom_Sys_LoadDll(const char *name, char *fqpath, int (**entryPoint)(int, ...), int (*systemcalls)(int, ...))
{
    hook_Sys_LoadDll->unhook();
    void *(*Sys_LoadDll)(const char *name, char *fqpath, int (**entryPoint)(int, ...), int (*systemcalls)(int, ...));
    *(int*)&Sys_LoadDll = hook_Sys_LoadDll->from;
    void* libHandle = Sys_LoadDll(name, fqpath, entryPoint, systemcalls);
    hook_Sys_LoadDll->hook();

    //// Unprotect game.mp.i386.so
    // See https://github.com/xtnded/codextended/blob/855df4fb01d20f19091d18d46980b5fdfa95a712/src/librarymodule.c#L55
    char libPath[512];
    char buf[512];
    char flags[4];
    void *low, *high;
    FILE *fp;

    if (fs_game && fs_game->string && *fs_game->string)
        sprintf(libPath, "%s/game.mp.uo.i386.so", fs_game->string);
    else
        sprintf(libPath, "uo/game.mp.uo.i386.so");
    
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
    
    //// Objects

    g_entities = (gentity_t*)dlsym(libHandle, "g_entities");
    g_clients = (gclient_t*)dlsym(libHandle, "g_clients");
//    level = (level_locals_t*)dlsym(ret, "level");
    pm = (pmove_t**)dlsym(libHandle, "pm");
    ////

    //// Functions
    ClientCommand = (ClientCommand_t)dlsym(libHandle, "ClientCommand");
    Scr_IsSystemActive = (Scr_IsSystemActive_t)dlsym(libHandle, "Scr_IsSystemActive");
    Scr_GetFunctionHandle = (Scr_GetFunctionHandle_t)dlsym(libHandle, "Scr_GetFunctionHandle");
    Scr_GetNumParam = (Scr_GetNumParam_t)dlsym(libHandle, "Scr_GetNumParam");
    Scr_GetFunction = (Scr_GetFunction_t)dlsym(libHandle, "Scr_GetFunction");
    Scr_GetMethod = (Scr_GetMethod_t)dlsym(libHandle, "Scr_GetMethod");
    Scr_Error = (Scr_Error_t)dlsym(libHandle, "Scr_Error");
    Scr_GetInt = (Scr_GetInt_t)dlsym(libHandle, "Scr_GetInt");
    Scr_GetString = (Scr_GetString_t)dlsym(libHandle, "Scr_GetString");
    Scr_AddBool = (Scr_AddBool_t)dlsym(libHandle, "Scr_AddBool");
    Scr_AddInt = (Scr_AddInt_t)dlsym(libHandle, "Scr_AddInt");
    Scr_AddFloat = (Scr_AddFloat_t)dlsym(libHandle, "Scr_AddFloat");
    Scr_AddString = (Scr_AddString_t)dlsym(libHandle, "Scr_AddString");
    Scr_AddUndefined = (Scr_AddUndefined_t)dlsym(libHandle, "Scr_AddUndefined");
    Scr_AddVector = (Scr_AddVector_t)dlsym(libHandle, "Scr_AddVector");
    Scr_MakeArray = (Scr_MakeArray_t)dlsym(libHandle, "Scr_MakeArray");
    Scr_AddArray = (Scr_AddArray_t)dlsym(libHandle, "Scr_AddArray");
    Scr_LoadScript = (Scr_LoadScript_t)dlsym(libHandle, "Scr_LoadScript");
    Scr_ExecEntThread = (Scr_ExecEntThread_t)dlsym(libHandle, "Scr_ExecEntThread");
    Scr_FreeThread = (Scr_FreeThread_t)dlsym(libHandle, "Scr_FreeThread");
    trap_Argv = (trap_Argv_t)dlsym(libHandle, "trap_Argv");
    va = (va_t)dlsym(libHandle, "va");
    VectorNormalize = (VectorNormalize_t)dlsym(libHandle, "VectorNormalize");
    BG_AddPredictableEventToPlayerstate = (BG_AddPredictableEventToPlayerstate_t)dlsym(libHandle, "BG_AddPredictableEventToPlayerstate");
    BG_CheckProne = (BG_CheckProne_t)dlsym(libHandle, "BG_CheckProne");
    BG_PlayAnim = (BG_PlayAnim_t)dlsym(libHandle, "BG_PlayAnim");
    PitchForYawOnNormal = (PitchForYawOnNormal_t)dlsym(libHandle, "PitchForYawOnNormal");
    AngleDelta = (AngleDelta_t)dlsym(libHandle, "AngleDelta");
    G_ClientStopUsingTurret = (G_ClientStopUsingTurret_t)dlsym(libHandle, "G_ClientStopUsingTurret");
    G_EntUnlink = (G_EntUnlink_t)dlsym(libHandle, "G_EntUnlink");
    trap_UnlinkEntity = (trap_UnlinkEntity_t)dlsym(libHandle, "trap_UnlinkEntity");
    Scr_SetString = (Scr_SetString_t)dlsym(libHandle, "Scr_SetString");
    G_SetClientContents = (G_SetClientContents_t)dlsym(libHandle, "G_SetClientContents");
    player_die = (player_die_t)dlsym(libHandle, "player_die");
    trap_GetUsercmd = (trap_GetUsercmd_t)dlsym(libHandle, "trap_GetUsercmd");
    G_SetOrigin = (G_SetOrigin_t)dlsym(libHandle, "G_SetOrigin");
    SetClientViewAngle = (SetClientViewAngle_t)dlsym(libHandle, "SetClientViewAngle");
    ClientEndFrame = (ClientEndFrame_t)dlsym(libHandle, "ClientEndFrame");
    ClientThink_real = (ClientThink_real_t)dlsym(libHandle, "ClientThink_real");
    G_ClientCanSpectateTeam = (G_ClientCanSpectateTeam_t)dlsym(libHandle, "G_ClientCanSpectateTeam");
    Cmd_FollowCycle_f = (Cmd_FollowCycle_f_t)dlsym(libHandle, "Cmd_FollowCycle_f");
    StopFollowing = (StopFollowing_t)dlsym(libHandle, "StopFollowing");
    HudElem_UpdateClient = (HudElem_UpdateClient_t)dlsym(libHandle, "HudElem_UpdateClient");
    AddLeanToPosition = (AddLeanToPosition_t)dlsym(libHandle, "AddLeanToPosition");
    UnGetLeanFraction = (UnGetLeanFraction_t)dlsym(libHandle, "UnGetLeanFraction");
    AngleNormalize180Accurate = (AngleNormalize180Accurate_t)dlsym(libHandle, "AngleNormalize180Accurate");
    AngleNormalize180 = (AngleNormalize180_t)dlsym(libHandle, "AngleNormalize180");
    BG_CheckProneValid = (BG_CheckProneValid_t)dlsym(libHandle, "BG_CheckProneValid");
    BG_GetNumWeapons = (BG_GetNumWeapons_t)dlsym(libHandle, "BG_GetNumWeapons");
    BG_GetInfoForWeapon = (BG_GetInfoForWeapon_t)dlsym(libHandle, "BG_GetInfoForWeapon");
    BG_GetWeaponIndexForName = (BG_GetWeaponIndexForName_t)dlsym(libHandle, "BG_GetWeaponIndexForName");
    trap_SendServerCommand = (trap_SendServerCommand_t)dlsym(libHandle, "trap_SendServerCommand");
    Jump_Set_f = (Jump_Set_f_t)((int)dlsym(libHandle, "PM_GetEffectiveStance") + 0xF9F);
    PM_NoclipMove = (PM_NoclipMove_t)((int)dlsym(libHandle, "PM_GetEffectiveStance") + 0x1FAA);
    StuckInClient = (StuckInClient_t)dlsym(libHandle, "StuckInClient");

    hook_call((int)dlsym(libHandle, "ClientEndFrame") + 0x44D, (int)hook_StuckInClient);

    // jump stuffs
    hook_jmp((int)dlsym(libHandle, "PM_GetEffectiveStance") + 0xAD, (int)custom_Jump_GetLandFactor);
    hook_jmp((int)dlsym(libHandle, "PM_GetEffectiveStance") + 0x4C, (int)custom_PM_GetReducedFriction);
    hook_call((int)dlsym(libHandle, "PM_GetEffectiveStance") + 0x11B3, (int)hook_Jump_Check);


    hook_call((int)dlsym(libHandle, "vmMain") + 0xF0, (int)hook_ClientCommand); // CALL clientcommand address - vmMain address

    hook_gametype_scripts = new cHook((int)dlsym(libHandle, "GScr_LoadGameTypeScript"), (int)custom_GScr_LoadGameTypeScript);
    hook_gametype_scripts->hook();
    hook_PM_FlyMove = new cHook((int)dlsym(libHandle, "PM_GetEffectiveStance") + 0x1457, (int)custom_PM_FlyMove);
    hook_PM_FlyMove->hook();
    return libHandle;
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

        hook_call(0x0809466C, (int)custom_SV_DirectConnect);

        printf("End  hook\n");


        hook_Sys_LoadDll = new cHook(0x080c9071, (int)custom_Sys_LoadDll);
        hook_Sys_LoadDll->hook();
        hook_com_init = new cHook(0x0807154e, (int)custom_Com_Init);
        hook_com_init->hook();
        //hook_cvar_set2 = new cHook(0x08073440, (int)custom_Cvar_Set2);
        //hook_cvar_set2->hook();

        hook_sv_begindownload_f = new cHook(0x808C25D, (int)custom_SV_BeginDownload_f);
        hook_sv_begindownload_f->hook();


        // infoboom patch
        //See: https://github.com/PrawyCoD1/coduoextended/blob/9c3ec32e7085266e136d9782cc9ba5c961189919/src/codextended.c#L294
	    *(byte*)0x807f459 = 1;


        //hook_sv_addoperatorcommands = new cHook(0x8089580, (int)custom_SV_AddOperatorCommands);
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