#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FServiceWrappersModule
	: public IModuleInterface
{
public:
	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }
};

IMPLEMENT_MODULE(FServiceWrappersModule, ServiceWrappers);