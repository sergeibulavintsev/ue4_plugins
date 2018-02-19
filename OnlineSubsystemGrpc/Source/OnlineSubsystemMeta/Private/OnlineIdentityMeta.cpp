#include "OnlineAsyncTaskManagerMeta.h"
#include "OnlineIdentityMeta.h"
#include "Misc/CommandLine.h"
#include "Misc/Guid.h"
#include "Misc/OutputDeviceRedirector.h"
#include "OnlineSubsystemMeta.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"
#include "OnlineError.h"
#include "OnlineNotification.h"
#include "OnlineNotificationHandler.h"

FString FUserOnlineAccountMeta::GetRealName() const
{
	const FString* FoundAttr = UserAttributes.Find(TEXT("username"));
	if (FoundAttr != NULL)
	{
		return *FoundAttr;
	}
	return FString();
}

FString FUserOnlineAccountMeta::GetDisplayName(const FString& Platform) const
{
	return GetRealName();
}

bool FUserOnlineAccountMeta::GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = AdditionalAuthData.Find(AttrName);
	if (FoundAttr != NULL)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

bool FUserOnlineAccountMeta::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr != NULL)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}
	return false;
}

bool FUserOnlineAccountMeta::SetUserAttribute(const FString& AttrName, const FString& AttrValue)
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr == NULL || *FoundAttr != AttrValue)
	{
		UserAttributes.Add(AttrName, AttrValue);
		return true;
	}
	return false;
}

bool FOnlineIdentityMeta::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	FOnlineNotificationHandlerPtr NotificationHandler = MetaSubsystem->GetOnlineNotificationHandler();
	if (NotificationHandler.IsValid())
	{
		auto HandleOnlineNotificationSignature = FHandleOnlineNotificationSignature::CreateLambda([this, LocalUserNum](const FOnlineNotification& Notification) {
			Logout(LocalUserNum);
			return EOnlineNotificationResult::Handled;
		});
		OnlineNotificationsFailedDelegate = NotificationHandler->AddSystemNotificationBinding_Handle(TEXT("NotificationError"), HandleOnlineNotificationSignature);
	}

	FString ErrorStr;
	TSharedPtr<FUserOnlineAccountMeta> UserAccountPtr;
	// valid local player index
	if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
	{
		ErrorStr = FString::Printf(TEXT("Invalid LocalUserNum=%d"), LocalUserNum);
	}
	else if (AccountCredentials.Id.IsEmpty())
	{
		ErrorStr = TEXT("Invalid account id, string empty");
	}
	else
	{
		TSharedPtr<const FUniqueNetId>* UserId = UserIds.Find(LocalUserNum);
		if (UserId == NULL)
		{
			FOnUserLoginComplete OnUserLoginCompleteDelegate = FOnUserLoginComplete::CreateLambda([this, LocalUserNum](const FLoginResult& LoginResult, bool bWasSuccessful, const FString& ErrorStr)
			{
				if (bWasSuccessful)
				{
					UE_LOG_ONLINE(Warning, TEXT("OnUserLoginCompleteDelegate success"));
					FString OnlineUserId = FString::FromInt(LoginResult.UserId);
					FUniqueNetIdString NewUserId(OnlineUserId);
					TSharedPtr<FUserOnlineAccountMeta> UserAccountPtr = MakeShareable(new FUserOnlineAccountMeta(OnlineUserId));
					UserAccountPtr->UserAttributes.Add(TEXT("id"), OnlineUserId);
					UserAccountPtr->UserAttributes.Add(TEXT("sessionId"), FString::FromInt(LoginResult.SessionId));
					UserAccountPtr->UserAttributes.Add(TEXT("lobbyEndPoint"), LoginResult.LobbyEndPoint);
					
					// update/add cached entry for user
					UserAccounts.Add(NewUserId, UserAccountPtr.ToSharedRef());

					// keep track of user ids for local users
					UserIds.Add(LocalUserNum, UserAccountPtr->GetUserId());
					TriggerOnLoginCompleteDelegates(LocalUserNum, bWasSuccessful, *UserAccountPtr->GetUserId(), FString());
				}
				else
				{
					UE_LOG_ONLINE(Warning, TEXT("OnUserLoginCompleted: %s"), *ErrorStr);
					TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdString(), ErrorStr);
				}
			});
			FOnlineAsyncTaskLoginMeta* NewTask = new FOnlineAsyncTaskLoginMeta(MetaSubsystem, AccountCredentials.Id, AccountCredentials.Token, OnUserLoginCompleteDelegate);
			MetaSubsystem->QueueAsyncTask(NewTask);
		}
		else
		{
			const FUniqueNetIdString* UniqueIdStr = (FUniqueNetIdString*)(UserId->Get());
			TSharedRef<FUserOnlineAccountMeta>* TempPtr = UserAccounts.Find(*UniqueIdStr);
			check(TempPtr);
			UserAccountPtr = *TempPtr;
			TriggerOnLoginCompleteDelegates(LocalUserNum, true, *UserAccountPtr->GetUserId(), ErrorStr);
		}
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE(Warning, TEXT("Login request failed. %s"), *ErrorStr);
		TriggerOnLoginCompleteDelegates(LocalUserNum, false, FUniqueNetIdString(), ErrorStr);
		return false;
	}
	return true;
}

bool FOnlineIdentityMeta::Logout(int32 LocalUserNum)
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		// remove cached user account
		UserAccounts.Remove(FUniqueNetIdString(*UserId));
		// remove cached user id
		UserIds.Remove(LocalUserNum);
		// not async but should call completion delegate anyway
		TriggerOnLogoutCompleteDelegates(LocalUserNum, true);
		FOnlineNotificationHandlerPtr NotificationHandler = MetaSubsystem->GetOnlineNotificationHandler();
		if (NotificationHandler.IsValid())
		{
			NotificationHandler->RemoveSystemNotificationBinding(TEXT("NotificationError"), OnlineNotificationsFailedDelegate);
		}
		return true;
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("No logged in user found for LocalUserNum=%d."),
			LocalUserNum);
		TriggerOnLogoutCompleteDelegates(LocalUserNum, false);
	}
	return false;
}

bool FOnlineIdentityMeta::AutoLogin(int32 LocalUserNum)
{
	FString LoginStr;
	FString PasswordStr;
	FString TypeStr;

	FParse::Value(FCommandLine::Get(), TEXT("AUTH_LOGIN="), LoginStr);
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), PasswordStr);
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), TypeStr);

	if (!LoginStr.IsEmpty())
	{
		if (!PasswordStr.IsEmpty())
		{
			if (!TypeStr.IsEmpty())
			{
				return Login(0, FOnlineAccountCredentials(TypeStr, LoginStr, PasswordStr));
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("AutoLogin missing AUTH_TYPE=<type>."));
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("AutoLogin missing AUTH_PASSWORD=<password>."));
		}
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("AutoLogin missing AUTH_LOGIN=<login id>."));
	}
	return false;
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityMeta::GetUserAccount(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> Result;

	FUniqueNetIdString StringUserId(UserId);
	const TSharedRef<FUserOnlineAccountMeta>* FoundUserAccount = UserAccounts.Find(StringUserId);
	if (FoundUserAccount != NULL)
	{
		Result = *FoundUserAccount;
	}

	return Result;
}

TArray<TSharedPtr<FUserOnlineAccount> > FOnlineIdentityMeta::GetAllUserAccounts() const
{
	TArray<TSharedPtr<FUserOnlineAccount> > Result;

	for (TMap<FUniqueNetIdString, TSharedRef<FUserOnlineAccountMeta>>::TConstIterator It(UserAccounts); It; ++It)
	{
		Result.Add(It.Value());
	}

	return Result;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityMeta::GetUniquePlayerId(int32 LocalUserNum) const
{
	const TSharedPtr<const FUniqueNetId>* FoundId = UserIds.Find(LocalUserNum);
	if (FoundId != NULL)
	{
		return *FoundId;
	}
	return NULL;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityMeta::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	if (Bytes != NULL && Size > 0)
	{
		FString StrId(Size, (TCHAR*)Bytes);
		return MakeShareable(new FUniqueNetIdString(StrId));
	}
	return NULL;
}

TSharedPtr<const FUniqueNetId> FOnlineIdentityMeta::CreateUniquePlayerId(const FString& Str)
{
	return MakeShareable(new FUniqueNetIdString(Str));
}

ELoginStatus::Type FOnlineIdentityMeta::GetLoginStatus(int32 LocalUserNum) const
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetLoginStatus(*UserId);
	}
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityMeta::GetLoginStatus(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(UserId);
	if (UserAccount.IsValid() &&
		UserAccount->GetUserId()->IsValid())
	{
		return ELoginStatus::LoggedIn;
	}
	return ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityMeta::GetPlayerNickname(int32 LocalUserNum) const
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		return GetPlayerNickname(*UserId);
	}
	return FString();
}

FString FOnlineIdentityMeta::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(UserId);
	if (UserAccount.IsValid() && UserAccount->GetUserId()->IsValid())
	{
		return UserAccount->GetDisplayName();
	}
	return FString();
}

FString FOnlineIdentityMeta::GetAuthToken(int32 LocalUserNum) const
{
	TSharedPtr<const FUniqueNetId> UserId = GetUniquePlayerId(LocalUserNum);
	if (UserId.IsValid())
	{
		TSharedPtr<FUserOnlineAccount> UserAccount = GetUserAccount(*UserId);
		if (UserAccount.IsValid())
		{
			return UserAccount->GetAccessToken();
		}
	}
	return FString();
}

void FOnlineIdentityMeta::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	UE_LOG(LogOnline, Display, TEXT("FOnlineIdentityMeta::RevokeAuthToken not implemented"));
	TSharedRef<const FUniqueNetId> UserIdRef(UserId.AsShared());
	MetaSubsystem->ExecuteNextTick([UserIdRef, Delegate]()
	{
		Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
	});
}

FOnlineIdentityMeta::FOnlineIdentityMeta(FOnlineSubsystemMeta* InSubsystem)
	: MetaSubsystem(InSubsystem)
{
}

FOnlineIdentityMeta::~FOnlineIdentityMeta()
{
}

void FOnlineIdentityMeta::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EPrivilegeResults::NoFailures);
}

FPlatformUserId FOnlineIdentityMeta::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		auto CurrentUniqueId = GetUniquePlayerId(i);
		if (CurrentUniqueId.IsValid() && (*CurrentUniqueId == UniqueNetId))
		{
			return i;
		}
	}

	return PLATFORMUSERID_NONE;
}

FString FOnlineIdentityMeta::GetAuthType() const
{
	return TEXT("");
}
