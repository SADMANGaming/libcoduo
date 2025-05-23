#ifndef _CRACKING_HPP_
#define _CRACKING_HPP_

#include <cstring>      // memcpy, memset
#include <sys/mman.h>   // mprotect

#include <unistd.h>   // For sysconf()
#include <cstdint>    // For uintptr_t
#include <cstdio>     // For perror()
#include "gsc.hpp"

void hook_jmp(int from, int to);
void hook_call(int from, int to);
void hook_nop(int from, int to);

class cHook
{
public:
    int from;
    int to;
    unsigned char oldCode[5];
    cHook(int from, int to);
    void hook();
    void unhook();
};

#endif