#ifndef _GSC_PLAYER_HPP_
#define _GSC_PLAYER_HPP_

#include "gsc.hpp"


void gsc_player_processclientcommand(scr_entref_t ref);
void gsc_player_setvelocity(scr_entref_t ref);
void gsc_player_getvelocity(scr_entref_t ref);
void gsc_player_connectionlesspackettoclient(scr_entref_t ref);
void gsc_player_getping(scr_entref_t ref);
void gsc_player_dropclient(scr_entref_t ref);

#endif