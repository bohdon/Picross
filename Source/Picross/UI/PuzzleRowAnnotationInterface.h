// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "Picross/PuzzleTypes.h"
#include "UObject/Interface.h"
#include "PuzzleRowAnnotationInterface.generated.h"


/**
 * Interface for an object that can display a puzzle row annotation
 */
UINTERFACE()
class UPuzzleRowAnnotationInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an object that can display a puzzle row annotation
 */
class PICROSS_API IPuzzleRowAnnotationInterface
{
	GENERATED_BODY()

public:
	/** Set whether the annotation should be visible */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAnnotationsVisible(bool bNewVisible);
	virtual void SetAnnotationsVisible_Implementation(bool bNewVisible) = 0;

	/** Set the annotations to display for this row */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPuzzleRowAnnotations(FPuzzleRowAnnotations InAnnotations);
	virtual void SetPuzzleRowAnnotations_Implementation(FPuzzleRowAnnotations InAnnotations) = 0;
};
