export module Common:Debug;
#ifdef NDEBUG
export constexpr bool InDebug = false;
#else
export constexpr bool InDebug = true;
#endif
