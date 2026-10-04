#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ChionSaveSystemEditorSettings.generated.h"


UCLASS(
	Config = EditorPerProjectUserSettings,
	meta = (DisplayName = "Chion Save System")
)
class CHIONSAVESYSTEMEDITOR_API UChionSaveSystemEditorSettings
	: public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UChionSaveSystemEditorSettings();


	/**
	 * If enabled, ChionSaveSystem checks GitHub once when
	 * the Unreal Editor starts for a newer release.
	 *
	 * No telemetry or project data is transmitted.
	 */
	UPROPERTY(
		Config,
		EditAnywhere,
		Category = "Updates",
		meta = (
			DisplayName = "Check for Updates",
			ToolTip = "Checks GitHub once when the Unreal Editor starts for a newer ChionSaveSystem release. No telemetry or project data is transmitted."
			)
	)
	bool bCheckForUpdates;
};