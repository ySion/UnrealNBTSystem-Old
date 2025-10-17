#include "NBTBindAS.h"

#include "CoreMinimal.h"
#include "AngelscriptBinds.h"
#include "AngelscriptDocs.h"

// =================================================================================
// Angelscript NBT绑定辅助宏
// =================================================================================

/**
 * @brief 为NBT基础数据类型绑定一套完整的访问和操作函数 (TryGet, TrySet, EnsureAndSet, OverrideTo)
 * @param TypeName      用于构成函数名的类型标识, 例如 Bool, Int32, String
 * @param CppType       实际的C++类型, 例如 bool, int32, FString
 * @param AsType        在AngelScript脚本中使用的类型签名, 通常与CppType相同, 但对float应为float32, 对const T&应为 const CppType&
 * @param ParamType     C++方法签名中使用的参数类型, 用于METHODPR_TRIVIAL宏, 例如 bool, const FString&
 * @param DocName       文档中显示的类型名称, 例如 "布尔值(bool)"
 * @param DocValue      文档中显示的参数值名称, 例如 "布尔值"
 * @param ... (__VA_ARGS__) 可选的AngelScript参数修饰符, 如 __any_implicit_integer
 */
#define BIND_NBT_ACCESSOR_BASE_TYPE(TypeName, CppType, AsType, CppParamType, AsParamType, DocName, DocValue, ...) \
    /* TryGet */ \
    FArzNBTDataAccessor_.Method("TOptional<" #AsType "> TryGet" #TypeName "() const", METHODPR_TRIVIAL(TOptional<CppType>, FNBTDataAccessor, TryGet##TypeName, ()const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 尝试从当前路径获取" DocName "类型的值 \n" \
        "* @return 如果路径存在且类型匹配则返回" #CppType "值，否则返回空的TOptional \n" \
    ); \
    /* TrySet */ \
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TrySet" #TypeName "(" #AsParamType " " #__VA_ARGS__ " Value) const", \
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TrySet##TypeName, (CppParamType)const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 尝试设置当前路径节点的值为" DocName "。这是最安全的操作模式。\n" \
        "* 此操作仅在路径存在且节点类型为" #TypeName "时才会成功。\n" \
        "* 它不会自动创建路径，也不会改变现有节点的类型。\n" \
        "* @param Value 要设置的" DocValue "。\n" \
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n" \
    ); \
    /* EnsureAndSet */ \
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail EnsureAndSet" #TypeName "(" #AsParamType " " #__VA_ARGS__ " Value) const", \
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, EnsureAndSet##TypeName, (CppParamType)const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 确保路径存在并设置节点值为" DocName "。主要用于数据初始化。\n" \
        "* 如果路径中的Map节点不存在，会自动创建。如果最终节点不存在，会创建为" #TypeName "类型并赋值。\n" \
        "* 如果最终节点已存在，只有当其类型为" #TypeName "或Empty时才会更新值。它不会覆盖其他不同类型的已有节点。\n" \
        "* @param Value 要设置的" DocValue "。\n" \
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n" \
    ); \
    /* OverrideTo */ \
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail OverrideTo" #TypeName "(" #AsParamType " " #__VA_ARGS__ " Value) const", \
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, OverrideTo##TypeName, (CppParamType)const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 强制将节点覆盖为" DocName "。这是一个危险操作，请谨慎使用！\n" \
        "* 无论节点当前是什么类型，都会被强制转换为" #TypeName "类型并赋值。会破坏原有的数据结构。\n" \
        "* 如果路径不存在，会创建整个路径并设置值。如果路径中存在无效List索引, 将无法跨越List。\n" \
        "* @param Value 要设置的" DocValue "。\n" \
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n" \
        "* @warning 此操作会无条件覆盖现有数据，可能会影响其他需要和此数据交互的程序模块工作！\n" \
    )

/**
 * @brief 为NBT数组数据类型绑定一套完整的访问和操作函数 (TryGet, EnsureAndSet, OverrideTo)
 * @param TypeName      用于构成函数名的类型标识, 例如 Int8, Int32, Float
 * @param ElementType   数组元素的实际C++类型, 例如 int8, int32, float
 * @param AsElementType 数组元素在AngelScript脚本中使用的类型签名, 例如 int8, int32, float32
 * @param DocName       文档中显示的类型名称, 例如 "8位整数数组(int8[])"
 * @param DocValue      文档中显示的参数值名称, 例如 "8位整数数组"
 */
#define BIND_NBT_ACCESSOR_ARRAY_TYPE(TypeName, ElementType, AsElementType, DocName, DocValue) \
    /* TryGet */ \
    FArzNBTDataAccessor_.Method("TArray<" #AsElementType "> TryGet" #TypeName "Array() const", \
                                METHODPR_TRIVIAL(TArray<ElementType>, FNBTDataAccessor, TryGet##TypeName##Array, ()const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 尝试从当前路径的NBT节点获取一个" DocName "。\n" \
        "* 如果路径无效、节点不存在或类型不匹配(不是Array" #TypeName ")，将返回一个空数组。\n" \
        "* @return 获取到的" #ElementType "数组。如果操作失败则返回空数组。\n" \
    ); \
    /* TrySet */ \
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TrySet" #TypeName "Array(const TArray<" #AsElementType ">& Value) const", \
    METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TrySet##TypeName##Array, (const TArray<ElementType>&)const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 尝试设置当前路径节点的值为" DocName "。这是最安全的操作模式。\n" \
        "* 此操作仅在路径存在且节点类型为Array" #TypeName "时才会成功赋值。\n" \
        "* 它不会自动创建路径，也不会改变现有节点的类型。\n" \
        "* @param Value 要设置的" DocValue "。\n" \
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n" \
    ); \
    /* EnsureAndSet */ \
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail EnsureAndSet" #TypeName "Array(const TArray<" #AsElementType ">& Value) const", \
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, EnsureAndSet##TypeName##Array, (const TArray<ElementType>&)const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 确保路径存在并设置节点值为" DocName "。主要用于数据初始化。\n" \
        "* 如果路径中的Map节点不存在，会自动创建。如果最终节点不存在，会创建为Array" #TypeName "类型并赋值。\n" \
        "* 如果最终节点已存在，只有当其类型为Array" #TypeName "或Empty时才会更新值。它不会覆盖其他不同类型的已有节点。\n" \
        "* @param Value 要设置的" DocValue "。\n" \
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n" \
    ); \
    /* OverrideTo */ \
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail OverrideTo" #TypeName "Array(const TArray<" #AsElementType ">& Value) const", \
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, OverrideTo##TypeName##Array, (const TArray<ElementType>&)const)); \
    SCRIPT_BIND_DOCUMENTATION( \
        "* 强制将节点覆盖为" DocName "。这是一个危险操作,请谨慎使用!\n" \
        "* 无论节点当前是什么类型,都会被强制转换为Array" #TypeName "类型并赋值。会破坏原有的数据结构。\n" \
        "* 如果路径不存在,会创建整个路径并设置值。如果路径中存在无效List索引,将无法跨越List。\n" \
        "* @param Value 要设置的" DocValue "。\n" \
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail),包含了操作是否成功以及详细信息。\n" \
        "* @warning 此操作会无条件覆盖现有数据,可能会影响其他需要和此数据交互的程序模块工作！\n" \
    )


AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_FArzNBTAttributeOperatorResultDetail(FAngelscriptBinds::EOrder::Late, [] {
    auto FArzNBTAttributeOperatorResultDetail_ = FAngelscriptBinds::ExistingClass("FNBTAttributeOpResultDetail");

    FArzNBTAttributeOperatorResultDetail_.Method("void ed() const", METHODPR_TRIVIAL(void, FNBTAttributeOpResultDetail, ed, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 错误诊断 - 当操作失败时输出错误日志\n"
        "* 仅在结果不是 Success 或 SameAndNotChange 时才会输出错误信息\n"
        "* 错误日志包含操作结果类型和错误消息\n"
    )
    FArzNBTAttributeOperatorResultDetail_.Method("void ed(const FString& Context) const",
                                                 METHODPR_TRIVIAL(void, FNBTAttributeOpResultDetail, ed, (const FString&) const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 带上下文的错误诊断 - 当操作失败时输出包含上下文信息的错误日志\n"
        "* @param Context 上下文描述，用于标识错误发生的位置或场景\n"
        "* 仅在结果不是 Success 或 SameAndNotChange 时才会输出错误信息\n"
    )
    FArzNBTAttributeOperatorResultDetail_.Method("void edv() const", METHODPR_TRIVIAL(void, FNBTAttributeOpResultDetail, edvas, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 详细错误诊断 - 输出包含调用堆栈的详细错误信息\n"
        "* 仅在结果不是 Success 或 SameAndNotChange 时才会输出\n"
        "* 包含错误结果、错误消息以及 Angelscript 调用堆栈信息\n"
        "* 用于调试复杂的错误情况\n"
    )

    FArzNBTAttributeOperatorResultDetail_.Method("FString GetResultString() const",
                                                 METHODPR_TRIVIAL(FString, FNBTAttributeOpResultDetail, GetResultString, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取操作结果的描述字符串\n"
        "* @return 返回当前操作结果的可读描述，例如 'Success'、'Node not found' 等\n"
    )
    FArzNBTAttributeOperatorResultDetail_.Method("bool IsSuccess() const", METHODPR_TRIVIAL(bool, FNBTAttributeOpResultDetail, IsSuccess, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查操作是否成功\n"
        "* @return 如果操作结果为 Success 返回 true，否则返回 false\n"
    )

    FArzNBTAttributeOperatorResultDetail_.Method("bool IsOk() const", METHODPR_TRIVIAL(bool, FNBTAttributeOpResultDetail, IsOk, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查操作是否正常完成\n"
        "* @return 如果操作结果为 Success 或 SameAndNotChange（值相同无需修改）返回 true，否则返回 false\n"
        "* 比 IsSuccess() 更宽松，允许'无需修改'的情况也视为正常\n"
    )
    {
        FAngelscriptBinds::FNamespace ns("FNBTAttributeOpResultDetail");
    }
});

//因为AngelScript内需要区分float32和float64, 所以不能直接填写float, 所有float部分都是这样, 都需要用float32或者double, 不能直接使用float
AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_FArzNBTContainer(FAngelscriptBinds::EOrder::Late, [] {
    auto FArzNBTContainer_ = FAngelscriptBinds::ExistingClass("FNBTContainer");

    FArzNBTContainer_.Method("FNBTDataAccessor GetAccessor() const", METHODPR_TRIVIAL(FNBTDataAccessor, FNBTContainer, GetAccessor, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取容器的数据访问器\n"
        "* @return 返回一个数据访问器，用于读取和修改容器中的NBT数据\n"
        "* 访问器提供了安全的路径访问方式，支持链式调用来访问嵌套数据\n"
    )

    FArzNBTContainer_.Method("void Reset()", METHODPR_TRIVIAL(void, FNBTContainer, Reset, ()));
    SCRIPT_BIND_DOCUMENTATION(
        "* 重置容器到初始状态\n"
        "* 清空所有数据节点，重新初始化根节点\n"
        "* 重置后容器将只包含一个空的根Map节点\n"
    )

    FArzNBTContainer_.Method("void CopyFrom(const FNBTContainer& Other)", METHODPR_TRIVIAL(void, FNBTContainer, CopyFrom, (const FNBTContainer&)));
    SCRIPT_BIND_DOCUMENTATION(
        "* 从另一个容器深拷贝所有数据\n"
        "* @param Other 源容器，将被完整复制\n"
        "* 执行深拷贝，包括所有子节点和数据\n"
        "* 原有数据将被完全替换\n"
    )

    FArzNBTContainer_.Method("int32 GetNodeCount() const", METHODPR_TRIVIAL(int32, FNBTContainer, GetNodeCount, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取容器中活跃节点的总数\n"
        "* @return 返回当前容器中所有节点（包括Map、List和值节点）的数量\n"
    )

    FArzNBTContainer_.Method("FString ToString() const", METHODPR_TRIVIAL(FString, FNBTContainer, ToString, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 将容器转换为字符串表示\n"
        "* @return 返回容器内容的字符串表示，以树形结构显示所有数据\n"
        "* 适用于日志输出和调试\n"
    )

    FArzNBTContainer_.Method("FString ToDebugString() const", METHODPR_TRIVIAL(FString, FNBTContainer, ToDebugString, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 生成容器的调试字符串\n"
        "* @return 返回包含详细内部信息的调试字符串\n"
        "* 包含更多技术细节，如节点ID、版本号等信息\n"
    )

    FArzNBTContainer_.Method("bool ValidateIntegrity() const", METHODPR_TRIVIAL(bool, FNBTContainer, ValidateIntegrity, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 验证容器数据的完整性\n"
        "* @return 如果容器数据结构完整且有效返回 true，否则返回 false\n"
        "* 检查节点引用、循环引用、内存分配等潜在问题\n"
    )

    FArzNBTContainer_.Method("FArzNBTContainerStats GetStatistics() const", METHODPR_TRIVIAL(FArzNBTContainerStats, FNBTContainer, GetStatistics, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取容器的统计信息\n"
        "* @return 返回包含节点数量、类型分布、最大深度等统计数据的结构体\n"
        "* 用于性能分析和容器使用情况监控\n"
    )

    FArzNBTContainer_.Method("int32 GetContainerDataVersion() const", METHODPR_TRIVIAL(int32, FNBTContainer, GetContainerDataVersion, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取容器的数据版本号\n"
        "* @return 返回数据版本号，每次数据修改时递增\n"
        "* 用于检测数据变化和网络同步\n"
    )

    FArzNBTContainer_.Method("int32 GetContainerStructVersion() const", METHODPR_TRIVIAL(int32, FNBTContainer, GetContainerStructVersion, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取容器的结构版本号\n"
        "* @return 返回结构版本号，在添加/删除节点时递增\n"
        "* 结构版本变化表示容器的树形结构发生了改变\n"
    )
    FArzNBTContainer_.Method("bool IsContainerReplicated() const", METHODPR_TRIVIAL(bool, FNBTContainer, IsContainerReplicated, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查容器是否支持网络复制\n"
        "* @return 如果容器配置为网络复制模式返回 true，否则返回 false\n"
        "* 网络复制模式下容器会自动进行网络同步\n"
    )

    {
        FAngelscriptBinds::FNamespace ns("FNBTContainer");
    }
});

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_FArzNBTDataAccessor(FAngelscriptBinds::EOrder::Late, [] {
    auto FArzNBTDataAccessor_ = FAngelscriptBinds::ExistingClass("FNBTDataAccessor");

    FArzNBTDataAccessor_.Method("bool IsAccessorValid() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsAccessorValid, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查访问器是否有效\n"
        "* @return 如果访问器关联的容器仍然有效返回 true，否则返回 false\n"
        "* 当容器被销毁后，访问器将变为无效状态\n"
    )

    FArzNBTDataAccessor_.Method("TOptional<ENBTAttributeType> TryGetType() const",
                                METHODPR_TRIVIAL(TOptional<ENBTAttributeType>, FNBTDataAccessor, GetType, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试获取当前路径节点的数据类型\n"
        "* @return 如果路径有效返回节点类型，否则返回空的Optional\n"
        "* 可能的类型包括: Empty、Boolean、Int8-64、Float、Double、String、Vector、Map、List、Array等\n"
    )
    FArzNBTDataAccessor_.Method("FString TryGetTypeString() const",
                                METHODPR_TRIVIAL(FString, FNBTDataAccessor, GetTypeString, () const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取当前节点类型的字符串描述\n"
        "* @return 返回类型的可读字符串，如 'Map'、'List'、'Int32' 等\n"
        "* 如果路径无效返回 'Invalid'\n"
    )

    FArzNBTDataAccessor_.Method("bool IsDataChangedAndMark() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsDataChangedAndMark, ()const));
    SCRIPT_BIND_DOCUMENTATION(
       "* 检查自上次Mark()调用后数据是否发生变化, 并且自动Mark()\n"
       "* @return 如果数据已改变返回 true，否则返回 false\n"
       "* 用于监控特定节点的数据变化，整合了Mark, 不需要再次手动调用Mark()\n"
    )

    FArzNBTDataAccessor_.Method("bool IsSubtreeChangedAndMark() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsSubtreeChangedAndMark, ()const));
    SCRIPT_BIND_DOCUMENTATION(
       "* 检查子树是否变化\n"
       "* @return 如果数据已改变返回 true，否则返回 false\n"
       "* 用于监控特定节点的数据变化，整合了Mark, 不需要再次手动调用Mark()\n"
    )
    

    FArzNBTDataAccessor_.Method("bool IsContainerValid() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsContainerValid, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查访问器关联的容器是否有效\n"
        "* @return 如果容器存在且未被销毁返回 true，否则返回 false\n"
        "* 容器无效时不应继续使用该访问器\n"
    )
    FArzNBTDataAccessor_.Method("bool IsDataExists() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsDataExists, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前路径是否存在有效数据\n"
        "* @return 如果路径指向的节点存在返回 true，否则返回 false\n"
        "* 用于在访问数据前验证路径有效性\n"
    )

    FArzNBTDataAccessor_.Method("bool IsEqual(const FNBTDataAccessor& Other) const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsEqual, (const FNBTDataAccessor&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查是否和其他节点的值相同(深对比)\n"
        "* @return 相同 返回 true，否则返回 false\n"
    )

    FArzNBTDataAccessor_.Method("bool IsParent(const FNBTDataAccessor& Other) const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsParent, (const FNBTDataAccessor&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查目标节点是不是当前节点的某一个上级\n"
        "* @return 是上级返回 true，否则返回 false\n"
    )

    FArzNBTDataAccessor_.Method("bool IsChild(const FNBTDataAccessor& Other) const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsChild, (const FNBTDataAccessor&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查目标节点是不是当前节点的某一个下级\n"
        "* @return 是下级返回 true，否则返回 false\n"
    )
    
    FArzNBTDataAccessor_.Method("bool IsEmpty() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsEmpty, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为空节点\n"
        "* @return 如果节点类型为 Empty 返回 true，否则返回 false\n"
        "* 空节点可以被设置为任何其他类型\n"
    )
    
    FArzNBTDataAccessor_.Method("bool IsMap() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsMap, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为Map类型\n"
        "* @return 如果节点是Map（键值对容器）返回 true，否则返回 false\n"
        "* Map类型使用FName作为键来存储子节点\n"
    )
    FArzNBTDataAccessor_.Method("bool IsEmptyMap() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsEmptyMap, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为空Map类型\n"
        "* @return 如果节点是空Map（键值对容器）返回 true，否则返回 false\n"
        "* Map类型使用FName作为键来存储子节点\n"
    )

    FArzNBTDataAccessor_.Method("bool IsFilledMap() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsFilledMap, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为非空Map类型\n"
        "* @return 如果节点是Map（键值对容器）,并且非空, 返回 true，否则返回 false\n"
        "* Map类型使用FName作为键来存储子节点\n"
    )
    
    FArzNBTDataAccessor_.Method("bool IsList() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsList, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为List类型\n"
        "* @return 如果节点是List（有序列表）返回 true，否则返回 false\n"
        "* List类型使用整数索引访问子节点\n"
    )

    FArzNBTDataAccessor_.Method("bool IsEmptyList() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsEmptyList, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为空List类型\n"
        "* @return 如果节点是空List（有序列表）返回 true，否则返回 false\n"
        "* List类型使用整数索引访问子节点\n"
    )

    FArzNBTDataAccessor_.Method("bool IsFilledList() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsFilledList, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为非空List类型\n"
        "* @return 如果节点是List（有序列表）, 并且非空, 返回 true，否则返回 false\n"
        "* List类型使用整数索引访问子节点\n"
    )
    
    FArzNBTDataAccessor_.Method("bool IsArray() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsArray, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为数组类型\n"
        "* @return 如果节点是数值数组（如Int32Array、FloatArray等）返回 true，否则返回 false\n"
        "* 数组类型存储同类型的基础数值\n"
    )
    
    FArzNBTDataAccessor_.Method("bool IsBaseType() const", METHODPR_TRIVIAL(bool, FNBTDataAccessor, IsBaseType, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前节点是否为基础值类型\n"
        "* @return 如果节点是基础类型（非Map、List或Array）返回 true，否则返回 false\n"
        "* 基础类型包括: Boolean、Int、Float、String、Vector等单值类型\n"
    )

    FArzNBTDataAccessor_.Method("TOptional<int64> TryGetGenericInt() const", METHODPR_TRIVIAL(TOptional<int64>, FNBTDataAccessor, TryGetGenericInt, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试获取整数值（支持自动类型转换）\n"
        "* @return 如果节点包含整数类型数据返回转换后的int64值，否则返回空的Optional\n"
        "* 自动转换Int8/16/32/64到int64，方便统一处理整数\n"
    )

    FArzNBTDataAccessor_.Method("TOptional<double> TryGetGenericDouble() const",
                                METHODPR_TRIVIAL(TOptional<double>, FNBTDataAccessor, TryGetGenericDouble, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试获取浮点数值（支持自动类型转换）\n"
        "* @return 如果节点包含浮点类型数据返回转换后的double值，否则返回空的Optional\n"
        "* 自动转换Float/Double以及整数类型到double\n"
    )
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TrySetGenericInt(int64 __any_implicit_integer Value) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TrySetGenericInt, (int64)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试将一个整数值设置到当前路径的NBT节点。这是最安全的操作模式。\n"
        "* 此函数会自动处理类型转换，可以对Int8, Int16, Int32, Int64等任何整数类型的节点进行设置。\n"
        "* 操作仅在路径存在且节点为任意整数类型时才会成功。它不会自动创建路径。\n"
        "* @param Value 要设置的64位整数值，可以由脚本中的任意整型隐式转换。\n"
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n"
    )
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TrySetGenericDouble(double Value) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TrySetGenericDouble, (double)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试将一个浮点数值设置到当前路径的NBT节点。这是最安全的操作模式。\n"
        "* 此函数会自动处理类型转换，可以对Float或Double类型的节点进行设置。\n"
        "* 操作仅在路径存在且节点为Float或Double类型时才会成功。它不会自动创建路径。\n"
        "* @param Value 要设置的双精度浮点数值。\n"
        "* @return 返回一个操作结果对象(FNBTAttributeOpResultDetail)，包含了操作是否成功以及详细信息。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor go(int __any_implicit_integer Index) const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, go, (int32)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 导航到List中指定索引的子节点\n"
        "* @param Index 子节点的整数索引\n"
        "* @return 返回新的访问器，指向子节点路径\n"
        "* 用于List类型的索引访问\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor go(const FString& String) const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, go, (const FString&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 导航到Map中指定键名的子节点\n"
        "* @param String 子节点的字符串键名\n"
        "* @return 返回新的访问器，指向子节点路径\n"
        "* 字符串会转换为FName用于Map访问\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor go(FName Name) const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, go, (FName)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 导航到Map中指定键名的子节点\n"
        "* @param Name 子节点的FName键名\n"
        "* @return 返回新的访问器，指向子节点路径\n"
        "* 最高效的Map访问方式\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor clone() const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, clone, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 克隆当前访问器\n"
        "* @return 返回访问器的副本，指向相同路径\n"
        "* 用于保存访问器状态或并行访问\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor Getclone() const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, clone, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 克隆当前访问器（属性版本）\n"
        "* @return 返回访问器的副本，指向相同路径\n"
        "* 功能与clone()相同，提供属性风格的访问\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapHasKey(FName Key) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapHasKey, (FName)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 检查当前Map节点是否包含指定的键。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Key 要检查的键名。\n"
        "* @return 返回操作结果对象。如果键存在返回Success,如果节点不是Map类型返回NodeTypeMismatch。\n"
        "* @note 这是一个只读操作,不会修改数据结构。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapGetKeys(TArray<FName>& Keys) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapGetKeys, (TArray<FName>&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取Map节点的所有键名列表。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Keys [输出参数] 用于接收所有键名的数组,会先清空数组再填充。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是Map类型返回NodeTypeMismatch。\n"
        "* @note 这是一个只读操作,不会修改数据结构。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapGetSize(int32& Size) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapGetSize, (int32&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取Map节点的子节点数量。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Size [输出参数] 用于接收Map中键值对的数量。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是Map类型返回NodeTypeMismatch。\n"
        "* @note 这是一个只读操作,不会修改数据结构。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapRemoveSubNode(FName Key) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapRemoveSubNode, (FName)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 从Map节点中移除指定键的子节点。\n"
        "* 当前节点必须是Map类型才能执行此操作。会递归删除子节点及其所有子孙节点。\n"
        "* @param Key 要移除的子节点的键名。\n"
        "* @return 返回操作结果对象。成功返回Success,键不存在返回NotFoundSubNode,节点不是Map返回NodeTypeMismatch。\n"
        "* @warning 此操作会删除子节点及其所有子孙节点,请谨慎使用!\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapClear() const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapClear, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 清空Map节点的所有子节点。\n"
        "* 当前节点必须是Map类型才能执行此操作。会递归删除所有子节点及其子孙节点。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是Map类型返回NodeTypeMismatch。\n"
        "* @warning 此操作会删除Map中的所有数据,请谨慎使用!\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MakeAccessorFromMap(TArray<FNBTDataAccessor>& Accessors) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MakeAccessorFromMap,
                                                 (TArray<FNBTDataAccessor>&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 从Map节点创建所有子节点的访问器数组。\n"
        "* 当前节点必须是Map类型才能执行此操作。为每个键值对创建一个访问器。\n"
        "* @param Accessors [输出参数] 用于接收所有子节点访问器的数组,会先清空数组再填充。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是Map类型返回NodeTypeMismatch。\n"
        "* @note 返回的访问器可以用来访问和修改对应的子节点。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor MapMakeAccessorByCondition(ENBTSearchCondition Condition) const",
                               METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, MapMakeAccessorByCondition, (ENBTSearchCondition)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在Map中按条件搜索符合条件的节点, 如果找到就返回, 否则返回空访问器。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Condition 搜索条件。\n"
        "* @return 无搜索结果或者搜索失败都会返回空访问器。\n"
    )

    
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapMakeAccessorsByCondition(TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition Condition) const",
                              METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapMakeAccessorsByCondition, (TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在Map中按条件搜索符合条件的节点集合, 如果找到就返回, 否则返回空集合。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Accessors 返回集合。\n"
        "* @param Condition 搜索条件。\n"
        "* @return 无搜索结果或者搜索失败都会返回空集合。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor MapMakeAccessorIfEqual(const FNBTDataAccessor& Accessor) const",
                               METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, MapMakeAccessorIfEqual, (const FNBTDataAccessor& Accessor)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在Map中搜索和提供的数据相等的节点, 如果找到就返回, 否则返回空访问器。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param FNBTDataAccessor 其他数据源。\n"
        "* @return 无搜索结果或者搜索失败都会返回空访问器。\n"
    )
    
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MapMakeAccessorsIfEqual(TArray<FNBTDataAccessor>& Accessors, const FNBTDataAccessor& Accessor) const",
                              METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MapMakeAccessorsIfEqual, (TArray<FNBTDataAccessor>& Accessors, const FNBTDataAccessor& Accessor)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在Map中按条件搜索和提供访问器数据相等的节点集合, 如果找到就返回, 否则返回空集合。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Accessors 返回集合。\n"
        "* @param FNBTDataAccessor 其他数据源。\n"
        "* @return 无搜索结果或者搜索失败都会返回空集合。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor MapMakeAccessorByParameter(const FNBTSearchParameter& Param) const",
                              METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, MapMakeAccessorByParameter, (const FNBTSearchParameter&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在Map中按参数搜索, 如果找到就返回, 否则返回空访问器。\n"
        "* 当前节点必须是Map类型才能执行此操作。\n"
        "* @param Param 搜索参数。\n"
        "* @return 无搜索结果或者搜索失败都会返回空访问器。\n"
    )

    FArzNBTDataAccessor_.Method("TArray<FNBTDataAccessor> MakeAccessorFromMapNow() const",
                                METHODPR_TRIVIAL(TArray<FNBTDataAccessor>, FNBTDataAccessor, MakeAccessorFromMapNow, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 立即从Map节点创建并返回所有子节点的访问器数组。\n"
        "* 当前节点必须是Map类型才能执行此操作。为每个键值对创建一个访问器。\n"
        "* @return 返回包含所有子节点访问器的数组。如果节点不是Map类型,返回空数组。\n"
        "* @note 这是MakeAccessorFromMap的便捷版本,直接返回数组而不是通过输出参数。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail ListGetSize(int32& Size) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, ListGetSize, (int32&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取List节点的元素数量。\n"
        "* 当前节点必须是List类型才能执行此操作。\n"
        "* @param Size [输出参数] 用于接收List中元素的数量。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是List类型返回NodeTypeMismatch。\n"
        "* @note 这是一个只读操作,不会修改数据结构。\n"
    )

    FArzNBTDataAccessor_.Method("TOptional<int32> ListGetCurrentIndex() const",
                               METHODPR_TRIVIAL(TOptional<int32>, FNBTDataAccessor, ListGetCurrentIndex, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 如果当前Accessor是某个List的一个根成员, 那么返回所在索引。\n"
        "* 当前节点必须是List的根成员才能成功返回值。\n"
        "* @return 返回一个int32索引, 否则返回空。\n"
        "* @note 这是一个只读操作,不会修改数据结构。\n"
    )

    FArzNBTDataAccessor_.Method("TOptional<int32> ListGetLastParentIndex() const",
                               METHODPR_TRIVIAL(TOptional<int32>, FNBTDataAccessor, ListGetLastParentIndex, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 路径向上寻找最近的一个List, 如果成功找到, 那么返回那个List根成员的在List中的索引(Index)。\n"
        "* 路径中必须存在List才能执行成功。\n"
        "* @return 返回一个int32索引, 否则返回空。\n"
        "* @note 这是一个只读操作,不会修改数据结构。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail ListRemoveSubNode(int32 Index, bool bSwapRemove = false) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, ListRemoveSubNode, (int32, bool)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 从List节点中移除指定索引的元素。\n"
        "* 当前节点必须是List类型才能执行此操作。会递归删除该元素及其所有子孙节点。\n"
        "* @param Index 要移除的元素索引(从0开始)。\n"
        "* @param bSwapRemove 是否使用交换移除(将最后一个元素移到被删除位置,更高效但会改变顺序)。默认为false。\n"
        "* @return 返回操作结果对象。成功返回Success,索引越界返回NotFoundSubNode,节点不是List返回NodeTypeMismatch。\n"
        "* @warning 此操作会删除元素及其所有子孙节点,并可能改变后续元素的索引!\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail ListClear() const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, ListClear, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 清空List节点的所有元素。\n"
        "* 当前节点必须是List类型才能执行此操作。会递归删除所有元素及其子孙节点。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是List类型返回NodeTypeMismatch。\n"
        "* @warning 此操作会删除List中的所有数据,请谨慎使用!\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor ListAddSubNode() const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, ListAddSubNode, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List节点末尾添加一个新的空子节点。\n"
        "* 当前节点必须是List类型才能执行此操作。新节点初始化为Empty类型。\n"
        "* @return 返回新创建子节点的访问器。如果操作失败(节点不是List或分配失败),返回无效的访问器。\n"
        "* @note 新节点添加在List末尾,可以通过返回的访问器进行进一步操作。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor ListInsertSubNode(int32 Index) const",
                                METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, ListInsertSubNode, (int32)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List节点的指定位置插入一个新的空子节点。\n"
        "* 当前节点必须是List类型才能执行此操作。新节点初始化为Empty类型。\n"
        "* @param Index 插入位置的索引(从0开始)。如果超出范围,会在末尾添加。\n"
        "* @return 返回新创建子节点的访问器。如果操作失败(节点不是List或分配失败),返回无效的访问器。\n"
        "* @note 插入操作会导致该位置及之后的元素索引增加1。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor ListMakeAccessorByCondition(ENBTSearchCondition Condition) const",
                               METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, ListMakeAccessorByCondition, (ENBTSearchCondition)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List中按条件搜索符合条件的节点, 如果找到就返回, 否则返回空访问器。\n"
        "* 当前节点必须是List类型才能执行此操作。\n"
        "* @param Condition 搜索条件。\n"
        "* @return 无搜索结果或者搜索失败都会返回空访问器。\n"
    )
    
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail ListMakeAccessorsByCondition(TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition Condition) const",
                              METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, ListMakeAccessorsByCondition, (TArray<FNBTDataAccessor>& Accessors, ENBTSearchCondition)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List中按条件搜索符合条件的节点集合, 如果找到就返回, 否则返回空集合。\n"
        "* 当前节点必须是List类型才能执行此操作。\n"
        "* @param Accessors 返回集合。\n"
        "* @param Condition 搜索条件。\n"
        "* @return 无搜索结果或者搜索失败都会返回空集合。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor ListMakeAccessorIfEqual(const FNBTDataAccessor& Accessor) const",
                               METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, ListMakeAccessorIfEqual, (const FNBTDataAccessor& Accessor)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List中搜索和提供的数据相等的节点, 如果找到就返回, 否则返回空访问器。\n"
        "* 当前节点必须是List类型才能执行此操作。\n"
        "* @param FNBTDataAccessor 其他数据源。\n"
        "* @return 无搜索结果或者搜索失败都会返回空访问器。\n"
    )
    
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail ListMakeAccessorsIfEqual(TArray<FNBTDataAccessor>& Accessors, const FNBTDataAccessor& Accessor) const",
                              METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, ListMakeAccessorsIfEqual, (TArray<FNBTDataAccessor>& Accessors, const FNBTDataAccessor& Accessor)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List中按条件搜索和提供访问器数据相等的节点集合, 如果找到就返回, 否则返回空集合。\n"
        "* 当前节点必须是List类型才能执行此操作。\n"
        "* @param Accessors 返回集合。\n"
        "* @param FNBTDataAccessor 其他数据源。\n"
        "* @return 无搜索结果或者搜索失败都会返回空集合。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor ListMakeAccessorByParameter(const FNBTSearchParameter& Param) const",
                              METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, ListMakeAccessorByParameter, (const FNBTSearchParameter&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 在List中按参数搜索, 如果找到就返回, 否则返回空访问器。\n"
        "* 当前节点必须是List类型才能执行此操作。\n"
        "* @param Param 搜索参数。\n"
        "* @return 无搜索结果或者搜索失败都会返回空访问器。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail MakeAccessorFromList(TArray<FNBTDataAccessor>& Accessors) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, MakeAccessorFromList,
                                                 (TArray<FNBTDataAccessor>&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 从List节点创建所有元素的访问器数组。\n"
        "* 当前节点必须是List类型才能执行此操作。为每个元素创建一个访问器。\n"
        "* @param Accessors [输出参数] 用于接收所有元素访问器的数组,会先清空数组再填充。\n"
        "* @return 返回操作结果对象。成功返回Success,如果节点不是List类型返回NodeTypeMismatch。\n"
        "* @note 返回的访问器按照List中的顺序排列,可以用来访问和修改对应的元素。\n"
    )

    FArzNBTDataAccessor_.Method("TArray<FNBTDataAccessor> MakeAccessorFromListNow() const",
                                METHODPR_TRIVIAL(TArray<FNBTDataAccessor>, FNBTDataAccessor, MakeAccessorFromListNow, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 立即从List节点创建并返回所有元素的访问器数组。\n"
        "* 当前节点必须是List类型才能执行此操作。为每个元素创建一个访问器。\n"
        "* @return 返回包含所有元素访问器的数组。如果节点不是List类型,返回空数组。\n"
        "* @note 这是MakeAccessorFromList的便捷版本,直接返回数组而不是通过输出参数。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor EnsureList() const allow_discard", METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, EnsureList, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 确保当前节点是List类型。\n"
        "* 如果当前节点是空节点(Empty)，会将其转换为List类型。如果节点已经是List类型则直接返回。\n"
        "* @return 返回指向当前节点的访问器，可以链式调用进行后续操作。\n"
        "* @note 如果节点是其他类型(如Map、基础类型等)，操作将失败并保持原类型不变。\n"
        "* @note 这个函数适用于初始化时确保节点为List结构，常用于MOD初始化数据。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTDataAccessor EnsureMap() const allow_discard", METHODPR_TRIVIAL(FNBTDataAccessor, FNBTDataAccessor, EnsureMap, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 确保当前节点是Map类型。\n"
        "* 如果当前节点是空节点(Empty)，会将其转换为Map类型。如果节点已经是Map类型则直接返回。\n"
        "* @return 返回指向当前节点的访问器，可以链式调用进行后续操作。\n"
        "* @note 如果节点是其他类型(如List、基础类型等)，操作将失败并保持原类型不变。\n"
        "* @note 这个函数适用于初始化时确保节点为Map结构，常用于创建键值对存储。\n"
    )

    FArzNBTDataAccessor_.Method("int Remove() const", METHODPR_TRIVIAL(int, FNBTDataAccessor, Remove, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 删除当前路径指向的节点及其所有子节点。\n"
        "* 递归删除当前节点及其包含的所有子节点，释放占用的内存。\n"
        "* @return 返回被删除的节点总数(包括当前节点和所有子节点)。如果节点不存在返回0。\n"
        "* @note 删除后，指向该节点的访问器将变为无效，继续使用会返回NotFoundNode错误。\n"
        "* @warning 此操作不可逆，删除的数据无法恢复，请谨慎使用。\n"
    )

    FArzNBTDataAccessor_.Method("FString ToString(bool bShowVersion = false) const", METHODPR_TRIVIAL(FString, FNBTDataAccessor, ToString, (bool)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 将当前节点及其子节点转换为字符串表示。\n"
        "* 递归遍历当前节点，生成树形结构的字符串表示，便于调试和查看数据结构。\n"
        "* @param bShowVersion [可选] 是否在输出中包含节点的版本信息，默认为false。\n"
        "* @return 返回格式化的字符串，展示节点的层级结构和数据内容。\n"
        "* @note 对于大型数据结构，此操作可能消耗较多性能，建议仅在调试时使用。\n"
    )

    FArzNBTDataAccessor_.Method("FString GetPath() const", METHODPR_TRIVIAL(FString, FNBTDataAccessor, GetPath, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取当前访问器的完整路径字符串, 前提是这个路径是有效的。\n"
        "* 返回从根节点到当前节点的完整路径，使用点号(->)分隔各级节点。\n"
        "* @return 返回路径字符串，如\"Root -> Player -> Stats -> Health\"。如果是根节点返回\"Root\"。\n"
        "* @note 路径中的List索引会以[index]形式表示，如\"Root -> Container -> [2] -> Name\"。\n"
    )

    FArzNBTDataAccessor_.Method("FString GetPreviewPath() const", METHODPR_TRIVIAL(FString, FNBTDataAccessor, GetPreviewPath, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 获取当前访问器的指向路径, 无论这个路径是否有效, 但只有在容器有效的情况下才会返回有意义的路径。\n"
        "* 返回从根节点到当前节点的完整路径，使用点号(->)分隔各级节点。\n"
        "* @return 返回路径字符串，如\"Root -> Player -> Stats -> Health\"。如果是根节点返回\"Root\"。\n"
        "* @note 路径中的List索引会以[index]形式表示，如\"Root -> Container -> [2] -> Name\"。\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TryCopyFrom(const FNBTDataAccessor& Source) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TryCopyFrom, (const FNBTDataAccessor&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试从源访问器复制数据到当前位置。\n"
        "* 要求当前路径和源路径都必须存在且有效，才能执行复制操作。\n"
        "* @param Source 源数据访问器，提供要复制的数据。\n"
        "* @return 返回操作结果对象：\n"
        "*   - Success: 复制成功\n"
        "*   - NotFoundNode: 当前路径不存在\n"
        "*   - 源路径无效时返回相应的源错误\n"
        "*   - AllocateFailed: 分配新节点失败（复合类型时）\n"
        "* @note 对于基础类型，执行值复制；对于复合类型（Map/List），执行深拷贝。\n"
        "* @note 如果源和目标是同一个节点，直接返回Success。\n"
        "* @note 复制会完全覆盖当前节点的内容，包括删除原有的所有子节点。\n"
        "* @warning 如果当前路径不存在，操作会失败。需要创建路径请使用 EnsureAndCopyFrom。\n"
    )
    
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TrySwap(const FNBTDataAccessor& Source) const",
                                   METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TrySwap, (const FNBTDataAccessor&)const));

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail EnsureAndCopyFrom(const FNBTDataAccessor& Source) const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, EnsureAndCopyFrom, (const FNBTDataAccessor&)const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 确保路径存在并从源访问器复制数据。\n"
        "* 如果当前路径不存在，会尝试创建路径（EnsureCreate模式），然后执行复制。\n"
        "* @param Source 源数据访问器，提供要复制的数据。\n"
        "* @return 返回操作结果对象：\n"
        "*   - Success: 复制成功\n"
        "*   - 源路径无效时返回相应的源错误\n"
        "*   - NodeTypeMismatch: 路径中遇到类型不匹配（如期望Map但实际是其他类型）\n"
        "*   - PermissionDenied: 无法创建路径（如List索引不连续）\n"
        "*   - AllocateFailed: 分配新节点失败\n"
        "* @note 路径创建规则：\n"
        "*   - Map键：如果不存在会自动创建\n"
        "*   - List索引：必须是有效索引，不能通过索引创建新元素\n"
        "*   - 空节点：会根据路径需要转换为Map或List\n"
        "* @note 对于基础类型，执行值复制；对于复合类型（Map/List），执行深拷贝。\n"
        "* @note 适用于初始化场景，需要确保目标路径存在并复制数据。\n"
        "* @example\n"
        "*   // 正确用法：通过Map路径\n"
        "*   target[\"config\"][\"settings\"].EnsureAndCopyFrom(source);\n"
        "*   \n"
        "*   // 错误用法：List索引不存在\n"
        "*   target[\"list\"][999].EnsureAndCopyFrom(source); // 失败：List索引999不存在\n"
    )

    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail TrySetEmpty() const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, TrySetEmpty, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 尝试将当前路径节点设置为空(Empty)类型。\n"
        "* 仅当节点存在且类型为Empty或基础类型时才会成功。\n"
        "* @return 返回操作结果。\n"
    );
    FArzNBTDataAccessor_.Method("FNBTAttributeOpResultDetail EnsureAndSetEmpty() const",
                                METHODPR_TRIVIAL(FNBTAttributeOpResultDetail, FNBTDataAccessor, EnsureAndSetEmpty, ()const));
    SCRIPT_BIND_DOCUMENTATION(
        "* 确保路径存在并将其设置为空(Empty)类型。\n"
        "* 如果路径不存在，会自动创建。如果节点已存在但类型不匹配，操作会失败。\n"
        "* @return 返回操作结果。\n"
    );
    

    // =================================================================================
    // 使用宏绑定基础类型
    // =================================================================================
    //BIND_NBT_ACCESSOR_BASE_TYPE(TypeName, CppType, AsType, CppParamType, AsParamType, DocName, DocValue, ...)
    BIND_NBT_ACCESSOR_BASE_TYPE(Bool, bool, bool, bool, bool, "布尔值(bool)", "布尔值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Int8, int8, int8, int8, int8, "8位整数(int8)", "8位整数值", __any_implicit_integer);
    BIND_NBT_ACCESSOR_BASE_TYPE(Int16, int16, int16, int16, int16, "16位整数(int16)", "16位整数值", __any_implicit_integer);
    BIND_NBT_ACCESSOR_BASE_TYPE(Int32, int32, int32, int32, int32, "32位整数(int32)", "32位整数值", __any_implicit_integer);
    BIND_NBT_ACCESSOR_BASE_TYPE(Int64, int64, int64, int64, int64, "64位整数(int64)", "64位整数值", __any_implicit_integer);
    BIND_NBT_ACCESSOR_BASE_TYPE(Float, float, float32, float, float32, "单精度浮点数(float)", "单精度浮点数值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Double, double, double, double, double, "双精度浮点数(double)", "双精度浮点数值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Name, FName, FName, FName, FName, "名称(FName)", "FName值");
    BIND_NBT_ACCESSOR_BASE_TYPE(String, FString, FString, const FString&, const FString&, "字符串(FString)", "FString值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Color, FColor, FColor, FColor, FColor, "颜色(FColor)", "FColor值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Guid, FGuid, FGuid, FGuid, FGuid, "唯一识别码(FGuid)", "FGuid值");
    
    BIND_NBT_ACCESSOR_BASE_TYPE(SoftClassPath, FSoftClassPath, FSoftClassPath, const FSoftClassPath&, const FSoftClassPath&, "类型引用(FSoftClassPath)",
                                "FSoftClassPath值");
    BIND_NBT_ACCESSOR_BASE_TYPE(SoftObjectPath, FSoftObjectPath, FSoftObjectPath, const FSoftObjectPath&, const FSoftClassPath&, "类型引用(FSoftObjectPath)",
                                "FSoftObjectPath值");
    
    BIND_NBT_ACCESSOR_BASE_TYPE(DateTime, FDateTime, FDateTime, FDateTime, FDateTime, "类型引用(FDateTime)", "FDateTime值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Rotator, FRotator, FRotator, FRotator, FRotator, "三维旋转(FRotator)", "FRotator值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Vector2D, FVector2D, FVector2D, FVector2D, FVector2D, "二维向量(FVector2D)", "FVector2D值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Vector, FVector, FVector, FVector, FVector, "三维向量(FVector)", "FVector值");
    BIND_NBT_ACCESSOR_BASE_TYPE(IntVector2, FIntVector2, FIntVector2, FIntVector2, FIntVector2, "二维整形向量(FIntVector2)", "FIntVector2值");
    BIND_NBT_ACCESSOR_BASE_TYPE(IntVector, FIntVector, FIntVector, FIntVector, FIntVector, "三维整形向量(FIntVector)", "FIntVector值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Int64Vector2, FInt64Vector2, FInt64Vector2, FInt64Vector2, FInt64Vector2, "二维长整形向量(FInt64Vector2)", "FInt64Vector2值");
    BIND_NBT_ACCESSOR_BASE_TYPE(Int64Vector, FInt64Vector, FInt64Vector, FInt64Vector, FInt64Vector, "三维长整形向量(FInt64Vector)", "FInt64Vector值");

    BIND_NBT_ACCESSOR_ARRAY_TYPE(Int8, int8, int8, "8位整数数组(int8[])", "8位整数数组");
    BIND_NBT_ACCESSOR_ARRAY_TYPE(Int16, int16, int16, "16位整数数组(int16[])", "16位整数数组");
    BIND_NBT_ACCESSOR_ARRAY_TYPE(Int32, int32, int32, "32位整数数组(int32[])", "32位整数数组");
    BIND_NBT_ACCESSOR_ARRAY_TYPE(Int64, int64, int64, "64位整数数组(int64[])", "64位整数数组");
    BIND_NBT_ACCESSOR_ARRAY_TYPE(Float, float, float32, "单精度浮点数组(float[])", "单精度浮点数组");
    BIND_NBT_ACCESSOR_ARRAY_TYPE(Double, double, double, "双精度浮点数组(double[])", "双精度浮点数组");

    {
        FAngelscriptBinds::FNamespace ns("FNBTDataAccessor");
    }
});