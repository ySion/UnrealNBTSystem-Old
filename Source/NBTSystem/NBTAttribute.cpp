#include "NBTAttribute.h"
#include "Misc/TVariant.h"
#include "NBTHelper.h"

using namespace ArzNBT;

template <typename T>
constexpr bool is_strict_integer_v =
    std::is_same_v<T, int8_t>  || std::is_same_v<T, uint8_t>  ||
    std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t> ||
    std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>;


bool FNBTAttribute::EqualsValues(const FNBTAttribute& Other) const {
    // 首先比较类型，如果类型不同，值肯定不相等
    const ENBTAttributeType MyType = GetType();
    if (MyType != Other.GetType()) {
        return false;
    }

    // 类型相同，根据具体类型比较值
    switch (MyType) {
        case ENBTAttributeType::Empty:
            return true;

        case ENBTAttributeType::Boolean:
            return Value.Get<bool>() == Other.Value.Get<bool>();
        case ENBTAttributeType::Int8:
            return Value.Get<int8>() == Other.Value.Get<int8>();
        case ENBTAttributeType::Int16:
            return Value.Get<int16>() == Other.Value.Get<int16>();
        case ENBTAttributeType::Int32:
            return Value.Get<int32>() == Other.Value.Get<int32>();
        case ENBTAttributeType::Int64:
            return Value.Get<int64>() == Other.Value.Get<int64>();

        case ENBTAttributeType::Float:
            return FMath::IsNearlyEqual(Value.Get<float>(), Other.Value.Get<float>(), 0.0001f);
        case ENBTAttributeType::Double:
            return FMath::IsNearlyEqual(Value.Get<double>(), Other.Value.Get<double>(), 0.0001f);

        case ENBTAttributeType::Name:
            return Value.Get<FName>() == Other.Value.Get<FName>();
        case ENBTAttributeType::String:
            return Value.Get<FString>() == Other.Value.Get<FString>();

        case ENBTAttributeType::Color:
            return Value.Get<FColor>() == Other.Value.Get<FColor>();
        case ENBTAttributeType::Guid:
            return Value.Get<FGuid>() == Other.Value.Get<FGuid>();
        case ENBTAttributeType::SoftClassPath:
            return Value.Get<FSoftClassPath>() == Other.Value.Get<FSoftClassPath>();
        case ENBTAttributeType::SoftObjectPath:
            return Value.Get<FSoftObjectPath>() == Other.Value.Get<FSoftObjectPath>();
        case ENBTAttributeType::DateTime:
            return Value.Get<FDateTime>() == Other.Value.Get<FDateTime>();

        case ENBTAttributeType::Rotator:
            return Value.Get<FRotator>().Equals(Other.Value.Get<FRotator>());
        case ENBTAttributeType::Vector2D:
            return Value.Get<FVector2D>().Equals(Other.Value.Get<FVector2D>());
        case ENBTAttributeType::Vector:
            return Value.Get<FVector>().Equals(Other.Value.Get<FVector>());

        case ENBTAttributeType::IntVector2:
            return Value.Get<FIntVector2>() == Other.Value.Get<FIntVector2>();
        case ENBTAttributeType::IntVector:
            return Value.Get<FIntVector>() == Other.Value.Get<FIntVector>();

        case ENBTAttributeType::Int64Vector2:
            return Value.Get<FInt64Vector2>() == Other.Value.Get<FInt64Vector2>();
        case ENBTAttributeType::Int64Vector:
            return Value.Get<FInt64Vector>() == Other.Value.Get<FInt64Vector>();

        case ENBTAttributeType::ArrayInt8:
            return Value.Get<TArray<int8>>() == Other.Value.Get<TArray<int8>>();
        case ENBTAttributeType::ArrayInt16:
            return Value.Get<TArray<int16>>() == Other.Value.Get<TArray<int16>>();
        case ENBTAttributeType::ArrayInt32:
            return Value.Get<TArray<int32>>() == Other.Value.Get<TArray<int32>>();
        case ENBTAttributeType::ArrayInt64:
            return Value.Get<TArray<int64>>() == Other.Value.Get<TArray<int64>>();

        case ENBTAttributeType::ArrayFloat32: {
            const auto& ArrayA = Value.Get<TArray<float>>();
            const auto& ArrayB = Other.Value.Get<TArray<float>>();
            return HelperCompareFloatArray<float>(ArrayA, ArrayB);
        }
        case ENBTAttributeType::ArrayDouble: {
            const auto& ArrayA = Value.Get<TArray<double>>();
            const auto& ArrayB = Other.Value.Get<TArray<double>>();
            return HelperCompareFloatArray<double>(ArrayA, ArrayB);
        }
        case ENBTAttributeType::Map:
            return false;
        case ENBTAttributeType::List:
            return false;
        default:
            check(false); //不应该到达这个位置
            return false;
    }
}

FString FNBTAttribute::ToString() const {
    switch (GetType()) {
        case ENBTAttributeType::Empty:
            return "$Empty$";
        case ENBTAttributeType::Boolean:
            return Value.Get<bool>() ? "True" : "False";
        case ENBTAttributeType::Int8:
            return FString::FromInt(Value.Get<int8>()) + " (Int8)";
        case ENBTAttributeType::Int16:
            return FString::FromInt(Value.Get<int16>()) + " (Int16)";
        case ENBTAttributeType::Int32:
            return FString::FromInt(Value.Get<int32>()) + " (Int32)";
        case ENBTAttributeType::Int64:
            return FString::Printf(TEXT("%lld (Int64)"), Value.Get<int64>());
        case ENBTAttributeType::Float:
            return FString::SanitizeFloat(Value.Get<float>()) + " (Float)";
        case ENBTAttributeType::Double:
            return FString::SanitizeFloat(Value.Get<double>()) + " (Double)";
        case ENBTAttributeType::Name:
            return "\"" + Value.Get<FName>().ToString() + "\" (Name)";
        case ENBTAttributeType::String:
            return "\"" + Value.Get<FString>() + "\" (String)";

        case ENBTAttributeType::Color:
            return FString::Printf(TEXT("%s (Color)"), *Value.Get<FColor>().ToString());
        case ENBTAttributeType::Guid:
            return FString::Printf(TEXT("%s (Guid)"), *Value.Get<FGuid>().ToString());
        case ENBTAttributeType::SoftClassPath:
            return FString::Printf(TEXT("%s (SoftClassPath)"), *Value.Get<FSoftClassPath>().ToString());
        case ENBTAttributeType::SoftObjectPath:
            return FString::Printf(TEXT("%s (SoftObjectPath)"), *Value.Get<FSoftObjectPath>().ToString());
        case ENBTAttributeType::DateTime:
            return FString::Printf(TEXT("%s (DateTime)"), *Value.Get<FDateTime>().ToString());

        case ENBTAttributeType::Rotator:
            return "\"" + Value.Get<FRotator>().ToString() + "\" (Rotator)";
        case ENBTAttributeType::Vector2D:
            return Value.Get<FVector2D>().ToString() + " (Vector2D)";
        case ENBTAttributeType::Vector:
            return Value.Get<FVector>().ToString() + " (Vector)";
        case ENBTAttributeType::IntVector2:
            return Value.Get<FIntVector2>().ToString() + " (IntVector2)";
        case ENBTAttributeType::IntVector:
            return Value.Get<FIntVector>().ToString() + " (IntVector)";
        case ENBTAttributeType::Int64Vector2:
            return Value.Get<FInt64Vector2>().ToString() + " (Int64Vector2)";
        case ENBTAttributeType::Int64Vector:
            return Value.Get<FInt64Vector>().ToString() + " (Int64Vector)";
        case ENBTAttributeType::ArrayInt8:
            return FString::Printf(TEXT("[%s] (ArrayInt8)"), *FString::JoinBy(Value.Get<TArray<int8>>(), TEXT(", "),
                                                                             [](int8 InValue) {
                                                                                 return FString::FromInt(InValue);
                                                                             }));
        case ENBTAttributeType::ArrayInt16:
            return FString::Printf(TEXT("[%s] (ArrayInt16)"), *FString::JoinBy(Value.Get<TArray<int16>>(), TEXT(", "),
                                                                              [](int16 InValue) {
                                                                                  return FString::FromInt(InValue);
                                                                              }));
        case ENBTAttributeType::ArrayInt32:
            return FString::Printf(TEXT("[%s] (ArrayInt32)"), *FString::JoinBy(Value.Get<TArray<int32>>(), TEXT(", "),
                                                                              [](int32 InValue) {
                                                                                  return FString::FromInt(InValue);
                                                                              }));
        case ENBTAttributeType::ArrayInt64:
            return FString::Printf(TEXT("[%s] (ArrayInt64)"), *FString::JoinBy(Value.Get<TArray<int64>>(), TEXT(", "),
                                                                              [](int64 InValue) {
                                                                                  return FString::Printf(
                                                                                      TEXT("%lld"), InValue);
                                                                              }));
        case ENBTAttributeType::ArrayFloat32:
            return FString::Printf(TEXT("[%s] (ArrayFloat32)"), *FString::JoinBy(Value.Get<TArray<float>>(), TEXT(", "),
                                                                                [](float InValue) {
                                                                                    return FString::SanitizeFloat(InValue);
                                                                                }));
        case ENBTAttributeType::ArrayDouble:
            return FString::Printf(TEXT("[%s] (ArrayDouble)"), *FString::JoinBy(Value.Get<TArray<double>>(), TEXT(", "),
                                                                                 [](double InValue) {
                                                                                     return FString::SanitizeFloat(InValue);
                                                                                 }));
        case ENBTAttributeType::Map: // Map会有另外处理
            return "(Map)";
        case ENBTAttributeType::List: // List会有另外处理
            return "(List)";
        default:
            return "$Unknown$";
    }
}

void FNBTAttribute::SerializeNBTData(FArchive& Ar, bool NetWorkMode) {
    uint8 TypeIndex;
    if (Ar.IsSaving()) {
        TypeIndex = static_cast<uint8>(Value.GetIndex());
    }
    Ar << TypeIndex;

    if (Ar.IsLoading()) {
        // 根据从存档中读取的类型，设置 Variant 的当前活动类型
        // 使用枚举代替魔法数字，更安全易读
        switch (static_cast<ENBTAttributeType>(TypeIndex)) {
            case ENBTAttributeType::Empty: Value.Set<FEmptyVariantState>({});
                break;
            case ENBTAttributeType::Boolean: Value.Set<bool>({});
                break;
            case ENBTAttributeType::Int8: Value.Set<int8>({});
                break;
            case ENBTAttributeType::Int16: Value.Set<int16>({});
                break;
            case ENBTAttributeType::Int32: Value.Set<int32>({});
                break;
            case ENBTAttributeType::Int64: Value.Set<int64>({});
                break;
            case ENBTAttributeType::Float: Value.Set<float>({});
                break;
            case ENBTAttributeType::Double: Value.Set<double>({});
                break;
            case ENBTAttributeType::Name: Value.Set<FName>({});
                break;
            case ENBTAttributeType::String: Value.Set<FString>({});
                break;

            case ENBTAttributeType::Color:
                Value.Set<FColor>({});
                break;
            case ENBTAttributeType::Guid:
                Value.Set<FGuid>({});
                break;
            case ENBTAttributeType::SoftClassPath:
                Value.Set<FSoftClassPath>({});
                break;
            case ENBTAttributeType::SoftObjectPath:
                Value.Set<FSoftObjectPath>({});
                break;
            case ENBTAttributeType::DateTime:
                Value.Set<FDateTime>({});
                break;
            case ENBTAttributeType::Rotator: Value.Set<FRotator>({});
                break;
            case ENBTAttributeType::Vector2D: Value.Set<FVector2D>({});
                break;
            case ENBTAttributeType::Vector: Value.Set<FVector>({});
                break;
            case ENBTAttributeType::IntVector2: Value.Set<FIntVector2>({});
                break;
            case ENBTAttributeType::IntVector: Value.Set<FIntVector>({});
                break;
            case ENBTAttributeType::Int64Vector2: Value.Set<FInt64Vector2>({});
                break;
            case ENBTAttributeType::Int64Vector: Value.Set<FInt64Vector>({});
                break;
            case ENBTAttributeType::ArrayInt8: Value.Set<TArray<int8>>({});
                break;
            case ENBTAttributeType::ArrayInt16: Value.Set<TArray<int16>>({});
                break;
            case ENBTAttributeType::ArrayInt32: Value.Set<TArray<int32>>({});
                break;
            case ENBTAttributeType::ArrayInt64: Value.Set<TArray<int64>>({});
                break;
            case ENBTAttributeType::ArrayFloat32: Value.Set<TArray<float>>({});
                break;
            case ENBTAttributeType::ArrayDouble: Value.Set<TArray<double>>({});
                break;
            case ENBTAttributeType::Map: Value.Set<FNBTMapData>(FNBTMapData{});
                break;
            case ENBTAttributeType::List: Value.Set<FNBTListData>(FNBTListData{});
                break;
            default:
                Value.Set<FEmptyVariantState>({});
                UE_LOG(NBTSystem, Warning, TEXT("Unknown NBT attribute type index %d encountered during serialization."),
                       TypeIndex);
                break;
        }
    }
    
    Visit([&Ar, NetWorkMode]<typename T0>(T0& ActiveValue) {
        using T = std::decay_t<T0>;
        if constexpr (!std::is_same_v<T, FEmptyVariantState>) {
            if constexpr (std::is_same_v<T, FVector2D> || std::is_same_v<T, FVector> || std::is_same_v<T, FRotator>) {
                bool bSuccess = true;
                ActiveValue.NetSerialize(Ar, nullptr, bSuccess);
            } else if constexpr (std::is_same_v<T, FNBTMapData> || std::is_same_v<T, FNBTListData>) {
                ActiveValue.SerializeNBTData(Ar, NetWorkMode);
            } else if constexpr (is_strict_integer_v<T>) {
                SerializeZigZag(Ar, ActiveValue);
            }else {
                Ar << ActiveValue;
            }
        }
    }, Value);
}

void FNBTMapData::SerializeNBTData(FArchive& Ar, bool NetWorkMode) {
    uint16 NumAttributes = Children.Num();
    Ar << NumAttributes;

    if (Ar.IsLoading()) {
        Children.Reset();
        Children.Reserve(NumAttributes);
        for (int16 i = 0; i < NumAttributes; ++i) {
            FName AttributeName;
            FNBTAttributeID AttributeID;
            Ar << AttributeName;
            Ar << AttributeID;
            Children.Emplace(AttributeName, AttributeID);
        }
    } else {
        for (auto& Pair : Children) {
            Ar << Pair.Key;
            Ar << Pair.Value;
        }
    }
}

FString FNBTAttribute::GetTypeString() const {
    switch (GetType()) {
        case ENBTAttributeType::Empty:
            return "$Empty$";
        case ENBTAttributeType::Boolean:
            return "Boolean";
        case ENBTAttributeType::Int8:
            return "Int8";
        case ENBTAttributeType::Int16:
            return "Int16";
        case ENBTAttributeType::Int32:
            return "Int32";
        case ENBTAttributeType::Int64:
            return "Int64";
        case ENBTAttributeType::Float:
            return "Float";
        case ENBTAttributeType::Double:
            return "Double";
        case ENBTAttributeType::Name:
            return "Name";
        case ENBTAttributeType::String:
            return "String";
        case ENBTAttributeType::Color:
            return "Color";
        case ENBTAttributeType::Guid:
            return "Guid";
        case ENBTAttributeType::DateTime:
            return "DateTime";
        case ENBTAttributeType::SoftClassPath:
            return "SoftClassPath";
        case ENBTAttributeType::SoftObjectPath:
            return "SoftObjectPath";
        case ENBTAttributeType::Rotator:
            return "Rotator";
        case ENBTAttributeType::Vector2D:
            return "Vector2D";
        case ENBTAttributeType::Vector:
            return "Vector";
        case ENBTAttributeType::IntVector2:
            return "IntVector2";
        case ENBTAttributeType::IntVector:
            return "IntVector";
        case ENBTAttributeType::Int64Vector2:
            return "Int64Vector2";
        case ENBTAttributeType::Int64Vector:
            return "Int64Vector";
        case ENBTAttributeType::ArrayInt8:
            return "ArrayInt8";
        case ENBTAttributeType::ArrayInt16:
            return "ArrayInt16";
        case ENBTAttributeType::ArrayInt32:
            return "ArrayInt32";
        case ENBTAttributeType::ArrayFloat32:
            return "ArrayFloat32";
        case ENBTAttributeType::ArrayDouble:
            return "ArrayDouble";
        case ENBTAttributeType::Map: // Map会有另外处理
            return "Map";
        case ENBTAttributeType::List: // List会有另外处理
            return "List";
        default:
            return "$Unknown$";
    }
}