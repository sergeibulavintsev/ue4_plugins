#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemMetaPackage.h"
#include "HAL/ThreadSafeCounter.h"

typedef TSharedPtr<class FOnlineSessionMeta, ESPMode::ThreadSafe> FOnlineSessionMetaPtr;
typedef TSharedPtr<class FOnlineIdentityMeta, ESPMode::ThreadSafe> FOnlineIdentityMetaPtr;
typedef TSharedPtr<class FOnlineIdentityMeta, ESPMode::ThreadSafe> FOnlineIdentityMetaPtr;;

/**
*	OnlineSubsystemMeta - Implementation of the online subsystem for Meta services
*/
class ONLINESUBSYSTEMMETA_API FOnlineSubsystemMeta :
	public FOnlineSubsystemImpl
{

public:
	static const FName MetaSubsystem;
	virtual ~FOnlineSubsystemMeta()
	{
	}

	// IOnlineSubsystem

	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override { return nullptr; }
	virtual IOnlinePartyPtr GetPartyInterface() const override { return nullptr; }
	virtual IOnlineGroupsPtr GetGroupsInterface() const override { return nullptr; }
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override { return nullptr; }
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override { return nullptr; }
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override { return nullptr; }
	virtual IOnlineVoicePtr GetVoiceInterface() const override { return nullptr; }
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override { return nullptr; }
	virtual IOnlineTimePtr GetTimeInterface() const override { return nullptr; }
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override { return nullptr; }
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override { return nullptr; }
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineStorePtr GetStoreInterface() const override { return nullptr; }
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override { return nullptr; }
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override { return nullptr; }
	virtual IOnlineEventsPtr GetEventsInterface() const override { return nullptr; }
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override { return nullptr; }
	virtual IOnlineSharingPtr GetSharingInterface() const override { return nullptr; }
	virtual IOnlineUserPtr GetUserInterface() const override { return nullptr; }
	virtual IOnlineMessagePtr GetMessageInterface() const override { return nullptr; }
	virtual IOnlinePresencePtr GetPresenceInterface() const override { return nullptr; }
	virtual IOnlineChatPtr GetChatInterface() const override { return nullptr; }
	virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override { return nullptr; }

	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual FString GetAppId() const override;
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual FText GetOnlineServiceName() const override;

	// FTickerObjectBase

	virtual bool Tick(float DeltaTime) override;

	// FOnlineSubsystemMeta

	/**
	* Is the Meta API available for use
	* @return true if Meta functionality is available, false otherwise
	*/
	bool IsEnabled();

PACKAGE_SCOPE:
	const FString& GetLoginServiceAddress() { return LoginServiceAddress; }

	void QueueAsyncTask(class FOnlineAsyncTask* AsyncTask);
	void QueueAsyncOutgoingItem(class FOnlineAsyncItem* AsyncItem);
	void StartMatchMakerNotifications(const FString& EndPoint, int32 UserId, int32 SessionId);

	/** Only the factory makes instances */
	FOnlineSubsystemMeta(FName InInstanceName) :
		FOnlineSubsystemImpl(MetaSubsystem, InInstanceName),
		OnlineAsyncTaskThreadRunnable(nullptr),
		OnlineMatchMakerNotificationThreadRunnable(nullptr),
		OnlineAsyncTaskThread(nullptr),
		OnlineMatchMakerThread(nullptr)
	{}

	FOnlineSubsystemMeta() :
		OnlineAsyncTaskThreadRunnable(nullptr),
		OnlineMatchMakerNotificationThreadRunnable(nullptr),
		OnlineAsyncTaskThread(nullptr),
		OnlineMatchMakerThread(nullptr)
	{}
private:
	template <class InterfacePtr> void DestructInterface(InterfacePtr& Interface);
private:
	/** Interface to the session services */
	FOnlineSessionMetaPtr SessionInterface;

	FOnlineIdentityMetaPtr IdentityInterface;
	/** Online async task runnable */
	class FOnlineAsyncTaskManagerMeta* OnlineAsyncTaskThreadRunnable;
	class FOnlineMatchMakerNotificationMeta* OnlineMatchMakerNotificationThreadRunnable;

	/** Online async task thread */
	class FRunnableThread* OnlineAsyncTaskThread;
	class FRunnableThread* OnlineMatchMakerThread;

	// task counter, used to generate unique thread names for each task
	static FThreadSafeCounter TaskCounter;

	FString LoginServiceAddress;
};

typedef TSharedPtr<FOnlineSubsystemMeta, ESPMode::ThreadSafe> FOnlineSubsystemMetaPtr;

