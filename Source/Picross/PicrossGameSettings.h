// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "PicrossGameSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class PICROSS_API UPicrossGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Block tag that when identified represents an empty space in the puzzle */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta = (Categories = "Block.Type"))
	FGameplayTag BlockEmptyTag;

	/** Block mesh tag to use for displaying unidentified blocks */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta = (Categories = "Block.Type"))
	FGameplayTag BlockUnidentifiedTag;
};
