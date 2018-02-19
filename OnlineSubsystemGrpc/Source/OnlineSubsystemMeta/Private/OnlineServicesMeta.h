#pragma once
#include "CoreMinimal.h"
#include "OnlineDelegateMacros.h"
#include "LoginServiceWrapper.h"
#include "OnlineAsyncTaskManagerMeta.h"
#include "OnlineSubsystemMeta.h"

class FOnlineAccountCredentials;

struct FLoginResult
{
	int32 UserId;
	int32 SessionId;
	FString LobbyEndPoint;
	FString MatchMakerEndPoint;
};

DECLARE_DELEGATE_ThreeParams(FOnUserLoginComplete, const FLoginResult& /*LoginResult*/, bool /*bWasSuccessful*/, const FString& /*ErrorStr*/);


class FOnlineAsyncTaskLoginMeta : public FOnlineAsyncTaskBasic<class FOnlineSubsystemMeta>
{
private:
	class FOnlineSubsystemMeta* MetaSubsystem;
	FString LoginName;
	FString Password;
	FLoginResult LoginResult;
	FString ErrorStr;

	FOnUserLoginComplete Delegate;

	FOnlineAsyncTaskLoginMeta()
	{
	}

	void OnFailed();
public:

	FOnlineAsyncTaskLoginMeta(class FOnlineSubsystemMeta* InMetaSubsystem, const FString& Username, const FString& Password, const FOnUserLoginComplete& Delegate);

	virtual FString ToString(void) const override
	{
		return TEXT("FOnlineIdenityMeta::Login() response for player login");
	}
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
};

class FStartMatchMaking : public FOnlineAsyncTaskBasic<class FOnlineSubsystemMeta>
{
private:
	int32 UserId;
	FString LobbyEndPoint;
public:
	FStartMatchMaking(class FOnlineSubsystemMeta* InMetaSubsystem, int32 InUserId, const FString& InLobbyEndPoint);

	virtual FString ToString(void) const override
	{
		return TEXT("FStartMatchMaking::LookForGame() response for player matchmaking");
	}
	virtual void Tick() override;
};