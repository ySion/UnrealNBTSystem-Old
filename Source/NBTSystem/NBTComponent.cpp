#include "NBTComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "StructUtils/InstancedStruct.h"

UNBTComponentBase::UNBTComponentBase() {
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNBTComponentBase::RequestTickNextFrame() {
    if (!bOneShotTickRequested) {
        bOneShotTickRequested = true;
        SetComponentTickEnabled(true);
    }
}

UNBTComponent::UNBTComponent() {
    SetIsReplicatedByDefault(true);
    NBTContainer.ParentComponent = this;
    NBTAccessorRoot = NBTContainer.GetAccessor();
}

void UNBTComponent::BeginPlay() {
    Super::BeginPlay();
    if (ShouldBroadcastWhenPlay) {
        if (!GetOwner()->HasAuthority()) {
            OnNBTContainerChanged.Broadcast();
        }
    }
}

void UNBTComponent::OnRep_NBTContainer() { //客户端
    if (!HasBegunPlay()) {
        ShouldBroadcastWhenPlay = true;
        return;
    }
    
    if (!GetOwner()->HasAuthority()) {
        OnNBTContainerChanged.Broadcast();
    }
}

void UNBTComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    FDoRepLifetimeParams SharedParams{};
    SharedParams.bIsPushBased = true;
    SharedParams.Condition = COND_None;
    DOREPLIFETIME_WITH_PARAMS_FAST(UNBTComponent, NBTContainer, SharedParams);
}

void UNBTComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) { // 服务端轮询
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bOneShotTickRequested)
        return;

    if (GetOwner()->HasAuthority()) {
        if (NBTAccessorRoot.IsSubtreeChangedAndMark()) {
            MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, NBTContainer, this);
            OnNBTContainerChanged.Broadcast();
        }
    }
    NBTContainer.ClearDirtyThisFrame();
    bOneShotTickRequested = false;
    PrimaryComponentTick.SetTickFunctionEnable(false);

    FInstancedStruct s;
    TInstancedStruct<FBox> aaa;
}

UNBTComponentLocal::UNBTComponentLocal() {
    NBTContainer.ParentComponent = this;
    NBTAccessorRoot = NBTContainer.GetAccessor();
}

void UNBTComponentLocal::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bOneShotTickRequested)
        return;
    
    if (NBTAccessorRoot.IsSubtreeChangedAndMark())
        OnNBTContainerChanged.Broadcast();

    NBTContainer.ClearDirtyThisFrame();
    bOneShotTickRequested = false;
    PrimaryComponentTick.SetTickFunctionEnable(false);
}