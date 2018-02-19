#include "CoreMinimal.h"
#include "MatchMakerServiceWrapper.h"
#include "Dom/JsonObject.h"

#include "GRPCUnrealHack.h"

#include "GeneratedProto/Common.pb.h"
#include "GeneratedProto/Common.grpc.pb.h"
#include "GeneratedProto/MatchmakerService.grpc.pb.h"
#include "GeneratedProto/MatchmakerService.pb.h"

FMatchMakerNotificationWrapper::FMatchMakerNotificationWrapper(const FString& InEndPoint)
	: RpcChannel(grpc::CreateChannel(TCHAR_TO_UTF8(*InEndPoint), grpc::InsecureChannelCredentials()))
	, Context(new grpc::ClientContext())
{}

FMatchMakerNotificationWrapper::~FMatchMakerNotificationWrapper()
{
	NotificationReader.reset();
}

bool FMatchMakerNotificationWrapper::SubscribeForNotifications(int32 UserId, int32 SessionId)
{
	try
	{
		auto rpcServiceStub = WGR3::MatchmakerNotificationsService::NewStub(RpcChannel);

		WGR3::UserSessionInfo userSessionInfo;
		userSessionInfo.set_userid(UserId);
		userSessionInfo.set_sessionid(SessionId);
		NotificationReader = rpcServiceStub->GetNotifications(Context.get(), userSessionInfo);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

void FMatchMakerNotificationWrapper::Stop()
{
	NotificationReader->Finish();
}

bool FMatchMakerNotificationWrapper::ReceiveNotification(FOnlineNotification& OutNotification, FString& ErrorStr)
{
	try
	{
		WGR3::PlayerNotification notification;
		if (!NotificationReader->Read(&notification))
		{
			return false;
		}
		ConvertToOnlineNotification(notification, OutNotification);
		return true;
	}
	catch (const std::exception& ex)
	{
		ErrorStr = UTF8_TO_TCHAR(ex.what());
		return false;
	}
}

void FMatchMakerNotificationWrapper::ConvertToOnlineNotification(const WGR3::PlayerNotification& InNotification, FOnlineNotification& OutOnlineNotification)
{
	if (InNotification.has_gameready())
	{
		auto& gameReadyNotification = InNotification.gameready();
		OutOnlineNotification.Payload = MakeShareable(new FJsonObject);
		OutOnlineNotification.TypeStr = TEXT("GameReady");
		OutOnlineNotification.Payload->SetStringField(TEXT("ArenaEndPoint"), UTF8_TO_TCHAR(gameReadyNotification.arenaendpoint().c_str()));
	}
}

#include "GRPCUnrealUnhack.h"
