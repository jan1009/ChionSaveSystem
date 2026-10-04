#pragma once

#include "Modules/ModuleManager.h"


class FChionSaveSystemEditorModule
	: public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};