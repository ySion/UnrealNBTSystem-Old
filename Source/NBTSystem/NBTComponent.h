#pragma once

#include "CoreMinimal.h"
#include "NBTAccessor.h"
#include "Components/ActorComponent.h"
#include "NBTComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnArzNBTContainerChanged);

UENUM()
enum class ENBTReplicateCondition : uint8 {
    All,
    SkipOwner,
    OwnerOnly,
};

UCLASS()
class NBTSYSTEM_API UNBTComponentBase : public UActorComponent {
    GENERATED_BODY()
public:
    UNBTComponentBase();
    
    bool bOneShotTickRequested = false;
    
    void RequestTickNextFrame();
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(ArzKit), meta=(BlueprintSpawnableComponent))
class NBTSYSTEM_API UNBTComponent : public UNBTComponentBase {
    GENERATED_BODY()

public:
    UNBTComponent();

    UPROPERTY(BlueprintReadWrite)
    TEnumAsByte<ELifetimeCondition> NBTContainerReplicationCondition = COND_None;

    UPROPERTY(BlueprintAssignable)
    FOnArzNBTContainerChanged OnNBTContainerChanged;

    UFUNCTION(BlueprintCallable)
    void ResetContainer() { if (GetOwner() && GetOwner()->HasAuthority()) NBTContainer.Reset(); }

    UFUNCTION(BlueprintCallable)
    FNBTDataAccessor GetAccessor() const { return NBTContainer.GetAccessor(); }

    UFUNCTION(BlueprintCallable)
    FString GetString() const { return NBTContainer.ToString(); }

    UFUNCTION(BlueprintCallable)
    FString GetStringDebug() const { return NBTContainer.ToDebugString(); }

    UFUNCTION(BlueprintCallable)
    FArzNBTContainerStats GetStatistics() const { return NBTContainer.GetStatistics(); }

    UFUNCTION(BlueprintCallable)
    int32 GetNodeCount() const { return NBTContainer.GetNodeCount(); }

    virtual void BeginPlay() override;

protected:

    UFUNCTION()
    void OnRep_NBTContainer();

    UPROPERTY(Replicated, ReplicatedUsing=OnRep_NBTContainer)
    FNBTContainer NBTContainer;

    FNBTDataAccessor NBTAccessorRoot;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool ShouldBroadcastWhenPlay = false;

    friend struct FNBTDataAccessor;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(ArzKit), meta=(BlueprintSpawnableComponent))
class NBTSYSTEM_API UNBTComponentLocal : public UNBTComponentBase {
    GENERATED_BODY()

public:
    UNBTComponentLocal();

    UPROPERTY(BlueprintAssignable)
    FOnArzNBTContainerChanged OnNBTContainerChanged;

    UFUNCTION(BlueprintCallable)
    void ResetContainer() { NBTContainer.Reset(); }

    UFUNCTION(BlueprintCallable)
    FNBTDataAccessor GetAccessor() const { return NBTContainer.GetAccessor(); }

    UFUNCTION(BlueprintCallable)
    FString GetString() const { return NBTContainer.ToString(); }

    UFUNCTION(BlueprintCallable)
    FString GetStringDebug() const { return NBTContainer.ToDebugString(); }

    UFUNCTION(BlueprintCallable)
    FArzNBTContainerStats GetStatistics() const { return NBTContainer.GetStatistics(); }

    UFUNCTION(BlueprintCallable)
    int32 GetNodeCount() const { return NBTContainer.GetNodeCount(); }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    FNBTContainer NBTContainer;

    FNBTDataAccessor NBTAccessorRoot;
};