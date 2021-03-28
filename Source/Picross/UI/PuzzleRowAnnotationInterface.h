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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPuzzleRowAnnotation(FPuzzleRowAnnotation InAnnotation);
	virtual void SetPuzzleRowAnnotation_Implementation(FPuzzleRowAnnotation InAnnotation) = 0;
};
