#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChionConfigLibrary.generated.h"


UCLASS()
class CHIONSAVESYSTEM_API UChionConfigLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Saves a float value to an INI file inside:
	 * Project/Saved/Config/
	 *
	 * Example:
	 * FileName = SoundSettings
	 * Section  = Audio
	 * Key      = Music
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Chion Save System|Config",
		meta = (DisplayName = "Save Config Float")
	)
	static bool SaveConfigFloat(
		const FString& FileName,
		const FString& Section,
		const FString& Key,
		float Value
	);


	/**
	 * Loads a float value from an INI file inside:
	 * Project/Saved/Config/
	 *
	 * If the file, section or key does not exist,
	 * DefaultValue is returned through OutValue.
	 *
	 * Return Value:
	 * true  = value was found
	 * false = default value was used
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Chion Save System|Config",
		meta = (DisplayName = "Load Config Float")
	)
	static bool LoadConfigFloat(
		const FString& FileName,
		const FString& Section,
		const FString& Key,
		float DefaultValue,
		float& OutValue
	);

private:

	static bool BuildConfigFilePath(
		const FString& FileName,
		FString& OutFilePath
	);
};