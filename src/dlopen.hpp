#ifndef _DLOPEN_HPP
#define _DLOPEN_HPP

struct dl_t;

#if defined(_MSC_VER)
#   define DL_EXTENSION ".dll"
#   define DL_EXTL 4
#elif defined(__GNUC__)
#   define DL_EXTENSION ".so"
#   define DL_EXTL 3
#endif

dl_t* dl_open(const char*);
void* dl_sym(dl_t*, const char*);
bool dl_close(dl_t*);

#endif /* _DLOPEN_HPP */
