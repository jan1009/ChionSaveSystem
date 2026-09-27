#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "ChionSaveTypes.generated.h"


USTRUCT()
struct CHIONSAVESYSTEM_API FChionActorSaveData
{
	GENERATED_BODY()

	/** Persistent ID used for placed / GUID based Actors. */
	UPROPERTY()
	FGuid SaveId;

	/** Optional stable key, for example "Player". */
	UPROPERTY()
	FName SaveKey = NAME_None;

	/**
	 * Class used to recreate a GUID based Actor if it no longer
	 * exists when an older save state is loaded.
	 */
	UPROPERTY()
	FSoftClassPath ActorClass;

	/**
	 * Transform used when the Actor has to be recreated.
	 * This is always stored and is independent of bSaveActorTransform.
	 */
	UPROPERTY()
	FTransform SpawnTransform = FTransform::Identity;

	/**
	 * Whether the regular Actor transform should be restored
	 * on an already existing Actor.
	 */
	UPROPERTY()
	bool bHasActorTransform = false;

	/** Saved Actor transform when bHasActorTransform is enabled. */
	UPROPERTY()
	FTransform ActorTransform = FTransform::Identity;

	/** Serialized properties marked with SaveGame. */
	UPROPERTY()
	TArray<uint8> Data;
};