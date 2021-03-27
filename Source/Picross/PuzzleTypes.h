// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "PuzzleTypes.generated.h"


/**
 * A single block within a FPuzzle
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleBlock
{
	GENERATED_BODY()

public:
	FPuzzleBlock()
		: Position(FIntVector::ZeroValue)
	{
	}

	/** The position of the block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Position;

	/** The type of block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Type;
};


/**
* Represents a single puzzle shape that can be solved
*/
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzle
{
	GENERATED_BODY()

public:
	FPuzzle()
		: Dimensions(1, 5, 5)
	{
	}

	/** The dimensions of the puzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Dimensions;

	/** The blocks making up this puzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPuzzleBlock> Blocks;
};


/**
 * Represents an annotation for a puzzle row for a single block type
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleRowTypeAnnotation
{
	GENERATED_BODY()

public:
	/** The type of blocks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Type;

	/** The number of puzzle blocks in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumBlocks;

	/** The number of connected groups of blocks in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumGroups;
};


/**
 * Represents an annotation for a puzzle row
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleRowAnnotation
{
	GENERATED_BODY()

public:
	/** The annotations for each type in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPuzzleRowTypeAnnotation> TypeAnnotations;
};
