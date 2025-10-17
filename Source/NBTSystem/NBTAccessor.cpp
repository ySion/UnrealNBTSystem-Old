#include "NBTAccessor.h"
#include "NBTAccessor.inl"
#include "NBTContainer.h"
#include "AngelscriptManager.h"
#include "NBTComponent.h"

FNBTDataAccessor FNBTDataAccessor::MakeAccessFromFName(FName Key) const {
    FNBTDataAccessor NewAccessor;
    NewAccessor.Container = this->Container;
    NewAccessor.ContainerLiveToken = this->ContainerLiveToken;
    NewAccessor.Path = this->Path;
    NewAccessor.Path.Add(TVariant<FName, int32>(TInPlaceType<FName>(), Key));

    if (CachedAttributePtr) {
        if (auto* MapData = CachedAttributePtr->GetMapData()) {
            auto It = MapData->Children.Find(Key);
            if (It) {
                NewAccessor.CachedAttributeID = *It;
                NewAccessor.CachedContainerStructVersion = this->CachedContainerStructVersion;
                NewAccessor.CachedAttributeVersionPtr = Container->GetAttributeVersion(NewAccessor.CachedAttributeID);
                NewAccessor.CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(
                    NewAccessor.CachedAttributeID);
                return NewAccessor;
            }
        }
    }

    return NewAccessor;
}

FNBTDataAccessor FNBTDataAccessor::MakeAccessFromIntIndex(int32 Index) const {
    FNBTDataAccessor NewAccessor;

    NewAccessor.Container = this->Container;
    NewAccessor.ContainerLiveToken = this->ContainerLiveToken;
    NewAccessor.Path = this->Path;
    NewAccessor.Path.Add(TVariant<FName, int32>(TInPlaceType<int32>(), Index));

    if (CachedAttributePtr) {
        if (auto* ListData = CachedAttributePtr->GetListData()) {
            if (ListData->Children.IsValidIndex(Index)) {
                const auto& Data = ListData->Children[Index];
                NewAccessor.CachedAttributeID = Data;
                NewAccessor.CachedContainerStructVersion = this->CachedContainerStructVersion;
                NewAccessor.CachedAttributeVersionPtr = Container->GetAttributeVersion(NewAccessor.CachedAttributeID);
                NewAccessor.CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(
                    NewAccessor.CachedAttributeID);
                return NewAccessor;
            }
        }
    }

    return NewAccessor;
}

FNBTDataAccessor FNBTDataAccessor::GetParent() const {
    if (!IsContainerValid() || Path.Num() == 0)
        return FNBTDataAccessor();

    FNBTDataAccessor Parent;
    Parent.Container = this->Container;
    Parent.ContainerLiveToken = this->ContainerLiveToken;
    Parent.Path = this->Path;
    Parent.Path.RemoveAt(Parent.Path.Num() - 1);

    auto Res = Parent.ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Res != ENBTAttributeOpResult::Success) {
        return FNBTDataAccessor();
    }

    return Parent;
}

FNBTDataAccessor FNBTDataAccessor::GetParentPreview() const {
    if (!IsContainerValid() || Path.Num() == 0)
        return FNBTDataAccessor();

    FNBTDataAccessor Parent;
    Parent.Container = this->Container;
    Parent.ContainerLiveToken = this->ContainerLiveToken;
    Parent.Path = this->Path;
    Parent.Path.RemoveAt(Parent.Path.Num() - 1);

    return Parent;
}

bool FNBTDataAccessor::IsAncestor(const FNBTDataAccessor& P, const FNBTDataAccessor& C) {
    if (P.Container != C.Container) return false;
    if (P.Path.Num() > C.Path.Num()) return false;
    for (int i = 0; i < P.Path.Num(); ++i) {
        const auto& a = P.Path[i];
        const auto& b = C.Path[i];
        if (a.GetIndex() != b.GetIndex()) return false;
        if (a.IsType<FName>()) { if (a.Get<FName>() != b.Get<FName>()) return false; } else { if (a.Get<int32>() != b.Get<int32>()) return false; }
    }
    return true;
}

bool FNBTDataAccessor::IsParent(const FNBTDataAccessor& OtherNode) const {
    if (OtherNode.Container == Container) {
        return IsAncestor(OtherNode, *this);
    }
    return false;
}

bool FNBTDataAccessor::IsChild(const FNBTDataAccessor& OtherNode) const {
    if (OtherNode.Container == Container) {
        return IsAncestor(*this, OtherNode);
    }
    return false;
}

bool FNBTDataAccessor::EqualNodeDeep(const FNBTContainer* ACont, FNBTAttributeID AID,
                                     const FNBTContainer* BCont, FNBTAttributeID BID) const {
    if (!ACont || !BCont) return false;
    if (!AID.IsValid() || !BID.IsValid()) return false;

    const FNBTAttribute* A = ACont->GetAttribute(AID);
    const FNBTAttribute* B = BCont->GetAttribute(BID);
    if (!A || !B) return false;

    const ENBTAttributeType AT = A->GetType();
    const ENBTAttributeType BT = B->GetType();
    if (AT != BT) return false;

    if (AT != ENBTAttributeType::Map && AT != ENBTAttributeType::List) {
        return A->EqualsValues(*B);
    }

    // Map：键集合一致 + 子节点逐键深度相等
    if (AT == ENBTAttributeType::Map) {
        const auto* AM = A->GetMapData();
        const auto* BM = B->GetMapData();
        if (!AM || !BM) return false;

        if (AM->Children.Num() != BM->Children.Num()) return false;

        for (const auto& Pair : AM->Children) {
            const FName& Key = Pair.Key;
            const FNBTAttributeID* BChild = BM->Children.Find(Key);
            if (!BChild) return false;

            const FNBTAttributeID AChild = Pair.Value;
            if (!EqualNodeDeep(ACont, AChild, BCont, *BChild)) return false;
        }
        return true;
    }

    if (AT == ENBTAttributeType::List) {
        const auto* AL = A->GetListData();
        const auto* BL = B->GetListData();
        if (!AL || !BL) return false;

        if (AL->Children.Num() != BL->Children.Num()) return false;

        const int32 N = AL->Children.Num();
        for (int32 i = 0; i < N; ++i) {
            if (!EqualNodeDeep(ACont, AL->Children[i], BCont, BL->Children[i])) return false;
        }
        return true;
    }

    // 理论到不了这里
    return false;
}

bool FNBTDataAccessor::IsEqual(const FNBTDataAccessor& Other) const {
    if (!IsContainerValid() || !Other.IsContainerValid()) return false;

    // 若同容器且同ID，必然相等（同一节点）
    if (Container == Other.Container && CachedAttributeID.IsValid() &&
        CachedAttributeID == Other.CachedAttributeID &&
        CachedAttributePtr && Other.CachedAttributePtr) {
        return true;
    }

    // 解析路径，确保缓存有效
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success) return false;
    if (Other.ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success) return false;

    if (!CachedAttributeID.IsValid() || !Other.CachedAttributeID.IsValid()) return false;

    // 快速同节点判断（解析后再比一次）
    if (Container == Other.Container && CachedAttributeID == Other.CachedAttributeID) return true;

    // 深度比较
    return EqualNodeDeep(Container, CachedAttributeID, Other.Container, Other.CachedAttributeID);
}

bool FNBTDataAccessor::IsDataExists() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return false;

    return true;
}

TOptional<ENBTAttributeType> FNBTDataAccessor::GetType() const {
    return IsDataExists()
               ? TOptional<ENBTAttributeType>(CachedAttributePtr->GetType())
               : TOptional<ENBTAttributeType>();
}

FString FNBTDataAccessor::GetTypeString() const {
    return IsDataExists() ? CachedAttributePtr->GetTypeString() : "!Type$Invalid Node$";
}

bool FNBTDataAccessor::IsSubtreeChanged() const {
    if (!IsContainerValid()) return false;
    if (LastObservedContainerDataVersion != Container->GetContainerDataVersion()) {
        auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
        if (Result != ENBTAttributeOpResult::Success) return LastObservedSubtreeVersion != -1;
        if (LastObservedAttributeID != CachedAttributeID) return true;
        if (CachedSubtreeVersionPtr && *CachedSubtreeVersionPtr != LastObservedSubtreeVersion) return true;
    }
    return false;
}

bool FNBTDataAccessor::IsSubtreeChangedAndMark() const {
    const bool b = IsSubtreeChanged();
    MarkSubtree();
    return b;
}

void FNBTDataAccessor::MarkSubtree() const {
    if (IsDataExists()) {
        if (CachedSubtreeVersionPtr) {
            LastObservedSubtreeVersion = *CachedSubtreeVersionPtr;
        }
        //LastObservedContainerDataVersion = Container->GetContainerDataVersion();
        LastObservedAttributeID = CachedAttributeID;
    } else {
        LastObservedSubtreeVersion = -1;
        //LastObservedContainerDataVersion = Container->GetContainerDataVersion();
        LastObservedAttributeID = FNBTAttributeID();
    }
}

bool FNBTDataAccessor::IsDataChanged() const {
    if (!IsContainerValid()) return false;

    if (LastObservedContainerDataVersion == Container->GetContainerDataVersion())
        return false;

    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return LastObservedNodeVersion != -1;

    if (LastObservedAttributeID != CachedAttributeID) return true;

    return *CachedAttributeVersionPtr != LastObservedNodeVersion;
}

bool FNBTDataAccessor::IsDataChangedAndMark() const {
    const bool bResult = IsDataChanged();
    if (bResult)
        Mark();
    return bResult;
}

void FNBTDataAccessor::Mark() const {
    if (IsDataExists()) {
        LastObservedAttributeID = CachedAttributeID;
        LastObservedNodeVersion = *CachedAttributeVersionPtr;
        LastObservedContainerDataVersion = Container->GetContainerDataVersion();
        if (CachedSubtreeVersionPtr)
            LastObservedSubtreeVersion = *CachedSubtreeVersionPtr;
    } else {
        LastObservedNodeVersion = -1;
        LastObservedContainerDataVersion = Container->GetContainerDataVersion();
        LastObservedAttributeID = FNBTAttributeID();
    }
}

bool FNBTDataAccessor::IsEmpty() const {
    const auto Type = GetType();
    return Type.IsSet() && Type.GetValue() == ENBTAttributeType::Empty;
}

bool FNBTDataAccessor::IsMap() const {
    const auto Type = GetType();
    return Type.IsSet() && Type.GetValue() == ENBTAttributeType::Map;
}

bool FNBTDataAccessor::IsEmptyMap() const {
    if (!IsDataExists()) return false;
    auto Ptr = CachedAttributePtr->GetMapData();
    return Ptr && Ptr->Children.IsEmpty();
}

bool FNBTDataAccessor::IsFilledMap() const {
    if (!IsDataExists()) return false;
    auto Ptr = CachedAttributePtr->GetMapData();
    return Ptr && !Ptr->Children.IsEmpty();
}

bool FNBTDataAccessor::IsList() const {
    const auto Type = GetType();
    return Type.IsSet() && Type.GetValue() == ENBTAttributeType::List;
}

bool FNBTDataAccessor::IsEmptyList() const {
    if (!IsDataExists()) return false;
    auto Ptr = CachedAttributePtr->GetListData();
    return Ptr && Ptr->Children.IsEmpty();
}

bool FNBTDataAccessor::IsFilledList() const {
    if (!IsDataExists()) return false;
    auto Ptr = CachedAttributePtr->GetListData();
    return Ptr && !Ptr->Children.IsEmpty();
}

bool FNBTDataAccessor::IsArray() const {
    const auto Type = GetType();
    return Type.IsSet() && Type.GetValue() >= ENBTAttributeType::ArrayInt8 && Type.GetValue() < ENBTAttributeType::Map;
}

bool FNBTDataAccessor::IsBaseType() const {
    const auto Type = GetType();
    return Type.IsSet() && Type.GetValue() < ENBTAttributeType::ArrayInt8;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::ResolvePathInternal(ENBTPathResolveMode Mode) const {
    if (!IsContainerValid()) return ENBTAttributeOpResult::InvalidContainer; // 容器失效

    const auto CurrentContainerVersion = Container->GetContainerStructVersion();

    if (CachedContainerStructVersion == CurrentContainerVersion) {
        if (CachedAttributeID.IsValid() && CachedAttributePtr && CachedAttributeVersionPtr && CachedSubtreeVersionPtr) {
            return ENBTAttributeOpResult::Success; //缓存有效
        }

        CachedAttributePtr = Container->GetAttribute(CachedAttributeID);
        if (CachedAttributePtr) {
            CachedAttributeVersionPtr = Container->GetAttributeVersion(CachedAttributeID); // 依然有效
            CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(CachedAttributeID);
            if (CachedAttributeVersionPtr && CachedSubtreeVersionPtr) {
                return ENBTAttributeOpResult::Success;
            }
        }
    } else {
        // 结构版本已变化，但ID可能仍然有效, 直接检查是不是InvalidIndex, 如果是InvalidIndex也不用尝试获取数据, 因为本身就是无效的
        if (CachedAttributeID.IsValid()) {
            CachedAttributePtr = Container->GetAttribute(CachedAttributeID);
            if (CachedAttributePtr) {
                CachedAttributeVersionPtr = Container->GetAttributeVersion(CachedAttributeID);
                CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(CachedAttributeID);
                if (CachedAttributeVersionPtr && CachedSubtreeVersionPtr) { // 依然有效
                    CachedContainerStructVersion = CurrentContainerVersion;
                    return ENBTAttributeOpResult::Success;
                }
            }
        }
    }

    //否则进行重新构筑
    static constexpr int32 MaxPathDepth = 64; // 防止溢出

    if (Path.Num() > MaxPathDepth) {
        UE_LOG(NBTSystem, Error, TEXT("Path depth exceeds maximum limit of %d"), MaxPathDepth);
        return ENBTAttributeOpResult::InvalidContainer;
    }

    // ============ 慢速路径：重新解析路径 ============
    // 如果路径为空，直接返回根节点

    FNBTAttributeID CurrentID = Container->GetRootID();
    if (!CurrentID.IsValid()) {
        return ENBTAttributeOpResult::InvalidID;
    }

    FNBTAttribute* CurrentAttr = Container->GetAttribute(CurrentID);
    int32* CurrentVersion = Container->GetAttributeVersion(CurrentID);
    int32* CurrentSubtreeVersion = Container->GetAttributeSubtreeVersion(CurrentID);
    if (!CurrentAttr || !CurrentVersion || !CurrentSubtreeVersion) {
        return ENBTAttributeOpResult::InvalidID;
    }

    if (Path.Num() == 0) {
        CachedAttributeID = CurrentID;
        CachedAttributePtr = CurrentAttr;
        CachedAttributeVersionPtr = CurrentVersion;
        CachedContainerStructVersion = CurrentContainerVersion;
        CachedSubtreeVersionPtr = CurrentSubtreeVersion;
        return ENBTAttributeOpResult::Success;
    }

    for (int32 PathIndex = 0; PathIndex < Path.Num(); ++PathIndex) {
        const auto& PathElement = Path[PathIndex];

        if (const FName* KeyPtr = PathElement.TryGet<FName>()) {
            // ===== Map路径处理 =====

            // 确保当前节点是Map类型
            if (CurrentAttr->GetType() != ENBTAttributeType::Map) {
                if (Mode == ENBTPathResolveMode::ReadOnly) {
                    return {
                        ENBTAttributeOpResult::NodeTypeMismatch,
                        FString::Printf(TEXT("Node [%s] is Not Map."), *GetPathString(PathIndex))
                    };
                }

                if (Mode == ENBTPathResolveMode::ForceOverride) {
                    Container->ReleaseChildren(CurrentID);
                    CurrentAttr->OverrideToEmptyMap();
                    (*CurrentVersion)++;
                    UpdateContainerDataAndStructVersion();
                } else if (Mode == ENBTPathResolveMode::EnsureCreate && CurrentAttr->IsEmpty()) {
                    CurrentAttr->OverrideToEmptyMap();
                    (*CurrentVersion)++;
                    UpdateContainerDataAndStructVersion();
                } else {
                    return {
                        ENBTAttributeOpResult::PermissionDenied,
                        FString::Printf(
                            TEXT("Node [%s] is not allowed to Override type in Ensure mode."),
                            *GetPathString(PathIndex))
                    };
                }
            }

            FNBTMapData* MapData = CurrentAttr->GetMapData();
            if (!MapData) {
                return ENBTAttributeOpResult::InvalidContainer;
            }

            // 查找子节点
            auto ChildItor = MapData->Children.Find(*KeyPtr);
            if (ChildItor) {
                CurrentID = *ChildItor;
            } else {
                // 子节点不存在
                if (Mode == ENBTPathResolveMode::ReadOnly) {
                    return ENBTAttributeOpResult::NotFoundNode;
                }

                // 创建新节点
                FNBTAttributeID NewChildID = Container->AllocateNode();
                if (!NewChildID.IsValid()) {
                    return ENBTAttributeOpResult::InvalidID;
                }

                MapData->Children.Emplace(*KeyPtr, NewChildID);
                (*CurrentVersion)++;
                UpdateContainerDataAndStructVersion();
                CurrentID = NewChildID;
            }
        } else if (const int32* IndexPtr = PathElement.TryGet<int32>()) {
            // ===== List路径处理 =====
            const int32 Index = *IndexPtr;

            // 确保当前节点是List类型
            if (CurrentAttr->GetType() != ENBTAttributeType::List) {
                if (Mode == ENBTPathResolveMode::ReadOnly) {
                    return {
                        ENBTAttributeOpResult::NodeTypeMismatch,
                        FString::Printf(
                            TEXT("Node [%s] : is Not List, it's %s."), *GetPathString(PathIndex),
                            *CurrentAttr->GetTypeString())
                    };
                }

                if (Mode == ENBTPathResolveMode::ForceOverride) {
                    Container->ReleaseChildren(CurrentID);
                    CurrentAttr->OverrideToEmptyList();
                    (*CurrentVersion)++;
                    UpdateContainerDataAndStructVersion();
                } else if (Mode == ENBTPathResolveMode::EnsureCreate && CurrentAttr->IsEmpty()) {
                    CurrentAttr->OverrideToEmptyList();
                    (*CurrentVersion)++;
                    UpdateContainerDataAndStructVersion();
                } else {
                    return {
                        ENBTAttributeOpResult::NodeTypeMismatch,
                        FString::Printf(
                            TEXT("Node [%s] : is Not List, it's %s."), *GetPathString(PathIndex),
                            *CurrentAttr->GetTypeString())
                    };
                }
            }

            FNBTListData* ListData = CurrentAttr->GetListData();
            if (!ListData) {
                return ENBTAttributeOpResult::InvalidContainer;
            }

            // List不支持通过索引创建新元素
            if (!ListData->Children.IsValidIndex(Index)) {
                if (Mode != ENBTPathResolveMode::ReadOnly) {
                    return {
                        ENBTAttributeOpResult::PermissionDenied,
                        FString::Printf(
                            TEXT(
                                "Node [%s] not found, Create Path by go() or index[] through List is not allowed. Use AddListItem() to add new elements"),
                            *GetPathString(PathIndex))
                    };
                }
                return {ENBTAttributeOpResult::NotFoundNode};
            }

            CurrentID = ListData->Children[Index];
        } else {
            // 不应该到达这里，路径元素类型未知
            return ENBTAttributeOpResult::InvalidContainer;
        }

        CurrentAttr = Container->GetAttribute(CurrentID);
        CurrentVersion = Container->GetAttributeVersion(CurrentID);
        CurrentSubtreeVersion = Container->GetAttributeSubtreeVersion(CurrentID);
        if (!CurrentAttr || !CurrentVersion || !CurrentSubtreeVersion) {
            return ENBTAttributeOpResult::InvalidID;
        }
    }

    // 更新缓存
    CachedAttributeID = CurrentID;
    CachedAttributePtr = CurrentAttr;
    CachedAttributeVersionPtr = CurrentVersion;
    CachedContainerStructVersion = CurrentContainerVersion;
    CachedSubtreeVersionPtr = CurrentSubtreeVersion;

    return ENBTAttributeOpResult::Success;
}

FString FNBTDataAccessor::GetPathString(int ToIndex) const {
    FString Result = "Root";

    if (Path.Num() == 0) { return Result; }

    int idx = FMath::Clamp(ToIndex, 0, Path.Num() - 1);

    for (int i = 0; i <= idx; i++) {
        Result += " -> ";
        if (const FName* name = Path[i].TryGet<FName>()) {
            Result += name->ToString();
        } else {
            Result += FString::Printf(TEXT("[%d]"), Path[i].Get<int32>());
        }
    }
    return Result;
}

void FNBTDataAccessor::UpdateContainerDataAndStructVersion() const {
    Container->UpdateContainerDataAndStructVersion();
    CachedContainerStructVersion = Container->GetContainerStructVersion();
}

void FNBTDataAccessor::UpdateContainerDataVersion() const {
    Container->UpdateContainerDataVersion();
}

void FNBTDataAccessor::BubbleSubtreeVersionAlongPath() const {
    if (!IsContainerValid()) return;

    // Depth==-1 表示完整路径
    const int32 MaxDepth = Path.Num();

    // 从根开始
    FNBTAttributeID CurrentID = Container->GetRootID();
    if (!CurrentID.IsValid()) return;

    // 根也要++（子树包含自身）
    Container->IncAttributeSubtreeVersion(CurrentID);

    // 逐段下行
    for (int32 i = 0; i < MaxDepth; ++i) {
        FNBTAttribute* Attr = Container->GetAttribute(CurrentID);
        if (!Attr) break;

        const TVariant<FName, int32>& Elem = Path[i];

        if (const FName* Key = Elem.TryGet<FName>()) {
            // Map
            if (auto* Map = Attr->GetMapData()) {
                auto it = Map->Children.Find(*Key);
                if (!it) break;
                CurrentID = *it;
            } else { break; }
        } else if (const int32* Index = Elem.TryGet<int32>()) {
            // List
            if (auto* List = Attr->GetListData()) {
                if (!List->Children.IsValidIndex(*Index)) break;
                CurrentID = List->Children[*Index];
            } else { break; }
        }

        // 路径节点自增子树版本
        Container->IncAttributeSubtreeVersion(CurrentID);
    }
}

FNBTDataAccessor::FNBTDataAccessor(const FNBTDataAccessor& Other) {
    Container = Other.Container;
    ContainerLiveToken = Other.ContainerLiveToken;
    Path = Other.Path;
    CachedAttributeID = Other.CachedAttributeID;
    CachedContainerStructVersion = Other.CachedContainerStructVersion;
    CachedAttributePtr = Other.CachedAttributePtr;
    CachedSubtreeVersionPtr = Other.CachedSubtreeVersionPtr;
    LastObservedSubtreeVersion = Other.LastObservedSubtreeVersion;
}

FNBTDataAccessor& FNBTDataAccessor::operator=(const FNBTDataAccessor& Other) {
    if (this == &Other) return *this;
    Container = Other.Container;
    ContainerLiveToken = Other.ContainerLiveToken;
    Path = Other.Path;
    CachedAttributeID = Other.CachedAttributeID;
    CachedContainerStructVersion = Other.CachedContainerStructVersion;
    CachedAttributePtr = Other.CachedAttributePtr;
    CachedSubtreeVersionPtr = Other.CachedSubtreeVersionPtr;
    LastObservedSubtreeVersion = Other.LastObservedSubtreeVersion;
    return *this;
}

FNBTDataAccessor::FNBTDataAccessor(FNBTDataAccessor&& Other) {
    Container = Other.Container;
    Other.Container = nullptr;

    ContainerLiveToken = std::move(Other.ContainerLiveToken);

    Path = std::move(Other.Path);

    CachedAttributeID = Other.CachedAttributeID;
    Other.CachedAttributeID = FNBTAttributeID();

    CachedContainerStructVersion = Other.CachedContainerStructVersion;
    Other.CachedContainerStructVersion = -1;

    CachedAttributePtr = Other.CachedAttributePtr;
    Other.CachedAttributePtr = nullptr;

    CachedSubtreeVersionPtr = Other.CachedSubtreeVersionPtr;
    Other.CachedSubtreeVersionPtr = nullptr;

    LastObservedSubtreeVersion = Other.LastObservedSubtreeVersion;
    Other.LastObservedSubtreeVersion = -1;
}

FNBTDataAccessor& FNBTDataAccessor::operator=(FNBTDataAccessor&& Other) {
    if (this == &Other) return *this;

    Container = Other.Container;
    Other.Container = nullptr;

    ContainerLiveToken = std::move(Other.ContainerLiveToken);

    Path = std::move(Other.Path);

    CachedAttributeID = Other.CachedAttributeID;
    Other.CachedAttributeID = FNBTAttributeID();

    CachedContainerStructVersion = Other.CachedContainerStructVersion;
    Other.CachedContainerStructVersion = -1;

    CachedAttributePtr = Other.CachedAttributePtr;
    Other.CachedAttributePtr = nullptr;

    CachedSubtreeVersionPtr = Other.CachedSubtreeVersionPtr;
    Other.CachedSubtreeVersionPtr = nullptr;

    LastObservedSubtreeVersion = Other.LastObservedSubtreeVersion;
    Other.LastObservedSubtreeVersion = -1;

    return *this;
}

TOptional<int64> FNBTDataAccessor::TryGetGenericInt() const {
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success)
        return {};
    return CachedAttributePtr->GetGenericInt();
}

TOptional<double> FNBTDataAccessor::TryGetGenericDouble() const {
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success)
        return {};
    return CachedAttributePtr->GetGenericDouble();
}

TOptional<bool> FNBTDataAccessor::TryGetBool() const { return TryGet<bool>(); }

TOptional<int8> FNBTDataAccessor::TryGetInt8() const { return TryGet<int8>(); }

TOptional<int16> FNBTDataAccessor::TryGetInt16() const { return TryGet<int16>(); }

TOptional<int32> FNBTDataAccessor::TryGetInt32() const { return TryGet<int32>(); }

TOptional<int64> FNBTDataAccessor::TryGetInt64() const { return TryGet<int64>(); }

TOptional<float> FNBTDataAccessor::TryGetFloat() const { return TryGet<float>(); }

TOptional<double> FNBTDataAccessor::TryGetDouble() const { return TryGet<double>(); }

TOptional<FName> FNBTDataAccessor::TryGetName() const { return TryGet<FName>(); }

TOptional<FString> FNBTDataAccessor::TryGetString() const { return TryGet<FString>(); }

TOptional<FColor> FNBTDataAccessor::TryGetColor() const { return TryGet<FColor>(); }

TOptional<FGuid> FNBTDataAccessor::TryGetGuid() const { return TryGet<FGuid>(); }

TOptional<FSoftClassPath> FNBTDataAccessor::TryGetSoftClassPath() const { return TryGet<FSoftClassPath>(); }

TOptional<FSoftObjectPath> FNBTDataAccessor::TryGetSoftObjectPath() const { return TryGet<FSoftObjectPath>(); }

TOptional<FDateTime> FNBTDataAccessor::TryGetDateTime() const { return TryGet<FDateTime>(); }

TOptional<FRotator> FNBTDataAccessor::TryGetRotator() const { return TryGet<FRotator>(); }

TOptional<FVector2D> FNBTDataAccessor::TryGetVector2D() const { return TryGet<FVector2D>(); }

TOptional<FVector> FNBTDataAccessor::TryGetVector() const { return TryGet<FVector>(); }

TOptional<FIntVector2> FNBTDataAccessor::TryGetIntVector2() const { return TryGet<FIntVector2>(); }

TOptional<FIntVector> FNBTDataAccessor::TryGetIntVector() const { return TryGet<FIntVector>(); }

TOptional<FInt64Vector2> FNBTDataAccessor::TryGetInt64Vector2() const { return TryGet<FInt64Vector2>(); }

TOptional<FInt64Vector> FNBTDataAccessor::TryGetInt64Vector() const { return TryGet<FInt64Vector>(); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetBool(bool Value) const { return TrySetBaseType<bool>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt8(int8 Value) const { return TrySetBaseType<int8>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt16(int16 Value) const { return TrySetBaseType<int16>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt32(int32 Value) const { return TrySetBaseType<int32>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt64(int64 Value) const { return TrySetBaseType<int64>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetFloat(float Value) const { return TrySetBaseType<float>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetDouble(double Value) const { return TrySetBaseType<double>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetName(FName Value) const { return TrySetBaseType<FName>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetString(const FString& Value) const { return TrySetBaseTypeRef<FString>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetColor(FColor Value) const { return TrySetBaseType<FColor>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetGuid(FGuid Value) const { return TrySetBaseType<FGuid>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetSoftClassPath(const FSoftClassPath& Value) const { return TrySetBaseTypeRef<FSoftClassPath>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetSoftObjectPath(const FSoftObjectPath& Value) const { return TrySetBaseTypeRef<FSoftObjectPath>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetDateTime(FDateTime Value) const { return TrySetBaseType<FDateTime>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetRotator(FRotator Value) const { return TrySetBaseType<FRotator>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetVector2D(FVector2D Value) const { return TrySetBaseType<FVector2D>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetVector(FVector Value) const { return TrySetBaseType<FVector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetIntVector2(FIntVector2 Value) const { return TrySetBaseType<FIntVector2>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetIntVector(FIntVector Value) const { return TrySetBaseType<FIntVector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt64Vector2(FInt64Vector2 Value) const { return TrySetBaseType<FInt64Vector2>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt64Vector(FInt64Vector Value) const { return TrySetBaseType<FInt64Vector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt8Array(const TArray<int8>& Value) const { return TrySetArray<int8>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt16Array(const TArray<int16>& Value) const { return TrySetArray<int16>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt32Array(const TArray<int32>& Value) const { return TrySetArray<int32>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetInt64Array(const TArray<int64>& Value) const { return TrySetArray<int64>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetFloatArray(const TArray<float>& Value) const { return TrySetArray<float>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetDoubleArray(const TArray<double>& Value) const { return TrySetArray<double>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetBool(bool Value) const { return EnsureAndSetBaseType<bool>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt8(int8 Value) const { return EnsureAndSetBaseType<int8>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt16(int16 Value) const { return EnsureAndSetBaseType<int16>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt32(int32 Value) const { return EnsureAndSetBaseType<int32>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt64(int64 Value) const { return EnsureAndSetBaseType<int64>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetFloat(float Value) const { return EnsureAndSetBaseType<float>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetDouble(double Value) const { return EnsureAndSetBaseType<double>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetName(FName Value) const { return EnsureAndSetBaseType<FName>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetString(const FString& Value) const { return EnsureAndSetBaseTypeRef<FString>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetColor(FColor Value) const { return EnsureAndSetBaseType<FColor>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetGuid(FGuid Value) const { return EnsureAndSetBaseType<FGuid>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetSoftClassPath(const FSoftClassPath& Value) const { return EnsureAndSetBaseTypeRef<FSoftClassPath>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetSoftObjectPath(const FSoftObjectPath& Value) const {
    return EnsureAndSetBaseTypeRef<FSoftObjectPath>(Value);
}

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetDateTime(FDateTime Value) const { return EnsureAndSetBaseType<FDateTime>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetRotator(FRotator Value) const { return EnsureAndSetBaseType<FRotator>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetVector2D(FVector2D Value) const { return EnsureAndSetBaseType<FVector2D>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetVector(FVector Value) const { return EnsureAndSetBaseType<FVector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetIntVector2(FIntVector2 Value) const { return EnsureAndSetBaseType<FIntVector2>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetIntVector(FIntVector Value) const { return EnsureAndSetBaseType<FIntVector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt64Vector2(FInt64Vector2 Value) const { return EnsureAndSetBaseType<FInt64Vector2>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt64Vector(FInt64Vector Value) const { return EnsureAndSetBaseType<FInt64Vector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt8Array(const TArray<int8>& Value) const { return EnsureAndSetArray<int8>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt16Array(const TArray<int16>& Value) const { return EnsureAndSetArray<int16>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt32Array(const TArray<int32>& Value) const { return EnsureAndSetArray<int32>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetInt64Array(const TArray<int64>& Value) const { return EnsureAndSetArray<int64>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetFloatArray(const TArray<float>& Value) const { return EnsureAndSetArray<float>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetDoubleArray(const TArray<double>& Value) const { return EnsureAndSetArray<double>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToBool(bool Value) const { return OverrideToBaseType<bool>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt8(int8 Value) const { return OverrideToBaseType<int8>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt16(int16 Value) const { return OverrideToBaseType<int16>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt32(int32 Value) const { return OverrideToBaseType<int32>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt64(int64 Value) const { return OverrideToBaseType<int64>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToFloat(float Value) const { return OverrideToBaseType<float>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToDouble(double Value) const { return OverrideToBaseType<double>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToName(FName Value) const { return OverrideToBaseType<FName>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToString(const FString& Value) const { return OverrideToBaseTypeRef<FString>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToColor(FColor Value) const { return OverrideToBaseType<FColor>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToGuid(FGuid Value) const { return OverrideToBaseType<FGuid>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToSoftClassPath(const FSoftClassPath& Value) const { return OverrideToBaseTypeRef<FSoftClassPath>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToSoftObjectPath(const FSoftObjectPath& Value) const { return OverrideToBaseTypeRef<FSoftObjectPath>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToDateTime(FDateTime Value) const { return OverrideToBaseType<FDateTime>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToRotator(FRotator Value) const { return OverrideToBaseType<FRotator>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToVector2D(FVector2D Value) const { return OverrideToBaseType<FVector2D>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToVector(FVector Value) const { return OverrideToBaseType<FVector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToIntVector2(FIntVector2 Value) const { return OverrideToBaseType<FIntVector2>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToIntVector(FIntVector Value) const { return OverrideToBaseType<FIntVector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt64Vector2(FInt64Vector2 Value) const { return OverrideToBaseType<FInt64Vector2>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt64Vector(FInt64Vector Value) const { return OverrideToBaseType<FInt64Vector>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt8Array(const TArray<int8>& Value) const { return OverrideToArray<int8>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt16Array(const TArray<int16>& Value) const { return OverrideToArray<int16>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt32Array(const TArray<int32>& Value) const { return OverrideToArray<int32>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToInt64Array(const TArray<int64>& Value) const { return OverrideToArray<int64>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToFloatArray(const TArray<float>& Value) const { return OverrideToArray<float>(Value); }

FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToDoubleArray(const TArray<double>& Value) const { return OverrideToArray<double>(Value); }

TArray<int8> FNBTDataAccessor::TryGetInt8Array() const {
    auto ptr = TryGetArray<int8>();
    if (!ptr) return {};
    return *ptr;
}

TArray<int16> FNBTDataAccessor::TryGetInt16Array() const {
    auto ptr = TryGetArray<int16>();
    if (!ptr) return {};
    return *ptr;
}

TArray<int32> FNBTDataAccessor::TryGetInt32Array() const {
    auto ptr = TryGetArray<int32>();
    if (!ptr) return {};
    return *ptr;
}

TArray<int64> FNBTDataAccessor::TryGetInt64Array() const {
    auto ptr = TryGetArray<int64>();
    if (!ptr) return {};
    return *ptr;
}

TArray<float> FNBTDataAccessor::TryGetFloatArray() const {
    auto ptr = TryGetArray<float>();
    if (!ptr) return {};
    return *ptr;
}

TArray<double> FNBTDataAccessor::TryGetDoubleArray() const {
    auto ptr = TryGetArray<double>();
    if (!ptr) return {};
    return *ptr;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetEmpty() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    if (CachedAttributePtr->IsEmpty()) {
        return ENBTAttributeOpResult::SameAndNotChange;
    } else {
        if (!CachedAttributePtr->IsCompoundType()) {
            CachedAttributePtr->Reset();
            (*CachedAttributeVersionPtr)++;
            UpdateContainerDataVersion();
            BubbleSubtreeVersionAlongPath();
            return ENBTAttributeOpResult::Success;
        } else {
            const auto Type = CachedAttributePtr->GetType();
            if (Type == ENBTAttributeType::List) {
                return ListClear();
            } else if (Type == ENBTAttributeType::Map) {
                return MapClear();
            }
            return ENBTAttributeOpResult::NodeTypeMismatch;
        }
    }
}

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetGenericInt(int64 Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Result = CachedAttributePtr->TrySetGenericInt(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    }

    return Result;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetGenericDouble(double Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Result = CachedAttributePtr->TrySetGenericDouble(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    }

    return Result;
}

FNBTDataAccessor FNBTDataAccessor::EnsureMap() const {
    if (!IsAccessorValid()) return *this;

    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success)
        return FNBTDataAccessor(nullptr, nullptr);;

    if (CachedAttributePtr->GetType() == ENBTAttributeType::Map) {
        return *this;
    } else if (CachedAttributePtr->IsEmpty()) {
        CachedAttributePtr->OverrideToEmptyMap();
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataAndStructVersion();
        BubbleSubtreeVersionAlongPath();
        return *this;
    }

    return FNBTDataAccessor(nullptr, nullptr);
}

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetEmpty() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    if (CachedAttributePtr->IsEmpty()) {
        return ENBTAttributeOpResult::SameAndNotChange;
    } else {
        if (!CachedAttributePtr->IsCompoundType()) {
            CachedAttributePtr->Reset();
            (*CachedAttributeVersionPtr)++;
            UpdateContainerDataVersion();
            BubbleSubtreeVersionAlongPath();
            return ENBTAttributeOpResult::Success;
        } else {
            const auto Type = CachedAttributePtr->GetType();
            if (Type == ENBTAttributeType::List) {
                return ListClear();
            } else if (Type == ENBTAttributeType::Map) {
                return MapClear();
            }
            return ENBTAttributeOpResult::NodeTypeMismatch;
        }
    }
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapHasKey(FName Key) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        return (MapData->Children.Find(Key))
                   ? ENBTAttributeOpResult::Success
                   : ENBTAttributeOpResult::NotFoundSubNode;
    } else return ENBTAttributeOpResult::NodeTypeMismatch;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapGetKeys(TArray<FName>& Keys) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        Keys.Reset();
        for (const auto& Pair : MapData->Children) {
            Keys.Add(Pair.Key);
        }
    } else return ENBTAttributeOpResult::NodeTypeMismatch;

    return ENBTAttributeOpResult::Success;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapGetSize(int32& Size) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        Size = MapData->Children.Num();
        return ENBTAttributeOpResult::Success;
    } else return ENBTAttributeOpResult::NodeTypeMismatch;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapRemoveSubNode(FName Key) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        const auto Itor = MapData->Children.Find(Key);
        if (Itor) {
            if (Container->ReleaseRecursive(*Itor) > 0) {
                MapData->Children.Remove(Key);
                (*CachedAttributeVersionPtr)++;
                UpdateContainerDataAndStructVersion();
                BubbleSubtreeVersionAlongPath();
            }
            return ENBTAttributeOpResult::Success;
        } else {
            return ENBTAttributeOpResult::NotFoundSubNode;
        }
    } else {
        return ENBTAttributeOpResult::NodeTypeMismatch;
    }
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapClear() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (CachedAttributePtr->GetMapData()) {
        if (Container->ReleaseChildren(CachedAttributeID) > 0) {
            (*CachedAttributeVersionPtr)++;
            UpdateContainerDataAndStructVersion();
            BubbleSubtreeVersionAlongPath();
        }
        return ENBTAttributeOpResult::Success;
    } else {
        return ENBTAttributeOpResult::NodeTypeMismatch;
    }
}

FNBTDataAccessor FNBTDataAccessor::MapMakeAccessorByCondition(ENBTSearchCondition Condition) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return FNBTDataAccessor();

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        switch (Condition) {
            case ENBTSearchCondition::IfEmpty: {
                for (const auto& Pair : MapData->Children) {
                    auto Attr = Container->GetAttribute(Pair.Value);
                    if (!Attr) continue;
                    if (Attr->IsEmpty()) return MakeAccessFromFName(Pair.Key);
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyMap: {
                for (const auto& Pair : MapData->Children) {
                    auto Attr = Container->GetAttribute(Pair.Value);
                    if (!Attr) continue;
                    if (auto sMapData = Attr->GetMapData()) {
                        if (sMapData->Children.IsEmpty()) return MakeAccessFromFName(Pair.Key);
                    }
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyList: {
                for (const auto& Pair : MapData->Children) {
                    auto Attr = Container->GetAttribute(Pair.Value);
                    if (!Attr) continue;
                    if (auto sListData = Attr->GetListData()) {
                        if (sListData->Children.IsEmpty()) return MakeAccessFromFName(Pair.Key);
                    }
                }
            }
            break;

            default: break;
        }
    }

    return FNBTDataAccessor();
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapMakeAccessorsByCondition(
    TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition Condition) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Accessors.Reset();

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        switch (Condition) {
            case ENBTSearchCondition::IfEmpty: {
                for (const auto& Pair : MapData->Children) {
                    auto Attr = Container->GetAttribute(Pair.Value);
                    if (!Attr) continue;
                    if (Attr->IsEmpty()) Accessors.Add(MakeAccessFromFName(Pair.Key));
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyMap: {
                for (const auto& Pair : MapData->Children) {
                    auto Attr = Container->GetAttribute(Pair.Value);
                    if (!Attr) continue;
                    if (auto sMapData = Attr->GetMapData()) {
                        if (sMapData->Children.IsEmpty()) Accessors.Add(MakeAccessFromFName(Pair.Key));
                    }
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyList: {
                for (const auto& Pair : MapData->Children) {
                    auto Attr = Container->GetAttribute(Pair.Value);
                    if (!Attr) continue;
                    if (auto sListData = Attr->GetListData()) {
                        if (sListData->Children.IsEmpty()) Accessors.Add(MakeAccessFromFName(Pair.Key));
                    }
                }
            }
            break;

            default: break;
        }
    }

    return ENBTAttributeOpResult::Success;
}

FNBTDataAccessor FNBTDataAccessor::MapMakeAccessorByParameter(const FNBTSearchParameter& P) const {
    // 0) 解析当前访问器自身，仅一次
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success)
        return FNBTDataAccessor();

    const FNBTMapData* MapData = CachedAttributePtr ? CachedAttributePtr->GetMapData() : nullptr;
    if (!MapData) return FNBTDataAccessor();

    // 1) 工具：大小写策略
    const ESearchCase::Type SearchCase = P.IgnoreCase ? ESearchCase::IgnoreCase : ESearchCase::CaseSensitive;

    // 2) 解析参数值（尽量只做一次）
    auto ParseBool = [&](const FString& S, bool& Out)-> bool {
        if (S.Equals(TEXT("true"), ESearchCase::IgnoreCase) || S.Equals(TEXT("1"), SearchCase)
            || S.Equals(TEXT("yes"), ESearchCase::IgnoreCase) || S.Equals(TEXT("on"), ESearchCase::IgnoreCase)) {
            Out = true;
            return true;
        }
        if (S.Equals(TEXT("false"), ESearchCase::IgnoreCase) || S.Equals(TEXT("0"), SearchCase)
            || S.Equals(TEXT("no"), ESearchCase::IgnoreCase) || S.Equals(TEXT("off"), ESearchCase::IgnoreCase)) {
            Out = false;
            return true;
        }
        bool tmp = false;
        if (LexTryParseString(tmp, *S)) {
            Out = tmp;
            return true;
        }
        return false;
    };

    int64 ParamI64 = 0;
    bool bHasI64 = false;
    double ParamDbl = 0.0;
    bool bHasDbl = false;
    bool ParamBool = false;
    bool bHasBool = false;

    auto PrimeParamCache = [&]() {
        switch (P.ValueType) {
            case ENBTAttributeType::Int8:
            case ENBTAttributeType::Int16:
            case ENBTAttributeType::Int32:
            case ENBTAttributeType::Int64:
            case ENBTAttributeType::Boolean: {
                if (P.ValueType == ENBTAttributeType::Boolean) {
                    bHasBool = ParseBool(P.Value, ParamBool);
                } else {
                    bHasI64 = LexTryParseString(ParamI64, *P.Value);
                }
            }
            break;
            case ENBTAttributeType::Float:
            case ENBTAttributeType::Double: {
                bHasDbl = LexTryParseString(ParamDbl, *P.Value);
            }
            break;
            default: break; // String/Name 走字符串比较
        }
        // 若开启泛型搜索且是字符串，也预尝试解析数值/布尔（支持 "123" 与数字节点比较）
        if (P.EnableGenericSearch && P.ValueType == ENBTAttributeType::String) {
            int64 iTmp;
            double dTmp;
            bool bTmp;
            if (LexTryParseString(iTmp, *P.Value)) {
                ParamI64 = iTmp;
                bHasI64 = true;
            }
            if (LexTryParseString(dTmp, *P.Value)) {
                ParamDbl = dTmp;
                bHasDbl = true;
            }
            if (ParseBool(P.Value, bTmp)) {
                ParamBool = bTmp;
                bHasBool = true;
            }
        }
    };
    PrimeParamCache();

    // 3) 比较器们
    auto CmpInt = [&](int64 A)-> bool {
        if (!bHasI64) return false;
        switch (P.Op) {
            case ENBTCompareOp::Eq: return A == ParamI64;
            case ENBTCompareOp::Ne: return A != ParamI64;
            case ENBTCompareOp::Gt: return A > ParamI64;
            case ENBTCompareOp::Ge: return A >= ParamI64;
            case ENBTCompareOp::Lt: return A < ParamI64;
            case ENBTCompareOp::Le: return A <= ParamI64;
            default: return false;
        }
    };
    auto NearlyEqual = [](double A, double B) { return FMath::IsNearlyEqual(A, B, 1e-4); };
    auto CmpDbl = [&](double A)-> bool {
        if (!bHasDbl) return false;
        switch (P.Op) {
            case ENBTCompareOp::Eq: return NearlyEqual(A, ParamDbl);
            case ENBTCompareOp::Ne: return !NearlyEqual(A, ParamDbl);
            case ENBTCompareOp::Gt: return A > ParamDbl;
            case ENBTCompareOp::Ge: return A >= ParamDbl;
            case ENBTCompareOp::Lt: return A < ParamDbl;
            case ENBTCompareOp::Le: return A <= ParamDbl;
            default: return false;
        }
    };
    auto CmpBool = [&](bool A)-> bool {
        if (!bHasBool) return false;
        switch (P.Op) {
            case ENBTCompareOp::Eq: return A == ParamBool;
            case ENBTCompareOp::Ne: return A != ParamBool;
            default: return false;
        }
    };
    auto CmpStr = [&](const FString& A)-> bool {
        switch (P.Op) {
            case ENBTCompareOp::Eq: return A.Equals(P.Value, SearchCase);
            case ENBTCompareOp::Ne: return !A.Equals(P.Value, SearchCase);
            case ENBTCompareOp::Contains: return A.Contains(P.Value, SearchCase);
            case ENBTCompareOp::StartsWith: return A.StartsWith(P.Value, SearchCase);
            case ENBTCompareOp::EndsWith: return A.EndsWith(P.Value, SearchCase);
            default: return false;
        }
    };

    auto AttrToString = [&](const FNBTAttribute& A)-> FString {
        switch (A.GetType()) {
            case ENBTAttributeType::String: {
                if (auto v = A.GetBaseType<FString>()) return v.GetValue();
                break;
            }
            case ENBTAttributeType::Name: {
                if (auto v = A.GetBaseType<FName>()) return v.GetValue().ToString();
                break;
            }
            case ENBTAttributeType::Boolean: {
                if (auto v = A.GetBaseType<bool>()) return v.GetValue() ? TEXT("true") : TEXT("false");
                break;
            }
            case ENBTAttributeType::Int8: {
                if (auto v = A.GetBaseType<int8>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Int16: {
                if (auto v = A.GetBaseType<int16>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Int32: {
                if (auto v = A.GetBaseType<int32>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Int64: {
                if (auto v = A.GetBaseType<int64>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Float: {
                if (auto v = A.GetBaseType<float>()) return FString::SanitizeFloat(v.GetValue());
                break;
            }
            case ENBTAttributeType::Double: {
                if (auto v = A.GetBaseType<double>()) return FString::SanitizeFloat(v.GetValue());
                break;
            }
            default: break;
        }
        return FString();
    };

    auto MatchValue = [&](const FNBTAttribute& A)-> bool {
        const ENBTAttributeType T = A.GetType();

        // 目标类型优先匹配
        switch (P.ValueType) {
            case ENBTAttributeType::Boolean: {
                if (T == ENBTAttributeType::Boolean) {
                    if (auto v = A.GetBaseType<bool>()) return CmpBool(v.GetValue());
                    return false;
                }
            }
            break;

            case ENBTAttributeType::Int8:
            case ENBTAttributeType::Int16:
            case ENBTAttributeType::Int32:
            case ENBTAttributeType::Int64: {
                if (auto v = A.GetGenericInt(); v.IsSet()) return CmpInt(v.GetValue());
            }
            break;

            case ENBTAttributeType::Float:
            case ENBTAttributeType::Double: {
                if (auto v = A.GetGenericDouble(); v.IsSet()) return CmpDbl(v.GetValue());
            }
            break;

            case ENBTAttributeType::Name: {
                if (T == ENBTAttributeType::Name) {
                    if (auto v = A.GetBaseType<FName>()) return CmpStr(v.GetValue().ToString());
                }
                if (P.EnableGenericSearch && T == ENBTAttributeType::String) {
                    if (auto v = A.GetBaseType<FString>()) return CmpStr(v.GetValue());
                }
            }
            break;

            case ENBTAttributeType::String: {
                if (T == ENBTAttributeType::String) {
                    if (auto v = A.GetBaseType<FString>()) return CmpStr(v.GetValue());
                }
                if (P.EnableGenericSearch && T == ENBTAttributeType::Name) {
                    if (auto v = A.GetBaseType<FName>()) return CmpStr(v.GetValue().ToString());
                }
            }
            break;

            default: break;
        }

        if (P.EnableGenericSearch) {
            switch (P.Op) {
                case ENBTCompareOp::Contains:
                case ENBTCompareOp::StartsWith:
                case ENBTCompareOp::EndsWith:
                case ENBTCompareOp::Eq:
                case ENBTCompareOp::Ne: {
                    const FString S = AttrToString(A);
                    if (!S.IsEmpty()) return CmpStr(S);
                }
                break;

                default: {
                    // 数值比较的泛型回退：int/float
                    if (auto vi = A.GetGenericInt(); vi.IsSet() && bHasI64) return CmpInt(vi.GetValue());
                    if (auto vd = A.GetGenericDouble(); vd.IsSet() && bHasDbl) return CmpDbl(vd.GetValue());
                    if (P.ValueType == ENBTAttributeType::String && bHasBool) {
                        if (auto vb = A.GetBaseType<bool>()) return CmpBool(vb.GetValue());
                    }
                }
                break;
            }
        }

        return false;
    };

    auto BuildResultForMapKey = [&](FName Key, FNBTAttributeID ChildID)-> FNBTDataAccessor {
        FNBTDataAccessor Out;
        Out.Container = this->Container;
        Out.ContainerLiveToken = this->ContainerLiveToken;
        Out.Path = this->Path;
        Out.Path.Add(TVariant<FName, int32>(TInPlaceType<FName>(), Key));
        Out.CachedAttributeID = ChildID;
        Out.CachedContainerStructVersion = this->CachedContainerStructVersion;
        Out.CachedAttributeVersionPtr = Container->GetAttributeVersion(ChildID);
        Out.CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(ChildID);
        Out.CachedAttributePtr = Container->GetAttribute(ChildID);
        return Out;
    };

    auto ChildMatches = [&](FNBTAttributeID ChildID)-> bool {
        const FNBTAttribute* Attr = Container->GetAttribute(ChildID);
        if (!Attr) return false;

        if (P.SubKey.IsNone()) {
            return MatchValue(*Attr);
        } else {
            if (const FNBTMapData* SubMap = Attr->GetMapData()) {
                const FNBTAttributeID* SubID = SubMap->Children.Find(P.SubKey);
                if (!SubID) return false;
                const FNBTAttribute* SubAttr = Container->GetAttribute(*SubID);
                return SubAttr ? MatchValue(*SubAttr) : false;
            }
            return false;
        }
    };

    // 4) 遍历策略：单Key或全表
    if (!P.Key.IsNone()) {
        const FNBTAttributeID* Found = MapData->Children.Find(P.Key);
        if (!Found) return FNBTDataAccessor();
        if (ChildMatches(*Found)) return BuildResultForMapKey(P.Key, *Found);
        return FNBTDataAccessor();
    } else {
        for (const auto& Pair : MapData->Children) {
            if (ChildMatches(Pair.Value)) {
                return BuildResultForMapKey(Pair.Key, Pair.Value);
            }
        }
        return FNBTDataAccessor();
    }
}

FNBTDataAccessor FNBTDataAccessor::MapMakeAccessorIfEqual(const FNBTDataAccessor& Accessor) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return {};

    if (!Accessor.IsDataExists()) return {};

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        for (const auto& Pair : MapData->Children) {
            auto Attr = Container->GetAttribute(Pair.Value);
            if (!Attr) continue;
            if (EqualNodeDeep(Container, Pair.Value,
                              Accessor.Container, Accessor.CachedAttributeID))
                return MakeAccessFromFName(Pair.Key);
        }
    }

    return {};
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MapMakeAccessorsIfEqual(TArray<FNBTDataAccessor>& Accessors,
                                                                      const FNBTDataAccessor& Accessor) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;
    if (!Accessor.IsDataExists()) return {};

    Accessors.Reset();

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        for (const auto& Pair : MapData->Children) {
            if (EqualNodeDeep(Container, Pair.Value,
                              Accessor.Container, Accessor.CachedAttributeID))
                Accessors.Add(MakeAccessFromFName(Pair.Key));
        }
    }

    return ENBTAttributeOpResult::Success;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MakeAccessorFromMap(TArray<FNBTDataAccessor>& Accessors) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* MapData = CachedAttributePtr->GetMapData()) {
        Accessors.Reset();
        Accessors.Reserve(MapData->Children.Num());
        for (const auto& Pair : MapData->Children) {
            Accessors.Add(MakeAccessFromFName(Pair.Key));
        }
    } else return ENBTAttributeOpResult::NodeTypeMismatch;

    return ENBTAttributeOpResult::Success;
}

TArray<FNBTDataAccessor> FNBTDataAccessor::MakeAccessorFromMapNow() const {
    TArray<FNBTDataAccessor> AccessorResult{};
    MakeAccessorFromMap(AccessorResult);
    return AccessorResult;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::ListGetSize(int32& Size) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* ListData = CachedAttributePtr->GetListData()) {
        Size = ListData->Children.Num();
        return ENBTAttributeOpResult::Success;
    } else return ENBTAttributeOpResult::NodeTypeMismatch;
}

FNBTDataAccessor FNBTDataAccessor::ListAddSubNode() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);

    if (Result != ENBTAttributeOpResult::Success) {
        UE_LOG(NBTSystem, Warning, TEXT("Cannot add item to an invalid path"));
        return FNBTDataAccessor(nullptr, nullptr); // 返回无效迭代器
    }

    if (CachedAttributePtr->IsEmpty()) {
        CachedAttributePtr->OverrideToEmptyList();
    }

    auto* ListData = CachedAttributePtr->GetListData();
    if (!ListData) {
        UE_LOG(NBTSystem, Warning, TEXT("Cannot add item to non-list attribute"));
        return FNBTDataAccessor(nullptr, nullptr); // 返回无效迭代器
    }

    FNBTAttributeID NewID = Container->AllocateNode();
    if (!NewID.IsValid()) {
        UE_LOG(NBTSystem, Warning, TEXT("Cannot allocate new node"));
        return FNBTDataAccessor(nullptr, nullptr); // 返回无效迭代器
    }

    int32 NewIndex = ListData->Children.Add(NewID);

    (*CachedAttributeVersionPtr)++;
    UpdateContainerDataAndStructVersion();
    BubbleSubtreeVersionAlongPath();

    return MakeAccessFromIntIndex(NewIndex);
}

TOptional<int32> FNBTDataAccessor::ListGetCurrentIndex() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return {};

    if (!Path.Last().IsType<int32>()) return {};

    return Path.Last().Get<int32>();
}

TOptional<int32> FNBTDataAccessor::ListGetLastParentIndex() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return {};

    for (int32 i = Path.Num() - 1; i >= 0; i--) {
        if (Path[i].IsType<int32>()) {
            return Path[i].Get<int32>();
        }
    }

    return {};
}

FNBTAttributeOpResultDetail FNBTDataAccessor::ListRemoveSubNode(int32 Index, bool bSwapRemove) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    auto* ListData = CachedAttributePtr->GetListData();
    if (!ListData) return ENBTAttributeOpResult::NodeTypeMismatch;

    if (!ListData->Children.IsValidIndex(Index))
        return ENBTAttributeOpResult::NotFoundSubNode;

    FNBTAttributeID ChildID = ListData->Children[Index];

    if (bSwapRemove)
        ListData->Children.RemoveAtSwap(Index);
    else
        ListData->Children.RemoveAt(Index);

    if (Container->ReleaseRecursive(ChildID) > 0) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataAndStructVersion();
        BubbleSubtreeVersionAlongPath();
    }

    return Result;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::ListClear() const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    auto* ListData = CachedAttributePtr->GetListData();
    if (!ListData) return ENBTAttributeOpResult::NodeTypeMismatch;

    if (Container->ReleaseChildren(CachedAttributeID) > 0) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataAndStructVersion();
        BubbleSubtreeVersionAlongPath();
    }

    return ENBTAttributeOpResult::Success;
}

FNBTDataAccessor FNBTDataAccessor::ListInsertSubNode(int32 Index) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) {
        UE_LOG(NBTSystem, Warning, TEXT("Cannot add item to a invalid Path"));
        return FNBTDataAccessor(nullptr, nullptr);
    }

    auto* ListData = CachedAttributePtr->GetListData();
    if (!ListData) {
        UE_LOG(NBTSystem, Warning, TEXT("Cannot add item to non-list attribute"));
        return FNBTDataAccessor(nullptr, nullptr);
    }

    // 确保索引有效（可以等于Num()表示插入到末尾）
    if (Index < 0 || Index > ListData->Children.Num()) {
        UE_LOG(NBTSystem, Warning, TEXT("Invalid insert index %d for list of size %d"),
               Index, ListData->Children.Num());
        return FNBTDataAccessor(nullptr, nullptr);
    }

    FNBTAttributeID NewID = Container->AllocateNode();
    if (!NewID.IsValid()) {
        UE_LOG(NBTSystem, Warning, TEXT("Allocate Node Failed"));
        return FNBTDataAccessor(nullptr, nullptr);
    }

    ListData->Children.Insert(NewID, Index);

    (*CachedAttributeVersionPtr)++;
    UpdateContainerDataAndStructVersion();
    BubbleSubtreeVersionAlongPath();

    return MakeAccessFromIntIndex(Index);
}

FNBTDataAccessor FNBTDataAccessor::ListMakeAccessorByCondition(ENBTSearchCondition Condition) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return FNBTDataAccessor();
    if (auto* ListData = CachedAttributePtr->GetListData()) {
        switch (Condition) {
            case ENBTSearchCondition::IfEmpty: {
                for (int i = 0; i < ListData->Children.Num(); i++) {
                    auto Attr = Container->GetAttribute(ListData->Children[i]);
                    if (!Attr) continue;
                    if (Attr->IsEmpty()) return MakeAccessFromIntIndex(i);
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyMap: {
                for (int i = 0; i < ListData->Children.Num(); i++) {
                    auto Attr = Container->GetAttribute(ListData->Children[i]);
                    if (!Attr) continue;
                    if (auto MapData = Attr->GetMapData()) {
                        if (MapData->Children.IsEmpty()) return MakeAccessFromIntIndex(i);
                    }
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyList: {
                for (int i = 0; i < ListData->Children.Num(); i++) {
                    auto Attr = Container->GetAttribute(ListData->Children[i]);
                    if (!Attr) continue;
                    if (auto MapData = Attr->GetListData()) {
                        if (MapData->Children.IsEmpty()) return MakeAccessFromIntIndex(i);
                    }
                }
            }
            break;

            default: break;
        }
    }

    return FNBTDataAccessor();
}

FNBTAttributeOpResultDetail FNBTDataAccessor::ListMakeAccessorsByCondition(
    TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition Condition) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Accessors.Reset();

    if (auto* ListData = CachedAttributePtr->GetListData()) {
        switch (Condition) {
            case ENBTSearchCondition::IfEmpty: {
                for (int i = 0; i < ListData->Children.Num(); i++) {
                    auto Attr = Container->GetAttribute(ListData->Children[i]);
                    if (!Attr) continue;
                    if (Attr->IsEmpty()) Accessors.Add(MakeAccessFromIntIndex(i));
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyMap: {
                for (int i = 0; i < ListData->Children.Num(); i++) {
                    auto Attr = Container->GetAttribute(ListData->Children[i]);
                    if (!Attr) continue;
                    if (auto MapData = Attr->GetMapData()) {
                        if (MapData->Children.IsEmpty()) Accessors.Add(MakeAccessFromIntIndex(i));
                    }
                }
            }
            break;

            case ENBTSearchCondition::IfEmptyList: {
                for (int i = 0; i < ListData->Children.Num(); i++) {
                    auto Attr = Container->GetAttribute(ListData->Children[i]);
                    if (!Attr) continue;
                    if (auto MapData = Attr->GetListData()) {
                        if (MapData->Children.IsEmpty()) Accessors.Add(MakeAccessFromIntIndex(i));
                    }
                }
            }
            break;

            default: break;
        }
    }

    return ENBTAttributeOpResult::Success;
}

FNBTDataAccessor FNBTDataAccessor::ListMakeAccessorIfEqual(const FNBTDataAccessor& Accessor) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return {};

    if (!Accessor.IsDataExists()) return {};

    if (auto* ListData = CachedAttributePtr->GetListData()) {
        for (int i = 0; i < ListData->Children.Num(); i++) {
            if (EqualNodeDeep(Container, ListData->Children[i],
                              Accessor.Container, Accessor.CachedAttributeID))
                return MakeAccessFromIntIndex(i);
        }
    }

    return {};
}

FNBTAttributeOpResultDetail FNBTDataAccessor::ListMakeAccessorsIfEqual(TArray<FNBTDataAccessor>& Accessors,
                                                                       const FNBTDataAccessor& Accessor) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    if (!Accessor.IsDataExists()) return {};

    Accessors.Reset();

    if (auto* ListData = CachedAttributePtr->GetListData()) {
        for (int i = 0; i < ListData->Children.Num(); i++) {
            if (EqualNodeDeep(Container, ListData->Children[i],
                              Accessor.Container, Accessor.CachedAttributeID))
                Accessors.Add(MakeAccessFromIntIndex(i));
        }
    }

    return ENBTAttributeOpResult::Success;
}

FNBTDataAccessor FNBTDataAccessor::ListMakeAccessorByParameter(const FNBTSearchParameter& P) const {
    // 0) 解析当前访问器，仅一次
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success)
        return FNBTDataAccessor();

    const FNBTListData* ListData = CachedAttributePtr ? CachedAttributePtr->GetListData() : nullptr;
    if (!ListData) return FNBTDataAccessor();

    const ESearchCase::Type SearchCase = P.IgnoreCase ? ESearchCase::IgnoreCase : ESearchCase::CaseSensitive;

    // 与 Map 版本保持一致的解析与比较器（抽出来以免重复）
    auto ParseBool = [&](const FString& S, bool& Out)-> bool {
        if (S.Equals(TEXT("true"), ESearchCase::IgnoreCase) || S.Equals(TEXT("1"), SearchCase)
            || S.Equals(TEXT("yes"), ESearchCase::IgnoreCase) || S.Equals(TEXT("on"), ESearchCase::IgnoreCase)) {
            Out = true;
            return true;
        }
        if (S.Equals(TEXT("false"), ESearchCase::IgnoreCase) || S.Equals(TEXT("0"), SearchCase)
            || S.Equals(TEXT("no"), ESearchCase::IgnoreCase) || S.Equals(TEXT("off"), ESearchCase::IgnoreCase)) {
            Out = false;
            return true;
        }
        bool tmp = false;
        if (LexTryParseString(tmp, *S)) {
            Out = tmp;
            return true;
        }
        return false;
    };

    int64 ParamI64 = 0;
    bool bHasI64 = false;
    double ParamDbl = 0.0;
    bool bHasDbl = false;
    bool ParamBool = false;
    bool bHasBool = false;

    auto PrimeParamCache = [&]() {
        switch (P.ValueType) {
            case ENBTAttributeType::Int8:
            case ENBTAttributeType::Int16:
            case ENBTAttributeType::Int32:
            case ENBTAttributeType::Int64:
            case ENBTAttributeType::Boolean: {
                if (P.ValueType == ENBTAttributeType::Boolean) {
                    bHasBool = ParseBool(P.Value, ParamBool);
                } else {
                    bHasI64 = LexTryParseString(ParamI64, *P.Value);
                }
            }
            break;
            case ENBTAttributeType::Float:
            case ENBTAttributeType::Double: {
                bHasDbl = LexTryParseString(ParamDbl, *P.Value);
            }
            break;
            default: break;
        }
        if (P.EnableGenericSearch && P.ValueType == ENBTAttributeType::String) {
            int64 iTmp;
            double dTmp;
            bool bTmp;
            if (LexTryParseString(iTmp, *P.Value)) {
                ParamI64 = iTmp;
                bHasI64 = true;
            }
            if (LexTryParseString(dTmp, *P.Value)) {
                ParamDbl = dTmp;
                bHasDbl = true;
            }
            if (ParseBool(P.Value, bTmp)) {
                ParamBool = bTmp;
                bHasBool = true;
            }
        }
    };
    PrimeParamCache();

    auto CmpInt = [&](int64 A)-> bool {
        if (!bHasI64) return false;
        switch (P.Op) {
            case ENBTCompareOp::Eq: return A == ParamI64;
            case ENBTCompareOp::Ne: return A != ParamI64;
            case ENBTCompareOp::Gt: return A > ParamI64;
            case ENBTCompareOp::Ge: return A >= ParamI64;
            case ENBTCompareOp::Lt: return A < ParamI64;
            case ENBTCompareOp::Le: return A <= ParamI64;
            default: return false;
        }
    };
    auto NearlyEqual = [](double A, double B) { return FMath::IsNearlyEqual(A, B, 1e-4); };
    auto CmpDbl = [&](double A)-> bool {
        if (!bHasDbl) return false;
        switch (P.Op) {
            case ENBTCompareOp::Eq: return NearlyEqual(A, ParamDbl);
            case ENBTCompareOp::Ne: return !NearlyEqual(A, ParamDbl);
            case ENBTCompareOp::Gt: return A > ParamDbl;
            case ENBTCompareOp::Ge: return A >= ParamDbl;
            case ENBTCompareOp::Lt: return A < ParamDbl;
            case ENBTCompareOp::Le: return A <= ParamDbl;
            default: return false;
        }
    };
    auto CmpBool = [&](bool A)-> bool {
        if (!bHasBool) return false;
        switch (P.Op) {
            case ENBTCompareOp::Eq: return A == ParamBool;
            case ENBTCompareOp::Ne: return A != ParamBool;
            default: return false;
        }
    };
    auto CmpStr = [&](const FString& A)-> bool {
        switch (P.Op) {
            case ENBTCompareOp::Eq: return A.Equals(P.Value, SearchCase);
            case ENBTCompareOp::Ne: return !A.Equals(P.Value, SearchCase);
            case ENBTCompareOp::Contains: return A.Contains(P.Value, SearchCase);
            case ENBTCompareOp::StartsWith: return A.StartsWith(P.Value, SearchCase);
            case ENBTCompareOp::EndsWith: return A.EndsWith(P.Value, SearchCase);
            default: return false;
        }
    };
    auto AttrToString = [&](const FNBTAttribute& A)-> FString {
        switch (A.GetType()) {
            case ENBTAttributeType::String: {
                if (auto v = A.GetBaseType<FString>()) return v.GetValue();
                break;
            }
            case ENBTAttributeType::Name: {
                if (auto v = A.GetBaseType<FName>()) return v.GetValue().ToString();
                break;
            }
            case ENBTAttributeType::Boolean: {
                if (auto v = A.GetBaseType<bool>()) return v.GetValue() ? TEXT("true") : TEXT("false");
                break;
            }
            case ENBTAttributeType::Int8: {
                if (auto v = A.GetBaseType<int8>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Int16: {
                if (auto v = A.GetBaseType<int16>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Int32: {
                if (auto v = A.GetBaseType<int32>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Int64: {
                if (auto v = A.GetBaseType<int64>()) return LexToString(v.GetValue());
                break;
            }
            case ENBTAttributeType::Float: {
                if (auto v = A.GetBaseType<float>()) return FString::SanitizeFloat(v.GetValue());
                break;
            }
            case ENBTAttributeType::Double: {
                if (auto v = A.GetBaseType<double>()) return FString::SanitizeFloat(v.GetValue());
                break;
            }
            default: break;
        }
        return FString();
    };
    auto MatchValue = [&](const FNBTAttribute& A)-> bool {
        const ENBTAttributeType T = A.GetType();
        switch (P.ValueType) {
            case ENBTAttributeType::Boolean: {
                if (T == ENBTAttributeType::Boolean) {
                    if (auto v = A.GetBaseType<bool>()) return CmpBool(v.GetValue());
                    return false;
                }
            }
            break;

            case ENBTAttributeType::Int8:
            case ENBTAttributeType::Int16:
            case ENBTAttributeType::Int32:
            case ENBTAttributeType::Int64: {
                if (auto v = A.GetGenericInt(); v.IsSet()) return CmpInt(v.GetValue());
            }
            break;

            case ENBTAttributeType::Float:
            case ENBTAttributeType::Double: {
                if (auto v = A.GetGenericDouble(); v.IsSet()) return CmpDbl(v.GetValue());
            }
            break;

            case ENBTAttributeType::Name: {
                if (T == ENBTAttributeType::Name) {
                    if (auto v = A.GetBaseType<FName>()) return CmpStr(v.GetValue().ToString());
                }
                if (P.EnableGenericSearch && T == ENBTAttributeType::String) {
                    if (auto v = A.GetBaseType<FString>()) return CmpStr(v.GetValue());
                }
            }
            break;

            case ENBTAttributeType::String: {
                if (T == ENBTAttributeType::String) {
                    if (auto v = A.GetBaseType<FString>()) return CmpStr(v.GetValue());
                }
                if (P.EnableGenericSearch && T == ENBTAttributeType::Name) {
                    if (auto v = A.GetBaseType<FName>()) return CmpStr(v.GetValue().ToString());
                }
            }
            break;

            default: break;
        }

        if (P.EnableGenericSearch) {
            switch (P.Op) {
                case ENBTCompareOp::Contains:
                case ENBTCompareOp::StartsWith:
                case ENBTCompareOp::EndsWith:
                case ENBTCompareOp::Eq:
                case ENBTCompareOp::Ne: {
                    const FString S = AttrToString(A);
                    if (!S.IsEmpty()) return CmpStr(S);
                }
                break;
                default: {
                    if (auto vi = A.GetGenericInt(); vi.IsSet() && bHasI64) return CmpInt(vi.GetValue());
                    if (auto vd = A.GetGenericDouble(); vd.IsSet() && bHasDbl) return CmpDbl(vd.GetValue());
                    if (P.ValueType == ENBTAttributeType::String && bHasBool) {
                        if (auto vb = A.GetBaseType<bool>()) return CmpBool(vb.GetValue());
                    }
                }
                break;
            }
        }

        return false;
    };

    auto BuildResultForIndex = [&](int32 Index, FNBTAttributeID ChildID)-> FNBTDataAccessor {
        FNBTDataAccessor Out;
        Out.Container = this->Container;
        Out.ContainerLiveToken = this->ContainerLiveToken;
        Out.Path = this->Path;
        Out.Path.Add(TVariant<FName, int32>(TInPlaceType<int32>(), Index));
        Out.CachedAttributeID = ChildID;
        Out.CachedContainerStructVersion = this->CachedContainerStructVersion;
        Out.CachedAttributeVersionPtr = Container->GetAttributeVersion(ChildID);
        Out.CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(ChildID);
        Out.CachedAttributePtr = Container->GetAttribute(ChildID);
        return Out;
    };

    auto ChildMatches = [&](FNBTAttributeID ChildID)-> bool {
        const FNBTAttribute* Attr = Container->GetAttribute(ChildID);
        if (!Attr) return false;

        if (P.SubKey.IsNone()) {
            return MatchValue(*Attr);
        } else {
            if (const FNBTMapData* SubMap = Attr->GetMapData()) {
                const FNBTAttributeID* SubID = SubMap->Children.Find(P.SubKey);
                if (!SubID) return false;
                const FNBTAttribute* SubAttr = Container->GetAttribute(*SubID);
                return SubAttr ? MatchValue(*SubAttr) : false;
            }
            return false;
        }
    };

    // 遍历整个列表（List 下没有 Key 的概念；若你想“限定某个固定索引”，可在参数里约定 Key 的字符串表示某个整数索引，再做转换）
    for (int32 i = 0; i < ListData->Children.Num(); ++i) {
        const FNBTAttributeID ChildID = ListData->Children[i];
        if (ChildMatches(ChildID)) {
            return BuildResultForIndex(i, ChildID);
        }
    }
    return FNBTDataAccessor();
}

FNBTAttributeOpResultDetail FNBTDataAccessor::MakeAccessorFromList(TArray<FNBTDataAccessor>& Accessors) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (auto* ListData = CachedAttributePtr->GetListData()) {
        Accessors.Reset();
        Accessors.Reserve(ListData->Children.Num());
        for (int i = 0; i < ListData->Children.Num(); i++) {
            Accessors.Add(MakeAccessFromIntIndex(i));
        }
    } else return ENBTAttributeOpResult::NodeTypeMismatch;

    return ENBTAttributeOpResult::Success;
}

TArray<FNBTDataAccessor> FNBTDataAccessor::MakeAccessorFromListNow() const {
    TArray<FNBTDataAccessor> AccessorResult{};
    MakeAccessorFromList(AccessorResult);
    return AccessorResult;
}

int FNBTDataAccessor::Remove() { // 删除当前节点

    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);

    if (Result != ENBTAttributeOpResult::Success) //当前节点本身就不存在
        return 0;

    int RemoveNum = Container->ReleaseRecursive(CachedAttributeID);

    UpdateContainerDataAndStructVersion();
    BubbleSubtreeVersionAlongPath();

    CachedAttributeID = FNBTAttributeID();
    CachedAttributePtr = nullptr;
    CachedAttributeVersionPtr = nullptr;
    CachedSubtreeVersionPtr = nullptr;
    return RemoveNum;
}

FString FNBTDataAccessor::ToString(bool bShowVersion) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return "$Node Not Exist$";

    FString ResStr{};

    int Deep = 0;
    ToStringImp(ResStr, Deep, bShowVersion);

    return ResStr;
}

FString FNBTDataAccessor::GetPath() const {
    if (!IsAccessorValid()) return "$IsAccessorValid$";
    FString PathStr = "Root";

    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        PathStr = "$Node Not Exist$ ";

    for (int i = 0; i < Path.Num(); i++) {
        PathStr += " -> ";
        if (Path[i].GetIndex() == 0) {
            PathStr += Path[i].Get<FName>().ToString();
        } else {
            PathStr += FString::FromInt(Path[i].Get<int32>());
        }
    }

    return PathStr;
}

FString FNBTDataAccessor::GetPreviewPath() const {
    if (!IsAccessorValid()) return "$IsAccessorValid$";
    FString PathStr = "Root";
    for (int i = 0; i < Path.Num(); i++) {
        PathStr += " -> ";
        if (Path[i].GetIndex() == 0) {
            PathStr += Path[i].Get<FName>().ToString();
        } else {
            PathStr += FString::FromInt(Path[i].Get<int32>());
        }
    }
    return PathStr;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::TryCopyFrom(const FNBTDataAccessor& Source) const {
    auto SourceResult = Source.ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (SourceResult != ENBTAttributeOpResult::Success)
        return SourceResult;

    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    return CopyImp(Source);
}

FNBTAttributeOpResultDetail FNBTDataAccessor::TrySwap(const FNBTDataAccessor& Target) const {
    auto SourceResult = Target.ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (SourceResult != ENBTAttributeOpResult::Success)
        return SourceResult;

    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    if (CachedAttributePtr == Target.CachedAttributePtr)
        return ENBTAttributeOpResult::Success;

    if (Container == Target.Container && CachedAttributeID == Target.CachedAttributeID)
        return ENBTAttributeOpResult::SameAndNotChange;


    if (Target.Container == Container) {
        if (IsAncestor(Target, *this) || IsAncestor(*this, Target)) {
            return ENBTAttributeOpResult::InvalidID;
        }

        if (!Container->IsRemainingSpaceSupportDoubleCopy(CachedAttributeID, Target.CachedAttributeID))
            return ENBTAttributeOpResult::AllocateFailed;
    } else {
        bool TagetSupport = Target.Container->IsRemainingSpaceSupportCopy(CachedAttributeID, *Container);
        bool ThisSupport = Container->IsRemainingSpaceSupportCopy(Target.CachedAttributeID, *Target.Container);
        if (!TagetSupport || !ThisSupport) return ENBTAttributeOpResult::AllocateFailed;
    }

    FNBTAttributeID ThisOldID = CachedAttributeID;
    FNBTAttributeID TargetOldID = Target.CachedAttributeID;

    FNBTAttributeID ThisNewID = Container->DeepCopyNode(TargetOldID, *Target.Container); //自己这里拷贝对面的
    FNBTAttributeID TargetNewID = Target.Container->DeepCopyNode(ThisOldID, *Container); //对面拷贝自己的

    auto ThisResult = this->RedirectNode(ThisOldID, ThisNewID);
    auto TargetResult = Target.RedirectNode(TargetOldID, TargetNewID);

    if (ThisResult != ENBTAttributeOpResult::Success || TargetResult != ENBTAttributeOpResult::Success) {
        check(false); //不应该到达这里;
    }

    return ENBTAttributeOpResult::Success;
}

bool FNBTDataAccessor::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) {
    bOutSuccess = false;

    uint8 bHasData = 0;

    if (Ar.IsSaving()) {
        const bool bValid =
            IsContainerValid() &&
            Container->ParentComponent.IsValid() &&
            Container->ParentComponent->GetIsReplicated();
        
        bHasData = bValid ? 1 : 0;
        Ar << bHasData;

        UObject* Obj = bValid ? static_cast<UObject*>(Container->ParentComponent.Get()) : nullptr;
        const bool bObjOk = Map->SerializeObject(Ar, UNBTComponent::StaticClass(), Obj);
        if (!bObjOk) {
            return true;
        }

        if (!bValid) {
            bOutSuccess = true;
            return true;
        }

        uint32 Count = static_cast<uint32>(Path.Num());
        Ar.SerializeIntPacked(Count);

        for (const TVariant<FName, int32>& V : Path) {
            const uint8 Tag = V.IsType<FName>() ? 0 : 1;
            Ar << const_cast<uint8&>(Tag);

            if (Tag == 0) {
                FName Name = V.Get<FName>();
                Ar << Name;
            } else {
                int32 I = V.Get<int32>();
                Ar << I;
            }
        }
        
        bOutSuccess = true;
        return true;
    } else {
        ResetAll();

        Ar << bHasData;

        UObject* Obj = nullptr;
        const bool bObjOk = Map->SerializeObject(Ar, UNBTComponent::StaticClass(), Obj);
        if (!bObjOk) {
            UE_LOG(NBTSystem, Warning, TEXT("NBTDataAccessor::NetSerialize(recv): SerializeObject failed"));
            return true;
        }

        if (!bHasData || Obj == nullptr) {
            bOutSuccess = true;
            return true;
        }

        UNBTComponent* Comp = Cast<UNBTComponent>(Obj);
        if (!Comp) {
            UE_LOG(NBTSystem, Warning, TEXT("NBTDataAccessor::NetSerialize(recv): Cast to UNBTComponent failed"));
            return true;
        }

        Container = &(Comp->NBTContainer);
        ContainerLiveToken = Container->LiveToken.ToWeakPtr();

        uint32 Count = 0;
        Ar.SerializeIntPacked(Count);

        constexpr uint32 kMaxPathEntries = 2048u;
        if (Count > kMaxPathEntries) {
            UE_LOG(NBTSystem, Warning, TEXT("NBTDataAccessor::NetSerialize(recv): Path count %u exceeds limit %u"), Count, kMaxPathEntries);
            return true;
        }

        Path.Empty(Count);

        for (uint32 i = 0; i < Count; ++i) {
            uint8 Tag = 0;
            Ar << Tag;

            if (Tag == 0) {
                FName N;
                Ar << N;
                Path.Emplace(TVariant<FName, int32>(TInPlaceType<FName>(), N));
            } else if (Tag == 1) {
                int32 I = 0;
                Ar << I;
                Path.Emplace(TVariant<FName, int32>(TInPlaceType<int32>(), I));
            } else {
                UE_LOG(NBTSystem, Warning, TEXT("NBTDataAccessor::NetSerialize(recv): Invalid tag %u at index %u"), Tag, i);
                return true;
            }
        }

        bOutSuccess = true;
        return true;
    }
}


ENBTAttributeOpResult FNBTDataAccessor::RedirectNode(FNBTAttributeID OldID, FNBTAttributeID NewID) const {
    bool bRepointed = false;

    if (Path.Num() == 0) {
        Container->RootID = NewID;
        bRepointed = true;
    } else {
        auto ParentAccessor = GetParent();
        if (!ParentAccessor.IsDataExists()) return ENBTAttributeOpResult::InvalidID;

        auto ParentAttr = Container->GetAttribute(ParentAccessor.CachedAttributeID);
        if (!ParentAttr) return ENBTAttributeOpResult::InvalidID;

        if (const FName* Key = Path.Last().TryGet<FName>()) {
            auto* MapData = ParentAttr->GetMapData();
            if (!MapData) return ENBTAttributeOpResult::NodeTypeMismatch;
            if (FNBTAttributeID* Slot = MapData->Children.Find(*Key)) {
                *Slot = NewID;
                (*ParentAccessor.CachedAttributeVersionPtr)++;
                bRepointed = true;
            } else {
                return ENBTAttributeOpResult::NotFoundSubNode;
            }
        } else if (const int32* Index = Path.Last().TryGet<int32>()) {
            auto* ListData = ParentAttr->GetListData();
            if (!ListData) return ENBTAttributeOpResult::NodeTypeMismatch;
            if (!ListData->Children.IsValidIndex(*Index)) return ENBTAttributeOpResult::NotFoundSubNode;

            ListData->Children[*Index] = NewID;
            (*ParentAccessor.CachedAttributeVersionPtr)++;
            bRepointed = true;
        } else {
            check(false);
            return ENBTAttributeOpResult::InvalidContainer;
        }
    }

    CachedAttributeID = NewID;
    CachedAttributePtr = Container->GetAttribute(NewID);
    CachedAttributeVersionPtr = Container->GetAttributeVersion(NewID);
    CachedSubtreeVersionPtr = Container->GetAttributeSubtreeVersion(NewID);

    if (CachedAttributeVersionPtr) {
        (*CachedAttributeVersionPtr)++;
    }

    if (bRepointed) {
        UpdateContainerDataAndStructVersion();
    } else {
        check(false);
        UpdateContainerDataVersion(); // 不应该到达这里
    }

    BubbleSubtreeVersionAlongPath();

    if (OldID.IsValid() && OldID != NewID) {
        Container->ReleaseRecursive(OldID);
    }

    return ENBTAttributeOpResult::Success;
}

FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndCopyFrom(const FNBTDataAccessor& Source) const {
    auto SourceResult = Source.ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (SourceResult != ENBTAttributeOpResult::Success)
        return SourceResult;

    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success)
        return Result;

    return CopyImp(Source);
}

ENBTAttributeOpResult FNBTDataAccessor::CopyImp(const FNBTDataAccessor& Source) const {
    if (CachedAttributePtr == Source.CachedAttributePtr)
        return ENBTAttributeOpResult::Success;

    if (!CachedAttributePtr->IsCompoundType() && !Source.CachedAttributePtr->IsCompoundType()) {
        auto& PtrA = *CachedAttributePtr;
        auto& PtrB = *Source.CachedAttributePtr;
        auto const OpResult = PtrA.OverrideFromIfNotCompound(PtrB);
        if (OpResult == ENBTAttributeOpResult::Success) { //还可能返回Same, 但是返回Same则说明数据没有改变, 不可能返回其他错误
            (*CachedAttributeVersionPtr)++;
            UpdateContainerDataVersion();
            BubbleSubtreeVersionAlongPath();
        }
        return OpResult;
    } else {
        FNBTAttributeID OldID = CachedAttributeID;
        FNBTAttributeID NewID{};
        bool bRepointed = false;

        NewID = Container->DeepCopyNode(Source.CachedAttributeID, *Source.Container);
        if (!NewID.IsValid()) return ENBTAttributeOpResult::AllocateFailed;

        return RedirectNode(OldID, NewID);
    }
}

FNBTDataAccessor FNBTDataAccessor::EnsureList() const {
    if (!IsAccessorValid()) return *this;

    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success)
        return FNBTDataAccessor(nullptr, nullptr);

    if (CachedAttributePtr->GetType() == ENBTAttributeType::List) {
        return *this;
    } else if (CachedAttributePtr->IsEmpty()) {
        CachedAttributePtr->OverrideToEmptyList();
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataAndStructVersion();
        BubbleSubtreeVersionAlongPath();
        return *this;
    }

    return FNBTDataAccessor(nullptr, nullptr);
}

void FNBTDataAccessor::ToStringImp(FString& Str, int& Deep, bool bShowVersion) const {
    if (!IsContainerValid() || !CachedAttributeID.IsValid()) {
        Str += TEXT("$Invalid$");
        return;
    }

    if (Deep > 64) {
        Str += "Deep > 64, Break";
        return;
    }

    FNBTAttribute* CurAttr = Container->GetAttribute(CachedAttributeID);
    if (!CurAttr) {
        Str += TEXT("$Invalid$");
        return;
    }

    int32* CurVer = Container->GetAttributeVersion(CachedAttributeID);
    int32* CurSub = Container->GetAttributeSubtreeVersion(CachedAttributeID);
    if (!CurVer || !CurSub) {
        Str += TEXT("$InvalidVersion$");
        return;
    }
    const auto Type = CurAttr->GetType();

    // 缩进辅助
    auto GetIndent = [](int Depth) -> FString {
        return FString::ChrN(Depth * 2, ' ');
    };

    if (Type == ENBTAttributeType::Map) {
        auto* MapData = CurAttr->GetMapData();
        if (!MapData) {
            Str += TEXT("$InvalidMap$");
            return;
        }

        if (bShowVersion) {
            if (CurSub) {
                Str += FString::Printf(TEXT("[V:%d][SV:%d] "), *CurVer, *CurSub);
            } else {
                Str += FString::Printf(TEXT("[V:%d] "), *CurVer);
            }
        }

        Str += TEXT("{\n");
        Deep++;

        bool bFirst = true;
        for (const auto& Pair : MapData->Children) {
            if (!bFirst) Str += TEXT(",\n");
            bFirst = false;

            Str += GetIndent(Deep);
            Str += TEXT("\"");
            Str += Pair.Key.ToString();
            Str += TEXT("\": ");

            // 递归处理子节点
            FNBTDataAccessor ChildAccessor = MakeAccessFromFName(Pair.Key);
            if (ChildAccessor.ResolvePathInternal(ENBTPathResolveMode::ReadOnly) == ENBTAttributeOpResult::Success) {
                ChildAccessor.ToStringImp(Str, Deep, bShowVersion);
            } else {
                Str += TEXT("$Invalid$");
            }
        }

        Deep--;
        if (!bFirst) Str += TEXT("\n");
        Str += GetIndent(Deep);
        Str += TEXT("}");
    } else if (Type == ENBTAttributeType::List) {
        auto* ListData = CurAttr->GetListData();
        if (!ListData) {
            Str += TEXT("$InvalidList$");
            return;
        }

        if (bShowVersion) {
            if (CurSub) {
                Str += FString::Printf(TEXT("[V:%d][SV:%d] "), *CurVer, *CurSub);
            } else {
                Str += FString::Printf(TEXT("[V:%d] "), *CurVer);
            }
        }

        Str += TEXT("[\n");
        Deep++;

        for (int32 i = 0; i < ListData->Children.Num(); ++i) {
            if (i > 0) Str += TEXT(",\n");

            Str += GetIndent(Deep);

            // 递归处理子节点
            FNBTDataAccessor ChildAccessor = MakeAccessFromIntIndex(i);
            if (ChildAccessor.ResolvePathInternal(ENBTPathResolveMode::ReadOnly) == ENBTAttributeOpResult::Success) {
                ChildAccessor.ToStringImp(Str, Deep, bShowVersion);
            } else {
                Str += TEXT("$Invalid$");
            }
        }

        Deep--;
        if (ListData->Children.Num() > 0) Str += TEXT("\n");
        Str += GetIndent(Deep);
        Str += TEXT("]");
    } else {
        if (bShowVersion) {
            Str += FString::Printf(TEXT("[V:%d] "), *CurVer);
        }
        Str += CurAttr->ToString();
    }
}

void UFArzNBTDataAccessorScriptMixinLibrary::VisitData(const FNBTDataAccessor& TargetAccessor, UObject* Object,
                                                       FName FunctionName) {
    if (IsValid(Object)) {
        auto Func = Object->FindFunction(FunctionName);
        if (Func) {
            FArzNBTDataVisitorSignature Sig;
            Sig.BindUFunction(Object, FunctionName);

            if (!Sig.IsBound()) {
                UE_LOG(NBTSystem, Error, TEXT("==== VisitData(..): Function %s is not valid"), *FunctionName.ToString());
                UE_LOG(NBTSystem, Error, TEXT("==== tack Trace:"));
                const auto Stack = FAngelscriptManager::Get().GetAngelscriptCallstack();
                for (const auto& Frame : Stack) {
                    UE_LOGFMT(NBTSystem, Error, "==== {0}", Frame);
                }
                return;
            }

            TargetAccessor.VisitData([&Sig](int Deep, ENBTAttributeType Type, FName AttrName, int32 AttrIdx, FNBTDataAccessor Accessor) {
                Sig.ExecuteIfBound(Deep, Type, AttrName, AttrIdx, Accessor);
            });
        }
    }
}