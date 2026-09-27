#include "ChionSaveWorldSubsystem.h"


void UChionSaveWorldSubsystem::MarkActorDestroyed(
	const FGuid& SaveId)
{
	if (SaveId.IsValid())
	{
		DestroyedActorIds.Add(SaveId);
	}
}


bool UChionSaveWorldSubsystem::IsActorDestroyed(
	const FGuid& SaveId) const
{
	return SaveId.IsValid()
		&& DestroyedActorIds.Contains(SaveId);
}


void UChionSaveWorldSubsystem::GetDestroyedActorIds(
	TArray<FGuid>& OutIds) const
{
	OutIds.Reset();
	OutIds.Reserve(DestroyedActorIds.Num());

	for (const FGuid& SaveId : DestroyedActorIds)
	{
		OutIds.Add(SaveId);
	}
}


void UChionSaveWorldSubsystem::SetDestroyedActorIds(
	const TArray<FGuid>& InIds)
{
	DestroyedActorIds.Reset();

	for (const FGuid& SaveId : InIds)
	{
		if (SaveId.IsValid())
		{
			DestroyedActorIds.Add(SaveId);
		}
	}
}
