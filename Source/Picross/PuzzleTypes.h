// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "PuzzleTypes.generated.h"


/**
 * The possible states of a puzzle block during puzzle play
 */
UENUM(BlueprintType)
enum class EPuzzleBlockState : uint8
{
	/** Default, unidentified, generic block */
	Unidentified,
	/** Correctly identified type */
	Identified,
	/** Revealed, true form of the block */
	TrueForm,
};


/**
 * A single row within a puzzle, represented by a position and axis
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleRow
{
	GENERATED_BODY()

public:
	FPuzzleRow()
		: Position(FIntVector::ZeroValue),
		  Axis(0)
	{
	}

	FPuzzleRow(FIntVector InPosition, int32 InAxis)
		: Position(InPosition),
		  Axis(InAxis)
	{
		Normalize();
	}

	/** The starting position of the row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Position;

	/** The axis of the row (0, 1, or 2 representing X, Y, and Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 2))
	int32 Axis;

	/**
	 * Return the string representation of this row.
	 * Make sure the row is normalized before calling this to get reliable results.
	 */
	FString ToString() const;

	FORCEINLINE bool IsValid() const { return Axis >= 0 && Axis <= 2; }

	/**
	 * Normalize the position of this row, ensuring that it represents
	 * the starting position, and not some position in the middle of the row
	 */
	void Normalize()
	{
		if (IsValid())
		{
			Position[Axis] = 0;
		}
	}

	bool operator==(const FPuzzleRow& Other) const
	{
		return Axis == Other.Axis && Position == Other.Position;
	}

	bool operator!=(const FPuzzleRow& Other) const
	{
		return !operator==(Other);
	}
};


/**
 * The definition of a single block within a puzzle
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleBlockDef
{
	GENERATED_BODY()

public:
	FPuzzleBlockDef()
		: Position(FIntVector::ZeroValue)
	{
	}

	/** The position of the block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Position;

	/** The type of block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Type;

	FString ToString() const;

	/** Return true if this block definition represents a valid block */
	FORCEINLINE bool IsValid() const { return Type.IsValid(); }
};


/**
 * A block within a puzzle, as well as it's current state
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleBlock
{
	GENERATED_BODY()

public:
	FPuzzleBlock()
		: State(EPuzzleBlockState::Unidentified)
	{
	}

	/** The definition of the block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleBlockDef Def;

	/** The current state of the block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPuzzleBlockState State;

	FString ToString() const;
};


/**
 * The definition of a puzzle shape that can be solved
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleDef
{
	GENERATED_BODY()

public:
	FPuzzleDef()
		: Dimensions(1, 5, 5)
	{
	}

	/** The dimensions of the puzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Dimensions;

	/** The block definitions making up this puzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPuzzleBlockDef> Blocks;
	
	int32 GetBlockIndexAtPosition(FIntVector Position) const;

	FPuzzleBlockDef GetBlockAtPosition(FIntVector Position) const;
};


/**
 * A puzzle and it's current state
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzle
{
	GENERATED_BODY()

public:
	FPuzzle()
	{
	}

	/** The definition of the puzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleDef Def;

	/** The block definitions making up this puzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPuzzleBlockDef> Blocks;

	FPuzzleBlockDef GetBlockAtPosition(FIntVector Position) const;
};


/**
 * Represents an annotation for a puzzle row for a single block type
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleRowTypeAnnotation
{
	GENERATED_BODY()

public:
	FPuzzleRowTypeAnnotation()
		: NumBlocks(0),
		  NumGroups(0),
		  bAreIdentified(false)
	{
	}

	/** The type of blocks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Type;

	/** The number of puzzle blocks in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumBlocks;

	/** The number of connected groups of blocks in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumGroups;

	/** Have the blocks for this type been correctly identified? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAreIdentified;
};


/**
 * Represents annotations for a single puzzle row
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleRowAnnotations
{
	GENERATED_BODY()

public:
	FPuzzleRowAnnotations()
		: bIsVisible(true)
	{
	}

	/** The annotations for each type in this row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPuzzleRowTypeAnnotation> TypeAnnotations;

	/** Are these annotations visible to the player? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVisible;

	/** Return true if this row has no blocks */
	FORCEINLINE bool IsZeroAnnotation() const { return TypeAnnotations.Num() == 0; }
};


/**
 * A full set of annotations to display on a block.
 * I.e. an annotation for each side of the block.
 */
USTRUCT(BlueprintType)
struct PICROSS_API FPuzzleBlockAnnotations
{
	GENERATED_BODY()

public:
	/** Annotations for the X row of this block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleRowAnnotations XAnnotations;

	/** Annotations for the Y row of this block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleRowAnnotations YAnnotations;

	/** Annotations for the Z row of this block */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleRowAnnotations ZAnnotations;
};


/**
 * A full set of annotations for a puzzle
 */
USTRUCT(BlueprintType)
struct FPuzzleAnnotations
{
	GENERATED_BODY()

public:
	/** Annotations for every row, indexed by row name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FPuzzleRowAnnotations> RowAnnotations;

public:
	/** Get annotations for a single block */
	void GetBlockAnnotations(FIntVector Position, FPuzzleBlockAnnotations& OutBlockAnnotations) const;

	/** Get annotations for a single row */
	void GetRowAnnotations(FPuzzleRow Row, FPuzzleRowAnnotations& OutRowAnnotations) const;

public:
	/**
	 * Calculate all annotations for a puzzle
	 * @param PuzzleDef The puzzle definition for which to generate annotations
	 * @param OutAnnotations The resulting annotations
	 */
	static void GenerateAnnotations(const FPuzzleDef& PuzzleDef, FPuzzleAnnotations& OutAnnotations);

	/**
	 * Generate the annotations for a row in a puzzle
	 * @param InPuzzle A puzzle used to calculate the annotation
	 * @param Row The row of the puzzle
	 */
	static FPuzzleRowAnnotations GenerateRowAnnotation(const FPuzzleDef& InPuzzle, FPuzzleRow Row);
};


/**
 * Puzzle solver used to both solve puzzles and provide information
 * needed to determine annotations based on puzzle difficulty
 */
class PICROSS_API FPuzzleSolver
{
	/** The current puzzle being solved */
	FPuzzle Puzzle;

	/** The current annotations available to solve the puzzle */
	FPuzzleAnnotations Annotations;

	/**
	 * Try to solve the whole puzzle with the current annotations.
	 * @return True if the puzzle could be solved, false if the annotations did not provide enough information.
	 */
	bool SolvePuzzle();
};
