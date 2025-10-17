#pragma once

#include "CoreMinimal.h"
#include "NBTAllocator.h"
#include "NBTAttribute.h"
#include "NBTAttributeID.h"
#include "Engine/NetSerialization.h"
#include "UObject/Object.h"
#include "NBTContainer.generated.h"

class UNBTComponentBase;
class FArzNBTContainerBaseState;

enum class EArzNBTDeltaOp : uint8 {
    Add,
    Update,
    Remove,
    EndOfDeltas
};

USTRUCT(BlueprintType)
struct FArzNBTContainerStats {
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    int32 TotalNodes = 0;

    UPROPERTY(VisibleAnywhere)
    int32 MapNodes = 0;

    UPROPERTY(VisibleAnywhere)
    int32 ListNodes = 0;

    UPROPERTY(VisibleAnywhere)
    int32 ValueNodes = 0;

    UPROPERTY(VisibleAnywhere)
    int32 MaxDepth = 0;

    UPROPERTY(VisibleAnywhere)
    TMap<ENBTAttributeType, int32> TypeCounts;
};

DECLARE_DELEGATE(FNBTContainerDeltaCallback);

USTRUCT(BlueprintType)
struct FNBTContainer {
    GENERATED_BODY()

private:
    friend class UNBTComponent;
    friend class UNBTComponentLocal;

    bool bIsContainerReplicated;

    FNBTAllocator Allocator;

    FNBTAttributeID RootID;

    int32 ContainerDataVersion = 0;

    int32 ContainerStructVersion = 0;

    bool bShouldOperatorEffectVersion = true;

    TWeakObjectPtr<UNBTComponentBase> ParentComponent;

    TSharedPtr<uint8> LiveToken;

    TMap<FNBTAttributeID, FNBTAttributeID> ParentOf; // 客户端专用

    TSet<FNBTAttributeID> FrameBubbleUniqueKey; // 客户端专用

    bool bDirtyThisFrame = false;

    friend struct FNBTDataAccessor;

    friend class FArzNBTContainerBaseState;

    void CreateLiveToken() { LiveToken = MakeShared<uint8>(); }

    void MarkDirtyThisFrame();

    void ClearDirtyThisFrame() { bDirtyThisFrame = false; }

    void UpdateContainerDataVersion() {
        if (!bShouldOperatorEffectVersion) return;
        ContainerDataVersion++;
        if (!bDirtyThisFrame)
            MarkDirtyThisFrame();
    }

    void UpdateContainerDataAndStructVersion() {
        if (!bShouldOperatorEffectVersion) return;
        ContainerDataVersion++;
        ContainerStructVersion++;
        if (!bDirtyThisFrame)
            MarkDirtyThisFrame();
    }

    FNBTAttributeID AllocateNode();

    void Initialize();
    void Clear();

    int32 ReleaseNode(FNBTAttributeID ID);
    int32 ReleaseRecursive(FNBTAttributeID ID);
    int32 ReleaseChildren(FNBTAttributeID ID);
    void ReleaseSubtreeImp(FNBTAttributeID ID, int32& ClearNum);

    int32 GetNodeNumRecursive(FNBTAttributeID ID) const;
    void GetNodeNumRecursiveImp(FNBTAttributeID ID, int32& ClearNum) const;

    void UpdateNodeDataVersion(FNBTAttributeID ID);

    void RebuildAllParents();
    void RebuildParentsForNode(FNBTAttributeID ParentID);
    void RebuildParentsForDirectChildren(FNBTAttributeID ParentID);
    void BubbleSubtreeVersionAlongPathForID(FNBTAttributeID LeafID);

    bool IsRemainingSpaceSupportCopy(FNBTAttributeID SourceID, const FNBTContainer& Source);
    bool IsRemainingSpaceSupportDoubleCopy(FNBTAttributeID A, FNBTAttributeID B);
    FNBTAttributeID DeepCopyNode(FNBTAttributeID SourceID, const FNBTContainer& Source);
    FNBTAttributeID DeepCopyNodeImpl(FNBTAttributeID SourceID, const FNBTContainer& Source); //从其他容器深层拷贝

public:
    FNBTContainer();
    FNBTContainer(const FNBTContainer& Other) = delete;
    FNBTContainer& operator=(const FNBTContainer& Other);

    FNBTContainer(FNBTContainer&& Other) = delete;
    FNBTContainer& operator=(FNBTContainer&& Other) = delete;

    ~FNBTContainer() = default;

    bool IsContainerReplicated() const { return bIsContainerReplicated; }

    void Reset();

    void CopyFrom(const FNBTContainer& Other);

    int32 GetContainerDataVersion() const { return ContainerDataVersion; }

    int32 GetContainerStructVersion() const { return ContainerStructVersion; }

    int32 GetNodeCount() const { return Allocator.GetCurrentActive(); }

    FNBTDataAccessor GetAccessor() const;

    FString ToString() const;

    FString ToDebugString() const;

    bool ValidateIntegrity() const;

    FArzNBTContainerStats GetStatistics() const;

    bool SerializeData(FArchive& Ar, bool NetWorkMode);

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

    //本地序列化
    bool Serialize(FArchive& Ar);

    //本地序列化
    friend FArchive& operator<<(FArchive& Ar, FNBTContainer& S);

private:
    bool ValidateNodeRecursive(FNBTAttributeID NodeID, TSet<FNBTAttributeID>& VisitedNodes) const;

    void GetNodeStatisticsRecursive(FNBTAttributeID NodeID, FArzNBTContainerStats& Stats, int32& CurrentDepth) const;

    inline int32* GetAttributeSubtreeVersion(FNBTAttributeID ID) const {
        return Allocator.GetNodeSubtreeVersion(ID);
    }

    inline void IncAttributeSubtreeVersion(FNBTAttributeID ID) const {
        Allocator.IncNodeSubtreeVersion(ID);
    }

    inline FNBTAttribute* GetAttribute(FNBTAttributeID ID) const {
        return Allocator.GetAttribute(ID);
    }

    inline int32* GetAttributeVersion(FNBTAttributeID ID) const {
        return Allocator.GetNodeVersion(ID);
    }

    inline bool IsAttributeValid(FNBTAttributeID ID) const {
        return Allocator.IsNodeValid(ID);
    }

    FNBTAttributeID GetRootID() const { return RootID; }
};

class FArzNBTContainerBaseState : public INetDeltaBaseState {
public:
    int32 ContainerVersion;

    TArray<FNBTAttributeChunkMetaData> VersionChunks;

    FArzNBTContainerBaseState() : ContainerVersion(0) {}

    void CreateVersionSnapshotFromContainer(const FNBTContainer& Container) {
        ContainerVersion = Container.ContainerDataVersion;

        const int32 NumChunks = Container.Allocator.GetChunkCount();
        VersionChunks.Reset(NumChunks);
        VersionChunks.SetNum(NumChunks);
        for (int32 i = 0; i < NumChunks; ++i) {
            if (const FNBTAttributeChunkMetaData* Meta = Container.Allocator.GetChunkMetadata(i)) {
                FMemory::Memcpy(&VersionChunks[i], Meta, sizeof(FNBTAttributeChunkMetaData));
            }
        }
    }

    const int32* GetVersionForID(FNBTAttributeID ID) const {
        if (!ID.IsValid()) return nullptr;

        const uint16 ChunkIndex = ID.Index >> FNBTAllocator::CHUNK_SHIFT;
        const uint16 LocalIndex = ID.Index & FNBTAllocator::CHUNK_MASK;

        if (!VersionChunks.IsValidIndex(ChunkIndex)) return nullptr;

        const FNBTAttributeChunkMetaData& Chunk = VersionChunks[ChunkIndex];
        if ((Chunk.UsedMask & (1ULL << LocalIndex)) && Chunk.Generations[LocalIndex] == ID.Generation) {
            return &Chunk.Versions[LocalIndex];
        }

        return nullptr;
    }

    virtual bool IsStateEqual(INetDeltaBaseState* OtherState) override {
        FArzNBTContainerBaseState* Other = static_cast<FArzNBTContainerBaseState*>(OtherState);
        if (!Other) return false;

        if (ContainerVersion == Other->ContainerVersion) return true;
        return false;
    }

    virtual void CountBytes(FArchive& Ar) const override {
        Ar.CountBytes(sizeof(FArzNBTContainerBaseState), sizeof(FArzNBTContainerBaseState));
        VersionChunks.CountBytes(Ar);
    }
};

template <>
struct TStructOpsTypeTraits<FNBTContainer> : public TStructOpsTypeTraitsBase2<FNBTContainer> {
    enum {
        WithNetDeltaSerializer = true,
        WithSerializer = true,
    };
};