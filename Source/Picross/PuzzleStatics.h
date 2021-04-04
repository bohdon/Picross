// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PuzzleTypes.h"

#include "PuzzleStatics.generated.h"


/**
 * Static functions for working with puzzles
 */
UCLASS()
class PICROSS_API UPuzzleStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Return true if a row annotation represents a 0 row */
	UFUNCTION(BlueprintCallable)
	static bool IsZeroAnnotation(const FPuzzleRowAnnotations& RowAnnotations);
};
