#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NBTCommon.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(NBTSystem, Log, All);
UENUM(Blueprintable, BlueprintType)
enum class ENBTAttributeType : uint8 {
    Empty = 0,
    Boolean = 1,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Name,
    String,
    Color,
    Guid,
    SoftClassPath,
    SoftObjectPath,
    DateTime,
    Rotator,
    Vector2D,
    Vector,
    IntVector2,
    IntVector,
    Int64Vector2,
    Int64Vector,
    ArrayInt8,
    ArrayInt16,
    ArrayInt32,
    ArrayInt64,
    ArrayFloat32,
    ArrayDouble,
    Map,
    List,
};

UENUM(Blueprintable, BlueprintType)
enum class ENBTAttributeOpResult : uint8 {
    Success,
    SameAndNotChange,
    NotFoundNode,
    NodeTypeMismatch,
    PermissionDenied,
    InvalidID,
    InvalidContainer,
    NotFoundSubNode,
    AllocateFailed
};

USTRUCT(BlueprintType)
struct FNBTAttributeOpResultDetail {
    GENERATED_BODY()

    UPROPERTY()
    ENBTAttributeOpResult Result = ENBTAttributeOpResult::Success;

    UPROPERTY()
    FString ResultMessage {};
    
    FNBTAttributeOpResultDetail() = default;

    FNBTAttributeOpResultDetail(ENBTAttributeOpResult OpResult) : Result(OpResult) {};

    FNBTAttributeOpResultDetail(ENBTAttributeOpResult OpResult, const FString& Message) : Result(OpResult), ResultMessage(Message) {};

    bool operator==(ENBTAttributeOpResult Res) { return Result == Res;}
    
    bool operator!=(ENBTAttributeOpResult Res) { return Result != Res;}

    void ed() const {
        if (Result != ENBTAttributeOpResult::Success && Result != ENBTAttributeOpResult::SameAndNotChange) {
            UE_LOG(NBTSystem, Error, TEXT("NBT Op Failed: [%s] : %s"), *GetResultString(), *ResultMessage);
        }
    }

    // 带上下文的诊断
    void ed(const FString& Context) const {
        if (Result != ENBTAttributeOpResult::Success && Result != ENBTAttributeOpResult::SameAndNotChange) {
            UE_LOG(NBTSystem, Error, TEXT("[%s] NBT Op Failed: [%s] : %s"), *Context, *GetResultString(), *ResultMessage);
        }
    }

    // 详细诊断（包含堆栈追踪）
    void edv() const { // Error Diagnose Verbose
        if (Result != ENBTAttributeOpResult::Success && Result != ENBTAttributeOpResult::SameAndNotChange) {
            UE_LOG(NBTSystem, Error, TEXT("\n==== NBT Operation Failed: ===="));
            UE_LOG(NBTSystem, Error, TEXT("==== Result: [%s]"), *GetResultString());
            UE_LOG(NBTSystem, Error, TEXT("==== Message: %s"), *ResultMessage);
            UE_LOG(NBTSystem, Error, TEXT("==== Stack Trace:"));
            // 打印调用堆栈
            TArray<FProgramCounterSymbolInfo> Stack = FPlatformStackWalk::GetStack(1, 5);
            for (const auto& Frame : Stack) {
                UE_LOGFMT(NBTSystem, Error, "==== {0} [{1}:{2}]",
                          Frame.FunctionName, Frame.Filename, Frame.LineNumber);
            }
        }
    }

    // 详细诊断（包含堆栈追踪）
    void edvas() const;

    // 获取结果描述字符串
    FString GetResultString() const {
        switch (Result) {
            case ENBTAttributeOpResult::Success:
                return TEXT("Success");
            case ENBTAttributeOpResult::SameAndNotChange:
                return TEXT("Same value, no change needed");
            case ENBTAttributeOpResult::NotFoundNode:
                return TEXT("Node not found in path");
            case ENBTAttributeOpResult::NodeTypeMismatch:
                return TEXT("Node type mismatch");
            case ENBTAttributeOpResult::PermissionDenied:
                return TEXT("Permission Denied");
            case ENBTAttributeOpResult::InvalidID:
                return TEXT("Invalid node ID");
            case ENBTAttributeOpResult::InvalidContainer:
                return TEXT("Invalid or destroyed container");
            case ENBTAttributeOpResult::NotFoundSubNode:
                return TEXT("Sub-node not found");
            case ENBTAttributeOpResult::AllocateFailed:
                return TEXT("Failed to allocate new node");
            default:
                return FString::Printf(TEXT("Unknown error (%d)"), (int32)Result);
        }
    }

    // 便捷的成功检查
    bool IsSuccess() const {
        return Result == ENBTAttributeOpResult::Success;
    }

    bool IsOk() const {
        return Result == ENBTAttributeOpResult::Success ||
            Result == ENBTAttributeOpResult::SameAndNotChange;
    }

    // bool转换操作符
    operator bool() const {
        return IsOk();
    }
};

UENUM(Blueprintable, BlueprintType)
enum class ENBTSearchCondition : uint8 {
    IfEmpty,
    IfEmptyList,
    IfEmptyMap
};

UENUM(BlueprintType)
enum class ENBTCompareOp : uint8 {
    Eq,
    Ne,
    Gt,
    Ge,
    Lt,
    Le,
    Contains,
    StartsWith,
    EndsWith
};

USTRUCT(BlueprintType)
struct FNBTSearchParameter {
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    ENBTCompareOp Op = ENBTCompareOp::Eq;

    UPROPERTY(BlueprintReadWrite)
    ENBTAttributeType ValueType = ENBTAttributeType::String;

    UPROPERTY(BlueprintReadWrite)
    bool IgnoreCase = false;

    UPROPERTY(BlueprintReadWrite)
    bool EnableGenericSearch = false;
    
    // 作用对象：Map 的子节点
    // 子节点可能是基础类型，或是 Map；若是 Map，则在这个 Map 内继续找 SubKey
    UPROPERTY(BlueprintReadWrite)
    FName    Key = NAME_None;      // 可选：先按 Key 选子节点（若为空则遍历所有子节点）
    
    UPROPERTY(BlueprintReadWrite)
    FName    SubKey = NAME_None;;   // 可选：子节点为 Map 时，在其内部用这个键匹配（如 "name"）
    
    UPROPERTY(BlueprintReadWrite)
    FString  Value {};
};