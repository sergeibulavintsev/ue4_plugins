#pragma once
#include "CoreMinimal.h"
#include "OnlineNotification.h"
#include <memory>

namespace WGR3
{
	class MatchmakerNotificationsService;
	class PlayerNotification;
}

namespace grpc
{
	class Channel;
	class ClientContext;
	template<typename T> class ClientReader;
}

class SERVICEWRAPPERS_API FMatchMakerNotificationWrapper
{
public:
	FMatchMakerNotificationWrapper(const FString& InEndPoint);
	~FMatchMakerNotificationWrapper();
	bool SubscribeForNotifications(int32 UserId, int32 SessionId);
	bool ReceiveNotification(FOnlineNotification& OutNotification, FString& ErrorStr);
	void Stop();
private:
	void ConvertToOnlineNotification(const WGR3::PlayerNotification& InNotification, FOnlineNotification& OutOnlineNotification);

private:
	std::unique_ptr<grpc::ClientContext> Context;
	std::shared_ptr<grpc::Channel> RpcChannel;
	std::unique_ptr<grpc::ClientReader<WGR3::PlayerNotification>> NotificationReader;
};