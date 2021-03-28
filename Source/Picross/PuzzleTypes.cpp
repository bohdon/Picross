// Copyright Bohdon Sayre.


#include "PuzzleTypes.h"


FString FPuzzleBlock::ToString() const
{
	return FString::Printf(TEXT("Block(%s, %s)"), *Position.ToString(), *Type.ToString());
}

FPuzzleBlock FPuzzle::GetBlockAtPosition(FIntVector Position) const
{
	// slow, but works for now
	for (int32 Idx = 0; Idx < Blocks.Num(); ++Idx)
	{
		if (Blocks[Idx].Position == Position)
		{
			return Blocks[Idx];
		}
	}
	return FPuzzleBlock();
}
