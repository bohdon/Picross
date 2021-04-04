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

	/** The puzzle to be solved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleDef PuzzleDef;

	/** The puzzle grid class to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APuzzleGrid> PuzzleGridClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothInputSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPitchAngle;

	/** Start playing the puzzle */
	UFUNCTION(BlueprintCallable)
	void Start();

	/** Return the current puzzle grid */
	UFUNCTION(BlueprintPure)
	APuzzleGrid* GetPuzzleGrid() const { return PuzzleGrid; }

	/** Set the current rotation of the puzzle */
	UFUNCTION(BlueprintCallable)
	void SetPuzzleRotation(float Pitch, float Yaw);

	/** Add input to rotate the puzzle right or left */
	UFUNCTION(BlueprintCallable)
	void AddRotateRightInput(float Value);

	/** Add input to rotate the puzzle up or down */
	UFUNCTION(BlueprintCallable)
	void AddRotateUpInput(float Value);

	/** Automatically identify all empty blocks on rows with visible 0 annotations */
	UFUNCTION(BlueprintCallable)
	void BreakZeroRows();

	/** Automatically identify all blocks for a row */
	UFUNCTION(BlueprintCallable)
	void AutoIdentifyBlocksInRow(FPuzzleRow Row);

	/** Refresh the annotations displayed for all blocks */
	UFUNCTION(BlueprintCallable)
	void RefreshAllBlockAnnotations();

	/** Return true if all blocks in a row have been identified */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	bool IsRowIdentified(FPuzzleRow Row) const;

	/** Return true if all blocks of a single type has been identified in a row */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	bool IsRowTypeIdentified(FPuzzleRow Row, FGameplayTag BlockType) const;

	/**
	 * Get all annotations for a block.
	 * Updates the row type annotations to include whether each type is identified for that row
	 */
	UFUNCTION(BlueprintCallable)
	FPuzzleBlockAnnotations GetBlockAnnotations(FIntVector Position) const;

	UFUNCTION(BlueprintCallable)
	FPuzzleRowAnnotations GetRowAnnotation(FPuzzleRow Row) const;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Called when the true form of all blocks in a row has been revealed.
	 * Not called for empty rows.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRowRevealed"))
	void OnRowRevealed_BP(FPuzzleRow Row);

	/**
	 * Called when the puzzle has been fully solved
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnPuzzleSolved"))
	void OnPuzzleSolved_BP();

protected:
	/** Has the puzzle been started? */
	UPROPERTY(Transient)
	bool bIsStarted;

	/** Has the puzzle been solved? */
	UPROPERTY(Transient)
	bool bIsSolved;

	/** The current puzzle grid */
	UPROPERTY(Transient)
	APuzzleGrid* PuzzleGrid;

	/** All annotations for the puzzle */
	UPROPERTY(Transient)
	FPuzzleAnnotations Annotations;

	/** Regenerate all annotations */
	void RegenerateAllAnnotations();

	float RotateRightInput;
	float RotateUpInput;

	float SmoothRotateRightInput;
	float SmoothRotateUpInput;

	/** The current yaw rotation of the puzzle */
	float RotateYaw;
	/** The current pitch rotation of the puzzle */
	float RotatePitch;

	APuzzleGrid* CreatePuzzleGrid();

	void SetAllBlockAnnotationsVisible(bool bNewVisible);

	FRotator GetPlayerCameraRotation();

	/** Check if the puzzle has been solved */
	void CheckPuzzleSolved();

	/**
	 * Check if a row has been solved, and reveal the true form of blocks if so
	 */
	void CheckRowSolved(FPuzzleRow Row);

	/** Called when a block has been identified correctly */
	void OnBlockIdentified(APuzzleBlockAvatar* BlockAvatar);

	/** Called when all blocks in a row have been identified */
	void OnRowIdentified(FPuzzleRow Row);
};
