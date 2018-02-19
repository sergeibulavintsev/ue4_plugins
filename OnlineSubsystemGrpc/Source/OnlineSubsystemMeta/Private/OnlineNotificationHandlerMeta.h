#pragma once
#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "MatchMakerServiceWrapper.h"
#include "OnlineNotificationEventMeta.h"
#include "OnlineSubsystemMeta.h"

template<class T>
class FOnlineNotificationHandlerMeta : public FRunnable, FSingleThreadRunnable
{
public:
	FOnlineNotificationHandlerMeta(class FOnlineSubsystemMeta* InOnlineSubsystem, int32 InUserId, int32 InSessionId, const FString& InEndPoint)
		: MetaSubsystem(InOnlineSubsystem)
		, UserId(InUserId)
		, SessionId(InSessionId)
		, NotificationService(InEndPoint)
		, NotificationThreadId(0)
	{
	}
	virtual bool Init() override 
	{
		return true; 
	}

	virtual uint32 Run() override
	{
		check(NotificationThreadId == 0);
		FPlatformAtomics::InterlockedExchange((volatile int32*)&NotificationThreadId, FPlatformTLS::GetCurrentThreadId());

		if (!NotificationService.SubscribeForNotifications(UserId, SessionId))
		{
			UE_LOG_ONLINE(Warning, TEXT("NotificationService.SubscribeForNotification has failed"));
			FOnlineNotification OnlineNotification;
			OnlineNotification.TypeStr = TEXT("NotificationError");
			auto NewEvent = new FOnlinePlayerNotificationEventMeta(MetaSubsystem, OnlineNotification);
			MetaSubsystem->QueueAsyncOutgoingItem(NewEvent);
			return 0;
		}
		do
		{
			if (!bRequestingExit)
			{
				Tick();
			}
		} while (!bRequestingExit);
		return 0;
	}

	virtual void Tick() override
	{
		FOnlineNotification OnlineNotification;
		FString ErrorStr;
		if (!NotificationService.ReceiveNotification(OnlineNotification, ErrorStr))
		{
			UE_LOG_ONLINE(Warning, TEXT("NotificationService.ReceiveNotification has failed with error %s"), *ErrorStr);
			NotificationThreadId = 0;
			bRequestingExit = true;
			return;
		}
		auto NewEvent = new FOnlinePlayerNotificationEventMeta(MetaSubsystem, OnlineNotification);
		MetaSubsystem->QueueAsyncOutgoingItem(NewEvent);
	}

	virtual void Stop() override
	{
		bRequestingExit = true;
		NotificationService.Stop();
	}

	virtual void Exit() override
	{
		NotificationThreadId = 0;
	}

	virtual ~FOnlineNotificationHandlerMeta() {}
	virtual FSingleThreadRunnable* GetSingleThreadInterface() { return this; }

private:
	class FOnlineSubsystemMeta* MetaSubsystem;

	FEvent* StartEvent;
	int32 UserId;
	int32 SessionId;

	T NotificationService;
	FThreadSafeBool bRequestingExit;
	volatile uint32 NotificationThreadId;
};

class FOnlineMatchMakerNotificationMeta : public FOnlineNotificationHandlerMeta<FMatchMakerNotificationWrapper>
{
public:
	FOnlineMatchMakerNotificationMeta(class FOnlineSubsystemMeta* InOnlineSubsystem, int32 InUserId, int32 InSessionId, const FString& InEndPoint)
		: FOnlineNotificationHandlerMeta(InOnlineSubsystem, InUserId, InSessionId, InEndPoint)
	{
	}
	virtual ~FOnlineMatchMakerNotificationMeta() {}
};