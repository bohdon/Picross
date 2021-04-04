// Copyright Bohdon Sayre


#include "PuzzleGrid.h"

#include "PuzzleBlockAvatar.h"


APuzzleGrid::APuzzleGrid()
	: bGenerateEmptyBlocks(true)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockAvatarClass = APuzzleBlockAvatar::StaticClass();
}

void APuzzleGrid::SetPuzzle(const FPuzzleDef& InPuzzle, bool bRegenerateBlocks)
{
	Puzzle = InPuzzle;

	if (bRegenerateBlocks)
	{
		RegenerateBlockAvatars();
	}
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
	for (const FPuzzleBlockDef& Block : Puzzle.Blocks)
	{
		CreateBlockAvatar(Block);
	}

	// generate any empty blocks
	if (bGenerateEmptyBlocks)
	{
		for (int32 X = 0; X < Puzzle.Dimensions.X; ++X)
		{
			for (int32 Y = 0; Y < Puzzle.Dimensions.Y; ++Y)
			{
				for (int32 Z = 0; Z < Puzzle.Dimensions.Z; ++Z)
				{
					const FIntVector Position(X, Y, Z);
					if (!GetBlockAtPosition(Position))
					{
						FPuzzleBlockDef EmptyBlock;
						EmptyBlock.Type = EmptyBlockType;
						EmptyBlock.Position = Position;
						CreateBlockAvatar(EmptyBlock);
					}
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

void APuzzleGrid::SetSlicerPosition(int32 Axis, int32 Position)
{
	SlicerAxis = FMath::Clamp(Axis, 0, 2);
	const int32 SlicerMaxValue = Puzzle.Dimensions[Axis] - 1;
	SlicerPosition = FMath::Clamp(Position, -SlicerMaxValue, SlicerMaxValue);

	OnSlicerChanged();
}

void APuzzleGrid::ResetSlicers()
{
	SlicerAxis = 0;
	SlicerPosition = 0;

	OnSlicerChanged();
}

APuzzleBlockAvatar* APuzzleGrid::GetBlockAtPosition(const FIntVector& Position) const
{
	if (BlocksByPosition.Contains(Position.ToString()))
	{
		return BlocksByPosition[Position.ToString()];
	}
	return nullptr;
}

APuzzleBlockAvatar* APuzzleGrid::CreateBlockAvatar(const FPuzzleBlockDef& Block)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags = RF_Transient;

	APuzzleBlockAvatar* BlockAvatar = GetWorld()->SpawnActor<APuzzleBlockAvatar>(BlockAvatarClass, SpawnParameters);
	if (BlockAvatar)
	{
		BlockAvatar->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		BlockAvatar->SetActorRelativeLocation(CalculateBlockLocation(Block.Position));
		BlockAvatar->BlockMeshSet = BlockMeshSet;
		BlockAvatar->SetBlock(Block);
		BlockAvatar->OnStateChangedEvent.AddUObject(this, &APuzzleGrid::OnBlockStateChanged, BlockAvatar);

		BlockAvatars.Add(BlockAvatar);
		BlocksByPosition.Add(Block.Position.ToString(), BlockAvatar);
	}

	return BlockAvatar;
}

FVector APuzzleGrid::CalculateBlockLocation(FIntVector Position) const
{
	const FVector BlockSize = BlockMeshSet->BlockSize;
	const FVector CenterOffset = FVector(Puzzle.Dimensions) * BlockSize * 0.5f - BlockSize * 0.5f;
	const FVector PositionLoc = FVector(Position) * BlockSize;
	return PositionLoc - CenterOffset;
}

void APuzzleGrid::OnSlicerChanged()
{
	for (APuzzleBlockAvatar* BlockAvatar : BlockAvatars)
	{
		check(BlockAvatar);

		const bool bVisible = IsBlockVisibleWithSlicing(BlockAvatar->Block.Position);
		BlockAvatar->SetIsBlockHidden(!bVisible);
	}
}

bool APuzzleGrid::IsBlockVisibleWithSlicing(FIntVector Position)
{
	if (SlicerPosition == 0 || SlicerAxis < 0 || SlicerAxis > 2)
	{
		// no slicing or invalid axis
		return true;
	}

	if (SlicerPosition > 0)
	{
		return Position[SlicerAxis] >= SlicerPosition;
	}
	else
	{
		return Position[SlicerAxis] < Puzzle.Dimensions[SlicerAxis] + SlicerPosition;
	}
}

void APuzzleGrid::OnBlockStateChanged(EPuzzleBlockState NewState, EPuzzleBlockState OldState,
                                      APuzzleBlockAvatar* BlockAvatar)
{
	if (NewState == EPuzzleBlockState::Identified)
	{
		OnBlockIdentifiedEvent.Broadcast(BlockAvatar);
	}
}
