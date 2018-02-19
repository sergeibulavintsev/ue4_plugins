#include "OnlineAsyncTaskManagerMeta.h"
#include "OnlineSubsystemMeta.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineSubsystemMetaModule.h"
#include "HAL/RunnableThread.h"
#include "OnlineNotificationHandler.h"

#include "OnlineSessionInterfaceMeta.h"
#include "OnlineIdentityMeta.h"
#include "OnlineNotificationHandlerMeta.h"


FThreadSafeCounter FOnlineSubsystemMeta::TaskCounter;
const FName FOnlineSubsystemMeta::MetaSubsystem = TEXT("META");


IOnlineSessionPtr FOnlineSubsystemMeta::GetSessionInterface() const
{
	return SessionInterface;
}

IOnlineIdentityPtr FOnlineSubsystemMeta::GetIdentityInterface() const
{
	return IdentityInterface;
}

bool FOnlineSubsystemMeta::Tick(float DeltaTime)
{
	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		OnlineAsyncTaskThreadRunnable->GameTick();
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->Tick(DeltaTime);
	}

	return true;
}

bool FOnlineSubsystemMeta::Init()
{
	GConfig->GetString(TEXT("OnlineSubsystemMeta"), TEXT("LoginServiceAddress"), LoginServiceAddress, GEngineIni);

	// Create the online async task thread
	OnlineAsyncTaskThreadRunnable = new FOnlineAsyncTaskManagerMeta(this);
	check(OnlineAsyncTaskThreadRunnable);
	OnlineAsyncTaskThread = FRunnableThread::Create(OnlineAsyncTaskThreadRunnable, *FString::Printf(TEXT("OnlineAsyncTaskThreadMeta %s(%d)"),
		*InstanceName.ToString(), TaskCounter.Increment()), 128 * 1024, TPri_Normal);
	check(OnlineAsyncTaskThread);

	SessionInterface = MakeShareable(new FOnlineSessionMeta(this));
	IdentityInterface = MakeShareable(new FOnlineIdentityMeta(this));
	OnlineNotificationHandler = MakeShareable(new FOnlineNotificationHandler());
	return true;
}

bool FOnlineSubsystemMeta::Shutdown()
{
	FOnlineSubsystemImpl::Shutdown();
	if (OnlineAsyncTaskThread)
	{
		// Destroy the online async task thread
		delete OnlineAsyncTaskThread;
		OnlineAsyncTaskThread = nullptr;
	}

	if (OnlineMatchMakerThread)
	{
		delete OnlineMatchMakerThread;
		OnlineMatchMakerThread = nullptr;
	}

	if (OnlineAsyncTaskThreadRunnable)
	{
		delete OnlineAsyncTaskThreadRunnable;
		OnlineAsyncTaskThreadRunnable = nullptr;
	}

	if (OnlineMatchMakerNotificationThreadRunnable)
	{
		delete OnlineMatchMakerNotificationThreadRunnable;
		OnlineMatchMakerNotificationThreadRunnable = nullptr;
	}

	// Destruct the interfaces
	DestructInterface(IdentityInterface);
	DestructInterface(SessionInterface);
	DestructInterface(OnlineNotificationHandler);
	return true;
}

FString FOnlineSubsystemMeta::GetAppId() const
{
	return TEXT("");
}

bool FOnlineSubsystemMeta::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FOnlineSubsystemImpl::Exec(InWorld, Cmd, Ar))
	{
		return true;
	}
	return false;
}

FText FOnlineSubsystemMeta::GetOnlineServiceName() const
{
	return NSLOCTEXT("OnlineSubsystemMeta", "OnlineServiceName", "Meta");
}

bool FOnlineSubsystemMeta::IsEnabled()
{
	return true;
}

template <class InterfacePtr>
void FOnlineSubsystemMeta::DestructInterface(InterfacePtr& Interface)
{
	if (Interface.IsValid())
	{
		ensure(Interface.IsUnique());
		Interface = nullptr;
	}
}

void FOnlineSubsystemMeta::QueueAsyncTask(FOnlineAsyncTask* AsyncTask)
{
	check(OnlineAsyncTaskThreadRunnable);
	OnlineAsyncTaskThreadRunnable->AddToInQueue(AsyncTask);
}

void FOnlineSubsystemMeta::QueueAsyncOutgoingItem(FOnlineAsyncItem* AsyncItem)
{
	check(OnlineAsyncTaskThreadRunnable);
	OnlineAsyncTaskThreadRunnable->AddToOutQueue(AsyncItem);
}

void FOnlineSubsystemMeta::StartMatchMakerNotifications(const FString& EndPoint, int32 UserId, int32 SessionId)
{
	check(!OnlineMatchMakerNotificationThreadRunnable);
	OnlineMatchMakerNotificationThreadRunnable = new FOnlineMatchMakerNotificationMeta(this, UserId, SessionId, EndPoint);
	check(OnlineMatchMakerNotificationThreadRunnable);
	OnlineMatchMakerThread = FRunnableThread::Create(OnlineMatchMakerNotificationThreadRunnable, *FString::Printf(TEXT("OnlineAsyncTaskThreadMeta %s(%d)"),
		*InstanceName.ToString(), TaskCounter.Increment()), 128 * 1024, TPri_Normal);
	check(OnlineMatchMakerThread);
}