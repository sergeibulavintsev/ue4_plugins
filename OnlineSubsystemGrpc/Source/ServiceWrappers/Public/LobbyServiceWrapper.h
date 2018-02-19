#pragma once
#include "CoreMinimal.h"
#include "Misc/AssertionMacros.h"
#include <memory>


namespace WGR3
{
	class LobbyNotificationsService;
	class LobbyService;
	class PlayerNotification;
}
namespace grpc
{
	class Channel;
	template<typename T> class ClientReader;
}

class SERVICEWRAPPERS_API FLobbyServiceWrapper
{
public:
	FLobbyServiceWrapper(const FString& InLobbyEndPoint);
	~FLobbyServiceWrapper();
	bool LookForGame(int32 UserId, FString& ErrorStr);
	bool GetMatchmaker(int32 UserId, int32 SessionId, FString& OutMatchmakerEndPoint, FString& ErrorStr);
private:
	std::shared_ptr<grpc::Channel> RpcChannel;
};

class SERVICEWRAPPERS_API FLobbyServiceNotificationWrapper
{
public:
	FLobbyServiceNotificationWrapper(const FString& InLobbyEndPoint);
	~FLobbyServiceNotificationWrapper();

	bool SubscribeForNotifications(int32 UserId, int32 SessionId);
	bool ReceiveNotification(struct FOnlineNotification& OutNotification);
	void Stop();

private:
	FString LobbyEndPoint;
	std::shared_ptr<grpc::Channel> RpcChannel;
	std::unique_ptr<grpc::ClientReader<WGR3::PlayerNotification>> NotificationReader;
};