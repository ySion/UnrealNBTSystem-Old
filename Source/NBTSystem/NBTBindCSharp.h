#pragma once

#include "CoreMinimal.h"
#include "AngelscriptManager.h"
#include "NBTBindAS.h"
#include "NBTAccessor.inl"
#include "NBTBindCSharp.generated.h"

UCLASS(Blueprintable, BlueprintType)
 class NBTSYSTEM_API UNBTSystemContainerCSharpBind : public UBlueprintFunctionLibrary {
     GENERATED_BODY()
 public:
     /**
      * 检查NBT容器是否启用了网络复制功能。
      * @param Target 要检查的NBT容器引用
      * @return 如果容器启用了网络复制则返回true，否则返回false
      * @note 启用网络复制的容器会自动同步数据变化到客户端
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsContainerReplicated(const FNBTContainer& Target) {
         return Target.IsContainerReplicated();
     }
     
     /**
      * 重置NBT容器，清除所有数据并释放内存。
      * 此操作会删除容器中的所有节点和数据，并重置版本信息。
      * @param Target 要重置的NBT容器引用
      * @warning 此操作不可逆！会永久删除容器中的所有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static void Reset(const FNBTContainer& Target) {
         const_cast<FNBTContainer*>(&Target)->Reset();
     }

     /**
      * 从另一个NBT容器复制所有数据到当前容器。
      * 会完全替换当前容器的内容，包括所有节点结构和数据。
      * @param Target 目标NBT容器引用（将被覆盖）
      * @param Other 源NBT容器引用（数据来源）
      * @warning 此操作会完全覆盖目标容器的现有数据！
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static void CopyFrom(const FNBTContainer& Target, const FNBTContainer& Other) {
         const_cast<FNBTContainer*>(&Target)->CopyFrom(Other);
     }

     /**
      * 获取容器的数据版本号。
      * 数据版本号在容器内容发生变化时会自动递增，用于网络同步和变化检测。
      * @param Target 要查询的NBT容器引用
      * @return 当前数据版本号（递增的整数值）
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static int32 GetContainerDataVersion(const FNBTContainer& Target) {
         return Target.GetContainerDataVersion();
     }

     /**
      * 获取容器的结构版本号。
      * 结构版本号在容器的节点结构（而非数据内容）发生变化时递增。
      * @param Target 要查询的NBT容器引用
      * @return 当前结构版本号（递增的整数值）
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static int32 GetContainerStructVersion(const FNBTContainer& Target) {
         return Target.GetContainerStructVersion();
     }

     /**
      * 获取容器中的节点总数。
      * 包括所有类型的节点：Map、List、Array和基础类型节点。
      * @param Target 要查询的NBT容器引用
      * @return 容器中的节点总数量
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static int32 GetNodeCount(const FNBTContainer& Target) {
         return Target.GetNodeCount();
     }

     /**
      * 获取容器的根数据访问器。
      * 数据访问器提供了类型安全的方式来读写NBT数据。
      * @param Target 要获取访问器的NBT容器引用
      * @return 指向容器根节点的数据访问器
      * @note 通过访问器可以导航到容器中的任何节点
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor GetAccessor(const FNBTContainer& Target) {
         return const_cast<FNBTContainer*>(&Target)->GetAccessor();
     }
     
     /**
      * 将容器内容转换为可读的字符串表示。
      * 生成简洁的字符串格式，适合日志输出和调试显示。
      * @param Target 要转换的NBT容器引用
      * @return 容器内容的字符串表示
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString ToString(const FNBTContainer& Target) {
         return Target.ToString();
     }

     /**
      * 将容器内容转换为详细的调试字符串。
      * 包含更多技术细节，如节点类型、内存地址等，主要用于开发调试。
      * @param Target 要转换的NBT容器引用
      * @return 容器内容的详细调试字符串
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString ToDebugString(const FNBTContainer& Target) {
         return Target.ToDebugString();
     }

     /**
      * 验证容器的数据完整性。
      * 检查容器内部结构是否一致，节点引用是否有效等。
      * @param Target 要验证的NBT容器引用
      * @return 如果容器数据完整性正常则返回true，发现问题则返回false
      * @note 建议在发现异常数据行为时调用此函数进行诊断
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool ValidateIntegrity(const FNBTContainer& Target) {
         return Target.ValidateIntegrity();
     }

     /**
      * 获取容器的详细统计信息。
      * 包括内存使用量、节点数量分布、性能指标等统计数据。
      * @param Target 要分析的NBT容器引用
      * @return 包含各项统计数据的结构体
      * @note 用于性能分析和内存使用监控
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FArzNBTContainerStats GetStatistics(const FNBTContainer& Target) {
         return Target.GetStatistics();
     }
 };

 UCLASS(Blueprintable, BlueprintType)
 class NBTSYSTEM_API UNBTSystemAccessorCSharpBind : public UBlueprintFunctionLibrary {
     GENERATED_BODY()
 public:
     
     /**
      * 使用FName作为键导航到子节点（Map节点）。
      * 从当前节点导航到指定名称的子节点，适用于Map类型节点。
      * @param Target 当前NBT数据访问器引用
      * @param Index 要导航到的子节点的FName键
      * @return 指向子节点的新访问器，如果键不存在则返回无效访问器
      * @note 仅对Map类型节点有效，List节点请使用goi()
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor gon(const FNBTDataAccessor& Target, FName Index) {
         return Target.go(Index);
     }

     /**
      * 使用整数索引导航到子节点（List或Array节点）。
      * 从当前节点导航到指定索引的子节点，适用于List和Array类型节点。
      * @param Target 当前NBT数据访问器引用
      * @param Index 要导航到的子节点索引（从0开始）
      * @return 指向子节点的新访问器，如果索引越界则返回无效访问器
      * @note 仅对List和Array类型节点有效，Map节点请使用gon()或gos()
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor goi(const FNBTDataAccessor& Target, int32 Index) {
         return Target.go(Index);
     }

     /**
      * 使用字符串作为键导航到子节点（Map节点）。
      * 从当前节点导航到指定字符串键的子节点，适用于Map类型节点。
      * @param Target 当前NBT数据访问器引用
      * @param Key 要导航到的子节点的字符串键
      * @return 指向子节点的新访问器，如果键不存在则返回无效访问器
      * @note 仅对Map类型节点有效，内部会将字符串转换为FName
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor gos(const FNBTDataAccessor& Target, const FString& Key) {
         return Target.go(Key);
     }

     /**
      * 克隆当前访问器。
      * 创建一个指向相同节点的新访问器实例，两个访问器独立操作。
      * @param Target 要克隆的NBT数据访问器引用
      * @return 新的访问器实例，指向相同的节点
      * @note 克隆的访问器具有独立的状态，不会互相影响
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor clone(const FNBTDataAccessor& Target) {
         return Target.clone();
     }

     /**
      * 获取当前节点的数据类型。
      * 返回节点存储的数据类型枚举值，如果节点无效则返回空。
      * @param Target 要查询的NBT数据访问器引用
      * @return 包含数据类型的Optional，节点无效时为空
      * @note 可用于类型检查和数据验证
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TOptional<ENBTAttributeType> GetType(const FNBTDataAccessor& Target) {
         return Target.GetType();
     }

     /**
      * 获取当前节点数据类型的字符串表示。
      * 返回人类可读的数据类型名称字符串。
      * @param Target 要查询的NBT数据访问器引用
      * @return 数据类型的字符串表示（如"Int32"、"String"、"Map"等）
      * @note 主要用于调试和日志输出
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString GetTypeString(const FNBTDataAccessor& Target) {
         return Target.GetTypeString();
     }

     /**
      * 检查访问器本身是否有效。
      * 验证访问器对象的内部状态是否正确，不检查目标节点。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果访问器有效则返回true，否则返回false
      * @note 访问器无效通常意味着初始化失败或已被销毁
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsAccessorValid(const FNBTDataAccessor& Target) {
         return Target.IsAccessorValid();
     }

     /**
      * 检查访问器关联的容器是否有效。
      * 验证访问器所属的NBT容器是否仍然存在且可用。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果关联容器有效则返回true，否则返回false
      * @warning 容器无效的访问器不应继续使用！
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsContainerValid(const FNBTDataAccessor& Target) {
         return Target.IsContainerValid();
     }

     /**
      * 检查当前指向的数据节点是否存在。
      * 验证访问器指向的路径上是否确实存在数据节点。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果数据节点存在则返回true，否则返回false
      * @note 即使访问器和容器都有效，数据节点也可能不存在
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsDataExists(const FNBTDataAccessor& Target) {
         return Target.IsDataExists();
     }

     /**
      * 检查当前节点是否为空（Empty类型）。
      * Empty类型节点不包含任何实际数据，用作占位符。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果节点为Empty类型则返回true，否则返回false
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsEmpty(const FNBTDataAccessor& Target) {
         return Target.IsEmpty();
     }

     /**
      * 检查当前节点是否为Map类型。
      * Map类型节点以键值对形式存储子节点，类似于字典结构。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果节点为Map类型则返回true，否则返回false
      * @note Map节点可通过gon()或gos()导航到子节点
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsMap(const FNBTDataAccessor& Target) {
         return Target.IsMap();
     }

     /**
      * 检查当前节点是否为List类型。
      * List类型节点以有序数组形式存储子节点，支持动态增删。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果节点为List类型则返回true，否则返回false
      * @note List节点可通过goi()按索引导航到子节点
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsList(const FNBTDataAccessor& Target) {
         return Target.IsList();
     }

     /**
      * 检查当前节点是否为Array类型。
      * Array类型节点存储相同类型元素的固定数组，如int32[]、float[]等。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果节点为Array类型则返回true，否则返回false
      * @note Array节点通过专门的GetXXXArray()函数读取数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsArray(const FNBTDataAccessor& Target) {
         return Target.IsArray();
     }

     /**
      * 检查当前节点是否为基础类型。
      * 基础类型包括所有原始数据类型：整数、浮点、布尔、字符串等。
      * @param Target 要检查的NBT数据访问器引用
      * @return 如果节点为基础类型则返回true，否则返回false
      * @note 基础类型节点直接存储值，不包含子节点
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsBaseType(const FNBTDataAccessor& Target) {
         return Target.IsBaseType();
     }

     /**
      * 获取通用整数值。
      * 尝试将当前节点数据转换为64位有符号整数，支持多种整数类型的自动转换。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含整数值的Optional，转换失败或数据不存在时为空
      * @note 支持int8、int16、int32、int64类型的自动转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int64> GetGenericInt(const FNBTDataAccessor& Target) {
         return Target.TryGetGenericInt();
     }

     /**
      * 获取通用浮点值。
      * 尝试将当前节点数据转换为双精度浮点数，支持多种数值类型的自动转换。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含浮点值的Optional，转换失败或数据不存在时为空
      * @note 支持float、double和整数类型的自动转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TOptional<double> GetGenericDouble(const FNBTDataAccessor& Target) {
         return Target.TryGetGenericDouble();
     }

     /**
      * 获取布尔值。
      * 读取当前节点存储的布尔类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含布尔值的Optional，类型不匹配或数据不存在时为空
      * @note 仅支持严格的布尔类型，不进行自动转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TOptional<bool> GetBool(const FNBTDataAccessor& Target) {
         return Target.TryGetBool();
     }

     /**
      * 获取8位有符号整数值。
      * 读取当前节点存储的int8类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int8值的Optional，类型不匹配或数据不存在时为空
      * @note 值范围：-128到127
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int8> GetInt8(const FNBTDataAccessor& Target) {
         return Target.TryGetInt8();
     }

     /**
      * 获取16位有符号整数值。
      * 读取当前节点存储的int16类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int16值的Optional，类型不匹配或数据不存在时为空
      * @note 值范围：-32,768到32,767
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int16> GetInt16(const FNBTDataAccessor& Target) {
         return Target.TryGetInt16();
     }

     /**
      * 获取32位有符号整数值。
      * 读取当前节点存储的int32类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int32值的Optional，类型不匹配或数据不存在时为空
      * @note 值范围：约-21亿到21亿
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int32> GetInt32(const FNBTDataAccessor& Target) {
         return Target.TryGetInt32();
     }

     /**
      * 获取64位有符号整数值。
      * 读取当前节点存储的int64类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int64值的Optional，类型不匹配或数据不存在时为空
      * @note 值范围：约-922万亿到922万亿
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int64> GetInt64(const FNBTDataAccessor& Target) {
         return Target.TryGetInt64();
     }

     /**
      * 获取单精度浮点值。
      * 读取当前节点存储的float类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含float值的Optional，类型不匹配或数据不存在时为空
      * @note 32位IEEE 754浮点数，精度约7位十进制数字
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<float> GetFloat(const FNBTDataAccessor& Target) {
         return Target.TryGetFloat();
     }

     /**
      * 获取双精度浮点值。
      * 读取当前节点存储的double类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含double值的Optional，类型不匹配或数据不存在时为空
      * @note 64位IEEE 754浮点数，精度约15-17位十进制数字
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<double> GetDouble(const FNBTDataAccessor& Target) {
         return Target.TryGetDouble();
     }

     /**
      * 获取FName值。
      * 读取当前节点存储的FName类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FName值的Optional，类型不匹配或数据不存在时为空
      * @note FName是UE中的优化字符串类型，常用于标识符
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FName> GetName(const FNBTDataAccessor& Target) {
         return Target.TryGetName();
     }

     /**
      * 获取旋转值。
      * 读取当前节点存储的FRotator类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FRotator值的Optional，类型不匹配或数据不存在时为空
      * @note FRotator表示3D空间中的旋转（Pitch、Yaw、Roll）
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FRotator> GetRotator(const FNBTDataAccessor& Target) {
         return Target.TryGetRotator();
     }

     /**
      * 获取字符串值。
      * 读取当前节点存储的FString类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FString值的Optional，类型不匹配或数据不存在时为空
      * @note 支持Unicode字符串
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FString> GetString(const FNBTDataAccessor& Target) {
         return Target.TryGetString();
     }

     /**
      * 获取2D向量值。
      * 读取当前节点存储的FVector2D类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FVector2D值的Optional，类型不匹配或数据不存在时为空
      * @note FVector2D包含X和Y两个浮点分量
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FVector2D> GetVector2D(const FNBTDataAccessor& Target) {
         return Target.TryGetVector2D();
     }

     /**
      * 获取3D向量值。
      * 读取当前节点存储的FVector类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FVector值的Optional，类型不匹配或数据不存在时为空
      * @note FVector包含X、Y、Z三个浮点分量
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FVector> GetVector3D(const FNBTDataAccessor& Target) {
         return Target.TryGetVector();
     }

     /**
      * 获取2D整数向量值。
      * 读取当前节点存储的FIntVector2类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FIntVector2值的Optional，类型不匹配或数据不存在时为空
      * @note FIntVector2包含X和Y两个int32分量
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FIntVector2> GetVector2I32(const FNBTDataAccessor& Target) {
         return Target.TryGetIntVector2();
     }

     /**
      * 获取3D整数向量值。
      * 读取当前节点存储的FIntVector类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FIntVector值的Optional，类型不匹配或数据不存在时为空
      * @note FIntVector包含X、Y、Z三个int32分量
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FIntVector> GetVector3I32(const FNBTDataAccessor& Target) {
         return Target.TryGetIntVector();
     }

     /**
      * 获取2D长整数向量值。
      * 读取当前节点存储的FInt64Vector2类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FInt64Vector2值的Optional，类型不匹配或数据不存在时为空
      * @note FInt64Vector2包含X和Y两个int64分量
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FInt64Vector2> GetVector2I64(const FNBTDataAccessor& Target) {
         return Target.TryGetInt64Vector2();
     }

     /**
      * 获取3D长整数向量值。
      * 读取当前节点存储的FInt64Vector类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含FInt64Vector值的Optional，类型不匹配或数据不存在时为空
      * @note FInt64Vector包含X、Y、Z三个int64分量
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TOptional<FInt64Vector> GetVector3I64(const FNBTDataAccessor& Target) {
         return Target.TryGetInt64Vector();
     }

     /**
      * 获取8位整数数组。
      * 读取当前节点存储的int8数组类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int8元素的数组，如果类型不匹配则返回空数组
      * @note 仅适用于Array类型节点，存储连续的int8值
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TArray<int8> GetInt8Array(const FNBTDataAccessor& Target) {
         return Target.TryGetInt8Array();
     }

     /**
      * 获取16位整数数组。
      * 读取当前节点存储的int16数组类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int16元素的数组，如果类型不匹配则返回空数组
      * @note 仅适用于Array类型节点，存储连续的int16值
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static TArray<int16> GetInt16Array(const FNBTDataAccessor& Target) {
         return Target.TryGetInt16Array();
     }

     /**
      * 获取32位整数数组。
      * 读取当前节点存储的int32数组类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int32元素的数组，如果类型不匹配则返回空数组
      * @note 仅适用于Array类型节点，存储连续的int32值
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TArray<int32> GetInt32Array(const FNBTDataAccessor& Target) {
         return Target.TryGetInt32Array();
     }

     /**
      * 获取64位整数数组。
      * 读取当前节点存储的int64数组类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含int64元素的数组，如果类型不匹配则返回空数组
      * @note 仅适用于Array类型节点，存储连续的int64值
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TArray<int64> GetInt64Array(const FNBTDataAccessor& Target) {
         return Target.TryGetInt64Array();
     }

     /**
      * 获取单精度浮点数组。
      * 读取当前节点存储的float数组类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含float元素的数组，如果类型不匹配则返回空数组
      * @note 仅适用于Array类型节点，存储连续的float值
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TArray<float> GetFloatArray(const FNBTDataAccessor& Target) {
         return Target.TryGetFloatArray();
     }

     /**
      * 确保当前节点为List类型。
      * 如果当前节点不是List类型，则将其转换为List类型节点。
      * @param Target 要处理的NBT数据访问器引用
      * @return 指向List节点的访问器
      * @note 此操作会清除节点的原有数据！转换不可逆
      * @warning 会丢失原有数据，请谨慎使用！
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor EnsureList(const FNBTDataAccessor& Target) {
         return Target.EnsureList();
     }

     /**
      * 确保当前节点为Map类型。
      * 如果当前节点不是Map类型，则将其转换为Map类型节点。
      * @param Target 要处理的NBT数据访问器引用
      * @return 指向Map节点的访问器
      * @note 此操作会清除节点的原有数据！转换不可逆
      * @warning 会丢失原有数据，请谨慎使用！
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor EnsureMap(const FNBTDataAccessor& Target) {
         return Target.EnsureMap();
     }

     /**
      * 获取双精度浮点数组。
      * 读取当前节点存储的double数组类型数据。
      * @param Target 要读取的NBT数据访问器引用
      * @return 包含double元素的数组，如果类型不匹配则返回空数组
      * @note 仅适用于Array类型节点，存储连续的double值
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TArray<double> GetDoubleArray(const FNBTDataAccessor& Target) {
         return Target.TryGetDoubleArray();
     }

     /**
      * 尝试设置通用整数值。
      * 在不改变节点类型的前提下，尝试将值设置为兼容的整数类型。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的64位整数值
      * @return 操作结果详情，包含成功/失败状态和错误信息
      * @note 仅在节点为兼容整数类型时成功，不会进行类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetGenericInt(const FNBTDataAccessor& Target, int64 Value) {
         return Target.TrySetGenericInt(Value);
     }

     /**
      * 尝试设置通用浮点值。
      * 在不改变节点类型的前提下，尝试将值设置为兼容的浮点类型。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的双精度浮点值
      * @return 操作结果详情，包含成功/失败状态和错误信息
      * @note 仅在节点为兼容浮点类型时成功，不会进行类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetGenericDouble(const FNBTDataAccessor& Target, double Value) {
         return Target.TrySetGenericDouble(Value);
     }

     /**
      * 尝试将当前节点设置为空值。
      * 在不改变节点类型的前提下，尝试清空节点内容。
      * @param Target 要设置的NBT数据访问器引用
      * @return 操作结果详情，包含成功/失败状态和错误信息
      * @note 仅在节点支持空值状态时成功
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetEmpty(const FNBTDataAccessor& Target) {
         return Target.TrySetEmpty();
     }

     /**
      * 尝试设置布尔值。
      * 仅在当前节点为布尔类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的布尔值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetBool(const FNBTDataAccessor& Target, bool Value) {
         return Target.TrySetBool(Value);
     }

     /**
      * 尝试设置8位整数值。
      * 仅在当前节点为int8类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int8值（范围：-128到127）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt8(const FNBTDataAccessor& Target, int8 Value) {
         return Target.TrySetInt8(Value);
     }

     /**
      * 尝试设置16位整数值。
      * 仅在当前节点为int16类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int16值（范围：-32,768到32,767）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt16(const FNBTDataAccessor& Target, int16 Value) {
         return Target.TrySetInt16(Value);
     }

     /**
      * 尝试设置32位整数值。
      * 仅在当前节点为int32类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int32值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt32(const FNBTDataAccessor& Target, int32 Value) {
         return Target.TrySetInt32(Value);
     }

     /**
      * 尝试设置64位整数值。
      * 仅在当前节点为int64类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int64值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt64(const FNBTDataAccessor& Target, int64 Value) {
         return Target.TrySetInt64(Value);
     }

     /**
      * 尝试设置单精度浮点值。
      * 仅在当前节点为float类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的float值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetFloat(const FNBTDataAccessor& Target, float Value) {
         return Target.TrySetFloat(Value);
     }

     /**
      * 尝试设置双精度浮点值。
      * 仅在当前节点为double类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的double值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetDouble(const FNBTDataAccessor& Target, double Value) {
         return Target.TrySetDouble(Value);
     }

     /**
      * 尝试设置FName值。
      * 仅在当前节点为FName类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的FName值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetName(const FNBTDataAccessor& Target, FName Value) {
         return Target.TrySetName(Value);
     }

     /**
      * 尝试设置字符串值。
      * 仅在当前节点为FString类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的字符串值
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetString(const FNBTDataAccessor& Target, const FString& Value) {
         return Target.TrySetString(Value);
     }

     /**
      * 尝试设置旋转值。
      * 仅在当前节点为FRotator类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的旋转值（Pitch、Yaw、Roll）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetRotator(const FNBTDataAccessor& Target, FRotator Value) {
         return Target.TrySetRotator(Value);
     }

     /**
      * 尝试设置2D向量值。
      * 仅在当前节点为FVector2D类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的2D向量值（X、Y分量）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetVector2D(const FNBTDataAccessor& Target, FVector2D Value) {
         return Target.TrySetVector2D(Value);
     }

     /**
      * 尝试设置3D向量值。
      * 仅在当前节点为FVector类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的3D向量值（X、Y、Z分量）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetVector3D(const FNBTDataAccessor& Target, FVector Value) {
         return Target.TrySetVector(Value);
     }

     /**
      * 尝试设置2D整数向量值。
      * 仅在当前节点为FIntVector2类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的2D整数向量值（X、Y分量为int32）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetVector2I32(const FNBTDataAccessor& Target, FIntVector2 Value) {
         return Target.TrySetIntVector2(Value);
     }

     /**
      * 尝试设置3D整数向量值。
      * 仅在当前节点为FIntVector类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的3D整数向量值（X、Y、Z分量为int32）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetVector3I32(const FNBTDataAccessor& Target, FIntVector Value) {
         return Target.TrySetIntVector(Value);
     }

     /**
      * 尝试设置2D长整数向量值。
      * 仅在当前节点为FInt64Vector2类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的2D长整数向量值（X、Y分量为int64）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetVector2I64(const FNBTDataAccessor& Target, FInt64Vector2 Value) {
         return Target.TrySetInt64Vector2(Value);
     }

     /**
      * 尝试设置3D长整数向量值。
      * 仅在当前节点为FInt64Vector类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的3D长整数向量值（X、Y、Z分量为int64）
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，不进行自动类型转换
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetVector3I64(const FNBTDataAccessor& Target, FInt64Vector Value) {
         return Target.TrySetInt64Vector(Value);
     }

     /**
      * 尝试设置8位整数数组。
      * 仅在当前节点为int8数组类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int8数组引用
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，会完全替换现有数组内容
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt8Array(const FNBTDataAccessor& Target, const TArray<int8>& Value) {
         return Target.TrySetInt8Array(Value);
     }

     /**
      * 尝试设置16位整数数组。
      * 仅在当前节点为int16数组类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int16数组引用
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，会完全替换现有数组内容
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt16Array(const FNBTDataAccessor& Target, const TArray<int16>& Value) {
         return Target.TrySetInt16Array(Value);
     }

     /**
      * 尝试设置32位整数数组。
      * 仅在当前节点为int32数组类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int32数组引用
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt32Array(const FNBTDataAccessor& Target, const TArray<int32>& Value) {
         return Target.TrySetInt32Array(Value);
     }

     /**
      * 尝试设置64位整数数组。
      * 仅在当前节点为int64数组类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int64数组引用
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetInt64Array(const FNBTDataAccessor& Target, const TArray<int64>& Value) {
         return Target.TrySetInt64Array(Value);
     }

     /**
      * 尝试设置单精度浮点数组。
      * 仅在当前节点为float数组类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的float数组引用
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetFloatArray(const FNBTDataAccessor& Target, const TArray<float>& Value) {
         return Target.TrySetFloatArray(Value);
     }

     /**
      * 尝试设置双精度浮点数组。
      * 仅在当前节点为double数组类型时设置值，否则操作失败。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的double数组引用
      * @return 操作结果详情，类型不匹配时返回错误
      * @note 严格类型检查，会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TrySetDoubleArray(const FNBTDataAccessor& Target, const TArray<double>& Value) {
         return Target.TrySetDoubleArray(Value);
     }

     /**
      * 确保节点为空类型并设置值。
      * 如果当前节点不是Empty类型，则自动转换为Empty类型后清空内容。
      * @param Target 要设置的NBT数据访问器引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetEmpty(const FNBTDataAccessor& Target) {
         return Target.EnsureAndSetEmpty();
     }

     /**
      * 确保节点为布尔类型并设置值。
      * 如果当前节点不是布尔类型，则自动转换为布尔类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的布尔值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetBool(const FNBTDataAccessor& Target, bool Value) {
         return Target.EnsureAndSetBool(Value);
     }

     /**
      * 确保节点为8位整数类型并设置值。
      * 如果当前节点不是int8类型，则自动转换为int8类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int8值（范围：-128到127）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt8(const FNBTDataAccessor& Target, int8 Value) {
         return Target.EnsureAndSetInt8(Value);
     }

     /**
      * 确保节点为16位整数类型并设置值。
      * 如果当前节点不是int16类型，则自动转换为int16类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int16值（范围：-32,768到32,767）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt16(const FNBTDataAccessor& Target, int16 Value) {
         return Target.EnsureAndSetInt16(Value);
     }

     /**
      * 确保节点为32位整数类型并设置值。
      * 如果当前节点不是int32类型，则自动转换为int32类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int32值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt32(const FNBTDataAccessor& Target, int32 Value) {
         return Target.EnsureAndSetInt32(Value);
     }

     /**
      * 确保节点为64位整数类型并设置值。
      * 如果当前节点不是int64类型，则自动转换为int64类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int64值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt64(const FNBTDataAccessor& Target, int64 Value) {
         return Target.EnsureAndSetInt64(Value);
     }

     /**
      * 确保节点为单精度浮点类型并设置值。
      * 如果当前节点不是float类型，则自动转换为float类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的float值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetFloat(const FNBTDataAccessor& Target, float Value) {
         return Target.EnsureAndSetFloat(Value);
     }

     /**
      * 确保节点为双精度浮点类型并设置值。
      * 如果当前节点不是double类型，则自动转换为double类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的double值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetDouble(const FNBTDataAccessor& Target, double Value) {
         return Target.EnsureAndSetDouble(Value);
     }

     /**
      * 确保节点为FName类型并设置值。
      * 如果当前节点不是FName类型，则自动转换为FName类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的FName值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetName(const FNBTDataAccessor& Target, FName Value) {
         return Target.EnsureAndSetName(Value);
     }

     /**
      * 确保节点为旋转类型并设置值。
      * 如果当前节点不是FRotator类型，则自动转换为FRotator类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的旋转值（Pitch、Yaw、Roll）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetRotator(const FNBTDataAccessor& Target, FRotator Value) {
         return Target.EnsureAndSetRotator(Value);
     }

     /**
      * 确保节点为字符串类型并设置值。
      * 如果当前节点不是FString类型，则自动转换为FString类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的字符串值
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetString(const FNBTDataAccessor& Target, const FString& Value) {
         return Target.EnsureAndSetString(Value);
     }

     /**
      * 确保节点为2D向量类型并设置值。
      * 如果当前节点不是FVector2D类型，则自动转换为FVector2D类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的2D向量值（X、Y分量）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetVector2D(const FNBTDataAccessor& Target, FVector2D Value) {
         return Target.EnsureAndSetVector2D(Value);
     }

     /**
      * 确保节点为3D向量类型并设置值。
      * 如果当前节点不是FVector类型，则自动转换为FVector类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的3D向量值（X、Y、Z分量）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetVector3D(const FNBTDataAccessor& Target, FVector Value) {
         return Target.EnsureAndSetVector(Value);
     }

     /**
      * 确保节点为2D整数向量类型并设置值。
      * 如果当前节点不是FIntVector2类型，则自动转换为FIntVector2类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的2D整数向量值（X、Y分量为int32）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetVector2I32(const FNBTDataAccessor& Target, FIntVector2 Value) {
         return Target.EnsureAndSetIntVector2(Value);
     }

     /**
      * 确保节点为3D整数向量类型并设置值。
      * 如果当前节点不是FIntVector类型，则自动转换为FIntVector类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的3D整数向量值（X、Y、Z分量为int32）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetVector3I32(const FNBTDataAccessor& Target, FIntVector Value) {
         return Target.EnsureAndSetIntVector(Value);
     }

     /**
      * 确保节点为2D长整数向量类型并设置值。
      * 如果当前节点不是FInt64Vector2类型，则自动转换为FInt64Vector2类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的2D长整数向量值（X、Y分量为int64）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetVector2I64(const FNBTDataAccessor& Target, FInt64Vector2 Value) {
         return Target.EnsureAndSetInt64Vector2(Value);
     }

     /**
      * 确保节点为3D长整数向量类型并设置值。
      * 如果当前节点不是FInt64Vector类型，则自动转换为FInt64Vector类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的3D长整数向量值（X、Y、Z分量为int64）
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetVector3I64(const FNBTDataAccessor& Target, FInt64Vector Value) {
         return Target.EnsureAndSetInt64Vector(Value);
     }

     /**
      * 确保节点为8位整数数组类型并设置值。
      * 如果当前节点不是int8数组类型，则自动转换为int8数组类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int8数组引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据；会完全替换现有数组内容
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt8Array(const FNBTDataAccessor& Target, const TArray<int8>& Value) {
         return Target.EnsureAndSetInt8Array(Value);
     }

     /**
      * 确保节点为16位整数数组类型并设置值。
      * 如果当前节点不是int16数组类型，则自动转换为int16数组类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int16数组引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据；会完全替换现有数组内容
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt16Array(const FNBTDataAccessor& Target, const TArray<int16>& Value) {
         return Target.EnsureAndSetInt16Array(Value);
     }

     /**
      * 确保节点为32位整数数组类型并设置值。
      * 如果当前节点不是int32数组类型，则自动转换为int32数组类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int32数组引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据；会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt32Array(const FNBTDataAccessor& Target, const TArray<int32>& Value) {
         return Target.EnsureAndSetInt32Array(Value);
     }

     /**
      * 确保节点为64位整数数组类型并设置值。
      * 如果当前节点不是int64数组类型，则自动转换为int64数组类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的int64数组引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据；会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetInt64Array(const FNBTDataAccessor& Target, const TArray<int64>& Value) {
         return Target.EnsureAndSetInt64Array(Value);
     }

     /**
      * 确保节点为单精度浮点数组类型并设置值。
      * 如果当前节点不是float数组类型，则自动转换为float数组类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的float数组引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据；会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetFloatArray(const FNBTDataAccessor& Target, const TArray<float>& Value) {
         return Target.EnsureAndSetFloatArray(Value);
     }

     /**
      * 确保节点为双精度浮点数组类型并设置值。
      * 如果当前节点不是double数组类型，则自动转换为double数组类型后设置值。
      * @param Target 要设置的NBT数据访问器引用
      * @param Value 要设置的double数组引用
      * @return 操作结果详情，转换和设置都成功时返回成功
      * @note 会自动进行类型转换，可能丢失原有数据；会完全替换现有数组内容
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndSetDoubleArray(const FNBTDataAccessor& Target, const TArray<double>& Value) {
         return Target.EnsureAndSetDoubleArray(Value);
     }

     /**
      * 强制重写节点为布尔类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为布尔类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的布尔值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToBool(const FNBTDataAccessor& Target, bool Value) {
         return Target.OverrideToBool(Value);
     }

     /**
      * 强制重写节点为8位整数类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int8类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int8值（范围：-128到127）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt8(const FNBTDataAccessor& Target, int8 Value) {
         return Target.OverrideToInt8(Value);
     }

     /**
      * 强制重写节点为16位整数类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int16类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int16值（范围：-32,768到32,767）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt16(const FNBTDataAccessor& Target, int16 Value) {
         return Target.OverrideToInt16(Value);
     }

     /**
      * 强制重写节点为32位整数类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int32类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int32值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt32(const FNBTDataAccessor& Target, int32 Value) {
         return Target.OverrideToInt32(Value);
     }

     /**
      * 强制重写节点为64位整数类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int64类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int64值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt64(const FNBTDataAccessor& Target, int64 Value) {
         return Target.OverrideToInt64(Value);
     }

     /**
      * 强制重写节点为单精度浮点类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为float类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的float值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToFloat(const FNBTDataAccessor& Target, float Value) {
         return Target.OverrideToFloat(Value);
     }

     /**
      * 强制重写节点为双精度浮点类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为double类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的double值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToDouble(const FNBTDataAccessor& Target, double Value) {
         return Target.OverrideToDouble(Value);
     }

     /**
      * 强制重写节点为FName类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FName类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的FName值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToName(const FNBTDataAccessor& Target, FName Value) {
         return Target.OverrideToName(Value);
     }

     /**
      * 强制重写节点为旋转类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FRotator类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的旋转值（Pitch、Yaw、Roll）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToRotator(const FNBTDataAccessor& Target, FRotator Value) {
         return Target.OverrideToRotator(Value);
     }

     /**
      * 强制重写节点为字符串类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FString类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的字符串值
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToString(const FNBTDataAccessor& Target, const FString& Value) {
         return Target.OverrideToString(Value);
     }

     /**
      * 强制重写节点为2D向量类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FVector2D类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的2D向量值（X、Y分量）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToVector2D(const FNBTDataAccessor& Target, FVector2D Value) {
         return Target.OverrideToVector2D(Value);
     }

     /**
      * 强制重写节点为3D向量类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FVector类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的3D向量值（X、Y、Z分量）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToVector3D(const FNBTDataAccessor& Target, FVector Value) {
         return Target.OverrideToVector(Value);
     }

     /**
      * 强制重写节点为2D整数向量类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FIntVector2类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的2D整数向量值（X、Y分量为int32）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToVector2I32(const FNBTDataAccessor& Target, FIntVector2 Value) {
         return Target.OverrideToIntVector2(Value);
     }

     /**
      * 强制重写节点为3D整数向量类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FIntVector类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的3D整数向量值（X、Y、Z分量为int32）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToVector3I32(const FNBTDataAccessor& Target, FIntVector Value) {
         return Target.OverrideToIntVector(Value);
     }

     /**
      * 强制重写节点为2D长整数向量类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FInt64Vector2类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的2D长整数向量值（X、Y分量为int64）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToVector2I64(const FNBTDataAccessor& Target, FInt64Vector2 Value) {
         return Target.OverrideToInt64Vector2(Value);
     }

     /**
      * 强制重写节点为3D长整数向量类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为FInt64Vector类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的3D长整数向量值（X、Y、Z分量为int64）
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToVector3I64(const FNBTDataAccessor& Target, FInt64Vector Value) {
         return Target.OverrideToInt64Vector(Value);
     }

     /**
      * 强制重写节点为8位整数数组类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int8数组类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int8数组引用
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！会完全替换现有数组内容
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt8Array(const FNBTDataAccessor& Target, const TArray<int8>& Value) {
         return Target.OverrideToInt8Array(Value);
     }

     /**
      * 强制重写节点为16位整数数组类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int16数组类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int16数组引用
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！会完全替换现有数组内容
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION(meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt16Array(const FNBTDataAccessor& Target, const TArray<int16>& Value) {
         return Target.OverrideToInt16Array(Value);
     }

     /**
      * 强制重写节点为32位整数数组类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int32数组类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int32数组引用
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！会完全替换现有数组内容
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt32Array(const FNBTDataAccessor& Target, const TArray<int32>& Value) {
         return Target.OverrideToInt32Array(Value);
     }

     /**
      * 强制重写节点为64位整数数组类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为int64数组类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的int64数组引用
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！会完全替换现有数组内容
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToInt64Array(const FNBTDataAccessor& Target, const TArray<int64>& Value) {
         return Target.OverrideToInt64Array(Value);
     }

     /**
      * 强制重写节点为单精度浮点数组类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为float数组类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的float数组引用
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！会完全替换现有数组内容
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToFloatArray(const FNBTDataAccessor& Target, const TArray<float>& Value) {
         return Target.OverrideToFloatArray(Value);
     }

     /**
      * 强制重写节点为双精度浮点数组类型并设置值。
      * 无论当前节点是什么类型，都会被强制转换为double数组类型并设置指定值。
      * @param Target 要重写的NBT数据访问器引用
      * @param Value 要设置的double数组引用
      * @return 操作结果详情，重写操作成功时返回成功
      * @warning 此操作会无条件丢失原有数据和类型信息！会完全替换现有数组内容
      * @note 与EnsureAndSet的区别：此函数会强制重写，无论原类型是什么
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail OverrideToDoubleArray(const FNBTDataAccessor& Target, const TArray<double>& Value) {
         return Target.OverrideToDoubleArray(Value);
     }


     /**
      * 检查Map类型节点是否包含指定键。
      * 仅对Map类型节点有效，检查是否存在指定FName键的子节点。
      * @param Target 要检查的NBT数据访问器引用
      * @param Key 要检查的FName键
      * @return 操作结果详情，包含是否存在该键的信息
      * @note 仅适用于Map类型节点，其他类型节点将返回失败
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MapHasKey(const FNBTDataAccessor& Target, FName Key) {
         return Target.MapHasKey(Key);
     }

     /**
      * 获取Map类型节点的所有键。
      * 仅对Map类型节点有效，返回Map中所有子节点的FName键列表。
      * @param Target 要查询的NBT数据访问器引用
      * @param Keys 输出参数，用于接收所有键的数组
      * @return 操作结果详情，成功时Keys参数将包含所有键
      * @note 仅适用于Map类型节点，其他类型节点将返回失败
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MapGetKeys(const FNBTDataAccessor& Target, TArray<FName>& Keys) {
         return Target.MapGetKeys(Keys);
     }

    /**
     * 获取Map类型节点中的键值对数量。
     * 仅对Map类型节点有效，返回Map中子节点的总数量。
     * @param Target 要查询的NBT数据访问器引用
     * @param Size 输出参数，用于接收Map的大小
     * @return 操作结果详情，成功时Size参数将包含Map大小
     * @note 仅适用于Map类型节点，其他类型节点将返回失败
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MapGetSize(const FNBTDataAccessor& Target, int32& Size) {
         return Target.MapGetSize(Size);
     }

    /**
     * 从Map类型节点中删除指定键的子节点。
     * 仅对Map类型节点有效，永久删除指定键及其关联的数据。
     * @param Target 要操作的NBT数据访问器引用
     * @param Key 要删除的子节点的FName键
     * @return 操作结果详情，成功删除或键不存在时返回成功
     * @warning 此操作不可逆！会永久删除指定键的所有数据
     * @note 仅适用于Map类型节点，其他类型节点将返回失败
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MapRemoveSubNode(const FNBTDataAccessor& Target, FName Key) {
         return Target.MapRemoveSubNode(Key);
     }

     /**
     * 清空Map类型节点中的所有键值对。
     * 仅对Map类型节点有效，删除Map中的所有子节点和数据。
     * @param Target 要清空的NBT数据访问器引用
     * @return 操作结果详情，清空操作成功时返回成功
     * @warning 此操作不可逆！会永久删除Map中的所有子节点和数据
     * @note 仅适用于Map类型节点，其他类型节点将返回失败
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MapClear(const FNBTDataAccessor& Target) {
         return Target.MapClear();
     }

     /**
     * 从Map类型节点创建所有子节点的访问器数组。
     * 仅对Map类型节点有效，为Map中的每个键值对创建独立的访问器。
     * @param Target 要处理的NBT数据访问器引用
     * @param Accessors 输出参数，用于接收所有子节点访问器的数组
     * @return 操作结果详情，成功时Accessors参数将包含所有子访问器
     * @note 仅适用于Map类型节点，访问器顺序与键的内部存储顺序相关
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MakeAccessorFromMap(const FNBTDataAccessor& Target, TArray<FNBTDataAccessor>& Accessors) {
         return Target.MakeAccessorFromMap(Accessors);
     }

     /**
     * 立即从Map类型节点创建所有子节点的访问器数组。
     * 仅对Map类型节点有效，直接返回包含所有子节点访问器的数组。
     * @param Target 要处理的NBT数据访问器引用
     * @return 包含所有子节点访问器的数组，失败时返回空数组
     * @note 仅适用于Map类型节点，这是MakeAccessorFromMap的便捷版本
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TArray<FNBTDataAccessor> MakeAccessorFromMapNow(const FNBTDataAccessor& Target) {
         return Target.MakeAccessorFromMapNow();
     }

     /**
     * 获取List类型节点中的元素数量。
     * 仅对List类型节点有效，返回List中子节点的总数量。
     * @param Target 要查询的NBT数据访问器引用
     * @param Size 输出参数，用于接收List的大小
     * @return 操作结果详情，成功时Size参数将包含List大小
     * @note 仅适用于List类型节点，其他类型节点将返回失败
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail ListGetSize(const FNBTDataAccessor& Target, int32& Size) {
         return Target.ListGetSize(Size);
     }

     /**
     * 获取当前访问器在List中的索引位置。
     * 如果当前访问器指向List中的某个元素，返回该元素的索引。
     * @param Target 要查询的NBT数据访问器引用
     * @return 包含索引的Optional，如果不在List中或无效则为空
     * @note 索引从0开始，仅当访问器指向List元素时有效
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int32> ListGetCurrentIndex(const FNBTDataAccessor& Target) {
         return Target.ListGetCurrentIndex();
     }

     /**
     * 获取访问器路径中最后一个List父节点的索引。
     * 在嵌套结构中，返回访问路径上最近的List父节点中的索引位置。
     * @param Target 要查询的NBT数据访问器引用
     * @return 包含父索引的Optional，如果路径中没有List父节点则为空
     * @note 用于在复杂嵌套结构中追踪List层级的索引信息
     */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TOptional<int32> ListGetLastParentIndex(const FNBTDataAccessor& Target) {
         return Target.ListGetLastParentIndex();
     }

     /**
      * 从List类型节点中删除指定索引的子节点。
      * 仅对List类型节点有效，永久删除指定索引位置的元素。
      * @param Target 要操作的NBT数据访问器引用
      * @param Index 要删除的子节点索引（从0开始）
      * @param bSwapRemove 是否使用交换删除（默认false）- true时将最后一个元素移动到删除位置，false时保持元素顺序
      * @return 操作结果详情，成功删除或索引无效时返回对应状态
      * @warning 此操作不可逆！会永久删除指定索引的所有数据
      * @note 仅适用于List类型节点，其他类型节点将返回失败；交换删除更高效但不保持顺序
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail ListRemoveSubNode(const FNBTDataAccessor& Target, int32 Index, bool bSwapRemove = false) {
         return Target.ListRemoveSubNode(Index, bSwapRemove);
     }

     /**
      * 清空List类型节点中的所有元素。
      * 仅对List类型节点有效，删除List中的所有子节点和数据。
      * @param Target 要清空的NBT数据访问器引用
      * @return 操作结果详情，清空操作成功时返回成功
      * @warning 此操作不可逆！会永久删除List中的所有子节点和数据
      * @note 仅适用于List类型节点，其他类型节点将返回失败
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail ListClear(const FNBTDataAccessor& Target) {
         return Target.ListClear();
     }

     /**
      * 向List类型节点末尾添加新的子节点。
      * 仅对List类型节点有效，在列表末尾创建一个新的Empty类型子节点。
      * @param Target 要操作的NBT数据访问器引用
      * @return 指向新创建子节点的访问器，如果操作失败则返回无效访问器
      * @note 仅适用于List类型节点，新节点初始为Empty类型，可通过返回的访问器设置具体数据
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor ListAddSubNode(const FNBTDataAccessor& Target) {
         return Target.ListAddSubNode();
     }

     /**
      * 在List类型节点的指定位置插入新的子节点。
      * 仅对List类型节点有效，在指定索引位置插入一个新的Empty类型子节点。
      * @param Target 要操作的NBT数据访问器引用
      * @param Index 插入位置的索引（从0开始），原有元素会向后移动
      * @return 指向新创建子节点的访问器，如果操作失败则返回无效访问器
      * @note 仅适用于List类型节点，索引超出范围时行为未定义；新节点初始为Empty类型
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTDataAccessor ListInsertSubNode(const FNBTDataAccessor& Target, int32 Index) {
         return Target.ListInsertSubNode(Index);
     }

     /**
      * 从List类型节点创建所有子节点的访问器数组。
      * 仅对List类型节点有效，为List中的每个元素创建独立的访问器。
      * @param Target 要处理的NBT数据访问器引用
      * @param Accessors 输出参数，用于接收所有子节点访问器的数组
      * @return 操作结果详情，成功时Accessors参数将包含所有子访问器
      * @note 仅适用于List类型节点，访问器顺序与List中元素的索引顺序一致
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail MakeAccessorFromList(const FNBTDataAccessor& Target, TArray<FNBTDataAccessor>& Accessors) {
         return Target.MakeAccessorFromList(Accessors);
     }

     /**
      * 立即从List类型节点创建所有子节点的访问器数组。
      * 仅对List类型节点有效，直接返回包含所有子节点访问器的数组。
      * @param Target 要处理的NBT数据访问器引用
      * @return 包含所有子节点访问器的数组，失败时返回空数组
      * @note 仅适用于List类型节点，这是MakeAccessorFromList的便捷版本
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static TArray<FNBTDataAccessor> MakeAccessorFromListNow(const FNBTDataAccessor& Target) {
         return Target.MakeAccessorFromListNow();
     }


     /**
      * 从父节点中删除当前访问器指向的节点。
      * 根据访问器路径，从其父节点（Map或List）中删除当前指向的子节点。
      * @param Target 要删除的NBT数据访问器引用
      * @return 删除操作的结果码，成功时返回正值，失败时返回负值或0
      * @warning 此操作不可逆！会永久删除节点及其所有子数据
      * @note 适用于Map和List的子节点，删除后访问器将变为无效状态
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static int Remove(const FNBTDataAccessor& Target) {
         return const_cast<FNBTDataAccessor*>(&Target)->Remove();
     }

     /**
      * 将当前访问器指向的数据转换为字符串表示。
      * 生成当前节点及其内容的简洁字符串格式，适合日志输出和调试显示。
      * @param Target 要转换的NBT数据访问器引用
      * @return 节点内容的字符串表示
      * @note 输出格式取决于节点类型，基础类型显示值，复合类型显示结构概览
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString ToString(const FNBTDataAccessor& Target) {
         return Target.ToString();
     }

     /**
      * 获取当前访问器的完整访问路径。
      * 返回从容器根节点到当前节点的完整路径字符串，用于调试和路径追踪。
      * @param Target 要查询的NBT数据访问器引用
      * @return 完整的访问路径字符串，格式类似"/root/map_key/list[0]/sub_key"
      * @note 路径格式：Map键用名称表示，List索引用[n]表示，用于精确定位节点位置
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString GetPath(const FNBTDataAccessor& Target) {
         return Target.GetPath();
     }

     /**
      * 获取当前访问器的预览路径。
      * 返回访问器路径的简化或预览版本，适合UI显示和用户友好的路径表示。
      * @param Target 要查询的NBT数据访问器引用
      * @return 预览路径字符串，比完整路径更简洁易读
      * @note 与GetPath()相比，此函数返回的路径可能经过简化处理，更适合界面展示
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString GetPreviewPath(const FNBTDataAccessor& Target) {
         return Target.GetPreviewPath();
     }


     /**
      * 尝试从源访问器复制数据到当前访问器。
      * 仅在当前访问器路径有效时执行复制操作，如果路径无效则不进行复制。
      * @param Target 目标NBT数据访问器引用（将被复制到的位置）
      * @param Source 源NBT数据访问器引用（数据来源）
      * @return 操作结果详情，路径无效时返回失败，复制成功时返回成功
      * @note 严格的路径检查，不会自动创建缺失的路径节点
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail TryCopyFrom(const FNBTDataAccessor& Target, const FNBTDataAccessor& Source) {
         return Target.TryCopyFrom(Source);
     }

     /**
      * 确保路径存在并从源访问器复制数据到当前访问器。
      * 如果当前访问器路径无效，会尝试创建路径后再执行复制操作。
      * @param Target 目标NBT数据访问器引用（将被复制到的位置）
      * @param Source 源NBT数据访问器引用（数据来源）
      * @return 操作结果详情，路径创建失败或复制失败时返回错误
      * @warning 无法处理路径中包含List的情况，List节点需要显式索引
      * @note 会自动创建Map类型的中间节点，但不能自动处理List索引
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FNBTAttributeOpResultDetail EnsureAndCopyFrom(const FNBTDataAccessor& Target, const FNBTDataAccessor& Source) {
         return Target.EnsureAndCopyFrom(Source);
     }

     /**
      * 深度遍历访问器指向的数据结构，对每个节点执行访问者函数。
      * 使用访问者模式递归遍历NBT数据树，为每个节点调用指定的回调函数。
      * @param TargetAccessor 要遍历的NBT数据访问器引用
      * @param Sig 访问者函数签名，接收参数：深度、类型、属性名、属性索引、访问器
      * @note 遍历顺序：深度优先，先访问父节点再访问子节点
      * @warning 如果访问者函数无效，会记录错误日志并打印调用栈
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static void VisitData(const FNBTDataAccessor& TargetAccessor, FArzNBTDataVisitorSignature Sig) {
         if (!Sig.IsBound()) {
             UE_LOG(NBTSystem, Error, TEXT("==== VisitData(..): Function %s is not valid"), *Sig.GetFunctionName().ToString());
             UE_LOG(NBTSystem, Error, TEXT("==== tack Trace:"));
             const auto Stack = FAngelscriptManager::Get().GetAngelscriptCallstack();
             for (const auto& Frame : Stack) {
                 UE_LOGFMT(NBTSystem, Error, "==== {0}", Frame);
             }
             return;
         }

         TargetAccessor.VisitData([&Sig](int Deep, ENBTAttributeType Type, FName AttrName, int32 AttrIdx, const FNBTDataAccessor& Accessor) {
             Sig.ExecuteIfBound(Deep, Type, AttrName, AttrIdx, Accessor);
         });
     }
 };


 UCLASS(Blueprintable, BlueprintType)
 class NBTSYSTEM_API UNBTSystemOperatorResultDetailCSharpBind : public UBlueprintFunctionLibrary {
     GENERATED_BODY()
 public:
     /**
      * 如果操作失败则输出错误调试信息到日志。
      * 检查操作结果，如果不成功则将错误详情输出到UE日志系统。
      * @param Target 要检查的NBT操作结果详情引用
      * @note 仅在操作失败时输出日志，成功时不产生日志输出
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static void ed(const FNBTAttributeOpResultDetail& Target) {
         Target.ed();
     }

     /**
      * 如果操作失败则输出带上下文的错误调试信息到日志。
      * 检查操作结果，如果不成功则将错误详情和自定义上下文信息输出到UE日志系统。
      * @param Target 要检查的NBT操作结果详情引用
      * @param Context 自定义上下文字符串，用于标识错误发生的位置或操作类型
      * @note 仅在操作失败时输出日志，上下文信息有助于调试和问题定位
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static void eds(const FNBTAttributeOpResultDetail& Target, const FString& Context) {
         Target.ed(Context);
     }

     /**
      * 如果操作失败则输出详细错误调试信息到日志（包含C++调用栈）。
      * 检查操作结果，如果不成功则输出错误详情，使用C++调用栈而非脚本调用栈。
      * @param Target 要检查的NBT操作结果详情引用
      * @note 专为UnrealSharp等C#脚本环境设计，提供更准确的C++层调用栈信息
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static void edv(const FNBTAttributeOpResultDetail& Target) {
         Target.edv(); // 使用C++调用栈，适合UnrealSharp
     }

     /**
      * 获取操作结果的字符串描述。
      * 返回操作结果的详细文本描述，包含成功/失败状态和错误原因。
      * @param Target 要查询的NBT操作结果详情引用
      * @return 操作结果的字符串表示，包含状态信息和可能的错误描述
      * @note 适用于UI显示、日志记录和调试输出
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static FString GetResultString(const FNBTAttributeOpResultDetail& Target) {
         return Target.GetResultString();
     }

     /**
      * 检查操作是否成功完成。
      * 返回操作结果的成功状态，用于判断NBT操作是否成功执行。
      * @param Target 要检查的NBT操作结果详情引用
      * @return 操作成功时返回true，失败时返回false
      * @note 这是检查操作结果最常用的方法
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsSuccess(const FNBTAttributeOpResultDetail& Target) {
         return Target.IsSuccess();
     }

     /**
      * 检查操作结果是否正常（OK状态）。
      * 返回操作结果的OK状态，这是IsSuccess()的别名方法。
      * @param Target 要检查的NBT操作结果详情引用
      * @return 操作正常时返回true，异常时返回false
      * @note 功能与IsSuccess()相同，提供更简洁的命名选择
      */
     UFUNCTION( meta=(ExtensionMethod, ScriptMethod))
     static bool IsOK(const FNBTAttributeOpResultDetail& Target) {
         return Target.IsOk();
     }
 };