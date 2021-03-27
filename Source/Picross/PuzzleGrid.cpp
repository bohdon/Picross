﻿// Copyright Bohdon Sayre


#include "PuzzleGrid.h"


APuzzleGrid::APuzzleGrid()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockAvatarClass = APuzzleBlockAvatar::StaticClass();
}

void APuzzleGrid::SetPuzzle(const FPuzzle& InPuzzle)
{
	Puzzle = InPuzzle;
	RegenerateBlockAvatars();
}

void APuzzleGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APuzzleGrid::BeginPlay()
{
	Super::BeginPlay();

	RegenerateBlockAvatars();
}

void APuzzleGrid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DestroyBlockAvatars();
}

void APuzzleGrid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.MemberProperty
		                           ? PropertyChangedEvent.MemberProperty->GetFName()
		                           : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleGrid, Puzzle))
	{
		RegenerateBlockAvatars();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void APuzzleGrid::GenerateBlockAvatars()
{
	if (BlockAvatars.Num() > 0)
	{
		return;
	}

	BlocksByPosition.Reset();

	// generate all real blocks from the puzzle
	for (const FPuzzleBlock& Block : Puzzle.Blocks)
	{
		CreateBlockAvatar(Block);
	}

	// generate any empty blocks
	for (int32 X = 0; X < Puzzle.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < Puzzle.Dimensions.Y; ++Y)
		{
			for (int32 Z = 0; Z < Puzzle.Dimensions.Z; ++Z)
			{
				const FIntVector Position(X, Y, Z);
				if (!GetBlockAtPosition(Position))
				{
					FPuzzleBlock EmptyBlock;
					EmptyBlock.Type = EmptyBlockType;
					EmptyBlock.Position = Position;
					CreateBlockAvatar(EmptyBlock);
				}
			}
		}
	}
}

void APuzzleGrid::RegenerateBlockAvatars()
{
	DestroyBlockAvatars();
	GenerateBlockAvatars();
}

void APuzzleGrid::DestroyBlockAvatars()
{
	for (APuzzleBlockAvatar* BlockAvatar : BlockAvatars)
	{
		if (BlockAvatar)
		{
			BlockAvatar->Destroy();
		}
	}

	BlockAvatars.Empty();
	BlocksByPosition.Empty();
}

APuzzleBlockAvatar* APuzzleGrid::GetBlockAtPosition(const FIntVector& Position) const
{
	if (BlocksByPosition.Contains(Position.ToString()))
	{
		return BlocksByPosition[Position.ToString()];
	}
	return nullptr;
}

APuzzleBlockAvatar* APuzzleGrid::CreateBlockAvatar(const FPuzzleBlock& Block)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags = RF_Transient;

	APuzzleBlockAvatar* BlockAvatar = GetWorld()->SpawnActor<APuzzleBlockAvatar>(BlockAvatarClass, SpawnParameters);
	if (BlockAvatar)
	{
		BlockAvatar->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		BlockAvatar->BlockMeshSet = BlockMeshSet;
		BlockAvatar->SetBlock(Block);

		BlockAvatars.Add(BlockAvatar);
		BlocksByPosition.Add(Block.Position.ToString(), BlockAvatar);
	}

	return BlockAvatar;
}
