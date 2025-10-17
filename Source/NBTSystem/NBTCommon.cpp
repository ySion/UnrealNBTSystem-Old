#include "NBTCommon.h"
#include "AngelscriptManager.h"

DEFINE_LOG_CATEGORY(NBTSystem)

void FNBTAttributeOpResultDetail::edvas() const {
    // Error Diagnose Verbose
    if (Result != ENBTAttributeOpResult::Success && Result != ENBTAttributeOpResult::SameAndNotChange) {
        UE_LOG(NBTSystem, Error, TEXT("\n==== NBT Operation Failed: ===="));
        UE_LOG(NBTSystem, Error, TEXT("==== Result: %s"), *GetResultString());
        UE_LOG(NBTSystem, Error, TEXT("==== Message: %s"), *ResultMessage);
            
        UE_LOG(NBTSystem, Error, TEXT("==== Stack Trace:"));
        const auto Stack = FAngelscriptManager::Get().GetAngelscriptCallstack();
        for (const auto& Frame : Stack) {
            UE_LOGFMT(NBTSystem, Error, "==== {0}", Frame);
        }
    }
}
