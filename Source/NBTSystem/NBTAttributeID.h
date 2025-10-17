#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NBTHelper.h"

struct FNBTAttributeID {
    static constexpr uint16 InvalidIndex = 0xFFFF;
    
    uint16 Index;      // 支持65534个节点
    uint16 Generation;  // 256个生成代，防止ID重用问题
    
    FNBTAttributeID() : Index(InvalidIndex), Generation(0) {}
    explicit FNBTAttributeID(uint16 InIndex, uint16 InGen) : Index(InIndex), Generation(InGen) {}

    FNBTAttributeID(const FNBTAttributeID& Other) : Index(Other.Index), Generation(Other.Generation) {}

    FNBTAttributeID& operator=(const FNBTAttributeID& Other) {
        Index = Other.Index;
        Generation = Other.Generation;
        return *this;
    }
    
    bool IsValid() const { return Index != InvalidIndex; }
    
    bool operator==(const FNBTAttributeID& Other) const {
        return Index == Other.Index && Generation == Other.Generation;
    }
    
    friend uint32 GetTypeHash(const FNBTAttributeID& ID) {
        return (static_cast<uint32>(ID.Index) << 16) | static_cast<uint32>(ID.Generation);
    }

    FString ToString() const {
        return FString::Printf(TEXT("ID[%d:%d]"), Index, Generation);
    }
    
    friend FArchive& operator<<(FArchive& Ar, FNBTAttributeID& ID) {
        ArzNBT::SerializeZigZag(Ar, ID.Index);
        ArzNBT::SerializeZigZag(Ar, ID.Generation);
        return Ar;
    }
};