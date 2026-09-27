#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentInstanceDataCache.h"
#include "ChionSaveComponent.generated.h"

class UChionSaveComponent;
class AActor;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChionSaveStateSavingSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChionSaveStateLoadedSignature);


USTRUCT()
struct CHIONSAVESYSTEM_API FChionSaveComponentInstanceData
	: public FActorComponentInstanceData
{
	GENERATED_BODY()

public:
	FChionSaveComponentInstanceData() = default;

	explicit FChionSaveComponentInstanceData(
		const UChionSaveComponent* SourceComponent
	);

	virtual bool ContainsData() const override;

	virtual void ApplyToComponent(
		UActorComponent* Component,
		const ECacheApplyPhase CacheApplyPhase
	) override;

	FGuid SaveId;
};


UCLASS(ClassGroup = (ChionSaveSystem), meta = (BlueprintSpawnableComponent))
class CHIONSAVESYSTEM_API UChionSaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChionSaveComponent();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Chion Save System")
	FGuid SaveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chion Save System")
	FName SaveKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chion Save System")
	bool bSaveEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chion Save System")
	bool bSaveActorTransform = false;

	UPROPERTY(BlueprintAssignable, Category = "Chion Save System|Events")
	FChionSaveStateSavingSignature OnSaveStateSaving;

	UPROPERTY(BlueprintAssignable, Category = "Chion Save System|Events")
	FChionSaveStateLoadedSignature OnSaveStateLoaded;

	UFUNCTION(CallInEditor, Category = "Chion Save System")
	void CopySaveIdToClipboard();

	UFUNCTION(BlueprintCallable, Category = "Chion Save System|Debug")
	bool SerializeOwnerToBytes(TArray<uint8>& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Chion Save System|Debug")
	bool DeserializeOwnerFromBytes(const TArray<uint8>& Data);

	void NotifySaveStateSaving();
	void NotifySaveStateLoaded();

protected:
	virtual void BeginPlay() override;
	virtual void OnComponentCreated() override;

	virtual TStructOnScope<FActorComponentInstanceData>
		GetComponentInstanceData() const override;

#if WITH_EDITOR
	virtual void PostEditImport() override;
#endif

	UFUNCTION()
	void HandleOwnerEndPlay(
		AActor* Actor,
		EEndPlayReason::Type EndPlayReason
	);
};