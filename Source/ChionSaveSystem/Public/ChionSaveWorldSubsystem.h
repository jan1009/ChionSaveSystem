#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChionSaveWorldSubsystem.generated.h"


/**
 * Merkt sich zur Laufzeit, welche persistenten Level-Actors
 * seit dem letzten Laden zerstÃ¶rt wurden.
 *
 * Die eigentliche Speicherung in der .sav-Datei Ã¼bernimmt
 * spÃ¤ter der ChionSaveManager.
 */
UCLASS()
class CHIONSAVESYSTEM_API UChionSaveWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Merkt eine zerstÃ¶rte Actor-ID vor */
	void MarkActorDestroyed(const FGuid& SaveId);

	/** PrÃ¼ft, ob eine Actor-ID als zerstÃ¶rt markiert ist */
	bool IsActorDestroyed(const FGuid& SaveId) const;

	/** Gibt alle aktuell zerstÃ¶rten Actor-IDs zurÃ¼ck */
	void GetDestroyedActorIds(TArray<FGuid>& OutIds) const;

	/**
	 * Ãœberschreibt den aktuellen Zustand,
	 * z. B. nach dem Laden einer Save-Datei.
	 */
	void SetDestroyedActorIds(const TArray<FGuid>& InIds);

private:

	/** Laufzeit-Liste aller persistent zerstÃ¶rten Level-Actors */
	TSet<FGuid> DestroyedActorIds;
};
