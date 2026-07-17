#pragma once

#include <locale>
#include <codecvt>
#include <string>

#define STR_CPY(return_type, name, ...) inline return_type name(__VA_ARGS__) { \
    size_t i = 0; \
    for (; i < n; i++) { \
        dest[i] = src[i]; \
        if (src[i] == 0) break; \
    } \
    return i; \
} \

namespace String {
    STR_CPY(size_t, wstrcpy, wchar_t *dest, const wchar_t *src, size_t n);
    STR_CPY(size_t, wstrcpy, wchar_t *dest, const char *src, size_t n);
    STR_CPY(size_t, strcpy, char *dest, const char *src, size_t n);
    STR_CPY(size_t, strcpy, char *dest, const wchar_t *src, size_t n);

    template <size_t _Size> inline size_t wstrcpy(wchar_t (&dest)[_Size], const wchar_t *src) {return wstrcpy(dest, src, _Size);}
    template <size_t _Size> inline size_t wstrcpy(wchar_t (&dest)[_Size], const char *src) {return wstrcpy(dest, src, _Size);}
    template <size_t _Size> inline size_t strcpy(char (&dest)[_Size], const char *src) {return strcpy(dest, src, _Size);}
    template <size_t _Size> inline size_t strcpy(char (&dest)[_Size], const wchar_t *src) {return strcpy(dest, src, _Size);}

    void convert(char* dest, const wchar_t* src, size_t n);
    void convert(wchar_t* dest, const char* src, size_t n);

    // Fixed-width wchar_t fields (e.g. CUserProfile::ServiceTag[4]) hold up to
    // exactly N code units with NO null terminator; a full field is legal.
    // These treat the array as exactly-N: no terminator slot is reserved (the
    // generic converters above would truncate an N-char value to N-1), and
    // reads never walk past the array.
    template <size_t N> inline std::string fixedToNarrow(const wchar_t (&src)[N]) {
        char buf[N];
        size_t i = 0;
        for (; i < N && src[i]; ++i) buf[i] = static_cast<char>(src[i]);
        return std::string(buf, i);
    }
    template <size_t N> inline void narrowToFixed(wchar_t (&dst)[N], const std::string& src) {
        size_t i = 0;
        for (; i < N && i < src.size(); ++i) dst[i] = static_cast<unsigned char>(src[i]);
        for (; i < N; ++i) dst[i] = 0;
    }
}
