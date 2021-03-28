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

	/** Called when a block has been identified correctly */
	void NotifyBlockIdentified(APuzzleBlockAvatar* BlockAvatar);

	/** Regenerate and refresh all displayed block annotations */
	UFUNCTION(BlueprintCallable)
	void UpdateAllAnnotations();

	/** Refresh the annotations displayed for all blocks */
	UFUNCTION(BlueprintCallable)
    void RefreshAllBlockAnnotations();

	UFUNCTION(BlueprintCallable)
	FPuzzleBlockAnnotations GetBlockAnnotations(FIntVector Position) const;

	UFUNCTION(BlueprintCallable)
    FPuzzleRowAnnotation GetRowAnnotation(FIntVector Position, int32 Axis) const;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Has the puzzle been started? */
	UPROPERTY(Transient)
	bool bIsStarted;

	/** The current puzzle grid */
	UPROPERTY(Transient)
	APuzzleGrid* PuzzleGrid;

	/** Annotations for every X row, indexed by starting position of the row */
	UPROPERTY(Transient)
	TMap<FString, FPuzzleRowAnnotation> XAnnotations;

	/** Annotations for every Y row, indexed by starting position of the row */
	UPROPERTY(Transient)
	TMap<FString, FPuzzleRowAnnotation> YAnnotations;

	/** Annotations for every Z row, indexed by starting position of the row */
	UPROPERTY(Transient)
	TMap<FString, FPuzzleRowAnnotation> ZAnnotations;

	/** Regenerate all annotations */
	void RegenerateAllAnnotations();

	/**
	 * Calculate the annotations for a row in the puzzle
	 * @param InPuzzle A puzzle used to calculate the annotation
	 * @param Position The starting block position of the row
	 * @param Axis The axis of the row, 0, 1, or 2 (X, Y, or Z)
	 */
	FPuzzleRowAnnotation CalculateRowAnnotation(const FPuzzle& InPuzzle, FIntVector Position, int32 Axis) const;

	float RotateRightInput;
	float RotateUpInput;

	float SmoothRotateRightInput;
	float SmoothRotateUpInput;

	/** The current yaw rotation of the puzzle */
	float RotateYaw;
	/** The current pitch rotation of the puzzle */
	float RotatePitch;

	APuzzleGrid* CreatePuzzleGrid();

	FRotator GetPlayerCameraRotation();
};
