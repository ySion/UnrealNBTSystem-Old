#include "NBTContainer.h"

#include "NBTAccessor.h"
#include "NBTComponent.h"

FNBTContainer::FNBTContainer() {
    Initialize();
}

FNBTContainer& FNBTContainer::operator=(const FNBTContainer& Other) {
    if (this == &Other) return *this;
    if (GetContainerDataVersion() == 0 &&
        GetContainerStructVersion() == 0 &&
        Other.GetContainerDataVersion() == 0 &&
        Other.GetContainerStructVersion() == 0 &&
        GetNodeCount() == 1 &&
        Other.GetNodeCount() == 1) {
        return *this;
    } else {
        CopyFrom(Other);
    }
    return *this;
}

bool FNBTContainer::IsRemainingSpaceSupportCopy(FNBTAttributeID SourceID, const FNBTContainer& Source) {
    auto* SourceAttr = Source.GetAttribute(SourceID);
    if (!SourceAttr) return false;
    
    int SubTreeNode = Source.GetNodeNumRecursive(SourceID);
    if (SubTreeNode == 0) return false;
    if (Allocator.GetFreeRemaining() < static_cast<uint32>(SubTreeNode)) return false;

    return true;
}

bool FNBTContainer::IsRemainingSpaceSupportDoubleCopy(FNBTAttributeID A, FNBTAttributeID B) {
    auto* Attr_A = GetAttribute(A);
    auto* Attr_B = GetAttribute(B);
    if (!Attr_A || !Attr_B) return false;
    
    int SubTreeNodeA = GetNodeNumRecursive(A);
    int SubTreeNodeB = GetNodeNumRecursive(B);
    
    if (SubTreeNodeA == 0 || SubTreeNodeB == 0) return false;
    if (Allocator.GetFreeRemaining() < static_cast<uint32>(SubTreeNodeA + SubTreeNodeB)) return false;

    return true;
}

FNBTAttributeID FNBTContainer::DeepCopyNode(FNBTAttributeID SourceID, const FNBTContainer& Source) {
    if (!IsRemainingSpaceSupportCopy(SourceID, Source)) return FNBTAttributeID();
    return DeepCopyNodeImpl(SourceID, Source);
}

FNBTAttributeID FNBTContainer::DeepCopyNodeImpl(FNBTAttributeID SourceID, const FNBTContainer& Source) {
    
    auto* SourceAttr = Source.GetAttribute(SourceID);
    
    auto NewID = AllocateNode(); //不需要检查是否有效, 因为上面经过IsRemainingSpaceSupportCopy检查, 一定是分配成功的.
    auto* NewAttr = GetAttribute(NewID);

    if (SourceAttr->GetType() == ENBTAttributeType::Map) {
        NewAttr->OverrideToEmptyMap();
        if (auto* SourceMapData = SourceAttr->GetMapData()) {
            if (auto* NewMapData = NewAttr->GetMapData()) {
                for (auto& KV : SourceMapData->Children) {
                    auto NewChildID = DeepCopyNodeImpl(KV.Value, Source);
                    if (NewChildID.IsValid()) {
                        NewMapData->Children.Emplace(KV.Key, NewChildID);
                    }
                }
            }
        }
    } else if (SourceAttr->GetType() == ENBTAttributeType::List) {
        NewAttr->OverrideToEmptyList();
        if (auto* SourceListData = SourceAttr->GetListData()) {
            if (auto* NewListData = NewAttr->GetListData()) {
                for (FNBTAttributeID ChildID : SourceListData->Children) {
                    auto NewChildID = DeepCopyNodeImpl(ChildID, Source);
                    if (NewChildID.IsValid()) {
                        NewListData->Children.Add(NewChildID);
                    }
                }
            }
        }
    } else {
        NewAttr->OverrideFromIfNotCompound(*SourceAttr);
    }

    return NewID;
}

void FNBTContainer::Initialize() {
    CreateLiveToken();
    RootID = AllocateNode();
    auto* Root = Allocator.GetAttribute(RootID);
    Root->OverrideToEmptyMap();
}

void FNBTContainer::Clear() {
    Allocator.Reset();
    RootID = FNBTAttributeID();
}

void FNBTContainer::Reset() {
    Allocator.Reset();
    RootID = AllocateNode();
    auto* Root = Allocator.GetAttribute(RootID);
    Root->OverrideToEmptyMap();
    ContainerDataVersion ++;
    ContainerStructVersion ++;
}

void FNBTContainer::CopyFrom(const FNBTContainer& Other) {
    if (this == &Other) return;
    bShouldOperatorEffectVersion = Other.bShouldOperatorEffectVersion;
    Allocator.Reset();
    RootID = DeepCopyNode(Other.RootID, Other);
    //ContainerDataVersion = Other.ContainerDataVersion;
    //ContainerStructVersion = Other.ContainerStructVersion;
    ContainerDataVersion ++;
    ContainerStructVersion ++;
}

void FNBTContainer::MarkDirtyThisFrame() {
    if (!bDirtyThisFrame) {
        bDirtyThisFrame = true;
        if (ParentComponent.IsValid()) {
            ParentComponent->RequestTickNextFrame();
        }
    }
}

FNBTAttributeID FNBTContainer::AllocateNode() {
    auto ID = Allocator.Allocate();
    return ID;
}

int32 FNBTContainer::ReleaseNode(FNBTAttributeID ID) {
    if (!ID.IsValid()) return 0;
    return Allocator.Deallocate(ID) ? 1 : 0;
}

int32 FNBTContainer::ReleaseRecursive(FNBTAttributeID ID) {
    int32 ClearNum = 0;
    ReleaseSubtreeImp(ID, ClearNum);
    return ClearNum;
}

void FNBTContainer::ReleaseSubtreeImp(FNBTAttributeID ID, int32& ClearNum) {
    if (!ID.IsValid()) return;

    auto* Attr = Allocator.GetAttribute(ID);
    if (!Attr) return;

    // 递归释放子节点
    if (auto* MapData = Attr->GetMapData()) {
        for (const auto& KV : MapData->Children) {
            ReleaseSubtreeImp(KV.Value, ClearNum);
        }
    } else if (auto* ListData = Attr->GetListData()) {
        for (const auto& i : ListData->Children) {
            ReleaseSubtreeImp(i, ClearNum);
        }
    }

    ClearNum += ReleaseNode(ID);
}

int32 FNBTContainer::GetNodeNumRecursive(FNBTAttributeID ID) const {
    int32 ClearNum = 0;
    GetNodeNumRecursiveImp(ID, ClearNum);
    return ClearNum;
}

void FNBTContainer::GetNodeNumRecursiveImp(FNBTAttributeID ID, int32& ClearNum) const {
    if (!ID.IsValid()) return;

    auto* Attr = Allocator.GetAttribute(ID);
    if (!Attr) return;

    // 递归释放子节点
    if (auto* MapData = Attr->GetMapData()) {
        for (const auto& KV : MapData->Children) {
            GetNodeNumRecursiveImp(KV.Value, ClearNum);
        }
    } else if (auto* ListData = Attr->GetListData()) {
        for (const auto& i : ListData->Children) {
            GetNodeNumRecursiveImp(i, ClearNum);
        }
    }

    ClearNum += 1;
}

int32 FNBTContainer::ReleaseChildren(FNBTAttributeID ID) {
    if (!ID.IsValid()) return 0;

    auto* Attr = Allocator.GetAttribute(ID);
    if (!Attr) return 0;

    int32 ClearNum = 0;

    if (auto* MapData = Attr->GetMapData()) {
        for (const auto& KV : MapData->Children) {
            ReleaseSubtreeImp(KV.Value, ClearNum);
        }
        MapData->Children.Reset();
    } else if (auto* ListData = Attr->GetListData()) {
        for (const auto& ChildID : ListData->Children) {
            ReleaseSubtreeImp(ChildID, ClearNum);
        }
        ListData->Children.Reset();
    }

    return ClearNum;
}

void FNBTContainer::UpdateNodeDataVersion(FNBTAttributeID ID) {
    if (const auto Ptr = GetAttributeVersion(ID); Ptr) {
        (*Ptr)++;
        UpdateContainerDataVersion();
    }
}

void FNBTContainer::RebuildAllParents() {
    ParentOf.Reset();
    RebuildParentsForNode(RootID);
}

void FNBTContainer::RebuildParentsForNode(FNBTAttributeID ParentID) {
    if (!ParentID.IsValid()) return;
    if (auto* Attr = Allocator.GetAttribute(ParentID)) { // 访问当前节点
        if (auto* MapData = Attr->GetMapData()) {
            for (const auto& KV : MapData->Children) {
                ParentOf.FindOrAdd(KV.Value) = ParentID;
                RebuildParentsForNode(KV.Value);
            }
        } else if (auto* ListData = Attr->GetListData()) {
            for (const auto& ChildID : ListData->Children) {
                ParentOf.FindOrAdd(ChildID) = ParentID;
                RebuildParentsForNode(ChildID);
            }
        }
    }
}

void FNBTContainer::RebuildParentsForDirectChildren(FNBTAttributeID ParentID) {
    if (!ParentID.IsValid()) return;
    if (auto* Attr = Allocator.GetAttribute(ParentID)) {
        // 1) 先收集该父节点“旧记录”的子集合（用来清理已删除的条目）
        TSet<FNBTAttributeID> OldChildren;
        for (auto& P : ParentOf) {
            if (P.Value == ParentID) OldChildren.Add(P.Key);
        }

        // 2) 按当前数据重建
        if (auto* MapData = Attr->GetMapData()) {
            for (const auto& KV : MapData->Children) {
                ParentOf.FindOrAdd(KV.Value) = ParentID;
                OldChildren.Remove(KV.Value);
            }
        } else if (auto* ListData = Attr->GetListData()) {
            for (const auto& ChildID : ListData->Children) {
                ParentOf.FindOrAdd(ChildID) = ParentID;
                OldChildren.Remove(ChildID);
            }
        }

        // 3) 清理不再存在的旧子
        for (auto& Gone : OldChildren) ParentOf.Remove(Gone);
    }
}

void FNBTContainer::BubbleSubtreeVersionAlongPathForID(FNBTAttributeID LeafID) {
    if (!FrameBubbleUniqueKey.Contains(LeafID)) {
        Allocator.IncNodeSubtreeVersion(LeafID);
        FrameBubbleUniqueKey.Add(LeafID);
    }

    FNBTAttributeID Cur = LeafID;
    while (FNBTAttributeID* P = ParentOf.Find(Cur)) {
        if (!FrameBubbleUniqueKey.Contains(*P)) {
            Allocator.IncNodeSubtreeVersion(*P);
            FrameBubbleUniqueKey.Add(*P);
        }
        Cur = *P;
    }
    if (!FrameBubbleUniqueKey.Contains(RootID)) {
        Allocator.IncNodeSubtreeVersion(RootID);
        FrameBubbleUniqueKey.Add(RootID);
    }
}

FNBTDataAccessor FNBTContainer::GetAccessor() const {
    FNBTDataAccessor Data = FNBTDataAccessor(const_cast<FNBTContainer*>(this), LiveToken.ToWeakPtr());
    Data.CachedAttributeID = RootID;
    Data.CachedContainerStructVersion = ContainerStructVersion;
    return Data;
}

FString FNBTContainer::ToString() const {
    FString Result;

    // 添加容器元信息
    Result += FString::Printf(TEXT("=== NBT Container ===\n"));
    Result += FString::Printf(TEXT("Data Version: %d\n"), ContainerDataVersion);
    Result += FString::Printf(TEXT("Struct Version: %d\n"), ContainerStructVersion);
    Result += FString::Printf(TEXT("Node Count: %d\n"), GetNodeCount());
    Result += FString::Printf(TEXT("Memory Usage: %llu bytes\n"), Allocator.GetMemoryUsage());
    Result += FString::Printf(TEXT("Op Effect Version: %s\n"), bShouldOperatorEffectVersion ?
       TEXT("True") : TEXT("False"));
    Result += FString::Printf(TEXT("===================\n"));

    // 获取根节点的访问器并打印内容
    FNBTContainer* MutableThis = const_cast<FNBTContainer*>(this);
    FNBTDataAccessor RootAccessor = MutableThis->GetAccessor();

    if (RootAccessor.IsDataExists()) {
        Result += TEXT("Root Info:");
        Result += RootAccessor.ToString();
    } else {
        Result += TEXT("$Empty Container$");
    }

    return Result;
}

FString FNBTContainer::ToDebugString() const {
    FString Result;

    Result += FString::Printf(TEXT("=== NBT Container Debug Info ===\n"));
    Result += FString::Printf(TEXT("Container Address: %p\n"), this);
    Result += FString::Printf(TEXT("LiveToken Valid: %s\n"), LiveToken.IsValid() ? TEXT("Yes") : TEXT("No"));
    Result += FString::Printf(TEXT("Root ID: %s\n"), *RootID.ToString());
    Result += FString::Printf(TEXT("Data Version: %d\n"), ContainerDataVersion);
    Result += FString::Printf(TEXT("Struct Version: %d\n"), ContainerStructVersion);
    Result += FString::Printf(TEXT("Op Effect Version: %s\n"), bShouldOperatorEffectVersion ?
       TEXT("True") : TEXT("False"));
    Result += FString::Printf(TEXT("\n--- Allocator Stats ---\n"));
    Result += FString::Printf(TEXT("Total Allocated: %u\n"), Allocator.GetTotalAllocated());
    Result += FString::Printf(TEXT("Total Deallocated: %u\n"), Allocator.GetTotalDeallocated());
    Result += FString::Printf(TEXT("Current Active: %u\n"), Allocator.GetCurrentActive());
    Result += FString::Printf(TEXT("Peak Active: %u\n"), Allocator.GetPeakActive());
    Result += FString::Printf(TEXT("Chunk Count: %u\n"), Allocator.GetChunkCount());
    Result += FString::Printf(TEXT("Free Remaining: %u\n"), Allocator.GetFreeRemaining());
    Result += FString::Printf(TEXT("Memory Usage: %llu bytes\n"), Allocator.GetMemoryUsage());
    Result += FString::Printf(TEXT("================================\n\n"));

    // 内容
    FNBTContainer* MutableThis = const_cast<FNBTContainer*>(this);
    FNBTDataAccessor RootAccessor = MutableThis->GetAccessor();

    if (RootAccessor.IsDataExists()) {
        Result += TEXT("Root Info:");
        Result += RootAccessor.ToString(true);
    } else {
        Result += TEXT("$Empty Container$");
    }

    return Result;
}

bool FNBTContainer::ValidateIntegrity() const {
    if (!RootID.IsValid()) {
        UE_LOG(NBTSystem, Error, TEXT("Container integrity check failed: Invalid root ID"));
        return false;
    }

    // 验证根节点
    auto* RootAttr = GetAttribute(RootID);
    if (!RootAttr) {
        UE_LOG(NBTSystem, Error, TEXT("Container integrity check failed: Root attribute not found"));
        return false;
    }

    if (RootAttr->GetType() != ENBTAttributeType::Map) {
        UE_LOG(NBTSystem, Error, TEXT("Container integrity check failed: Root is not a Map"));
        return false;
    }

    // 递归验证所有节点
    TSet<FNBTAttributeID> VisitedNodes;
    return ValidateNodeRecursive(RootID, VisitedNodes);
}

bool FNBTContainer::ValidateNodeRecursive(FNBTAttributeID NodeID, TSet<FNBTAttributeID>& VisitedNodes) const {
    if (!NodeID.IsValid()) {
        return false;
    }

    // 检查循环引用
    if (VisitedNodes.Contains(NodeID)) {
        UE_LOG(NBTSystem, Error, TEXT("Circular reference detected at node %s"), *NodeID.ToString());
        return false;
    }

    VisitedNodes.Add(NodeID);

    auto* Attr = GetAttribute(NodeID);
    if (!Attr) {
        UE_LOG(NBTSystem, Error, TEXT("Node %s exists in structure but attribute not found"), *NodeID.ToString());
        return false;
    }

    // 递归验证子节点
    if (auto* MapData = Attr->GetMapData()) {
        for (const auto& Pair : MapData->Children) {
            if (!ValidateNodeRecursive(Pair.Value, VisitedNodes)) {
                return false;
            }
        }
    } else if (auto* ListData = Attr->GetListData()) {
        for (const auto& ChildID : ListData->Children) {
            if (!ValidateNodeRecursive(ChildID, VisitedNodes)) {
                return false;
            }
        }
    }

    VisitedNodes.Remove(NodeID);
    return true;
}

FArzNBTContainerStats FNBTContainer::GetStatistics() const {
    FArzNBTContainerStats Stats;
    if (RootID.IsValid()) {
        int32 CurrentDepth = 0;
        GetNodeStatisticsRecursive(RootID, Stats, CurrentDepth);
    }
    return Stats;
}

void FNBTContainer::GetNodeStatisticsRecursive(FNBTAttributeID NodeID, FArzNBTContainerStats& Stats, int32& CurrentDepth) const {
    if (!NodeID.IsValid()) return;
    
    auto* Attr = GetAttribute(NodeID);
    if (!Attr) return;
    
    Stats.TotalNodes++;
    Stats.MaxDepth = FMath::Max(Stats.MaxDepth, CurrentDepth);
    
    ENBTAttributeType Type = Attr->GetType();
    Stats.TypeCounts.FindOrAdd(Type)++;
    
    if (Type == ENBTAttributeType::Map) {
        Stats.MapNodes++;
        if (auto* MapData = Attr->GetMapData()) {
            CurrentDepth++;
            for (const auto& Pair : MapData->Children) {
                GetNodeStatisticsRecursive(Pair.Value, Stats, CurrentDepth);
            }
            CurrentDepth--;
        }
    } else if (Type == ENBTAttributeType::List) {
        Stats.ListNodes++;
        if (auto* ListData = Attr->GetListData()) {
            CurrentDepth++;
            for (const auto& ChildID : ListData->Children) {
                GetNodeStatisticsRecursive(ChildID, Stats, CurrentDepth);
            }
            CurrentDepth--;
        }
    } else {
        Stats.ValueNodes++;
    }
}

bool FNBTContainer::SerializeData(FArchive& Ar, bool NetWorkMode) {
    if (NetWorkMode) {
        Ar << bIsContainerReplicated;
        Ar << ContainerDataVersion;
        Ar << ContainerStructVersion;
    }
    if (Ar.IsLoading()) {
        Clear();
    }
    
    Ar << RootID;
    
    uint32 ActiveNodeCount = Allocator.GetCurrentActive();
    Ar << ActiveNodeCount;
    if (Ar.IsLoading()) {
        
        for (uint32 i = 0; i < ActiveNodeCount; ++i) {
            FNBTAttributeID NodeID;
            Ar << NodeID;
            
            FNBTAttribute* NewAttr = Allocator.AllocateAt(NodeID);
            if (NewAttr) {
                NewAttr->SerializeNBTData(Ar, NetWorkMode);
            } else {
                UE_LOG(NBTSystem, Error, TEXT("FNBTContainer::SerializeData: Failed to allocate attribute at ID %s during loading. Archive may be corrupt."),
                       *NodeID.ToString());
                Ar.SetError();
                return false;
            }
        }

        // 从磁盘上加载之后默认记录变更, 但是网络同步不允许
        if (!NetWorkMode) UpdateContainerDataAndStructVersion();
    } else {
        Allocator.ForEachAttribute([&](FNBTAttributeID NodeID, FNBTAttribute& Attr) {
            Ar << NodeID;
            Attr.SerializeNBTData(Ar, NetWorkMode);
        });
    }
    
    return true;
}

bool FNBTContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms) {
    if (DeltaParms.bUpdateUnmappedObjects) {
        return true;
    }
    
    // =======================================================================
    // 服务器增量同步
    // =======================================================================
    if (DeltaParms.Writer) {
      
        FBitWriter& Writer = *DeltaParms.Writer;
        FArzNBTContainerBaseState* OldState = static_cast<FArzNBTContainerBaseState*>(DeltaParms.OldState);
        // 全量同步
        if (OldState == nullptr) {
            
            if (ContainerDataVersion == 0 && ContainerStructVersion == 0) { //如果是空的, 那么没必要同步, 有数据的时候再进行初始化
                return false;
            }
            
            bIsContainerReplicated = true;
            
            Writer.WriteBit(true);
            SerializeData(Writer, true);
            TSharedPtr<FArzNBTContainerBaseState> NewState = MakeShared<FArzNBTContainerBaseState>();
            NewState->CreateVersionSnapshotFromContainer(*this);
            *DeltaParms.NewState = NewState;
            UE_LOG(NBTSystem, Log, TEXT("NBTContainer: Sent initial full sync. Size: %lld bytes"), Writer.GetNumBytes());
            return true;
        }

        if (OldState->ContainerVersion == ContainerDataVersion) { //什么都没改
            return false;
        }

        //FString DebugRecord = "Delta Sync Data: \n";
        //DebugRecord += "    " + FString::FromInt(ContainerDataVersion) + "\n";
        //DebugRecord += "    " + FString::FromInt(ContainerStructVersion) + "\n";
        // 增量同步
        Writer.WriteBit(false);
        Writer << ContainerDataVersion;
        Writer << ContainerStructVersion;
        const int32 NumChunksMain = Allocator.GetChunkCount();
        const int32 NumChunksState = OldState->VersionChunks.Num();

        const int32 MaxChunks = FMath::Max(NumChunksMain, NumChunksState);

        TArray<FNBTAttributeID> Added;
        TArray<FNBTAttributeID> Modified;

        for (int32 ChunkIdx = 0; ChunkIdx < MaxChunks; ++ChunkIdx) {
            const FNBTAttributeChunkMetaData* MainChunkMeta = Allocator.GetChunkMetadata(ChunkIdx);
            const FNBTAttributeChunkMetaData* StateChunkMeta = OldState->VersionChunks.IsValidIndex(ChunkIdx) ? &OldState->VersionChunks[ChunkIdx] : nullptr;

            // 快速优化: 如果两个块都存在, 并且完全相同, 直接跳过
            if (MainChunkMeta && StateChunkMeta && FMemory::Memcmp(MainChunkMeta, StateChunkMeta, sizeof(FNBTAttributeChunkMetaData)) == 0) {
                continue;
            }
            const uint64 MainMask = MainChunkMeta ? MainChunkMeta->UsedMask : 0;
            const uint64 StateMask = StateChunkMeta ? StateChunkMeta->UsedMask : 0;

            if (MainMask == 0 && StateMask == 0) {
                continue;
            }
            // 找出所有需要检测的Slot
            
            uint64 CombinedMask = MainMask | StateMask;
            while (CombinedMask) {
                const uint32 LocalIndex = FMath::CountTrailingZeros64(CombinedMask);
                const uint64 CurrentBit = (1ULL << LocalIndex);
                const bool bIsInMain = (MainMask & CurrentBit) != 0;
                const bool bIsInState = (StateMask & CurrentBit) != 0;
                const uint16 GlobalIndex = (ChunkIdx << FNBTAllocator::CHUNK_SHIFT) | LocalIndex;
                if (bIsInMain && !bIsInState) { // add
                    FNBTAttributeID CurrentID(GlobalIndex, MainChunkMeta->Generations[LocalIndex]);
                    Added.Add(CurrentID);
                } else if (!bIsInMain && bIsInState) {  // remove
                    FNBTAttributeID OldID(GlobalIndex, StateChunkMeta->Generations[LocalIndex]);
                    uint8 Op = static_cast<uint8>(EArzNBTDeltaOp::Remove);
                    Writer << Op;
                    Writer << OldID;
                } else if (bIsInMain && bIsInState) {// modified
                    if (MainChunkMeta->Versions[LocalIndex] != StateChunkMeta->Versions[LocalIndex] ||
                        MainChunkMeta->Generations[LocalIndex] != StateChunkMeta->Generations[LocalIndex]) {
                        FNBTAttributeID CurrentID(GlobalIndex, MainChunkMeta->Generations[LocalIndex]);
                        Modified.Add(CurrentID);
                    }
                }
                
                CombinedMask &= ~CurrentBit; // 移除当前bit(Index)
            }
        }

        for (FNBTAttributeID& CurrentID : Added) {
            if (FNBTAttribute* Attr = Allocator.GetAttribute(CurrentID)) {
                uint8 Op = static_cast<uint8>(EArzNBTDeltaOp::Add);
                Writer << Op;
                Writer << CurrentID;
                Attr->SerializeNBTData(Writer, true);
            }
        }

        for (FNBTAttributeID& CurrentID : Modified) {
            if (FNBTAttribute* Attr = Allocator.GetAttribute(CurrentID)) {
                uint8 Op = static_cast<uint8>(EArzNBTDeltaOp::Update);
                Writer << Op;
                Writer << CurrentID;
                Attr->SerializeNBTData(Writer, true);
            }
        }
        
        uint8 EndOp = static_cast<uint8>(EArzNBTDeltaOp::EndOfDeltas);
        Writer << EndOp;

        FArzNBTContainerBaseState* NewState = new FArzNBTContainerBaseState();
        NewState->CreateVersionSnapshotFromContainer(*this);
        *DeltaParms.NewState = TSharedPtr<INetDeltaBaseState>(NewState);
        // UE_LOG(NBTSystem, Log, TEXT("NBTContainer: Sent delta sync. Size: %lld bytes"), Writer.GetNumBytes());
        // UE_LOG(NBTSystem, Log, TEXT("%s"), *DebugRecord);
        return true;
    }
    // =======================================================================
    // 客户端增量读取
    // =======================================================================
    if (DeltaParms.Reader) {
        FBitReader& Reader = *DeltaParms.Reader;
        bShouldOperatorEffectVersion = false; //这里是客户端, 所以所有操作都不会影响版本
        if (static_cast<bool>(Reader.ReadBit())) {
            // 全量
            // UE_LOG(NBTSystem, Log, TEXT("NBTContainer: Receiving full sync. Size: %lld bytes"), Reader.GetNumBytes());
            Clear();
            SerializeData(Reader, true); // Rebuild from scratch
        } else {
            // 增量 
            // UE_LOG(NBTSystem, Log, TEXT("NBTContainer: Receiving delta sync."));

            FrameBubbleUniqueKey.Reset();
            
            auto PreContainerStructVersion = ContainerStructVersion;
            bool Rebuilded = false;
            
            Reader << ContainerDataVersion;
            Reader << ContainerStructVersion;

            if (PreContainerStructVersion != ContainerStructVersion) {                                                                                                                                     
                RebuildAllParents();                                                                                                                                                                       
                Rebuilded = true;                                                                                                                                                                          
            } 

            while (!Reader.AtEnd() && !Reader.IsError()) {
                uint8 OpCode;
                
                Reader << OpCode;
                EArzNBTDeltaOp Op = static_cast<EArzNBTDeltaOp>(OpCode);
                if (Op == EArzNBTDeltaOp::EndOfDeltas) break;
                
                FNBTAttributeID ID;
                Reader << ID;
                
                if (Op == EArzNBTDeltaOp::Remove) {
                    BubbleSubtreeVersionAlongPathForID(ID);
                    ReleaseNode(ID);
                } else if (Op == EArzNBTDeltaOp::Add) {
                    if (!Rebuilded && PreContainerStructVersion != ContainerStructVersion) {
                        RebuildAllParents();
                        Rebuilded = true;
                    }
                    if (FNBTAttribute* Attr = Allocator.AllocateAt(ID)) {
                        Attr->SerializeNBTData(Reader, true);
                        BubbleSubtreeVersionAlongPathForID(ID);
                    } else {
                        UE_LOG(NBTSystem, Error, TEXT("NBTContainer: Failed to AllocateAt ID %s on client."), *ID.ToString());
                        Reader.SetError();
                        return false;
                    }
                } else if (Op == EArzNBTDeltaOp::Update) {
                    if (FNBTAttribute* Attr = Allocator.AllocateAt(ID)) {
                        Attr->SerializeNBTData(Reader, true);
                        BubbleSubtreeVersionAlongPathForID(ID);
                    } else {
                        UE_LOG(NBTSystem, Error, TEXT("NBTContainer: Failed to AllocateAt ID %s on client."), *ID.ToString());
                        Reader.SetError();
                        return false;
                    }
                } else {
                    UE_LOG(NBTSystem, Error, TEXT("NBTContainer: Invalid NBT Delta Op received: %d"), OpCode);
                    Reader.SetError();
                    return false;
                }
            }
        }
    }
    return true;
}

bool FNBTContainer::Serialize(FArchive& Ar) {
    return SerializeData(Ar, false);
}

FArchive& operator<<(FArchive& Ar, FNBTContainer& S) {
    S.SerializeData(Ar, false);
    return Ar;
}