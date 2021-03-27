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

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Has the puzzle been started? */
	UPROPERTY(Transient)
	bool bIsStarted;

	/** The current puzzle grid */
	UPROPERTY(Transient)
	APuzzleGrid* PuzzleGrid;

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
