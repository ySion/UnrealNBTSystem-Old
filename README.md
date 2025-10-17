<!-- 目前这个库已经停止维护, 请使用本作者的 DaxSystem, 功能更加全面和强大 -->

# NBTSystem (UE5 + Angelscript 绑定)

> 重要提示：目前本库已停止维护，请迁移到作者的 DaxSystem（功能更全面、性能与生态更佳）。

本项目提供一个在虚幻引擎 5 下运行的轻量级 NBT（类 NBT/树形键值结构）系统，并针对 Angelscript 做了完整的脚本绑定。核心目标：
- 在脚本中以“路径访问器”的方式安全地读写树形数据。
- 提供三种写入模式（TrySet / EnsureAndSet / OverrideTo）覆盖初始化、日常修改、强制覆盖等场景。
- 内置变更版本号与子树版本追踪，便于脏检查、网络复制与调试。
- 覆盖 Map / List / 基础类型 / 数组类型，含丰富的查询与遍历 API。

本文档将从概念解释、功能总览、优缺点分析，到详细 API 与大量 Angelscript 示例，帮助你快速上手与安全使用。

## 概念与数据模型

- 容器(`FNBTContainer`)：一棵以 Map 为根的树。用于保存完整的 NBT 数据，并支持序列化/网络复制/统计。
- 节点类型(`ENBTAttributeType`)：
  - 基础类型：`Boolean / Int8/16/32/64 / Float / Double / Name / String / Color / Guid / DateTime / Rotator / Vector2D / Vector / IntVector2 / IntVector / Int64Vector2 / Int64Vector`
  - 数组类型：`ArrayInt8 / ArrayInt16 / ArrayInt32 / ArrayInt64 / ArrayFloat32 / ArrayDouble`
  - 复合类型：`Map / List`
- 访问器(`FNBTDataAccessor`)：对容器中某一路径的“视图”。提供只读/确保创建/强制覆盖三种解析模式，暴露大量类型安全 API。
- 版本追踪：
  - 容器版本：`ContainerDataVersion`（数据变化递增）与 `ContainerStructVersion`（结构变化递增）。
  - 节点版本：单节点版本与“子树版本”(Subtree) 用于高效脏检查。

数据访问“路径”由 `FName`（Map 键）与 `int32`（List 索引）依次组成，例如：
- `Root -> Player -> Stats -> Health`
- `Root -> Inventory -> [2] -> Name`

访问器支持链式导航与缓存加速，能在大量读写场景中保持良好性能。
## 功能总览

- 类型安全的读取：`TryGetX()` / `TryGetGenericInt()` / `TryGetGenericDouble()`。
- 三种写入模式：
  - `TrySetX()` 最安全，仅在路径存在且类型匹配时生效。
  - `EnsureAndSetX()` 初始化利器，自动创建路径，节点为 Empty/匹配类型时赋值。
  - `OverrideToX()` 强覆盖，危险操作，直接改类型并赋值。
- Map 操作：查键、取键列表、大小、删除、清空、批量生成访问器、按条件/参数/等值搜索。
- List 操作：获取大小、增删插、清空、批量生成访问器、按条件/参数/等值搜索、定位索引。
- 路径导航：`go(FName)` / `go(int)` / `go(FString)` 与 `operator[]` 友好语法。
- 变更检测：`IsDataChangedAndMark()` / `IsSubtreeChangedAndMark()` / `Mark()`。
- 高级：拷贝与交换 `TryCopyFrom()`/`EnsureAndCopyFrom()`/`TrySwap()`，节点删除 `Remove()`。
- 可视化输出：`ToString(bShowVersion)` / `GetPath()` / `GetPreviewPath()`。
- 容器工具：重置、深拷贝、完整性校验、统计信息、网络复制 Delta 同步。
## 优缺点

- 优点
  - API 语义清晰，脚本侧有详尽文档绑定（Angelscript）。
  - 三段式写入模型覆盖初始化/运行时/工具修复等不同阶段需求。
  - 版本系统可做低成本脏检查，利于 UI 刷新与网络同步。
  - 支持多 UE 原生类型与数值数组、向量/旋转等复杂类型。
  - Map/List 搜索与批量访问器生成，有利于表驱动玩法与关卡脚本。
  - 支持容器全量/增量序列化，含网络 Delta 同步实现。
- 缺点
  - List 不允许通过索引“越界创建”节点，必须使用 `ListAddSubNode()/ListInsertSubNode()`。
  - `OverrideToX()` 为破坏性操作，需极其谨慎（会改变现有结构/类型）。
  - 设计上最多约 65534 个节点（分块分配器上限）；超大树需谨慎规划。
  - 浮点比较采用“近似相等”，极端精度场景需自行处理。
  - 本库已停止维护，如需长期演进请迁移 DaxSystem。
## 快速上手（Angelscript）

```angelscript
// 典型：角色组件中获取容器访问器
UNBTComponent@ nbt = Cast<UNBTComponent>(Actor.FindComponentByClass(UNBTComponent::StaticClass()));
FNBTDataAccessor root = nbt.GetAccessor();

// 初始化玩家属性（使用 EnsureAndSet，安全初始化，不覆盖已有不同类型）
root.go("Player").EnsureMap(); // 确保为Map
root.go("Player").go("Stats").EnsureMap();
root.go("Player").go("Stats").EnsureAndSetInt32(100);        // 错：需要落在具体键上
root.go("Player").go("Stats").go("Health").EnsureAndSetInt32(100); // 对

// 读取与修改（TrySet 最安全）
auto hp = root.go("Player").go("Stats").go("Health").TryGetInt32();
if (hp.IsSet()) {
    int32 newHp = hp.GetValue() - 10;
    root.go("Player").go("Stats").go("Health").TrySetInt32(newHp).ed();
}

// 强制覆盖（危险：会改变节点类型）
root.go("Debug").OverrideToString("force override").edv();
```
## 容器（FNBTContainer）常用 API

- `GetAccessor()`：获取根访问器。
- `Reset()`：清空容器（根重置为 Map）。
- `CopyFrom(Other)`：深拷贝另一个容器。
- `ToString()` / `ToDebugString()`：输出调试字符串。
- `ValidateIntegrity()`：结构校验。
- `GetStatistics()`：统计信息（节点数/类型分布/最大深度等）。
- `GetContainerDataVersion()` / `GetContainerStructVersion()`：版本号。

```angelscript
FNBTContainer@ c = FNBTContainer();
FNBTDataAccessor root = c.GetAccessor();
root.EnsureMap();
root.go("Config").go("Title").EnsureAndSetString("Hello NBT");

// 输出
string s = c.ToString();
string sd = c.ToDebugString();
// 校验
bool ok = c.ValidateIntegrity();
// 统计
FArzNBTContainerStats stats = c.GetStatistics();
```
## 访问器（FNBTDataAccessor）基础能力

- 有效性/存在性
  - `IsAccessorValid()` / `IsContainerValid()` / `IsDataExists()`
- 类型查询
  - `TryGetType()` / `TryGetTypeString()`；`IsEmpty()/IsMap()/IsList()/IsArray()/IsBaseType()`
  - `IsEmptyMap()/IsFilledMap()/IsEmptyList()/IsFilledList()`
- 变更检测
  - `IsDataChangedAndMark()`：当节点数据变化时返回 true，并自动 Mark。
  - `IsSubtreeChangedAndMark()`：当子树任一节点变化时返回 true，并自动 Mark。
  - `Mark()` / `MarkSubtree()`：手动记录版本快照。

```angelscript
FNBTDataAccessor acc = nbt.GetAccessor().go("Player").go("Stats").go("Health");

if (acc.IsDataChangedAndMark()) {
    Print("Health changed: " + acc.ToString());
}

FNBTDataAccessor p = nbt.GetAccessor().go("Player");
if (p.IsSubtreeChangedAndMark()) {
    Print("Player subtree changed");
}
```
## 路径导航与可视化

- 导航：
  - `go(FName)` / `go(int)` / `go(FString)`；或 `operator[]` 简写。
  - `clone()`：克隆访问器。
  - `GetPath()`：解析后显示“有效路径”，`GetPreviewPath()`：仅根据当前 Path 拼接（容器有效时有意义）。

```angelscript
auto inv2 = root["Inventory"][2]; // 等价于 go("Inventory").go(2)
Print(inv2.GetPreviewPath());
string p = inv2.GetPath();  // 若路径无效返回 "$Node Not Exist$"

FNBTDataAccessor copy = inv2.clone();
```
## 读取 API（TryGet 与泛型）

- 基础类型：`TryGetBool/Int8/Int16/Int32/Int64/Float(Double)/Name/String/Color/Guid/...`
- 数组类型：`TryGetInt32Array()/TryGetFloatArray()/...`（若失败返回空数组）。
- 泛型读取（自动转换）：
  - `TryGetGenericInt()`：在任意 Int/Boolean 上取 `int64`（bool 按 0/1）。
  - `TryGetGenericDouble()`：在 Float/Double/整数上取 `double`。

```angelscript
auto a = root.go("Stats").go("Level").TryGetInt32();
auto gi = root.go("Any").TryGetGenericInt();   // 可能来自 Int8/16/32/64/Boolean
auto gd = root.go("Any").TryGetGenericDouble();

TArray<int32> xs = root.go("Curve").TryGetInt32Array(); // 失败则为空数组
```
## 写入 API（三种模式）

- TrySet（最安全）：要求路径存在 + 类型匹配。
- EnsureAndSet（初始化）：自动创建 Map 路径；最终节点为 Empty/同类型时赋值。
- OverrideTo（强制覆盖）：改类型+赋值，危险，可能破坏结构。
- 通用写入：`TrySetGenericInt(int64)` / `TrySetGenericDouble(double)`。
- 空值相关：`TrySetEmpty()` / `EnsureAndSetEmpty()`（List/Map 将转为清空语义）。

```angelscript
// TrySet：日常运行时更新
root["Player"]["Stats"]["Health"].TrySetInt32(90).ed();

// EnsureAndSet：初始化（创建路径，若为空则赋值）
root["Player"]["Stats"]["Mana"].EnsureAndSetInt32(100).ed();

// OverrideTo：危险（无条件覆盖类型）
root["Debug"]["Flag"].OverrideToBool(true).edv();

// 泛型写入
root["Player"]["Stats"]["Health"].TrySetGenericInt(123).ed();
```
## Map 操作

```angelscript
FNBTDataAccessor m = root.go("Players").EnsureMap();

// 键存在检查
m.MapHasKey(FName("Alice")).ed();

// 获取键列表与大小
TArray<FName> keys; m.MapGetKeys(keys).ed();
int size = 0; m.MapGetSize(size).ed();

// 删除与清空
m.MapRemoveSubNode(FName("Alice")).ed();
m.MapClear().ed();

// 批量访问器
TArray<FNBTDataAccessor> children;
m.MakeAccessorFromMap(children).ed();
TArray<FNBTDataAccessor> now = m.MakeAccessorFromMapNow();

// 条件搜索（IfEmpty / IfEmptyMap / IfEmptyList）
FNBTDataAccessor emptySlot = m.MapMakeAccessorByCondition(ENBTSearchCondition::IfEmpty);

// 参数搜索（比较运算 + 可选子键）
FNBTSearchParameter p; p.Op = ENBTCompareOp::Contains; p.ValueType = ENBTAttributeType::String; p.Value = "boss";
FNBTDataAccessor hit = m.MapMakeAccessorByParameter(p);

// 等值搜索（深比较）
FNBTDataAccessor sample = root.go("Template");
FNBTDataAccessor found = m.MapMakeAccessorIfEqual(sample);
```
## List 操作

```angelscript
FNBTDataAccessor l = root.go("Inventory").EnsureList();

// 添加/插入元素（返回新元素的访问器）
FNBTDataAccessor item = l.ListAddSubNode();
FNBTDataAccessor item0 = l.ListInsertSubNode(0);

// 获取大小、索引定位
int32 n = 0; l.ListGetSize(n).ed();
auto idx = item0.ListGetCurrentIndex();
auto parentIdx = item0.ListGetLastParentIndex();

// 删除与清空
l.ListRemoveSubNode(3, /*bSwapRemove=*/false).ed();
l.ListClear().ed();

// 批量访问器
TArray<FNBTDataAccessor> xs; l.MakeAccessorFromList(xs).ed();
TArray<FNBTDataAccessor> ys = l.MakeAccessorFromListNow();

// 条件/参数/等值搜索
FNBTDataAccessor firstEmpty = l.ListMakeAccessorByCondition(ENBTSearchCondition::IfEmpty);
FNBTSearchParameter p; p.Op = ENBTCompareOp::Eq; p.ValueType = ENBTAttributeType::Int32; p.Value = "10";
FNBTDataAccessor eq = l.ListMakeAccessorByParameter(p);
FNBTDataAccessor eqNode = l.ListMakeAccessorIfEqual(sample);
```
## 复制 / 交换 / 删除 与诊断

- 拷贝：
  - `TryCopyFrom(Source)`：源与目标路径必须存在；基础类型值复制，复合类型深拷贝。
  - `EnsureAndCopyFrom(Source)`：在目标路径不存在时尝试创建（注意 List 索引不可用于创建路径）。
- 交换：`TrySwap(Other)`：在不同容器或同容器不同节点之间交换（含深拷贝与重定位逻辑）。
- 删除：`Remove()`：删除当前节点及子树，返回删除的节点数。
- 诊断：`FNBTAttributeOpResultDetail` 提供 `ed()` / `ed(Context)` / `edv()` 输出错误与堆栈。

```angelscript
FNBTDataAccessor a = root.go("A");
FNBTDataAccessor b = root.go("B");

a.EnsureAndCopyFrom(b).ed("Copy B -> A");
a.TrySwap(b).edv();

int removed = a.Remove();
```
## 输出与遍历

- 文本输出：`ToString(bool bShowVersion=false)`；容器层面也提供 `ToString()/ToDebugString()`。
- 访问器遍历：借助 `VisitData`（脚本混入，签名：`(int Deep, ENBTAttributeType AttrType, FName AttrName, int32 AttrIdx, FNBTDataAccessor accessor)`）。

```angelscript
class Dumper {
    void OnVisit(int Deep, ENBTAttributeType T, FName Name, int AttrIdx, FNBTDataAccessor acc) {
        string indent = string(Deep * 2, ' ');
        string key = (Name != NAME_None) ? Name.ToString() : ("[" + AttrIdx + "]");
        Print(indent + key + ": " + acc.ToString());
    }
}

void DebugDump(FNBTDataAccessor root) {
    Dumper d;
    // 调用混入方法（ScriptCallable）
    root.VisitData(&d, "OnVisit");
}
```
## 网络复制与组件集成

- 组件：
  - `UNBTComponent`（可复制）：服务端写入，按 Delta 同步到客户端；`OnNBTContainerChanged` 广播变化。
  - `UNBTComponentLocal`（本地）：仅本地使用，不走网络。
- 复制策略：
  - 容器内部实现全量/增量（Delta）复制，按块与节点版本差异高效同步。
  - 访问器可网络序列化（NetSerialize）携带路径，便于 RPC 传递“节点位置”。

```angelscript
// 服务端：修改数据
UNBTComponent@ comp = ...; // 仅服务器端可写
FNBTDataAccessor root = comp.GetAccessor();
root["Match"]["Score"].EnsureAndSetInt32(999).ed();

// 客户端：订阅变化
event void OnContainerChanged() {
    // 刷新 UI
}
comp.OnNBTContainerChanged.AddDynamic(this, n"OnContainerChanged");
```
## 常见陷阱与最佳实践

- 不要用 `go(index)` 试图“创建” List 元素；请使用 `ListAddSubNode()` 或 `ListInsertSubNode()`。
- 初始化请首选 `EnsureAndSetX()`；日常写入用 `TrySetX()`；仅在修复/重置/导入场景使用 `OverrideToX()`。
- 浮点数比较使用了“近似相等”，需要严格比较时自行处理或外部封装。
- 做 UI 刷新、动画驱动等场景时，优先使用 `IsDataChangedAndMark()` / `IsSubtreeChangedAndMark()`，避免每帧完整遍历。
- 大批量操作（如导入关卡数据）建议按 Map/List 分块，必要时分帧处理，避免一次性构建超大树。
- 深拷贝/交换涉及临时节点分配，超大树或接近上限时要注意 `AllocateFailed` 返回。
- 网络模式下服务端写入，客户端只读；避免在客户端调用写入 API（会无效且污染日志）。
## 支持的类型速查（部分）

- 基础类型写入/读取（示例）：
  - `TryGet/EnsureAndSet/OverrideTo` + `Bool/Int8/Int16/Int32/Int64/Float/Double/Name/String/Color/Guid/DateTime/Rotator/Vector2D/Vector/IntVector2/IntVector/Int64Vector2/Int64Vector`
- 数组类型（示例）：
  - `TryGet/EnsureAndSet/OverrideTo` + `Int8Array/Int16Array/Int32Array/Int64Array/FloatArray/DoubleArray`
- 判空/复合类型：`IsEmpty/IsMap/IsList/IsArray`，`EnsureMap/EnsureList`

```angelscript
root["T"]["F"].EnsureAndSetFloat(1.0f);
root["T"]["V"].EnsureAndSetVector(FVector(1,2,3));
TArray<int32> arr = {1,2,3};
root["T"]["Arr"].EnsureAndSetInt32Array(arr);
```
## 操作结果与错误码

- 结果类型（`ENBTAttributeOpResult`）：
  - `Success` / `SameAndNotChange`
  - `NotFoundNode` / `NotFoundSubNode`
  - `NodeTypeMismatch` / `PermissionDenied`
  - `InvalidID` / `InvalidContainer` / `AllocateFailed`
- 工具函数：
  - `GetResultString()` 获取可读字符串
  - `ed()/ed(Context)/edv()` 输出错误日志与调用堆栈（建议开发阶段开启）

```angelscript
auto r = root["A"]["B"].TrySetInt32(1);
if (!r.IsOk()) {
    Print("Set failed: " + r.GetResultString());
    r.edv();
}
```
## 迁移建议：转向 DaxSystem

由于本库已停止维护，建议尽快评估并迁移到作者的 DaxSystem：
- 更全面的功能集与更强的生态集成（脚本、网络、调试工具）。
- 长期维护与问题修复保障。

迁移时可保留“路径访问器 + 三段式写入 + 变更检测”的总体使用心智模型，降低迁移成本。
## 综合示例：背包系统（创建/查询/同步）

```angelscript
UNBTComponent@ nbt = ...; // 服务器端组件
FNBTDataAccessor root = nbt.GetAccessor();

// 初始化根结构
FNBTDataAccessor inv = root["Inventory"].EnsureList();

// 添加三件物品
FNBTDataAccessor item0 = inv.ListAddSubNode();
item0.EnsureMap();
item0["id"].EnsureAndSetString("potion");
item0["name"].EnsureAndSetString("Healing Potion");
item0["count"].EnsureAndSetInt32(3);

FNBTDataAccessor item1 = inv.ListAddSubNode();
item1.EnsureMap();
item1["id"].EnsureAndSetString("sword");
item1["name"].EnsureAndSetString("Iron Sword");
item1["atk"].EnsureAndSetInt32(12);

FNBTDataAccessor item2 = inv.ListAddSubNode();
item2.EnsureMap();
item2["id"].EnsureAndSetString("key");
item2["name"].EnsureAndSetString("Boss Key");

// 查询：找名称包含 "boss" 的物品
FNBTSearchParameter p; p.Op = ENBTCompareOp::Contains; p.ValueType = ENBTAttributeType::String; p.Value = "boss"; p.SubKey = FName("name");
FNBTDataAccessor boss = inv.ListMakeAccessorByParameter(p);
if (boss.IsDataExists()) {
    Print("Found: " + boss.ToString());
}

// 修改：泛型写入（整型）
item1["atk"].TrySetGenericInt(15).ed();

// 删除：移除第一个物品
inv.ListRemoveSubNode(0).ed();

// 客户端：订阅变更
// comp.OnNBTContainerChanged.AddDynamic(this, n"OnChanged");
```
