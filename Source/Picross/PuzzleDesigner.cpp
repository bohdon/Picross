// Copyright Bohdon Sayre.


#include "PuzzleDesigner.h"


#include "PuzzleBlockAvatar.h"
#include "PuzzleGrid.h"


APuzzleDesigner::APuzzleDesigner()
	: DefaultDimensions(3, 3, 3)
{
	PuzzleGridClass = APuzzleGrid::StaticClass();
}

void APuzzleDesigner::SetDimensions(FIntVector NewDimensions, bool bCommit)
{
	if (!PuzzleGrid)
	{
		return;
	}

	FPuzzleDef NewPuzzleDef = PuzzleGrid->PuzzleDef;
	NewPuzzleDef.Dimensions = NewDimensions;

	PuzzleGrid->SetPuzzle(NewPuzzleDef, false);

	if (bCommit)
	{
		CommitDimensions();
	}
}

void APuzzleDesigner::SetBlockType(FIntVector Position, FGameplayTag NewBlockType)
{
	if (!PuzzleGrid)
	{
		return;
	}

	FPuzzleDef NewPuzzleDef = PuzzleGrid->PuzzleDef;
	const int32 Idx = NewPuzzleDef.GetBlockIndexAtPosition(Position);
	if (Idx == INDEX_NONE)
	{
		FPuzzleBlockDef NewBlockDef;
		NewBlockDef.Position = Position;
		NewBlockDef.Type = NewBlockType;
		NewPuzzleDef.Blocks.Add(NewBlockDef);
	}
	else
	{
		NewPuzzleDef.Blocks[Idx].Type = NewBlockType;
	}

	PuzzleGrid->SetPuzzle(NewPuzzleDef, true);
}

void APuzzleDesigner::CommitDimensions()
{
	for (int32 Idx = PuzzleGrid->PuzzleDef.Blocks.Num() - 1; Idx >= 0; --Idx)
	{
		FPuzzleBlockDef& BlockDef = PuzzleGrid->PuzzleDef.Blocks[Idx];

		if (BlockDef.Position.X >= PuzzleGrid->PuzzleDef.Dimensions.X ||
			BlockDef.Position.Y >= PuzzleGrid->PuzzleDef.Dimensions.Y ||
			BlockDef.Position.Z >= PuzzleGrid->PuzzleDef.Dimensions.Z)
		{
			PuzzleGrid->PuzzleDef.Blocks.RemoveAt(Idx);
		}
	}

	PuzzleGrid->RegenerateBlockAvatars();
}

void APuzzleDesigner::BeginPlay()
{
	Super::BeginPlay();

	if (!PuzzleGrid)
	{
		PuzzleGrid = CreatePuzzleGrid();
	}

	if (PuzzleGrid)
	{
	}
}

void APuzzleDesigner::OnBlockIdentifyAttempt(APuzzleBlockAvatar* BlockAvatar, FGameplayTag BlockType)
{
	SetBlockType(BlockAvatar->Block.Position, BlockType);
}

APuzzleGrid* APuzzleDesigner::CreatePuzzleGrid()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.ObjectFlags = RF_Transient;

	APuzzleGrid* Grid = GetWorld()->SpawnActor<APuzzleGrid>(PuzzleGridClass, GetActorTransform(), SpawnParameters);
	if (Grid)
	{
		Grid->bGenerateEmptyBlocks = true;
		Grid->DefaultBlockState = EPuzzleBlockState::TrueForm;
		Grid->PuzzleDef.Dimensions = DefaultDimensions;
		Grid->RegenerateBlockAvatars();
		Grid->OnBlockIdentifyAttemptEvent.AddUObject(this, &APuzzleDesigner::OnBlockIdentifyAttempt);
	}
	return Grid;
}
