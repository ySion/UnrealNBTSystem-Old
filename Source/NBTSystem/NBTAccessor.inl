#pragma once

#include "NBTAccessor.h"
#include "NBTContainer.h"
#include "NBTAttribute.h"

template <typename T>
TOptional<T> FNBTDataAccessor::TryGet() const {
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success)
        return {};
    return CachedAttributePtr->GetBaseType<T>();
}

template <typename T>
const TArray<T>* FNBTDataAccessor::TryGetArray() const {
    if (ResolvePathInternal(ENBTPathResolveMode::ReadOnly) != ENBTAttributeOpResult::Success)
        return {};
    return CachedAttributePtr->GetArrayType<T>();
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetBaseType(T Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Result = CachedAttributePtr->TrySetBaseType(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    } else if (Result != ENBTAttributeOpResult::SameAndNotChange) {
        Result.ResultMessage = FString::Printf(TEXT("Node [%s] : is %s."), *GetPathString(Path.Num()), *CachedAttributePtr->GetTypeString());
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetBaseTypeRef(const T& Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Result = CachedAttributePtr->TrySetBaseTypeRef(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    } else if (Result != ENBTAttributeOpResult::SameAndNotChange) {
        Result.ResultMessage = FString::Printf(TEXT("Node [%s] : is %s."), *GetPathString(Path.Num()), *CachedAttributePtr->GetTypeString());
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::TrySetArray(const TArray<T>& Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    Result = CachedAttributePtr->TrySetArrayType<T>(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    } else if (Result != ENBTAttributeOpResult::SameAndNotChange) {
        Result.ResultMessage = FString::Printf(TEXT("Node [%s] : is %s."), *GetPathString(Path.Num()), *CachedAttributePtr->GetTypeString());
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetBaseType(T Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    if (CachedAttributePtr->IsEmpty()) {
        Result = CachedAttributePtr->OverriderToBaseType<T>(Value);
    } else {
        Result = CachedAttributePtr->TrySetBaseType<T>(Value);
    }

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    }  else if (Result != ENBTAttributeOpResult::SameAndNotChange) {
        Result.ResultMessage = FString::Printf(TEXT("Node [%s] : is %s."), *GetPathString(Path.Num()), *CachedAttributePtr->GetTypeString());
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetBaseTypeRef(const T& Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    if (CachedAttributePtr->IsEmpty()) {
        Result = CachedAttributePtr->OverriderToBaseTypeRef<T>(Value);
    } else {
        Result = CachedAttributePtr->TrySetBaseTypeRef<T>(Value);
    }

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    }  else if (Result != ENBTAttributeOpResult::SameAndNotChange) {
        Result.ResultMessage = FString::Printf(TEXT("Node [%s] : is %s."), *GetPathString(Path.Num()), *CachedAttributePtr->GetTypeString());
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::EnsureAndSetArray(const TArray<T>& Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::EnsureCreate);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    if (CachedAttributePtr->IsEmpty()) {
        Result = CachedAttributePtr->OverriderToArrayType<T>(Value);
    } else {
        Result = CachedAttributePtr->TrySetArrayType<T>(Value);
    }

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        UpdateContainerDataVersion();
        BubbleSubtreeVersionAlongPath();
    } else if (Result != ENBTAttributeOpResult::SameAndNotChange) {
        Result.ResultMessage = FString::Printf(TEXT("Node [%s] : is %s."), *GetPathString(Path.Num()), *CachedAttributePtr->GetTypeString());
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToBaseType(T Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ForceOverride);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    const bool bWasCompoundType = CachedAttributePtr->IsCompoundType();

    if (bWasCompoundType) { //如果是复合结构, 那么先移除所有子项
        Container->ReleaseChildren(CachedAttributeID);
    }

    Result = CachedAttributePtr->OverriderToBaseType(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        if (bWasCompoundType) {
            UpdateContainerDataAndStructVersion();
            BubbleSubtreeVersionAlongPath();
        } else {
            UpdateContainerDataVersion();
            BubbleSubtreeVersionAlongPath();
        }
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToBaseTypeRef(const T& Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ForceOverride);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    const bool bWasCompoundType = CachedAttributePtr->IsCompoundType();

    if (bWasCompoundType) { //如果是复合结构, 那么先移除所有子项
        Container->ReleaseChildren(CachedAttributeID);
    }

    Result = CachedAttributePtr->OverriderToBaseTypeRef(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        if (bWasCompoundType) {
            UpdateContainerDataAndStructVersion();
            BubbleSubtreeVersionAlongPath();
        } else {
            UpdateContainerDataVersion();
            BubbleSubtreeVersionAlongPath();
        }
    }
    return Result;
}

template <typename T>
FNBTAttributeOpResultDetail FNBTDataAccessor::OverrideToArray(const TArray<T>& Value) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ForceOverride);
    if (Result != ENBTAttributeOpResult::Success) return Result;

    const bool bWasCompoundType = CachedAttributePtr->IsCompoundType();

    if (bWasCompoundType) { //如果是复合结构, 那么先移除所有子项
        Container->ReleaseChildren(CachedAttributeID);
    }

    Result = CachedAttributePtr->OverriderToArrayType<T>(Value);

    if (Result == ENBTAttributeOpResult::Success) {
        (*CachedAttributeVersionPtr)++;
        if (bWasCompoundType) {
            UpdateContainerDataAndStructVersion();
            BubbleSubtreeVersionAlongPath();
        } else {
            UpdateContainerDataVersion();
            BubbleSubtreeVersionAlongPath();
        }
    }
    return Result;
}

template <typename Func>
void FNBTDataAccessor::VisitData(Func DataVisitor) const {
    auto Result = ResolvePathInternal(ENBTPathResolveMode::ReadOnly);
    if (Result != ENBTAttributeOpResult::Success)
        return;

    if (!CachedAttributePtr)
        return;

    int Deep = 0;
    VisitDataImp(Deep, Container->GetRootID() == CachedAttributeID ? FName("Root") : NAME_None, -1, *this, DataVisitor);
}

template <typename Func>
void FNBTDataAccessor::VisitDataImp(int32& Deep, FName AttrName, int idx, FNBTDataAccessor TargetAccessor,
    Func DataVisitor) const {
    const ENBTAttributeType Type = CachedAttributePtr->GetType();

    if (Type == ENBTAttributeType::Map) {
        auto* MapData = CachedAttributePtr->GetMapData();
        if (!MapData)
            return;
        DataVisitor(Deep, Type, AttrName, idx, TargetAccessor);
        Deep++;
        for (const auto& Pair : MapData->Children) {
            FNBTDataAccessor ChildAccessor = MakeAccessFromFName(Pair.Key);
            if (ChildAccessor.ResolvePathInternal(ENBTPathResolveMode::ReadOnly) == ENBTAttributeOpResult::Success) {
                ChildAccessor.VisitDataImp(Deep, Pair.Key, -1, ChildAccessor, DataVisitor);
            }
        }
        Deep--;
    } else if (Type == ENBTAttributeType::List) {
        auto* ListData = CachedAttributePtr->GetListData();
        if (!ListData)
            return;
        DataVisitor(Deep, Type, AttrName, idx, TargetAccessor);
        Deep++;
        for (int32 i = 0; i < ListData->Children.Num(); ++i) {
            FNBTDataAccessor ChildAccessor = MakeAccessFromIntIndex(i);
            if (ChildAccessor.ResolvePathInternal(ENBTPathResolveMode::ReadOnly) == ENBTAttributeOpResult::Success) {
                ChildAccessor.VisitDataImp(Deep, NAME_None, i, ChildAccessor, DataVisitor);
            }
        }
        Deep--;
    } else {
        DataVisitor(Deep, Type, AttrName, idx, TargetAccessor);
    }
}
