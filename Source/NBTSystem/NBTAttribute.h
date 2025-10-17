#pragma once

#include "CoreMinimal.h"
#include "NBTCommon.h"
#include "NBTAttributeID.h"
#include "UObject/Object.h"

struct FNBTAttribute;
struct FNBTMapData;
struct FNBTListData;

struct FNBTMapData {
    TMap<FName, FNBTAttributeID> Children;
    void SerializeNBTData(FArchive& Ar, bool NetWorkMode);
};

struct FNBTListData {
    TArray<FNBTAttributeID> Children;
    
    void SerializeNBTData(FArchive& Ar, bool NetWorkMode) {
        Ar << Children;
    }
};

struct FNBTAttribute {
    using AttributeType = TVariant<
        FEmptyVariantState,
        bool,
        int8, int16, int32, int64,
        float, double,
        FName,
        FString,
        FColor,
        FGuid,
        FSoftClassPath,
        FSoftObjectPath,
        FDateTime,
        FRotator,
        FVector2D, FVector,
        FIntVector2, FIntVector,
        FInt64Vector2, FInt64Vector,
        TArray<int8>, TArray<int16>, TArray<int32>, TArray<int64>,
        TArray<float>, TArray<double>,
        FNBTMapData,
        FNBTListData
    >;

private:
    friend struct FNBTContainer;

    friend struct FAttributeChunk;

    friend struct FNBTAllocator;

    friend struct FNBTDataAccessor;

    AttributeType Value;

    FNBTAttribute() { Reset(); };
    ~FNBTAttribute() = default;

    FNBTAttribute(const FNBTAttribute&) = delete;

    FNBTAttribute& operator=(const FNBTAttribute&) = delete;

    FNBTAttribute(FNBTAttribute&&) = delete;

    FNBTAttribute& operator=(FNBTAttribute&&) = delete;

    void Reset() { Value.Set<FEmptyVariantState>({}); }

    ENBTAttributeType GetType() const { return static_cast<ENBTAttributeType>(Value.GetIndex()); }

    FString GetTypeString() const;

    bool IsEmpty() const { return GetType() == ENBTAttributeType::Empty; }

    template <typename T>
    bool IsType() const { return Value.IsType<T>(); }

    bool IsCompoundType() const { return GetType() == ENBTAttributeType::Map || GetType() == ENBTAttributeType::List; }

    bool IsArrayType() const { return GetType() >= ENBTAttributeType::ArrayInt8 && GetType() <= ENBTAttributeType::ArrayDouble; }

    template <typename T>
    TOptional<T> GetBaseType() const {
        const T* p = Value.TryGet<T>();
        return p ? TOptional<T>(*p) : TOptional<T>();
    }

    template <typename T>
    TArray<T>* GetArrayType() {
        auto Ptr = Value.TryGet<TArray<T>>();
        return Ptr;
    }

    template <typename T>
    ENBTAttributeOpResult TrySetBaseType(T InValue) {
        if (T* Ptr = Value.TryGet<T>()) {
            if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                if (FMath::IsNearlyEqual(*Ptr, InValue, 0.0001f)) {
                    return ENBTAttributeOpResult::SameAndNotChange;
                }
            }
            
            if (*Ptr == InValue) {
                return ENBTAttributeOpResult::SameAndNotChange;
            }

            *Ptr = InValue;
            return ENBTAttributeOpResult::Success;
        }
        return ENBTAttributeOpResult::NodeTypeMismatch;
    }

    template<typename T>
    ENBTAttributeOpResult TrySetBaseTypeRef(const T& InValue) {
        if (T* Ptr = Value.TryGet<T>(); Ptr) {
            if (InValue == *Ptr) {
                return ENBTAttributeOpResult::SameAndNotChange;
            }
            *Ptr = InValue;
            return ENBTAttributeOpResult::Success;
        }
        return ENBTAttributeOpResult::NodeTypeMismatch;
    }


    template <typename T>
    ENBTAttributeOpResult TrySetArrayType(const TArray<T>& InValue) {
        if (TArray<T>* Ptr = Value.TryGet<TArray<T>>(); Ptr) {
            if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                if (HelperCompareFloatArray(*Ptr, InValue)) return ENBTAttributeOpResult::SameAndNotChange;
            } else {
                if (*Ptr == InValue) return ENBTAttributeOpResult::SameAndNotChange;
            }
            *Ptr = InValue;
            return ENBTAttributeOpResult::Success;
        }
        return ENBTAttributeOpResult::NodeTypeMismatch;
    }

    template <typename T>
    ENBTAttributeOpResult OverriderToBaseType(T InValue) {
        if (T* Ptr = Value.TryGet<T>()) {
            if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                if (FMath::IsNearlyEqual(*Ptr, InValue, 0.0001f)) {
                    return ENBTAttributeOpResult::SameAndNotChange;
                }
            }
           
            if (*Ptr == InValue) {
                return ENBTAttributeOpResult::SameAndNotChange;
            }
            
        }
        Value.Set<T>(InValue);
        return ENBTAttributeOpResult::Success;
    }

    template <typename T>
    ENBTAttributeOpResult OverriderToBaseTypeRef(const T& InValue) {
        if (T* Ptr = Value.TryGet<T>(); Ptr) {
            if (InValue == *Ptr) {
                return ENBTAttributeOpResult::SameAndNotChange;
            }
        }
        Value.Set<T>(InValue);
        return ENBTAttributeOpResult::Success;
    }

    template <typename T>
    ENBTAttributeOpResult OverriderToArrayType(const TArray<T>& InValue) {
        if (TArray<T>* Ptr = Value.TryGet<TArray<T>>(); Ptr) {
            if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                if (HelperCompareFloatArray(*Ptr, InValue)) return ENBTAttributeOpResult::SameAndNotChange;
            }
            
            if (*Ptr == InValue) return ENBTAttributeOpResult::SameAndNotChange;
        }
        Value.Set<TArray<T>>(InValue);
        return ENBTAttributeOpResult::Success;
    }

    const FNBTMapData* GetMapData() const {
        return Value.TryGet<FNBTMapData>();
    }

    FNBTMapData* GetMapData() {
        return Value.TryGet<FNBTMapData>();
    }

    const FNBTListData* GetListData() const {
        return Value.TryGet<FNBTListData>();
    }

    FNBTListData* GetListData() {
        return Value.TryGet<FNBTListData>();
    }

    ENBTAttributeOpResult OverrideToEmptyMap() {
        if (FNBTMapData* Ptr = Value.TryGet<FNBTMapData>(); Ptr) {
            if (Ptr->Children.Num() <= 0) return ENBTAttributeOpResult::SameAndNotChange;
        }
        Value.Set<FNBTMapData>(FNBTMapData{});
        return ENBTAttributeOpResult::Success;
    }

    ENBTAttributeOpResult OverrideToEmptyList() {
        if (FNBTListData* Ptr = Value.TryGet<FNBTListData>(); Ptr) {
            if (Ptr->Children.IsEmpty()) return ENBTAttributeOpResult::SameAndNotChange;
            else Ptr->Children.Reset();
        }
        Value.Set<FNBTListData>(FNBTListData{});
        return ENBTAttributeOpResult::Success;
    }

    TOptional<int64> GetGenericInt() const {
        switch (GetType()) {
            case ENBTAttributeType::Boolean:
                return Value.Get<bool>() ? 1 : 0;
            case ENBTAttributeType::Int8:
                return Value.Get<int8>();
            case ENBTAttributeType::Int16:
                return Value.Get<int16>();
            case ENBTAttributeType::Int32:
                return Value.Get<int32>();
            case ENBTAttributeType::Int64:
                return Value.Get<int64>();
            default:
                return {};
        }
    }

    TOptional<double> GetGenericDouble() const {
        switch (GetType()) {
            case ENBTAttributeType::Float:
                return Value.Get<float>();
            case ENBTAttributeType::Double:
                return Value.Get<double>();
            default:
                return {};
        }
    }

    ENBTAttributeOpResult TrySetGenericInt(int64 InValue) {
        const auto Idx = GetGenericInt();

        if (!Idx.IsSet()) return ENBTAttributeOpResult::NodeTypeMismatch;
        if (Idx.GetValue() == InValue) return ENBTAttributeOpResult::SameAndNotChange;

        switch (GetType()) {
            case ENBTAttributeType::Boolean:
                if (Value.Get<bool>() == (InValue != 0)) return ENBTAttributeOpResult::SameAndNotChange;
                Value.Set<bool>((InValue != 0));
                return ENBTAttributeOpResult::Success;
            case ENBTAttributeType::Int8:
                Value.Set<int8>(FMath::Clamp(InValue,
                                             static_cast<int64>(std::numeric_limits<int8>::min()), static_cast<int64>(std::numeric_limits<int8>::max())));
                return ENBTAttributeOpResult::Success;
            case ENBTAttributeType::Int16:
                Value.Set<int16>(FMath::Clamp(InValue,
                                              static_cast<int64>(std::numeric_limits<int16>::min()), static_cast<int64>(std::numeric_limits<int16>::max())));
                return ENBTAttributeOpResult::Success;
            case ENBTAttributeType::Int32:
                Value.Set<int32>(FMath::Clamp(InValue,
                                              static_cast<int64>(std::numeric_limits<int32>::min()), static_cast<int64>(std::numeric_limits<int32>::max())));
                return ENBTAttributeOpResult::Success;
            case ENBTAttributeType::Int64:
                Value.Set<int64>(InValue);
                return ENBTAttributeOpResult::Success;
            default:
                return ENBTAttributeOpResult::NodeTypeMismatch;
        }
    }

    ENBTAttributeOpResult TrySetGenericDouble(double InValue) {
        const auto Idx = GetGenericDouble();
        if (!Idx) return ENBTAttributeOpResult::NodeTypeMismatch;
        if (FMath::IsNearlyEqual(Idx.GetValue(), InValue, 0.0001f)) return ENBTAttributeOpResult::SameAndNotChange;
        switch (GetType()) {
            case ENBTAttributeType::Float:
                Value.Set<float>(static_cast<float>(InValue));
                return ENBTAttributeOpResult::Success;
            case ENBTAttributeType::Double:
                Value.Set<double>(InValue);
                return ENBTAttributeOpResult::Success;
            default:
                return ENBTAttributeOpResult::NodeTypeMismatch;
        }
    }

    ENBTAttributeOpResult OverrideFromIfNotCompound(const FNBTAttribute& Other) {
        if (IsCompoundType()) return ENBTAttributeOpResult::NodeTypeMismatch; //如果是复合结构, 无法重载
        if (Other.IsCompoundType()) return ENBTAttributeOpResult::NodeTypeMismatch; //如果是复合结构, 无法重载

        if (EqualsValues(Other))
            return ENBTAttributeOpResult::SameAndNotChange;

        Value = Other.Value; //基础值类型
        return ENBTAttributeOpResult::Success;
    }

    FString ToString() const;

    bool EqualsValues(const FNBTAttribute& Other) const;

    void SerializeNBTData(FArchive& Ar, bool NetWorkMode);

    template <typename T>
    static bool HelperCompareFloatArray(const TArray<T>& A, const TArray<T>& B) {
        if (A.Num() != B.Num()) return false;
        for (int32 i = 0; i < A.Num(); ++i) {
            if (!FMath::IsNearlyEqual(A[i], B[i], 0.0001f)) return false;
        }
        return true;
    }
};