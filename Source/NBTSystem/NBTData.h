#pragma once

#include "CoreMinimal.h"
#include "NBTContainer.h"
#include "UObject/Object.h"
#include "NBTData.generated.h"

UCLASS(BlueprintType)
class NBTSYSTEM_API UNBTData : public UDataAsset {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FNBTContainer NBTContainer;
};