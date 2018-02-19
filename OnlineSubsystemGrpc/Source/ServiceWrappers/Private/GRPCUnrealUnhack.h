// GRPCUnrealHack.h MUST BE INCLUDED AT THE BEGINNING OF SAME .cpp FILE

#ifdef UNREAL_GRPC_HACKS_IN_PLACE
#undef UNREAL_GRPC_HACKS_IN_PLACE

#undef MemoryBarrier
#define check(expr) { if(UNLIKELY(!(expr))) { FDebug::LogAssertFailedMessage( #expr, __FILE__, __LINE__, TEXT("") ); _DebugBreakAndPromptForRemote(); FDebug::AssertFailed( #expr, __FILE__, __LINE__ ); CA_ASSUME(false); } }
#include <HideWindowsPlatformAtomics.h>
#include <HideWindowsPlatformTypes.h>
#pragma warning(pop)

#endif