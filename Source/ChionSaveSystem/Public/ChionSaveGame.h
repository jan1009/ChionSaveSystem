#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ChionSaveTypes.h"
#include "ChionSaveGame.generated.h"


UCLASS()
class CHIONSAVESYSTEM_API UChionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UChionSaveGame();

	/** Version des Savegame-Formats */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chion Save System")
	int32 SaveVersion;

	/** Alle gespeicherten Actor-ZustÃ¤nde */
	UPROPERTY()
	TArray<FChionActorSaveData> SavedActors;

	/**
	 * IDs von persistenten Level-Actors,
	 * die im Spiel zerstÃ¶rt / entfernt wurden.
	 */
	UPROPERTY()
	TArray<FGuid> DestroyedActorIds;
};
