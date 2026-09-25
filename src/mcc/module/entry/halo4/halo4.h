#pragma once

#include <halo4.h>
#include "mcc/module/entry/entry.h"

extern EntrySet g_pHalo4EntrySet;
inline EntrySet* Halo4EntrySet() {return &g_pHalo4EntrySet;};

// The hooks CModule installs for Halo 4. The ones above (engine, render, world) were never switched on
// upstream and their offsets are unchecked, so the split-screen hooks have a set of their own.
extern EntrySet g_Halo4SplitscreenEntrySet;
inline EntrySet* Halo4SplitscreenEntrySet() {return &g_Halo4SplitscreenEntrySet;};

#define Halo4Entry(name, offset, returnType, pDetour, ...) \
    returnType pDetour(__VA_ARGS__); \
    typedef returnType (*pDetour##_t)(...); \
    ::Entry name(Halo4EntrySet(), offset, pDetour); \
    returnType pDetour(__VA_ARGS__)
