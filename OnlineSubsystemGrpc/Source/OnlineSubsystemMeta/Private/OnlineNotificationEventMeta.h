#pragma once
#include "CoreMinimal.h"
#include "OnlineDelegateMacros.h"
#include "OnlineAsyncTaskManager.h"
#include "OnlineNotification.h"

class FOnlinePlayerNotificationEventMeta : public FOnlineAsyncEvent<class FOnlineSubsystemMeta>
{
public:
	FOnlinePlayerNotificationEventMeta(FOnlineSubsystemMeta* InOnlineSubsystem, const FOnlineNotification& inNotification);

	virtual ~FOnlinePlayerNotificationEventMeta()
	{
	}

	virtual FString ToString(void) const override
	{
		return TEXT("FOnlinePlayerNotificationEventMeta Player Notification event");
	}

	virtual void TriggerDelegates() override;

private:
	FOnlineSubsystemMeta* MetaSubsystem;
	FOnlineNotification Notification;
};