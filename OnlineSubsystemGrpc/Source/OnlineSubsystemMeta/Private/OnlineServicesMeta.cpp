#include "OnlineAsyncTaskManagerMeta.h"
#include "OnlineServicesMeta.h"
#include "OnlineSubsystemMeta.h"
#include "LobbyServiceWrapper.h"
#include <string>

FOnlineAsyncTaskLoginMeta::FOnlineAsyncTaskLoginMeta(FOnlineSubsystemMeta* InMetaSubsystem, const FString& Username, const FString& Password, const FOnUserLoginComplete& Delegate) :
	FOnlineAsyncTaskBasic(InMetaSubsystem),
	MetaSubsystem(InMetaSubsystem),
	LoginName(Username),
	Password(Password),
	Delegate(Delegate)
{
}

void FOnlineAsyncTaskLoginMeta::OnFailed()
{
	bIsComplete = true;
	bWasSuccessful = false;
}

void FOnlineAsyncTaskLoginMeta::Tick()
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineAsyncTaskLoginMeta logging to server"));

	FLoginServiceWrapper LoginService(TCHAR_TO_UTF8(*Subsystem->GetLoginServiceAddress()));
	FAuthResult AuthResult;
	if (!LoginService.Login(LoginName, Password, AuthResult, ErrorStr))
	{
		UE_LOG_ONLINE(Warning, TEXT("LoginService.Login"));
		OnFailed();
		return;
	}
	UE_LOG_ONLINE(Warning, TEXT("Queueing for lobby, user id is %d"), AuthResult.UserId);
	FString LobbyEndPoint;
	if (!LoginService.QueueForLobby(AuthResult.UserId, AuthResult.SessionId, LobbyEndPoint, ErrorStr))
	{
		UE_LOG_ONLINE(Warning, TEXT("LoginService.QueueForLobby"));
		OnFailed();
		return;
	}
	UE_LOG_ONLINE(Warning, TEXT("Lobby address is %s"), *LobbyEndPoint);
	FLobbyServiceWrapper LobbyService(LobbyEndPoint);
	FString MatchmakerEndPoint;
	if (!LobbyService.GetMatchmaker(AuthResult.UserId, AuthResult.SessionId, MatchmakerEndPoint, ErrorStr))
	{
		UE_LOG_ONLINE(Warning, TEXT("LobbyService.GetMatchmaker"));
		OnFailed();
		return;
	}
	UE_LOG_ONLINE(Warning, TEXT("Matchmaker address is %s"), *MatchmakerEndPoint);
	LoginResult.UserId = AuthResult.UserId;
	LoginResult.SessionId = AuthResult.SessionId;
	LoginResult.LobbyEndPoint = LobbyEndPoint;
	LoginResult.MatchMakerEndPoint = MatchmakerEndPoint;
	bIsComplete = true;
	bWasSuccessful = true;
}

void FOnlineAsyncTaskLoginMeta::TriggerDelegates()
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineAsyncTaskLoginMeta::TriggerDelegates()"));
	MetaSubsystem->StartMatchMakerNotifications(LoginResult.MatchMakerEndPoint, LoginResult.UserId, LoginResult.SessionId);
	Delegate.ExecuteIfBound(LoginResult, bWasSuccessful, ErrorStr);
}


FStartMatchMaking::FStartMatchMaking(FOnlineSubsystemMeta* InMetaSubsystem, int32 InUserId, const FString& InLobbyEndPoint)
	: FOnlineAsyncTaskBasic(InMetaSubsystem)
	, UserId(InUserId)
	, LobbyEndPoint(InLobbyEndPoint)
{
}

void FStartMatchMaking::Tick()
{
	UE_LOG_ONLINE(Warning, TEXT("Sending request to look for game"));
	FString ErrorStr;
	FLobbyServiceWrapper LobbyService(LobbyEndPoint);
	bWasSuccessful = LobbyService.LookForGame(UserId, ErrorStr);
	if (!bWasSuccessful)
	{
		UE_LOG_ONLINE(Warning, TEXT("FLobbyService.LookForGame with error: %s"), *ErrorStr);
	}
	bIsComplete = true;
}