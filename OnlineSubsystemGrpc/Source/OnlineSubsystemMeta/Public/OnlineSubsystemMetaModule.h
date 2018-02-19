#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "OnlineSubsystemMetaPackage.h"

class FOnlineSubsystemMetaModule : public IModuleInterface
{
private:

	class FOnlineFactoryMeta* MetaFactory;

public:
	FOnlineSubsystemMetaModule() :
		MetaFactory(NULL)
	{}

	virtual ~FOnlineSubsystemMetaModule() {}

	// IModuleInterface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
};