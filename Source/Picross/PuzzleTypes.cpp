// Copyright Bohdon Sayre.


#include "PuzzleTypes.h"

#include "PicrossGameSettings.h"


FString FPuzzleRow::ToString() const
{
	return FString::Printf(TEXT("%s-%d"), *Position.ToString(), Axis);
}

FString FPuzzleBlockDef::ToString() const
{
	return FString::Printf(TEXT("Block(%s, %s)"), *Position.ToString(), *Type.ToString());
}

int32 FPuzzleDef::GetBlockIndexAtPosition(FIntVector Position) const
{
	for (int32 Idx = 0; Idx < Blocks.Num(); ++Idx)
	{
		if (Blocks[Idx].Position == Position)
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}

FPuzzleBlockDef FPuzzleDef::GetBlockAtPosition(FIntVector Position) const
{
	// slow, but works for now
	for (int32 Idx = 0; Idx < Blocks.Num(); ++Idx)
	{
		if (Blocks[Idx].Position == Position)
		{
			return Blocks[Idx];
		}
	}
	return FPuzzleBlockDef();
}

void FPuzzleAnnotations::GetBlockAnnotations(FIntVector Position, FPuzzleBlockAnnotations& OutBlockAnnotations) const
{
	GetRowAnnotations(FPuzzleRow(Position, 0), OutBlockAnnotations.XAnnotations);
	GetRowAnnotations(FPuzzleRow(Position, 1), OutBlockAnnotations.YAnnotations);
	GetRowAnnotations(FPuzzleRow(Position, 2), OutBlockAnnotations.ZAnnotations);
}

void FPuzzleAnnotations::GetRowAnnotations(FPuzzleRow Row, FPuzzleRowAnnotations& OutRowAnnotations) const
{
	Row.Normalize();
	OutRowAnnotations = RowAnnotations.FindRef(Row.ToString());
}

void FPuzzleAnnotations::GenerateAnnotations(const FPuzzleDef& PuzzleDef, FPuzzleAnnotations& OutAnnotations)
{
	OutAnnotations.RowAnnotations.Empty(PuzzleDef.Dimensions.Y * PuzzleDef.Dimensions.Z);

	// x-axis
	for (int32 Y = 0; Y < PuzzleDef.Dimensions.Y; ++Y)
	{
		for (int32 Z = 0; Z < PuzzleDef.Dimensions.Z; ++Z)
		{
			const FPuzzleRow Row(FIntVector(0, Y, Z), 0);
			FPuzzleRowAnnotations NewRowAnnotations = GenerateRowAnnotation(PuzzleDef, Row);
			OutAnnotations.RowAnnotations.Add(Row.ToString(), NewRowAnnotations);
		}
	}

	// y-axis
	for (int32 X = 0; X < PuzzleDef.Dimensions.X; ++X)
	{
		for (int32 Z = 0; Z < PuzzleDef.Dimensions.Z; ++Z)
		{
			const FPuzzleRow Row(FIntVector(X, 0, Z), 1);
			FPuzzleRowAnnotations NewRowAnnotations = GenerateRowAnnotation(PuzzleDef, Row);
			OutAnnotations.RowAnnotations.Add(Row.ToString(), NewRowAnnotations);
		}
	}

	// z-axis
	for (int32 X = 0; X < PuzzleDef.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < PuzzleDef.Dimensions.Y; ++Y)
		{
			const FPuzzleRow Row(FIntVector(X, Y, 0), 2);
			FPuzzleRowAnnotations NewRowAnnotations = GenerateRowAnnotation(PuzzleDef, Row);
			OutAnnotations.RowAnnotations.Add(Row.ToString(), NewRowAnnotations);
		}
	}
}

FPuzzleRowAnnotations FPuzzleAnnotations::GenerateRowAnnotation(const FPuzzleDef& InPuzzle, FPuzzleRow Row)
{
	// block type for empty spaces
	const FGameplayTag EmptyType = GetDefault<UPicrossGameSettings>()->BlockEmptyTag;

	FPuzzleRowAnnotations Result;
	// row-type annotations, indexed by type for faster lookup when being built out
	TMap<FGameplayTag, FPuzzleRowTypeAnnotation> TypeAnnotations;

	Row.Normalize();
	FIntVector CurrentPos = Row.Position;

	// iterate through this row of blocks along the given axis,
	// keep track of the last discovered block type to build group counts
	FGameplayTag LastType;
	const int32 Dimension = InPuzzle.Dimensions[Row.Axis];
	for (int32 Idx = 0; Idx < Dimension; ++Idx)
	{
		CurrentPos[Row.Axis] = Idx;

		const FPuzzleBlockDef& BlockDef = InPuzzle.GetBlockAtPosition(CurrentPos);
		if (BlockDef.IsValid())
		{
			// start a new row-type-annotation for this block type
			if (!TypeAnnotations.Contains(BlockDef.Type))
			{
				FPuzzleRowTypeAnnotation NewTypeAnnotation;
				NewTypeAnnotation.Type = BlockDef.Type;
				// assume all are identified, until proven false
				NewTypeAnnotation.bAreIdentified = true;
				TypeAnnotations.Add(BlockDef.Type, NewTypeAnnotation);
			}

			FPuzzleRowTypeAnnotation& TypeAnnotation = TypeAnnotations[BlockDef.Type];

			// increase number of found blocks
			++TypeAnnotation.NumBlocks;

			// increment group count if last block type was different
			if (LastType != BlockDef.Type)
			{
				++TypeAnnotation.NumGroups;
			}

			LastType = BlockDef.Type;
		}
		else
		{
			// no block at this position
			LastType = EmptyType;
		}
	}

	// store type annotations on the full annotation
	TypeAnnotations.GenerateValueArray(Result.TypeAnnotations);

	if (Result.IsZeroAnnotation())
	{
		Result.bIsVisible = FMath::RandBool();
	}

	return Result;
}
