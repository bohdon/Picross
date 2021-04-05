// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "PuzzlePlayer.h"
#include "GameFramework/GameModeBase.h"
#include "PicrossGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PICROSS_API APicrossGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	APicrossGameModeBase();

	/** The currently active puzzle player */
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<APuzzlePlayer> PuzzlePlayer;

	/** The currently active puzzle grid */
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<APuzzleGrid> PuzzleGrid;

	UFUNCTION(BlueprintCallable)
	APuzzlePlayer* GetPuzzlePlayer() const { return PuzzlePlayer.Get(); }

	UFUNCTION(BlueprintCallable)
	APuzzleGrid* GetPuzzleGrid() const { return PuzzleGrid.Get(); }
};
