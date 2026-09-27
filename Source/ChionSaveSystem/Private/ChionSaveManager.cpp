#include "ChionSaveManager.h"

#include "ChionSaveComponent.h"
#include "ChionSaveGame.h"
#include "ChionSaveTypes.h"
#include "ChionSaveWorldSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"


DEFINE_LOG_CATEGORY_STATIC(
	LogChionSaveSystem,
	Log,
	All
);


bool UChionSaveManager::SaveWorld(
	UObject* WorldContextObject,
	FString SlotName,
	int32 UserIndex)
{
	if (!WorldContextObject || SlotName.IsEmpty())
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContextObject,
		EGetWorldErrorMode::ReturnNull
	);

	if (!World)
	{
		return false;
	}


	// Alle vergebenen SaveKeys zählen.
	TMap<FName, int32> SaveKeyCounts;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!Actor)
		{
			continue;
		}

		UChionSaveComponent* SaveComponent =
			Actor->FindComponentByClass<UChionSaveComponent>();

		if (!SaveComponent || !SaveComponent->bSaveEnabled)
		{
			continue;
		}

		if (!SaveComponent->SaveKey.IsNone())
		{
			int32& Count =
				SaveKeyCounts.FindOrAdd(
					SaveComponent->SaveKey
				);

			++Count;
		}
	}


	UChionSaveGame* SaveGameObject = Cast<UChionSaveGame>(
		UGameplayStatics::CreateSaveGameObject(
			UChionSaveGame::StaticClass()
		)
	);

	if (!SaveGameObject)
	{
		return false;
	}

	SaveGameObject->SavedActors.Reset();
	SaveGameObject->DestroyedActorIds.Reset();


	/*
	 * Persistente Liste bereits zerstörter Level-Actors
	 * aus dem WorldSubsystem übernehmen.
	 */
	if (UChionSaveWorldSubsystem* SaveSubsystem =
		World->GetSubsystem<UChionSaveWorldSubsystem>())
	{
		SaveSubsystem->GetDestroyedActorIds(
			SaveGameObject->DestroyedActorIds
		);
	}


	UE_LOG(
		LogChionSaveSystem,
		Warning,
		TEXT("Saving DestroyedActorIds: %d"),
		SaveGameObject->DestroyedActorIds.Num()
	);


	for (const FGuid& DestroyedId :
		SaveGameObject->DestroyedActorIds)
	{
		UE_LOG(
			LogChionSaveSystem,
			Warning,
			TEXT("  Destroyed ID: %s"),
			*DestroyedId.ToString()
		);
	}


	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!Actor)
		{
			continue;
		}

		UChionSaveComponent* SaveComponent =
			Actor->FindComponentByClass<UChionSaveComponent>();

		if (!SaveComponent || !SaveComponent->bSaveEnabled)
		{
			continue;
		}


		// SaveKey muss eindeutig sein.
		if (!SaveComponent->SaveKey.IsNone())
		{
			const int32* Count =
				SaveKeyCounts.Find(
					SaveComponent->SaveKey
				);

			if (Count && *Count > 1)
			{
				UE_LOG(
					LogChionSaveSystem,
					Error,
					TEXT(
						"Duplicate SaveKey '%s' detected on Actor '%s'. Actor will NOT be saved."
					),
					*SaveComponent->SaveKey.ToString(),
					*Actor->GetName()
				);

				continue;
			}
		}


		if (SaveComponent->SaveKey.IsNone()
			&& !SaveComponent->SaveId.IsValid())
		{
			continue;
		}


		/*
		 * Blueprint darf direkt vor dem Speichern
		 * seinen Zustand aktualisieren.
		 */
		SaveComponent->NotifySaveStateSaving();


		FChionActorSaveData ActorSaveData;

		ActorSaveData.SaveId =
			SaveComponent->SaveId;

		ActorSaveData.SaveKey =
			SaveComponent->SaveKey;

		/*
		 * Klasse und Transform werden immer gespeichert,
		 * damit ein während des Spiels zerstörter Actor
		 * beim Laden eines älteren Saves wiederhergestellt
		 * werden kann.
		 */
		ActorSaveData.ActorClass =
			FSoftClassPath(Actor->GetClass());

		ActorSaveData.SpawnTransform =
			Actor->GetActorTransform();

		ActorSaveData.bHasActorTransform =
			SaveComponent->bSaveActorTransform;


		if (SaveComponent->bSaveActorTransform)
		{
			ActorSaveData.ActorTransform =
				Actor->GetActorTransform();
		}


		if (!SaveComponent->SerializeOwnerToBytes(
			ActorSaveData.Data))
		{
			continue;
		}


		SaveGameObject->SavedActors.Add(
			MoveTemp(ActorSaveData)
		);
	}


	return UGameplayStatics::SaveGameToSlot(
		SaveGameObject,
		SlotName,
		UserIndex
	);
}


bool UChionSaveManager::LoadWorld(
	UObject* WorldContextObject,
	FString SlotName,
	int32 UserIndex)
{
	if (!WorldContextObject || SlotName.IsEmpty())
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContextObject,
		EGetWorldErrorMode::ReturnNull
	);

	if (!World)
	{
		return false;
	}


	USaveGame* LoadedSaveGame =
		UGameplayStatics::LoadGameFromSlot(
			SlotName,
			UserIndex
		);


	UChionSaveGame* SaveGameObject =
		Cast<UChionSaveGame>(LoadedSaveGame);

	if (!SaveGameObject)
	{
		return false;
	}


	/*
	 * GUIDs der Actors sammeln, die laut geladener
	 * Save-Datei zerstört bleiben sollen.
	 */
	TSet<FGuid> DestroyedActorIdSet;

	for (const FGuid& SaveId :
		SaveGameObject->DestroyedActorIds)
	{
		if (SaveId.IsValid())
		{
			DestroyedActorIdSet.Add(SaveId);
		}
	}


	/*
	 * Runtime-Subsystem auf denselben Stand
	 * wie die geladene Save-Datei bringen.
	 *
	 * Das ist wichtig, wenn innerhalb derselben
	 * Spielsitzung ein älterer Save geladen wird.
	 */
	if (UChionSaveWorldSubsystem* SaveSubsystem =
		World->GetSubsystem<UChionSaveWorldSubsystem>())
	{
		SaveSubsystem->SetDestroyedActorIds(
			SaveGameObject->DestroyedActorIds
		);
	}


	/*
	 * Actors sammeln, die laut Save-Datei
	 * zerstört sein sollen.
	 *
	 * Während des TActorIterator zerstören wir
	 * absichtlich noch nichts.
	 */
	TArray<AActor*> ActorsToDestroy;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!Actor)
		{
			continue;
		}

		UChionSaveComponent* SaveComponent =
			Actor->FindComponentByClass<UChionSaveComponent>();

		if (!SaveComponent || !SaveComponent->bSaveEnabled)
		{
			continue;
		}

		// Nur GUID-basierte Level-Actors.
		if (!SaveComponent->SaveKey.IsNone()
			|| !SaveComponent->SaveId.IsValid())
		{
			continue;
		}

		if (DestroyedActorIdSet.Contains(
			SaveComponent->SaveId))
		{
			ActorsToDestroy.Add(Actor);
		}
	}


	for (AActor* Actor : ActorsToDestroy)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}


	/*
	 * Gespeicherte Actor-Daten für schnelles
	 * Nachschlagen vorbereiten.
	 */
	TMap<FGuid, const FChionActorSaveData*> SavedActorIdMap;
	TMap<FName, const FChionActorSaveData*> SavedActorKeyMap;

	TSet<FName> DuplicateSavedKeys;


	for (const FChionActorSaveData& ActorSaveData :
		SaveGameObject->SavedActors)
	{
		if (!ActorSaveData.SaveKey.IsNone())
		{
			if (SavedActorKeyMap.Contains(
				ActorSaveData.SaveKey))
			{
				DuplicateSavedKeys.Add(
					ActorSaveData.SaveKey
				);

				SavedActorKeyMap.Remove(
					ActorSaveData.SaveKey
				);

				UE_LOG(
					LogChionSaveSystem,
					Error,
					TEXT(
						"Duplicate SaveKey '%s' found inside save file. Entry will NOT be loaded."
					),
					*ActorSaveData.SaveKey.ToString()
				);

				continue;
			}


			if (!DuplicateSavedKeys.Contains(
				ActorSaveData.SaveKey))
			{
				SavedActorKeyMap.Add(
					ActorSaveData.SaveKey,
					&ActorSaveData
				);
			}
		}
		else if (ActorSaveData.SaveId.IsValid())
		{
			SavedActorIdMap.Add(
				ActorSaveData.SaveId,
				&ActorSaveData
			);
		}
	}


	/*
	 * Alle momentan existierenden GUID-basierten
	 * Actors sammeln.
	 */
	TSet<FGuid> ExistingActorIds;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!IsValid(Actor))
		{
			continue;
		}

		UChionSaveComponent* SaveComponent =
			Actor->FindComponentByClass<UChionSaveComponent>();

		if (!SaveComponent || !SaveComponent->bSaveEnabled)
		{
			continue;
		}

		if (!SaveComponent->SaveKey.IsNone()
			|| !SaveComponent->SaveId.IsValid())
		{
			continue;
		}

		ExistingActorIds.Add(
			SaveComponent->SaveId
		);
	}


	/*
	 * Actors wiederherstellen, die im geladenen Save
	 * existieren, in der aktuellen Welt aber inzwischen
	 * per Destroy Actor entfernt wurden.
	 */
	for (const TPair<
		FGuid,
		const FChionActorSaveData*>& Pair :
		SavedActorIdMap)
	{
		const FGuid& SaveId =
			Pair.Key;

		const FChionActorSaveData* ActorSaveData =
			Pair.Value;


		if (!ActorSaveData)
		{
			continue;
		}


		// Actor existiert bereits.
		if (ExistingActorIds.Contains(SaveId))
		{
			continue;
		}


		/*
		 * Laut geladenem Save soll der Actor
		 * zerstört bleiben.
		 */
		if (DestroyedActorIdSet.Contains(SaveId))
		{
			continue;
		}


		if (ActorSaveData->ActorClass.IsNull())
		{
			UE_LOG(
				LogChionSaveSystem,
				Warning,
				TEXT(
					"Cannot restore Actor with SaveId '%s': no ActorClass stored."
				),
				*SaveId.ToString()
			);

			continue;
		}


		UClass* ActorClass =
			ActorSaveData->ActorClass.TryLoadClass<AActor>();

		if (!ActorClass)
		{
			UE_LOG(
				LogChionSaveSystem,
				Warning,
				TEXT(
					"Cannot restore Actor with SaveId '%s': ActorClass could not be loaded."
				),
				*SaveId.ToString()
			);

			continue;
		}


		FActorSpawnParameters SpawnParameters;

		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		AActor* SpawnedActor =
			World->SpawnActor<AActor>(
				ActorClass,
				ActorSaveData->SpawnTransform,
				SpawnParameters
			);


		if (!SpawnedActor)
		{
			UE_LOG(
				LogChionSaveSystem,
				Warning,
				TEXT(
					"Failed to respawn Actor with SaveId '%s'."
				),
				*SaveId.ToString()
			);

			continue;
		}


		UChionSaveComponent* SpawnedSaveComponent =
			SpawnedActor->FindComponentByClass<UChionSaveComponent>();


		if (!SpawnedSaveComponent)
		{
			UE_LOG(
				LogChionSaveSystem,
				Error,
				TEXT(
					"Respawned Actor '%s' has no ChionSaveComponent."
				),
				*SpawnedActor->GetName()
			);

			SpawnedActor->Destroy();

			continue;
		}


		/*
		 * Beim Spawn erhält der Component zunächst
		 * automatisch eine neue SaveId.
		 *
		 * Für den wiederhergestellten Actor setzen
		 * wir anschließend die ursprüngliche SaveId
		 * aus der Save-Datei zurück.
		 */
		SpawnedSaveComponent->SaveId =
			SaveId;


		ExistingActorIds.Add(
			SaveId
		);


		UE_LOG(
			LogChionSaveSystem,
			Log,
			TEXT(
				"Respawned Actor '%s' with SaveId '%s'."
			),
			*SpawnedActor->GetName(),
			*SaveId.ToString()
		);
	}


	// Aktuelle Welt auf doppelte SaveKeys prüfen.
	TMap<FName, int32> WorldSaveKeyCounts;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!Actor)
		{
			continue;
		}

		UChionSaveComponent* SaveComponent =
			Actor->FindComponentByClass<UChionSaveComponent>();

		if (!SaveComponent || !SaveComponent->bSaveEnabled)
		{
			continue;
		}

		if (!SaveComponent->SaveKey.IsNone())
		{
			int32& Count =
				WorldSaveKeyCounts.FindOrAdd(
					SaveComponent->SaveKey
				);

			++Count;
		}
	}


	/*
	 * Alle vorhandenen und eventuell gerade
	 * wiederhergestellten Actors laden.
	 */
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!Actor)
		{
			continue;
		}


		UChionSaveComponent* SaveComponent =
			Actor->FindComponentByClass<UChionSaveComponent>();

		if (!SaveComponent || !SaveComponent->bSaveEnabled)
		{
			continue;
		}


		const FChionActorSaveData* ActorSaveData = nullptr;


		if (!SaveComponent->SaveKey.IsNone())
		{
			const int32* Count =
				WorldSaveKeyCounts.Find(
					SaveComponent->SaveKey
				);

			if (Count && *Count > 1)
			{
				UE_LOG(
					LogChionSaveSystem,
					Error,
					TEXT(
						"Duplicate SaveKey '%s' detected on Actor '%s'. Actor will NOT be loaded."
					),
					*SaveComponent->SaveKey.ToString(),
					*Actor->GetName()
				);

				continue;
			}


			if (DuplicateSavedKeys.Contains(
				SaveComponent->SaveKey))
			{
				continue;
			}


			const FChionActorSaveData* const* FoundSaveData =
				SavedActorKeyMap.Find(
					SaveComponent->SaveKey
				);

			if (FoundSaveData)
			{
				ActorSaveData = *FoundSaveData;
			}
		}
		else if (SaveComponent->SaveId.IsValid())
		{
			const FChionActorSaveData* const* FoundSaveData =
				SavedActorIdMap.Find(
					SaveComponent->SaveId
				);

			if (FoundSaveData)
			{
				ActorSaveData = *FoundSaveData;
			}
		}


		if (!ActorSaveData)
		{
			continue;
		}


		if (!SaveComponent->DeserializeOwnerFromBytes(
			ActorSaveData->Data))
		{
			continue;
		}


		if (ActorSaveData->bHasActorTransform)
		{
			Actor->SetActorTransform(
				ActorSaveData->ActorTransform,
				false,
				nullptr,
				ETeleportType::TeleportPhysics
			);
		}


		SaveComponent->NotifySaveStateLoaded();
	}


	return true;
}