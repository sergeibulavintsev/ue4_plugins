// GRPCUnrealUnhack.h MUST BE INCLUDED AT THE END OF SAME .cpp FILE

#ifndef UNREAL_GRPC_HACKS_IN_PLACE
#define UNREAL_GRPC_HACKS_IN_PLACE

#pragma warning(push)
#pragma warning(disable : 4800)
#pragma warning(disable : 4668)
#pragma warning(disable : 4005)
#include <AllowWindowsPlatformTypes.h>
#include <AllowWindowsPlatformAtomics.h>
#define MemoryBarrier  FGenericPlatformMisc::MemoryBarrier
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#endif