#include "OnlineAsyncTaskManagerMeta.h"

void FOnlineAsyncTaskManagerMeta::OnlineTick()
{
	check(MetaSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId || !FPlatformProcess::SupportsMultithreading());
}
