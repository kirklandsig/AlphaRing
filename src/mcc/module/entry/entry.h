#pragma once

#include "common.h"

class EntrySet;

struct Entry {
public:
    Entry(EntrySet* set, __int64 offset, void* pDetour);

    bool update(__int64 hModule);
    void remove();

    __int64 m_offset;
    void* m_pOriginal;
    __int64 m_target;
    void* m_pDetour;
};

class EntrySet {
public:
    void append(Entry* entry);
    bool update(__int64 hModule);
    // Unhook while the module is still mapped: MCC loads every game DLL at the menu and
    // reloads them at new addresses, so a hook left behind would later be "restored" into
    // whatever DLL occupies its old address.
    void remove();

private:
    inline static const int MAX_ENTRY = 20;
    int entryCount;
    Entry* entryArray[MAX_ENTRY];

};
