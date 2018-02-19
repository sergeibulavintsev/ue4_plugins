#include "CoreMinimal.h"
#include "LoginServiceWrapper.h"

#include "GRPCUnrealHack.h"

#include "GeneratedProto/Common.pb.h"
#include "GeneratedProto/Common.grpc.pb.h"
#include "GeneratedProto/LoginService.grpc.pb.h"
#include "GeneratedProto/LoginService.pb.h"
#include <memory>

FLoginServiceWrapper::FLoginServiceWrapper(const FString& InLoginServiceAddress)
	: RpcChannel(grpc::CreateChannel(TCHAR_TO_UTF8(*InLoginServiceAddress), grpc::InsecureChannelCredentials()))
{}

FLoginServiceWrapper::~FLoginServiceWrapper()
{
}

bool FLoginServiceWrapper::Login(const FString& InUserName, const FString& InPassword, FAuthResult& OutAuthResult, FString& ErrorStr)
{
	try
	{
		std::unique_ptr<WGR3::LoginService::Stub> loginServiceStub = WGR3::LoginService::NewStub(RpcChannel);

		grpc::Status status;

		WGR3::LoginInfo loginInfo;
		loginInfo.set_username(std::string(TCHAR_TO_UTF8(*InUserName)));
		WGR3::AuthChallenge authChallenge;
		{
			grpc::ClientContext context;
			status = loginServiceStub->UserLogin(&context, loginInfo, &authChallenge);
			if (!status.ok())
			{
				ErrorStr = UTF8_TO_TCHAR(status.error_message().c_str());
				return false;
			}
			if (authChallenge.param() != 0)
			{
				ErrorStr = TEXT("Server returned error code: ") + FString::FromInt(authChallenge.param());
			}
		}
		authChallenge.set_challengestring("SameOldChallenge");
		authChallenge.set_userid(authChallenge.userid());
		WGR3::AuthResult authResult;
		{
			grpc::ClientContext context;
			status = loginServiceStub->UserAuth(&context, authChallenge, &authResult);
			if (!status.ok())
			{
				ErrorStr = UTF8_TO_TCHAR(status.error_message().c_str());
				return false;
			}
			if (authResult.param() != 0)
			{
				ErrorStr = TEXT("Server returned error code: ") + FString::FromInt(authResult.param());
			}
		}
		OutAuthResult.UserId = authResult.userid();
		OutAuthResult.SessionId = authResult.sessionid();
		return true;
	}
	catch (const std::exception& ex)
	{
		ErrorStr = UTF8_TO_TCHAR(ex.what());
		return false;
	}
}

bool FLoginServiceWrapper::QueueForLobby(int32 UserId, int32 SessionId, FString& OutLobbyEndPoint, FString& ErrorStr)
{
	try
	{
		std::unique_ptr<WGR3::LoginService::Stub> loginServiceStub = WGR3::LoginService::NewStub(RpcChannel);
		WGR3::LoginQueueResult loginQueueResult;
		WGR3::AuthResult authResult;
		authResult.set_userid(UserId);
		authResult.set_sessionid(SessionId);
		grpc::Status status;
		{
			grpc::ClientContext context;
			status = loginServiceStub->QueueForLobby(&context, authResult, &loginQueueResult);
		}
		if (!status.ok())
		{
			ErrorStr = UTF8_TO_TCHAR(status.error_message().c_str());
			return false;
		}
		if (loginQueueResult.param() != 0)
		{
			ErrorStr = TEXT("Server returned error code: ") + FString::FromInt(loginQueueResult.param());
			return false;
		}
		OutLobbyEndPoint = UTF8_TO_TCHAR(loginQueueResult.lobbyserviceendpoint().c_str());
		return true;
	}
	catch (const std::exception& ex)
	{
		ErrorStr = UTF8_TO_TCHAR(ex.what());
		return false;
	}
}

#include "GRPCUnrealUnhack.h"
