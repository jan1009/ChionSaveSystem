#include "ChionSaveComponent.h"

#include "ChionSaveArchive.h"
#include "ChionSaveWorldSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"


FChionSaveComponentInstanceData::FChionSaveComponentInstanceData(
	const UChionSaveComponent* SourceComponent)
	: FActorComponentInstanceData(SourceComponent)
	, SaveId(SourceComponent ? SourceComponent->SaveId : FGuid())
{
}


bool FChionSaveComponentInstanceData::ContainsData() const
{
	return SaveId.IsValid()
		|| FActorComponentInstanceData::ContainsData();
}


void FChionSaveComponentInstanceData::ApplyToComponent(
	UActorComponent* Component,
	const ECacheApplyPhase CacheApplyPhase)
{
	FActorComponentInstanceData::ApplyToComponent(
		Component,
		CacheApplyPhase
	);

	UChionSaveComponent* SaveComponent =
		Cast<UChionSaveComponent>(Component);

	if (SaveComponent && SaveId.IsValid())
	{
		SaveComponent->SaveId = SaveId;
	}
}


TStructOnScope<FActorComponentInstanceData>
UChionSaveComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<
		FActorComponentInstanceData,
		FChionSaveComponentInstanceData
	>(this);
}


UChionSaveComponent::UChionSaveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UChionSaveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		Owner->OnEndPlay.AddUniqueDynamic(
			this,
			&UChionSaveComponent::HandleOwnerEndPlay
		);
	}
}


void UChionSaveComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (!IsTemplate() && !SaveId.IsValid())
	{
#if WITH_EDITOR
		Modify();
#endif

		SaveId = FGuid::NewGuid();

#if WITH_EDITOR
		MarkPackageDirty();
#endif
	}
}


void UChionSaveComponent::HandleOwnerEndPlay(
	AActor* Actor,
	EEndPlayReason::Type EndPlayReason)
{
	// Nur echtes Actor->Destroy() persistent merken.
	if (EndPlayReason != EEndPlayReason::Destroyed)
	{
		return;
	}

	if (!bSaveEnabled
		|| !SaveKey.IsNone()
		|| !SaveId.IsValid())
	{
		return;
	}

	UWorld* World = Actor ? Actor->GetWorld() : GetWorld();

	if (!World)
	{
		return;
	}

	if (UChionSaveWorldSubsystem* SaveSubsystem =
		World->GetSubsystem<UChionSaveWorldSubsystem>())
	{
		SaveSubsystem->MarkActorDestroyed(SaveId);
	}
}


#if WITH_EDITOR

void UChionSaveComponent::PostEditImport()
{
	Super::PostEditImport();

	if (!IsTemplate())
	{
		Modify();

		SaveId = FGuid::NewGuid();

		MarkPackageDirty();
	}
}

#endif


void UChionSaveComponent::CopySaveIdToClipboard()
{
	if (!SaveId.IsValid())
	{
		return;
	}

	const FString SaveIdString =
		SaveId.ToString(
			EGuidFormats::DigitsWithHyphensInBraces
		);

	FPlatformApplicationMisc::ClipboardCopy(
		*SaveIdString
	);
}


bool UChionSaveComponent::SerializeOwnerToBytes(
	TArray<uint8>& OutData) const
{
	const AActor* Owner = GetOwner();

	if (!Owner || !bSaveEnabled)
	{
		return false;
	}

	OutData.Reset();

	FMemoryWriter MemoryWriter(OutData, true);
	FChionSaveArchive SaveArchive(MemoryWriter);

	AActor* MutableOwner = const_cast<AActor*>(Owner);
	MutableOwner->Serialize(SaveArchive);

	SaveArchive.Close();
	MemoryWriter.Close();

	return !SaveArchive.IsError();
}


bool UChionSaveComponent::DeserializeOwnerFromBytes(
	const TArray<uint8>& Data)
{
	AActor* Owner = GetOwner();

	if (!Owner || !bSaveEnabled || Data.IsEmpty())
	{
		return false;
	}

	FMemoryReader MemoryReader(Data, true);
	FChionSaveArchive LoadArchive(MemoryReader);

	Owner->Serialize(LoadArchive);

	LoadArchive.Close();
	MemoryReader.Close();

	return !LoadArchive.IsError();
}


void UChionSaveComponent::NotifySaveStateSaving()
{
	OnSaveStateSaving.Broadcast();
}


void UChionSaveComponent::NotifySaveStateLoaded()
{
	OnSaveStateLoaded.Broadcast();
}