/*
 * Hoicked from pyramid wxwindows example
 */
#ifndef __APPLE__ // Apple defines everything on his own

#if defined(_WIN32) || defined(__WIN32__)
    #ifndef WIN32_LEAN_AND_MEAN
        // Reduce a bit header VC++ compilation time
        #define WIN32_LEAN_AND_MEAN 1
        #define LE_ME_ISDEF
    #endif

    /*
    APIENTRY is defined in oglpfuncs.h as well as by windows.h. Undefine
    it to prevent a macro redefinition warning.
    */
    #undef APIENTRY
    #include <windows.h> //For wglGetProcAddress
    #ifdef LE_ME_ISDEF
        #undef WIN32_LEAN_AND_MEAN
        #undef LE_ME_ISDEF
    #endif
    // Our macro
    #define MyGetProcAddress(name) wglGetProcAddress((LPCSTR)name)
#else // Linux
    // GLX_ARB_get_proc_address
    // glXGetProcAddressARB is statically exported by all libGL implementations,
    // while glXGetProcAddress may be not available.
    #ifdef __cplusplus
        extern "C" {
    #endif
    extern void (*glXGetProcAddressARB(const GLubyte *procName))();
    #ifdef __cplusplus
        }
    #endif
    #define MyGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
#endif
#endif // APPLE

auto mygetprocaddress(const char* name)
{
    // Convert name into uname
    std::string name_str (name);
    int len = name_str.size();
    std::unique_ptr<GLubyte[]> uname = std::make_unique<GLubyte[]>(len + 1);
    for (int i = 0; i < len; ++i) {
        uname[i] = static_cast<GLubyte>(name_str[i]);
    }
    uname[len] = 0u;
    return MyGetProcAddress(uname.get());
}
