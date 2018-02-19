#include "CoreMinimal.h"
#include "LobbyServiceWrapper.h"

#include "GRPCUnrealHack.h"

#include "GeneratedProto/Common.pb.h"
#include "GeneratedProto/Common.grpc.pb.h"
#include "GeneratedProto/LobbyService.grpc.pb.h"
#include "GeneratedProto/LobbyService.pb.h"


FLobbyServiceWrapper::FLobbyServiceWrapper(const FString& InLobbyEndPoint)
	: RpcChannel(grpc::CreateChannel(TCHAR_TO_UTF8(*InLobbyEndPoint), grpc::InsecureChannelCredentials()))
{
}

FLobbyServiceWrapper::~FLobbyServiceWrapper()
{
}

bool FLobbyServiceWrapper::LookForGame(int32 InUserId, FString& ErrorStr)
{
	try
	{
		WGR3::PlayerInfo playerInfo;
		playerInfo.set_userid(InUserId);

		grpc::Status status;
		WGR3::GenericResponse genericResponse;

		std::unique_ptr<WGR3::LobbyService::Stub> LobbyService = WGR3::LobbyService::NewStub(RpcChannel);
		{
			grpc::ClientContext context;
			status = LobbyService->LookForGame(&context, playerInfo, &genericResponse);
		}
		if (!status.ok())
		{
			ErrorStr = TCHAR_TO_UTF8(status.error_message().c_str());
			return false;
		}
			
		return genericResponse.param() == 0;
		return true;
	}
	catch (const std::exception& ex)
	{
		ErrorStr = TCHAR_TO_UTF8(ex.what());
		return false;
	}
}

bool FLobbyServiceWrapper::GetMatchmaker(int32 UserId, int32 SessionId, FString& OutMatchmakerEndPoint, FString& ErrorStr)
{
	try
	{
		WGR3::UserSessionInfo userSessionInfo;
		userSessionInfo.set_sessionid(SessionId);
		userSessionInfo.set_userid(UserId);

		WGR3::MatchmakerEndpoint matchMakerEndpoint;
		grpc::Status status;
		std::unique_ptr<WGR3::LobbyService::Stub> LobbyService = WGR3::LobbyService::NewStub(RpcChannel);
		{
			grpc::ClientContext context;
			status = LobbyService->GetMatchmaker(&context, userSessionInfo, &matchMakerEndpoint);
		}
		if (!status.ok())
		{
			ErrorStr = TCHAR_TO_UTF8(status.error_message().c_str());
			return false;
		}
		if (matchMakerEndpoint.param() != 0)
		{
			ErrorStr = TEXT("Server returned code: ") + FString::FromInt(matchMakerEndpoint.param());
			return false;
		}
		OutMatchmakerEndPoint = UTF8_TO_TCHAR(matchMakerEndpoint.matchmakerendpoint().c_str());
		return true;
	}
	catch (const std::exception& ex)
	{
		ErrorStr = TCHAR_TO_UTF8(ex.what());
		return false;
	}
	catch (...)
	{
		ErrorStr = TEXT("Unknown error");
		return false;
	}
}

FLobbyServiceNotificationWrapper::FLobbyServiceNotificationWrapper(const FString& InLobbyEndPoint)
	: LobbyEndPoint(InLobbyEndPoint)
{}

FLobbyServiceNotificationWrapper::~FLobbyServiceNotificationWrapper()
{
}

bool FLobbyServiceNotificationWrapper::SubscribeForNotifications(int32 UserId, int32 SessionId)
{
	try
	{
		std::string LobbyServiceAddress = TCHAR_TO_UTF8(*LobbyEndPoint);
		RpcChannel = grpc::CreateChannel(LobbyServiceAddress, grpc::InsecureChannelCredentials());
		auto rpcServiceStub = WGR3::LobbyNotificationsService::NewStub(RpcChannel);

		WGR3::UserSessionInfo userSessionInfo;
		userSessionInfo.set_userid(UserId);
		userSessionInfo.set_sessionid(SessionId);
		grpc::ClientContext notificationContext;
		NotificationReader = rpcServiceStub->GetNotifications(&notificationContext, userSessionInfo);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

void FLobbyServiceNotificationWrapper::Stop()
{ 
	NotificationReader->Finish();
}

bool FLobbyServiceNotificationWrapper::ReceiveNotification(struct FOnlineNotification& OutNotification)
{
	WGR3::PlayerNotification notification;
	return NotificationReader->Read(&notification);
	return true;
}

#include "GRPCUnrealUnhack.h"
