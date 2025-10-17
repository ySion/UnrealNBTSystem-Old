#pragma once

#include "NBTAttribute.h"
#include "NBTAttributeID.h"
#include "NBTContainer.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "NBTAccessor.generated.h"

struct FNBTContainer;
struct FNBTDataAccessor;

enum class ENBTPathResolveMode : uint8 {
	ReadOnly, // 只读访问
	EnsureCreate, // 确保路径存在（必要时创建）
	ForceOverride // 强制覆盖（可改变类型）
};

USTRUCT(BlueprintType)
struct FNBTDataAccessor {
	GENERATED_BODY()

private:
	FNBTContainer* Container = nullptr;
	TWeakPtr<uint8> ContainerLiveToken = nullptr;
	TArray<TVariant<FName, int32>> Path{};
	mutable FNBTAttributeID CachedAttributeID = FNBTAttributeID();
	mutable int32 CachedContainerStructVersion = -1;
    
	mutable FNBTAttribute* CachedAttributePtr = nullptr;
	mutable int32* CachedAttributeVersionPtr = nullptr;
    mutable int32* CachedSubtreeVersionPtr = nullptr;
    
	mutable int32 LastObservedNodeVersion = -1;
	mutable int32 LastObservedContainerDataVersion = -1;
    mutable int32 LastObservedSubtreeVersion = -1;
    mutable FNBTAttributeID LastObservedAttributeID = FNBTAttributeID();
private:
	FNBTAttributeOpResultDetail ResolvePathInternal(ENBTPathResolveMode Mode) const;

	FString GetPathString(int ToIndex) const;

	void UpdateContainerDataAndStructVersion() const;

    void UpdateContainerDataVersion() const;

    void BubbleSubtreeVersionAlongPath() const;

    bool EqualNodeDeep(const FNBTContainer* ACont, FNBTAttributeID AID,
                                  const FNBTContainer* BCont, FNBTAttributeID BID) const ;

	friend struct FNBTContainer;

    void ResetAll() {
        Container = nullptr;
        ContainerLiveToken = nullptr;
        Path.Reset();
        CachedAttributeID = FNBTAttributeID();
        CachedContainerStructVersion = -1;
    
        CachedAttributePtr = nullptr;
        CachedAttributeVersionPtr = nullptr;
        CachedSubtreeVersionPtr = nullptr;
    
        LastObservedNodeVersion = -1;
        LastObservedContainerDataVersion = -1;
        LastObservedSubtreeVersion = -1;
        LastObservedAttributeID = FNBTAttributeID();
    }

public:
	FNBTDataAccessor() = default;
	~FNBTDataAccessor() = default;

	FNBTDataAccessor(FNBTContainer* InContainer, TWeakPtr<uint8> InLiveToken) : Container(InContainer), ContainerLiveToken(InLiveToken) {}

	// 拷贝构造
	FNBTDataAccessor(const FNBTDataAccessor& Other);
	FNBTDataAccessor& operator=(const FNBTDataAccessor& Other);

	// 移动构造
	FNBTDataAccessor(FNBTDataAccessor&& Other);
	FNBTDataAccessor& operator=(FNBTDataAccessor&& Other);

	FNBTDataAccessor MakeAccessFromFName(FName Key) const;
	FNBTDataAccessor MakeAccessFromFString(const FString& Key) const { return MakeAccessFromFName(FName(Key)); }
	FNBTDataAccessor MakeAccessFromIntIndex(int32 Index) const;

	FNBTDataAccessor operator[](FName Key) const { return MakeAccessFromFName(Key); }
	FNBTDataAccessor operator[](int32 Index) const { return MakeAccessFromIntIndex(Index); }
	FNBTDataAccessor operator[](const FString& Key) const { return MakeAccessFromFName(FName(Key)); }
	FNBTDataAccessor operator[](const char* Key) const { return MakeAccessFromFName(FName(Key)); }

	FNBTDataAccessor go(FName Index) const { return MakeAccessFromFName(Index); }
	FNBTDataAccessor go(int32 Index) const { return MakeAccessFromIntIndex(Index); }
	FNBTDataAccessor go(const FString& Key) const { return MakeAccessFromFString(Key); }

	FNBTDataAccessor clone() const { return *this; }

	FNBTAttributeOpResultDetail TryResolvePathReadOnly() const { return ResolvePathInternal(ENBTPathResolveMode::ReadOnly); }
	TOptional<ENBTAttributeType> GetType() const;
	FString GetTypeString() const;

    bool IsSubtreeChanged() const;
    
    bool IsSubtreeChangedAndMark() const;

    void MarkSubtree() const;

    bool IsDataChanged() const;

    bool IsDataChangedAndMark() const;

    void Mark() const;

    FNBTDataAccessor GetParent() const;
    FNBTDataAccessor GetParentPreview() const;
    
    static bool IsAncestor(const FNBTDataAccessor& P, const FNBTDataAccessor& C);
    bool IsParent(const FNBTDataAccessor& OtherNode) const;
    bool IsChild(const FNBTDataAccessor& OtherNode) const;

    bool IsEqual(const FNBTDataAccessor& Other) const;

	bool IsAccessorValid() const { return IsContainerValid(); }
	bool IsContainerValid() const { return Container != nullptr && ContainerLiveToken.IsValid(); }
	bool IsDataExists() const;
	bool IsEmpty() const;
	bool IsMap() const;
	bool IsEmptyMap() const;
    bool IsFilledMap() const;
	bool IsList() const;
	bool IsEmptyList() const;
    bool IsFilledList() const;
	bool IsArray() const;
	bool IsBaseType() const;

	// ========== 安全的Get操作 ==========

	template <typename T>
	TOptional<T> TryGet() const;
	template <typename T>
	const TArray<T>* TryGetArray() const;

	// 通用Get操作(自动类型转换)
	TOptional<int64> TryGetGenericInt() const;
	TOptional<double> TryGetGenericDouble() const;

	// Get函数特化
	TOptional<bool> TryGetBool() const;
    TOptional<int8> TryGetInt8() const;
    TOptional<int16> TryGetInt16() const;
    TOptional<int32> TryGetInt32() const;
    TOptional<int64> TryGetInt64() const;
    TOptional<float> TryGetFloat() const;
    TOptional<double> TryGetDouble() const;

    TOptional<FName> TryGetName() const;
    TOptional<FString> TryGetString() const;

    TOptional<FColor> TryGetColor() const;
    TOptional<FGuid> TryGetGuid() const;
    TOptional<FSoftClassPath> TryGetSoftClassPath() const;
    TOptional<FSoftObjectPath> TryGetSoftObjectPath() const;
    TOptional<FDateTime> TryGetDateTime() const;

    TOptional<FRotator> TryGetRotator() const;
    TOptional<FVector2D> TryGetVector2D() const;
    TOptional<FVector> TryGetVector() const;

    TOptional<FIntVector2> TryGetIntVector2() const;
    TOptional<FIntVector> TryGetIntVector() const;
    TOptional<FInt64Vector2> TryGetInt64Vector2() const;
    TOptional<FInt64Vector> TryGetInt64Vector() const;

    TArray<int8> TryGetInt8Array() const;
	TArray<int16> TryGetInt16Array() const;
	TArray<int32> TryGetInt32Array() const;
	TArray<int64> TryGetInt64Array() const;
	TArray<float> TryGetFloatArray() const;
	TArray<double> TryGetDoubleArray() const;

	// ========== Set操作（三种模式） ==========

	// 1. TrySet (最安全，默认推荐)
	// 实现: ResolvePath(...,
	// ENBTPathResolveMode=ReadOnly)。如果返回nullptr（路径不存在），则返回NotFind。如果找到了，调用TargetData->SetBaseType<T>(Value)（这个函数内部会检查类型并返回Success或NotMatchType）。
	// 用途: 普通的游戏逻辑更新，例如玩家血量减少。Stats.Health.TrySet(90)。
	// 对MOD作者: 这是你们99%的情况下应该使用的函数。它绝对不会破坏数据结构。
	//
	// 2. EnsureAndSet (用于初始化)
	// 实现: ResolvePath(...,
	// ENBTPathResolveMode=EnsureCreate)。然后检查找到的TargetData的类型。如果类型是Empty（即新创建的）或与T匹配，则调用TargetData->OverriderToBaseType<T>(Value)。如果类型存在但不匹配，则返回NotMatchType。
	// 用途: MOD在加载时，需要确保自己的数据存在。例如，一个“魔法MOD”在玩家身上添加“Mana”属性。Stats.EnsureAndSet<int32>("Mana",
	// 100)。这个操作是安全的，因为它不会覆盖其他MOD的同名但不同类型的属性。 对MOD作者: 当你需要添加你自己的新属性时，使用这个函数。
	//
	// 3. Overrider() (危险，需要明确意图)
	// 实现: ResolvePath(..., ENBTPathResolveMode=ForceOverride)，然后无条件地调用TargetData->OverriderToBaseType<T>(Value)。
	// 用途:
	// 游戏核心系统初始化整个NBT结构。
	// 数据修复工具。
	// 一个大型MOD，它作为另一个MOD的前置，需要修改其数据结构。
	// 对MOD作者: 这是一个危险的函数！ 只有在完全理解后果的情况下才能使用它。
    
	// 1. TrySet - 最安全，只在路径存在且类型匹配时设置
	template <typename T>
	FNBTAttributeOpResultDetail TrySetBaseType(T Value) const;
	template <typename T>
	FNBTAttributeOpResultDetail TrySetBaseTypeRef(const T& Value) const;
	template <typename T>
	FNBTAttributeOpResultDetail TrySetArray(const TArray<T>& Value) const;

	// 2. EnsureAndSet - 用于初始化，创建路径但不覆盖已存在的不同类型
	template <typename T>
	FNBTAttributeOpResultDetail EnsureAndSetBaseType(T Value) const;
	template <typename T>
	FNBTAttributeOpResultDetail EnsureAndSetBaseTypeRef(const T& Value) const;
	template <typename T>
	FNBTAttributeOpResultDetail EnsureAndSetArray(const TArray<T>& Value) const;

	// 3. Override - 强制覆盖，危险操作
	template <typename T>
	FNBTAttributeOpResultDetail OverrideToBaseType(T Value) const;
	template <class T>
	FNBTAttributeOpResultDetail OverrideToBaseTypeRef(const T& Value) const;
	template <typename T>
	FNBTAttributeOpResultDetail OverrideToArray(const TArray<T>& Value) const;

	// 4. 通用Set操作
	FNBTAttributeOpResultDetail TrySetGenericInt(int64 Value) const;
	FNBTAttributeOpResultDetail TrySetGenericDouble(double Value) const;

	FNBTDataAccessor EnsureList() const; // 确保当前节点是列表结构, 如果是空节点就转换为列表结构
	FNBTDataAccessor EnsureMap() const;

	// 特化TrySet
	FNBTAttributeOpResultDetail TrySetEmpty() const;
	FNBTAttributeOpResultDetail TrySetBool(bool Value) const;
    FNBTAttributeOpResultDetail TrySetInt8(int8 Value) const;
    FNBTAttributeOpResultDetail TrySetInt16(int16 Value) const;
    FNBTAttributeOpResultDetail TrySetInt32(int32 Value) const;
    FNBTAttributeOpResultDetail TrySetInt64(int64 Value) const;
    FNBTAttributeOpResultDetail TrySetFloat(float Value) const;
    FNBTAttributeOpResultDetail TrySetDouble(double Value) const;
    FNBTAttributeOpResultDetail TrySetName(FName Value) const;
    FNBTAttributeOpResultDetail TrySetString(const FString& Value) const;

    FNBTAttributeOpResultDetail TrySetColor(FColor Value) const;
    FNBTAttributeOpResultDetail TrySetGuid(FGuid Value) const;
    FNBTAttributeOpResultDetail TrySetSoftClassPath(const FSoftClassPath& Value) const;
    FNBTAttributeOpResultDetail TrySetSoftObjectPath(const FSoftObjectPath& Value) const;
    FNBTAttributeOpResultDetail TrySetDateTime(FDateTime Value) const;

    FNBTAttributeOpResultDetail TrySetRotator(FRotator Value) const;
    FNBTAttributeOpResultDetail TrySetVector2D(FVector2D Value) const;
    FNBTAttributeOpResultDetail TrySetVector(FVector Value) const;
    FNBTAttributeOpResultDetail TrySetIntVector2(FIntVector2 Value) const;
    FNBTAttributeOpResultDetail TrySetIntVector(FIntVector Value) const;
    FNBTAttributeOpResultDetail TrySetInt64Vector2(FInt64Vector2 Value) const;
    FNBTAttributeOpResultDetail TrySetInt64Vector(FInt64Vector Value) const;
    FNBTAttributeOpResultDetail TrySetInt8Array(const TArray<int8>& Value) const;
    FNBTAttributeOpResultDetail TrySetInt16Array(const TArray<int16>& Value) const;
    FNBTAttributeOpResultDetail TrySetInt32Array(const TArray<int32>& Value) const;
    FNBTAttributeOpResultDetail TrySetInt64Array(const TArray<int64>& Value) const;
    FNBTAttributeOpResultDetail TrySetFloatArray(const TArray<float>& Value) const;
    FNBTAttributeOpResultDetail TrySetDoubleArray(const TArray<double>& Value) const;


    // 特化EnsureAndSet
	FNBTAttributeOpResultDetail EnsureAndSetEmpty() const;
	FNBTAttributeOpResultDetail EnsureAndSetBool(bool Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt8(int8 Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt16(int16 Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt32(int32 Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt64(int64 Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetFloat(float Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetDouble(double Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetName(FName Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetString(const FString& Value) const;

    FNBTAttributeOpResultDetail EnsureAndSetColor(FColor Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetGuid(FGuid Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetSoftClassPath(const FSoftClassPath& Value) const;

    FNBTAttributeOpResultDetail EnsureAndSetSoftObjectPath(const FSoftObjectPath& Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetDateTime(FDateTime Value) const;

    FNBTAttributeOpResultDetail EnsureAndSetRotator(FRotator Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetVector2D(FVector2D Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetVector(FVector Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetIntVector2(FIntVector2 Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetIntVector(FIntVector Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt64Vector2(FInt64Vector2 Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt64Vector(FInt64Vector Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt8Array(const TArray<int8>& Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt16Array(const TArray<int16>& Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt32Array(const TArray<int32>& Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetInt64Array(const TArray<int64>& Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetFloatArray(const TArray<float>& Value) const;
    FNBTAttributeOpResultDetail EnsureAndSetDoubleArray(const TArray<double>& Value) const;

    // 特化OverriderTo
	FNBTAttributeOpResultDetail OverrideToBool(bool Value) const;
    FNBTAttributeOpResultDetail OverrideToInt8(int8 Value) const;
    FNBTAttributeOpResultDetail OverrideToInt16(int16 Value) const;
    FNBTAttributeOpResultDetail OverrideToInt32(int32 Value) const;
    FNBTAttributeOpResultDetail OverrideToInt64(int64 Value) const;
    FNBTAttributeOpResultDetail OverrideToFloat(float Value) const;
    FNBTAttributeOpResultDetail OverrideToDouble(double Value) const;
    FNBTAttributeOpResultDetail OverrideToName(FName Value) const;
    FNBTAttributeOpResultDetail OverrideToString(const FString& Value) const;

    FNBTAttributeOpResultDetail OverrideToColor(FColor Value) const;
    FNBTAttributeOpResultDetail OverrideToGuid(FGuid Value) const;
    FNBTAttributeOpResultDetail OverrideToSoftClassPath(const FSoftClassPath& Value) const;
    FNBTAttributeOpResultDetail OverrideToSoftObjectPath(const FSoftObjectPath& Value) const;
    FNBTAttributeOpResultDetail OverrideToDateTime(FDateTime Value) const;

    FNBTAttributeOpResultDetail OverrideToRotator(FRotator Value) const;
    FNBTAttributeOpResultDetail OverrideToVector2D(FVector2D Value) const;
    FNBTAttributeOpResultDetail OverrideToVector(FVector Value) const;
    FNBTAttributeOpResultDetail OverrideToIntVector2(FIntVector2 Value) const;
    FNBTAttributeOpResultDetail OverrideToIntVector(FIntVector Value) const;
    FNBTAttributeOpResultDetail OverrideToInt64Vector2(FInt64Vector2 Value) const;
    FNBTAttributeOpResultDetail OverrideToInt64Vector(FInt64Vector Value) const;
    FNBTAttributeOpResultDetail OverrideToInt8Array(const TArray<int8>& Value) const;
    FNBTAttributeOpResultDetail OverrideToInt16Array(const TArray<int16>& Value) const;
    FNBTAttributeOpResultDetail OverrideToInt32Array(const TArray<int32>& Value) const;
    FNBTAttributeOpResultDetail OverrideToInt64Array(const TArray<int64>& Value) const;
    FNBTAttributeOpResultDetail OverrideToFloatArray(const TArray<float>& Value) const;
    FNBTAttributeOpResultDetail OverrideToDoubleArray(const TArray<double>& Value) const;

    // ========== Map操作 ==========

	FNBTAttributeOpResultDetail MapHasKey(FName Key) const;
	FNBTAttributeOpResultDetail MapGetKeys(TArray<FName>& Keys) const;
	FNBTAttributeOpResultDetail MapGetSize(int32& Size) const;
	FNBTAttributeOpResultDetail MapRemoveSubNode(FName Key) const;
	FNBTAttributeOpResultDetail MapClear() const;
    
    FNBTDataAccessor MapMakeAccessorByCondition(ENBTSearchCondition Condition) const;
    FNBTAttributeOpResultDetail MapMakeAccessorsByCondition(TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition Condition) const;
    FNBTDataAccessor MapMakeAccessorByParameter(const FNBTSearchParameter& P) const;
    FNBTDataAccessor MapMakeAccessorIfEqual(const FNBTDataAccessor& Accessor) const;
    FNBTAttributeOpResultDetail MapMakeAccessorsIfEqual(TArray<FNBTDataAccessor>& Accessors, const FNBTDataAccessor& Accessor) const;
	FNBTAttributeOpResultDetail MakeAccessorFromMap(TArray<FNBTDataAccessor>& Accessors) const;
	TArray<FNBTDataAccessor> MakeAccessorFromMapNow() const;

	// ========== List操作 ==========

	FNBTAttributeOpResultDetail ListGetSize(int32& Size) const;
    TOptional<int32> ListGetCurrentIndex() const;
    TOptional<int32> ListGetLastParentIndex() const;
	FNBTAttributeOpResultDetail ListRemoveSubNode(int32 Index, bool bSwapRemove = false) const;
	FNBTAttributeOpResultDetail ListClear() const;
	FNBTDataAccessor ListAddSubNode() const;
	FNBTDataAccessor ListInsertSubNode(int32 Index) const;
    FNBTDataAccessor ListMakeAccessorByCondition(ENBTSearchCondition Condition) const;
    FNBTAttributeOpResultDetail ListMakeAccessorsByCondition(TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition Condition) const;
    FNBTDataAccessor ListMakeAccessorIfEqual(const FNBTDataAccessor& Accessor) const;
    FNBTAttributeOpResultDetail ListMakeAccessorsIfEqual(TArray<FNBTDataAccessor>& Accessors, const FNBTDataAccessor& Accessor) const;
    FNBTDataAccessor ListMakeAccessorByParameter(const FNBTSearchParameter& P) const;
    
	FNBTAttributeOpResultDetail MakeAccessorFromList(TArray<FNBTDataAccessor>& Accessors) const;
	TArray<FNBTDataAccessor> MakeAccessorFromListNow() const;
    
	// ========== 实用函数 ==========

	int Remove(); // 删除当前路径的节点

	FString ToString(bool bShowVersion = false) const;
	FString GetPath() const;
	FString GetPreviewPath() const;

	// 如果当前路径无效, 那么就不拷贝
    FNBTAttributeOpResultDetail TryCopyFrom(const FNBTDataAccessor& Source) const;
    
	// 如果当前路径无效, 那么就创建路径, 但是无效路径中存在List是不会工作的, 并且拷贝
	FNBTAttributeOpResultDetail EnsureAndCopyFrom(const FNBTDataAccessor& Source) const;
    
    FNBTAttributeOpResultDetail TrySwap(const FNBTDataAccessor& Target) const;

	template <typename Func>
	void VisitData(Func DataVisitor) const;

    bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

    bool operator==(const FNBTDataAccessor& Other) const {
        if (Path.Num() != Other.Path.Num()) return false;
        for (int32 i = 0; i < Path.Num(); ++i) {
            const auto& A = Path[i];
            const auto& B = Other.Path[i];
            if (A.GetIndex() != B.GetIndex()) return false;
            if (A.GetIndex() == 0) {
                if (A.Get<FName>() != B.Get<FName>()) return false;
            } else { // int32
                if (A.Get<int32>() != B.Get<int32>()) return false;
            }
        }

        return ContainerLiveToken.Pin().Get() == Other.ContainerLiveToken.Pin().Get();
    }

private:

    ENBTAttributeOpResult RedirectNode(FNBTAttributeID OldID, FNBTAttributeID NewID) const;
    
	template <typename Func>
	void VisitDataImp(int32& Deep, FName AttrName, int idx, FNBTDataAccessor TargetAccessor, Func DataVisitor) const;

    void ToStringImp(FString& Str, int& Deep, bool bShowVersion) const;
	ENBTAttributeOpResult CopyImp(const FNBTDataAccessor& Source) const;
};

template <>
struct TStructOpsTypeTraits<FNBTDataAccessor> : public TStructOpsTypeTraitsBase2<FNBTDataAccessor> {
    enum {
        WithNetSerializer = true,
        WithIdenticalViaEquality = true,
    };
};

DECLARE_DYNAMIC_DELEGATE_FiveParams(FArzNBTDataVisitorSignature, int, Deep, ENBTAttributeType, AttrType, FName, AttrName, int32, AttrIdx,
									FNBTDataAccessor, accessor);

UCLASS(Meta = (ScriptMixin = "FNBTDataAccessor"))
class UFArzNBTDataAccessorScriptMixinLibrary : public UObject {
	GENERATED_BODY()
public:
	// Need Visitor: int Deep, ENBTAttributeType AttrType, FName AttrName, int32 AttrIdx, FNBTDataAccessor accessor
	UFUNCTION(ScriptCallable)
	static void VisitData(const FNBTDataAccessor& TargetAccessor, UObject* Object, FName FunctionName);
};

