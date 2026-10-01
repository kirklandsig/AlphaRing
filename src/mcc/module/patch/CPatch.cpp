#include "CPatch.h"
#include "CPatchSet.h"
#include <cstring>
#include <Windows.h>

bool CPatch::apply(void *dst, const void *src, size_t size)  {
    bool result = false;
    DWORD oldprotect;

    if (dst == nullptr || src == nullptr || size == 0)
        return result;

    if (VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect)) {
        memcpy(dst, src, size);
        result = true;
    }
    VirtualProtect(dst, size, oldprotect, &oldprotect);

    return result;
}

// Copies unless the source can't be read (a patch.xml offset outside the module).
static bool ReadBytes(void* dst, const void* src, size_t size) {
    __try {
        memcpy(dst, src, size);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void CPatch::capture() {
    m_captured = m_parent->moduleAddress() != 0 &&
                 ReadBytes(m_backup.data(), (const void*)(m_parent->moduleAddress() + m_offset), m_backup.size());
}

bool CPatch::setState(bool state) {
    if (m_enabled == state) return false;
    m_enabled = state;
    bool result = apply();
    // Restoring the module's bytes also undid any other enabled patch overlapping this one.
    if (!state) m_parent->apply();
    return result;
}

bool CPatch::apply()  {
    if (!m_captured) return false; // module not loaded (applied on load), or the patch is outside it
    // m_backup holds the module's own bytes, captured once when it loaded (CPatchSet::update),
    // so enabling and disabling can each be repeated safely in any order - e.g. a saved state
    // restored before the first apply, then CPatchSet::apply() sweeping every enabled patch.
    auto dst = (void*)(m_parent->moduleAddress() + m_offset);
    return m_enabled ? apply(dst, m_data.data(), m_data.size()) : apply(dst, m_backup.data(), m_backup.size());
}
