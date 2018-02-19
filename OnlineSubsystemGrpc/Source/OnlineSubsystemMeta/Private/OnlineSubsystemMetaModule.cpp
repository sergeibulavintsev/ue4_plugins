// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "OnlineAsyncTaskManagerMeta.h"
#include "OnlineSubsystemMetaModule.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemMeta.h"

IMPLEMENT_MODULE(FOnlineSubsystemMetaModule, OnlineSubsystemMeta);

/**
* Class responsible for creating instance(s) of the subsystem
*/
class FOnlineFactoryMeta : public IOnlineFactory
{
public:

	FOnlineFactoryMeta() {}
	virtual ~FOnlineFactoryMeta() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemMetaPtr OnlineSub = MakeShareable(new FOnlineSubsystemMeta(InstanceName));
		if (OnlineSub->IsEnabled())
		{
			if (!OnlineSub->Init())
			{
				UE_LOG_ONLINE(Warning, TEXT("Meta API failed to initialize!"));
				OnlineSub->Shutdown();
				OnlineSub = NULL;
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("Meta API disabled!"));
			OnlineSub->Shutdown();
			OnlineSub = NULL;
		}
		UE_LOG_ONLINE(Log, TEXT("Online Meta created!"));
		return OnlineSub;
	}
};

void FOnlineSubsystemMetaModule::StartupModule()
{
	MetaFactory = new FOnlineFactoryMeta();

	// Create and register our singleton factory with the main online subsystem for easy access
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService(FOnlineSubsystemMeta::MetaSubsystem, MetaFactory);
}

void FOnlineSubsystemMetaModule::ShutdownModule()
{
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService(FOnlineSubsystemMeta::MetaSubsystem);

	delete MetaFactory;
	MetaFactory = NULL;
}
