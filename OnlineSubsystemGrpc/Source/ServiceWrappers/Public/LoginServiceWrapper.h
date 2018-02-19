#pragma once
#include "CoreMinimal.h"
#include <memory>

struct SERVICEWRAPPERS_API FAuthResult
{
	FAuthResult()
		: UserId(-1)
		, SessionId(-1)
	{}

	int32 UserId;
	int32 SessionId;
};

namespace WGR3
{
	class LoginService;
}
namespace grpc
{
	class Channel;
}

class SERVICEWRAPPERS_API FLoginServiceWrapper
{
public:
	FLoginServiceWrapper(const FString& InLoginServiceAddress);
	~FLoginServiceWrapper();
	bool Login(const FString& InUserName, const FString& InPassword, FAuthResult& OutUserInfo, FString& ErrorStr);
	bool QueueForLobby(int32 UserId, int32 SessionId, FString& OutLobbyEndPoint, FString& ErrorStr);

private:
	std::shared_ptr<grpc::Channel> RpcChannel;
};