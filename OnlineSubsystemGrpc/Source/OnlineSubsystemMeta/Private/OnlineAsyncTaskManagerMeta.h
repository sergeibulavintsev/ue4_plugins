#pragma once

#include "CoreMinimal.h"
#include "OnlineAsyncTaskManager.h"

class FOnlineSubsystemMeta;
/**
*	Meta version of the async task manager to register the various Meta callbacks with the engine
*/
class FOnlineAsyncTaskManagerMeta : public FOnlineAsyncTaskManager
{
protected:
	/** Cached reference to the main online subsystem */
	FOnlineSubsystemMeta* MetaSubsystem;

public:

	FOnlineAsyncTaskManagerMeta(FOnlineSubsystemMeta* InOnlineSubsystem)
		: MetaSubsystem(InOnlineSubsystem)
	{
	}

	~FOnlineAsyncTaskManagerMeta()
	{
	}

	// FOnlineAsyncTaskManager
	virtual void OnlineTick() override;
};
