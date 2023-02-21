#ifdef __ANDROID__

#if defined(__ARM_ARCH_7A__)
    #define FEATURE __attribute__((target("arch=armeabi-v7a")))
#else
    #define FEATURE 
#endif

FEATURE void *_Unwind_Resume(){};
// if link c++_shared , only above needed; or if c++_static, below required
FEATURE void *_Unwind_DeleteException(){};
FEATURE void *_Unwind_SetGR(){};
FEATURE void *_Unwind_GetIPInfo(){};
FEATURE void *_Unwind_SetIP(){};
FEATURE void *_Unwind_GetIP(){};
FEATURE void *_Unwind_GetLanguageSpecificData(){};
FEATURE void *_Unwind_GetRegionStart(){};
FEATURE void *_Unwind_GetDataRelBase(){};
FEATURE void *_Unwind_GetTextRelBase(){};
FEATURE void *_Unwind_RaiseException(){};
FEATURE void *_Unwind_Resume_or_Rethrow(){};
FEATURE void *_Unwind_VRS_Get(){};
FEATURE void *_Unwind_VRS_Set(){};
FEATURE void *__gnu_unwind_frame(){};
#endif