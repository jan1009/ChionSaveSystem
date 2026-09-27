#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChionSaveManager.generated.h"


UCLASS()
class CHIONSAVESYSTEM_API UChionSaveManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Speichert alle Actors der aktuellen Welt,
	 * die einen aktivierten ChionSaveComponent besitzen.
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Chion Save System",
		meta = (WorldContext = "WorldContextObject")
	)
	static bool SaveWorld(
		UObject* WorldContextObject,
		FString SlotName,
		int32 UserIndex
	);

	/**
	 * LÃ¤dt alle gespeicherten Actor-ZustÃ¤nde
	 * und ordnet sie anhand ihrer SaveId wieder zu.
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Chion Save System",
		meta = (WorldContext = "WorldContextObject")
	)
	static bool LoadWorld(
		UObject* WorldContextObject,
		FString SlotName,
		int32 UserIndex
	);
};
