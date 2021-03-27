// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "PuzzleGrid.h"
#include "PuzzleTypes.h"
#include "GameFramework/Actor.h"

#include "PuzzlePlayer.generated.h"


/**
 * Handles playing a puzzle. Generates nonogram hints
 * about the puzzle, and manages the state of the puzzle solve.
 */
UCLASS()
class PICROSS_API APuzzlePlayer : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

public:
	APuzzlePlayer();

	/** The puzzle to be played */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzle Puzzle;

	/** The puzzle grid class to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APuzzleGrid> PuzzleGridClass;

	/** Start playing the puzzle */
	UFUNCTION(BlueprintCallable)
	void Start();

protected:
	/** Has the puzzle been started? */
	UPROPERTY(Transient)
	bool bIsStarted;

	/** The current puzzle grid */
	UPROPERTY(Transient)
	APuzzleGrid* PuzzleGrid;

	APuzzleGrid* CreatePuzzleGrid();
};
