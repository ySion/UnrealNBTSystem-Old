#pragma once

#include "CoreMinimal.h"
#include "NBTAttribute.h"
#include "NBTAttributeID.h"
#include "HAL/UnrealMemory.h"

#define ARZ_NBT_CHUNK_SIZE 64

struct FNBTAttributeChunkMetaData {
    uint64 UsedMask {};
    uint16 Generations[ARZ_NBT_CHUNK_SIZE] {};
    int32 Versions[ARZ_NBT_CHUNK_SIZE] {};
    int32 SubtreeVersions[ARZ_NBT_CHUNK_SIZE] {};
    uint8 UsedCount {};
    uint16 ChunkIndex {};
    uint8 Padding[5] {};
};

enum class FAttributeChunkAllocateAtResult : uint8 {
    Failed,     // 分配失败
    Exist,      // 返回已经存在的
    Replaced,   // 返回已经替换的
    NewOne      // 新对象
};

struct alignas(64) FAttributeChunk {
    
    uint8 AttributeBuffer[sizeof(FNBTAttribute) * ARZ_NBT_CHUNK_SIZE];

    FNBTAttributeChunkMetaData Meta;

    FAttributeChunk(uint16 Index) {
        FMemory::Memzero(this, sizeof(FAttributeChunk));
        Meta.ChunkIndex = Index;
    }

    ~FAttributeChunk() {
        if (Meta.UsedMask != 0) {
            FNBTAttribute* Attributes = reinterpret_cast<FNBTAttribute*>(AttributeBuffer);
            for (uint32 i = 0; i < ARZ_NBT_CHUNK_SIZE; ++i) {
                if (Meta.UsedMask & (1ULL << i)) {
                    Attributes[i].~FNBTAttribute();
                }
            }
        }
    }

    int32* GetVersion(uint16 LocalIndex) {
        if (!IsIndexValid(LocalIndex)) return nullptr;
        return &Meta.Versions[LocalIndex];
    }

    int32* GetSubtreeVersion(uint16 LocalIndex) {
        if (!IsIndexValid(LocalIndex)) return nullptr;
        return &Meta.SubtreeVersions[LocalIndex];
    }

    FNBTAttribute* GetAttribute(uint16 LocalIndex) {
        if (!IsIndexValid(LocalIndex)) return nullptr;
        return reinterpret_cast<FNBTAttribute*>(AttributeBuffer) + LocalIndex;
    }
    
    // 分配槽位
    TOptional<uint16> AllocateSlot() {
        uint64 FreeMask = ~Meta.UsedMask;
        if (FreeMask == 0) return {};
        
        uint16 LocalIndex = FMath::CountTrailingZeros64(FreeMask);
        
        Meta.UsedMask |= (1ULL << LocalIndex);
        Meta.UsedCount++;
        Meta.Generations[LocalIndex]++;
        Meta.Versions[LocalIndex] = 0;
        Meta.SubtreeVersions[LocalIndex] = 0;
        FNBTAttribute* Attributes = reinterpret_cast<FNBTAttribute*>(AttributeBuffer);
        new(&Attributes[LocalIndex]) FNBTAttribute();
        
        return LocalIndex;
    }

    // 确定性分配, 指定index和generation, 给网络同步使用
    FAttributeChunkAllocateAtResult AllocateSlotAt(uint16 LocalIndex, uint16 ExpectedGeneration) {
        if (!IsInRange(LocalIndex)) return FAttributeChunkAllocateAtResult::Failed;
        if (IsUsed(LocalIndex)) {
            Meta.Versions[LocalIndex] ++; // Hack Op, 用于客户端检测数据更新
            //Meta.SubtreeVersions[LocalIndex] = 0;
            if (Meta.Generations[LocalIndex] == ExpectedGeneration) return FAttributeChunkAllocateAtResult::Exist;
            FNBTAttribute* Attributes = reinterpret_cast<FNBTAttribute*>(AttributeBuffer);
            Attributes[LocalIndex].~FNBTAttribute();
            Meta.Generations[LocalIndex] = ExpectedGeneration;
            new(&Attributes[LocalIndex]) FNBTAttribute();
            return FAttributeChunkAllocateAtResult::Replaced;
        } else {
            Meta.UsedMask |= (1ULL << LocalIndex);
            Meta.UsedCount++;
            Meta.Generations[LocalIndex] = ExpectedGeneration;
            Meta.Versions[LocalIndex]++; // Hack Op, 用于客户端检测数据更新
            //Meta.SubtreeVersions[LocalIndex] = 0;
            FNBTAttribute* Attributes = reinterpret_cast<FNBTAttribute*>(AttributeBuffer);
            new(&Attributes[LocalIndex]) FNBTAttribute();
            return FAttributeChunkAllocateAtResult::NewOne;
        }
        
    }

    // 释放槽位
    bool DeallocateSlot(uint16 LocalIndex, uint16 ExpectedGeneration) {
        if (!IsIndexValid(LocalIndex)) return false;
        if (Meta.Generations[LocalIndex] != ExpectedGeneration) return false;

        // 调用析构函数
        FNBTAttribute* Attributes = reinterpret_cast<FNBTAttribute*>(AttributeBuffer);
        Attributes[LocalIndex].~FNBTAttribute();

        // 标记为未使用
        Meta.UsedMask &= ~(1ULL << LocalIndex);
        Meta.UsedCount--;
        Meta.Versions[LocalIndex] = 0;
        Meta.SubtreeVersions[LocalIndex] = 0;
        return true;
    }

    FORCEINLINE bool HasFreeSlot() const {
        return Meta.UsedCount < ARZ_NBT_CHUNK_SIZE;
    }

    FORCEINLINE uint8 GetUsedCount() const {
        return Meta.UsedCount;
    }
    
    FORCEINLINE bool IsInRange(uint32 LocalIndex) const {
        return LocalIndex < ARZ_NBT_CHUNK_SIZE;
    }
    
    FORCEINLINE bool IsUsed(uint32 LocalIndex) const {
        return Meta.UsedMask & (1ULL << LocalIndex);
    }
    
    FORCEINLINE bool IsIndexValid(uint32 LocalIndex) const {
        return IsInRange(LocalIndex) && IsUsed(LocalIndex);
    }
};

struct FNBTAllocator {
    static constexpr uint32 CHUNK_SIZE = ARZ_NBT_CHUNK_SIZE;
    static constexpr uint32 CHUNK_SHIFT = 6; // log2(64)
    static constexpr uint32 CHUNK_MASK = 0x3F; // 63
    static constexpr uint16 MAX_CHUNKS = 1024; // 64 * 1024 = 65536 总节点数
private:
    friend class FArzNBTContainerBaseState;
    
    // 块管理
    TArray<TUniquePtr<FAttributeChunk>> Chunks;

    // 统计信息
    struct {
        uint32 TotalAllocated = 0;
        uint32 TotalDeallocated = 0;
        uint32 CurrentActive = 0;
        uint32 PeakActive = 0;
    } Stats;

    uint16 RoundRobinIndex = 0;

public:
    FNBTAllocator() {
        AllocateNewChunk();
    }

    ~FNBTAllocator() = default;

    void Reset() {
        Chunks.Empty();
        Stats.TotalAllocated = 0;
        Stats.TotalDeallocated = 0;
        Stats.CurrentActive = 0;
        Stats.PeakActive = 0;
        RoundRobinIndex = 0;
        AllocateNewChunk();
    }

    // 分配属性
    FNBTAttributeID Allocate() {
        //分配满了就不分配了, 在正常使用中, 这个几乎是不可能的, 不存在这么大的NBT, 如果存在, 那么整个程序会陷入大麻烦.
        if (Stats.CurrentActive >= 65534) {
            UE_LOG(NBTSystem, Error, TEXT("NBT Allocator full! (65534 nodes)")); // 序列65535用于有效性校验
            return FNBTAttributeID();
        }
        
        uint16 ChunkIndex = SelectOrCreateChunkForBestAllocation(); // 根据策略选择块, 该函数自己会智能分配
        FAttributeChunk* Chunk = Chunks[ChunkIndex].Get();
        
        auto AllocateResult = Chunks[ChunkIndex]->AllocateSlot();
        if (!AllocateResult.IsSet()) {
            UE_LOG(NBTSystem, Error, TEXT("NBT Allocator AllocateSlot Logic Failed"));
            return FNBTAttributeID();
        }
        
        uint16 LocalIndex = AllocateResult.GetValue();
        uint16 GlobalIndex = (ChunkIndex << CHUNK_SHIFT) | LocalIndex;

        // 更新统计
        Stats.TotalAllocated++;
        Stats.CurrentActive++;
        Stats.PeakActive = FMath::Max(Stats.PeakActive, Stats.CurrentActive);

        // 生成ID
        uint16 Generation = Chunk->Meta.Generations[LocalIndex];
        return FNBTAttributeID(GlobalIndex, Generation);
    }

    //用于网络同步, 在指定位置分配, 但是如果指定位置的数据相同, 就返回现有节点
    FNBTAttribute* AllocateAt(FNBTAttributeID ID) {
        if (!ID.IsValid()) return nullptr;

        uint16 ChunkIndex = ID.Index >> CHUNK_SHIFT;
        uint16 LocalIndex = ID.Index & CHUNK_MASK;

        if (ChunkIndex >= MAX_CHUNKS) {
            UE_LOG(NBTSystem, Error, TEXT("NBT Allocator out of bounds! Cannot AllocateAt ID %s"), *ID.ToString());
            return nullptr;
        }

        while (ChunkIndex >= Chunks.Num()) { AllocateNewChunk(); } // 用于网络同步构建相同的内存布局, 确保有足够的chunks

        FAttributeChunk* Chunk = Chunks[ChunkIndex].Get();
       
        const auto Result = Chunk->AllocateSlotAt(LocalIndex, ID.Generation);
        
        if (Result == FAttributeChunkAllocateAtResult::Exist) {
            return reinterpret_cast<FNBTAttribute*>(Chunk->AttributeBuffer) + LocalIndex;
        } else if (Result == FAttributeChunkAllocateAtResult::NewOne) {
            Stats.TotalAllocated++;
            Stats.CurrentActive++;
            Stats.PeakActive = FMath::Max(Stats.PeakActive, Stats.CurrentActive);
            return reinterpret_cast<FNBTAttribute*>(Chunk->AttributeBuffer) + LocalIndex;
        } else if (Result == FAttributeChunkAllocateAtResult::Replaced) {
            Stats.TotalAllocated++;
            return reinterpret_cast<FNBTAttribute*>(Chunk->AttributeBuffer) + LocalIndex;
        }
        
        return nullptr;
    }

    // 释放属性
    bool Deallocate(FNBTAttributeID ID) {
        if (!ID.IsValid()) return false;

        uint16 ChunkIndex = ID.Index >> CHUNK_SHIFT;
        uint16 LocalIndex = ID.Index & CHUNK_MASK;
        
        if (!Chunks.IsValidIndex(ChunkIndex)) return false;

        if (Chunks[ChunkIndex]->DeallocateSlot(LocalIndex, ID.Generation)) {
            // 更新统计
            Stats.TotalDeallocated++;
            Stats.CurrentActive--;
            return true;
        }

        return false;
    }

    //获取版本
    int32* GetNodeVersion(FNBTAttributeID ID) const {
        if (!ID.IsValid()) return nullptr;

        uint16 ChunkIndex = ID.Index >> CHUNK_SHIFT;
        uint16 LocalIndex = ID.Index & CHUNK_MASK;

        if (ChunkIndex >= Chunks.Num()) return nullptr;

        FAttributeChunk* Chunk = Chunks[ChunkIndex].Get();
        if (Chunk->Meta.Generations[LocalIndex] != ID.Generation) return nullptr;

        return Chunk->GetVersion(LocalIndex);
    }

    int32* GetNodeSubtreeVersion(FNBTAttributeID ID) const {
        
        if (!ID.IsValid()) return nullptr;

        uint16 ChunkIndex = ID.Index >> CHUNK_SHIFT;
        uint16 LocalIndex = ID.Index & CHUNK_MASK;

        if (ChunkIndex >= Chunks.Num()) return nullptr;

        FAttributeChunk* Chunk = Chunks[ChunkIndex].Get();
        if (Chunk->Meta.Generations[LocalIndex] != ID.Generation) return nullptr;
        
        return Chunk->GetSubtreeVersion(LocalIndex);
    }

    void IncNodeSubtreeVersion(FNBTAttributeID ID) const {
        if (int32* P = GetNodeSubtreeVersion(ID)) { ++(*P); }
    }

    bool IsNodeValid(FNBTAttributeID ID) const {
        if (!ID.IsValid()) return false;

        uint16 ChunkIndex = ID.Index >> CHUNK_SHIFT;
        uint16 LocalIndex = ID.Index & CHUNK_MASK;

        if (ChunkIndex >= Chunks.Num()) return false;
        return Chunks[ChunkIndex]->IsIndexValid(LocalIndex);
    }

    // 获取属性
    FNBTAttribute* GetAttribute(FNBTAttributeID ID) {
        if (!ID.IsValid()) {
            UE_LOG(NBTSystem, VeryVerbose, TEXT("GetAttribute called with invalid ID"));
            return nullptr;
        }

        uint16 ChunkIndex = ID.Index >> CHUNK_SHIFT;
        uint16 LocalIndex = ID.Index & CHUNK_MASK;

        if (ChunkIndex >= Chunks.Num()) {
            UE_LOG(NBTSystem, Warning, TEXT("GetAttribute: ChunkIndex %d out of range"), ChunkIndex);
            return nullptr;
        }

        FAttributeChunk* Chunk = Chunks[ChunkIndex].Get();

        if (Chunk->Meta.Generations[LocalIndex] != ID.Generation) return nullptr;

        return Chunk->GetAttribute(LocalIndex);
    }

    FNBTAttribute* GetAttribute(FNBTAttributeID ID) const {
        return const_cast<FNBTAllocator*>(this)->GetAttribute(ID);
    }

    // 批量操作优化
    template <typename Func>
    void ForEachAttribute(Func&& Function) {
        for (const auto& Chunk : Chunks) {
            if (Chunk->Meta.UsedCount == 0) continue;

            FNBTAttribute* Attributes = reinterpret_cast<FNBTAttribute*>(Chunk->AttributeBuffer);
            uint64 Mask = Chunk->Meta.UsedMask;

            while (Mask) {
                uint32 LocalIndex = FMath::CountTrailingZeros64(Mask);
                uint16 GlobalIndex = static_cast<uint16>((Chunk->Meta.ChunkIndex << CHUNK_SHIFT) | LocalIndex);
                FNBTAttributeID ID(GlobalIndex, Chunk->Meta.Generations[LocalIndex]);

                Function(ID, Attributes[LocalIndex]);

                Mask &= Mask - 1;
            }
        }
    }

    // 统计信息
    uint32 GetTotalAllocated() const { return Stats.TotalAllocated; } //总共分配过多少次
    uint32 GetTotalDeallocated() const { return Stats.TotalDeallocated; } //总共删除过多少次
    uint32 GetCurrentActive() const { return Stats.CurrentActive; } // 当前数量
    uint32 GetPeakActive() const { return Stats.PeakActive; } // 最高使用
    uint32 GetChunkCount() const { return Chunks.Num(); } //当前块数量
    uint32 GetFreeRemaining() const { return 65535 - GetCurrentActive(); }

    const FNBTAttributeChunkMetaData* GetChunkMetadata(int32 ChunkIndex) const {
        if (Chunks.IsValidIndex(ChunkIndex)) {
            return &Chunks[ChunkIndex]->Meta;
        }
        return nullptr;
    }

    // 内存使用估算
    SIZE_T GetMemoryUsage() const { return Chunks.Num() * sizeof(FAttributeChunk); }

private:
    // 分配新块
    uint16 AllocateNewChunk() {
        uint16 NewIndex = Chunks.Num();
        check(NewIndex < MAX_CHUNKS);
        Chunks.Add(MakeUnique<FAttributeChunk>(NewIndex));
        return NewIndex;
    }

    // 根据策略选择块
    uint16 SelectOrCreateChunkForBestAllocation() {
        bool bHasFreeSlots = false;
        
        uint16 BestChunk = 0;
        uint8 MaxUsed = 0;
        
        for (int i = 0; i < Chunks.Num(); ++i){
            if (!Chunks[i]->HasFreeSlot()) continue;
            uint8 Used = Chunks[i]->GetUsedCount();
            if (Used > MaxUsed) {
                MaxUsed = Used;
                BestChunk = i;
                bHasFreeSlots = true;
            }
        }
        
        if (!bHasFreeSlots) {
            BestChunk = AllocateNewChunk();
        }
        
        return BestChunk;
    }
};