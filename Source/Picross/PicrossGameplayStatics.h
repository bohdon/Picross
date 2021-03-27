// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"


#include "PuzzlePlayer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "PicrossGameplayStatics.generated.h"


/**
 * 
 */
UCLASS()
class PICROSS_API UPicrossGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Get the current puzzle player */
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static APuzzlePlayer* GetPuzzlePlayer(const UObject* WorldContextObject);
};
