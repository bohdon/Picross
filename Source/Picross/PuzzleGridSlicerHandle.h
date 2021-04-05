// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "PuzzleGrid.h"
#include "GameFramework/Actor.h"
#include "PuzzleGridSlicerHandle.generated.h"


/**
 * Handle that can be dragged to change grid slicing
 */
UCLASS()
class PICROSS_API APuzzleGridSlicerHandle : public AActor
{
	GENERATED_BODY()

public:
	APuzzleGridSlicerHandle();

	/** The axis of this slicer handle */
	UPROPERTY(Transient, BlueprintReadWrite)
	int32 Axis;

	/**
	 * If true, invert the position of this slicer when setting the grid slicer position.
	 * True for slicer handles viewed from the back side of the puzzle.
	 */
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bInvertPosition;

	/** The cached dimension of the puzzle along this slicer's axis */
	UPROPERTY(Transient, BlueprintSetter=SetDimension, BlueprintReadWrite)
	int32 Dimension;

	UFUNCTION(BlueprintSetter)
	void SetDimension(int32 NewDimension);

	/** The size of a block in the grid */
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector BlockSize;

	/** The precise position of the slicer (0..Dimension-1) */
	UPROPERTY(Transient, BlueprintSetter=SetPrecisePosition, BlueprintReadWrite)
	float PrecisePosition;

	UFUNCTION(BlueprintSetter)
	void SetPrecisePosition(float NewPosition);

	/** The position of the slicer (0..Dimension-1) */
	UPROPERTY(Transient, BlueprintSetter=SetPosition, BlueprintReadWrite)
	int32 Position;

	UFUNCTION(BlueprintSetter)
	void SetPosition(int32 NewPosition);

	/** Get the current Position, taking into account bInvertPosition */
	UFUNCTION(BlueprintPure)
	int32 GetPosition() const;

	/** Set the axis and max dimension of the slicer */
	UFUNCTION(BlueprintCallable)
	void SetAxisAndDimensions(int32 InAxis, int32 InDimension, FVector InBlockSize);

	UFUNCTION(BlueprintCallable)
	void SetSlicerVisible(bool bNewVisible);

	UFUNCTION(BlueprintPure)
	int32 GetMaxPosition() const;

	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;
	virtual void NotifyActorOnReleased(FKey ButtonReleased) override;

	void StartDragging();
	void StopDragging();

	DECLARE_MULTICAST_DELEGATE_OneParam(FPositionChangedDelegate, int32 /* NewPosition */);

	/** Called when the position of the slicer has changed */
	FPositionChangedDelegate OnPositionChangedEvent;

	UFUNCTION(BlueprintCallable)
	void ResetPosition(bool bTriggerNotifiers = true);

protected:
	bool bIsSlicerVisible;
	bool bIsDragging;

	virtual void BeginPlay() override;

	void SetPositionImpl(int32 NewPosition, bool bTriggerNotifiers = true);

	void OnPositionChanged();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnPositionChanged"))
	void OnPositionChanged_BP();

public:
	virtual void Tick(float DeltaTime) override;
};
