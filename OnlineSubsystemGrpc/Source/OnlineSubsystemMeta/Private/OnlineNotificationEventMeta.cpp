#include "OnlineAsyncTaskManagerMeta.h"
#include "OnlineNotificationEventMeta.h"
#include "OnlineSubsystemMeta.h"
#include "OnlineNotificationHandler.h"


FOnlinePlayerNotificationEventMeta::FOnlinePlayerNotificationEventMeta(FOnlineSubsystemMeta* InOnlineSubsystem, const FOnlineNotification& inNotification)
	: FOnlineAsyncEvent(InOnlineSubsystem)
	, MetaSubsystem(InOnlineSubsystem)
	, Notification(inNotification)
{
}

void FOnlinePlayerNotificationEventMeta::TriggerDelegates()
{
	FOnlineNotificationHandlerPtr OnlineNotificationHandler = MetaSubsystem->GetOnlineNotificationHandler();
	if (OnlineNotificationHandler.IsValid())
	{
		OnlineNotificationHandler->DeliverNotification(Notification);
	}
}