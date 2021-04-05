// Copyright Bohdon Sayre


#include "PuzzleGrid.h"

#include "PuzzleBlockAvatar.h"
#include "PuzzleGridSlicerHandle.h"
#include "Kismet/GameplayStatics.h"


APuzzleGrid::APuzzleGrid()
	: bGenerateEmptyBlocks(true),
	  SlicerPadding(50.f)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockAvatarClass = APuzzleBlockAvatar::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void APuzzleGrid::SetPuzzle(const FPuzzleDef& InPuzzle, bool bRegenerateBlocks)
{
	Puzzle = InPuzzle;

	if (bRegenerateBlocks)
	{
		RegenerateBlockAvatars();
		CreateSlicerHandles();
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
	CreateSlicerHandles();
}

void APuzzleGrid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DestroyBlockAvatars();
}

void APuzzleGrid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// TODO(bsayre): cache
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->PlayerCameraManager)
	{
		const FVector CameraVector = PC->PlayerCameraManager->GetCameraRotation().Vector();

		// get camera dot for each axis of the grid
		FVector CameraDots;
		CameraDots.X = -CameraVector | GetActorForwardVector();
		CameraDots.Y = -CameraVector | GetActorRightVector();
		CameraDots.Z = -CameraVector | GetActorUpVector();

		const FVector BlockSize = BlockMeshSet->BlockSize;
		const FVector GridHalfSize = FVector(Puzzle.Dimensions) * BlockSize * 0.5f + SlicerPadding;

		for (APuzzleGridSlicerHandle* SlicerHandle : SlicerHandles)
		{
			// update slicer visibility
			const float SlicerDot = CameraDots[SlicerHandle->Axis];
			const bool bNewVisible = SlicerHandle->bInvertPosition
				                         ? SlicerDot > SMALL_NUMBER
				                         : SlicerDot < -SMALL_NUMBER;
			SlicerHandle->SetSlicerVisible(bNewVisible);

			// update slicer location (ensure it's on the back side of the grid)
			FVector Location = GridHalfSize;
			// flip location along slicer axis depending on which side (front or back)
			Location[SlicerHandle->Axis] *= SlicerHandle->bInvertPosition ? 1.f : -1.f;

			if (SlicerHandle->Axis < 2)
			{
				const int32 OtherAxis = SlicerHandle->Axis == 0 ? 1 : 0;

				// X and Y slicers are centered vertically
				Location.Z = 0.f;
				// X and Y slicers flip the off-axis based on view
				const float SlicerOtherDot = CameraDots[OtherAxis];
				Location[OtherAxis] *= SlicerOtherDot >= 0.f ? -1.f : 1.f;
			}
			else
			{
				// for z, use both X and Y dot, and position it at the back of the grid
				Location.X *= CameraDots.X >= 0.f ? -1.f : 1.f;
				Location.Y *= CameraDots.Y >= 0.f ? -1.f : 1.f;
			}

			SlicerHandle->SetActorRelativeLocation(Location);
		}
	}
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

bool APuzzleGrid::CanSetSlicerPositionForAxis(int32 Axis) const
{
	return SlicerPosition == 0 || SlicerAxis == Axis;
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

void APuzzleGrid::CreateSlicerHandles()
{
	// destroy existing handles
	for (APuzzleGridSlicerHandle* OldHandle : SlicerHandles)
	{
		if (OldHandle)
		{
			OldHandle->Destroy();
		}
	}

	const FVector BlockSize = BlockMeshSet->BlockSize;
	const FVector GridHalfSize = FVector(Puzzle.Dimensions) * BlockSize * 0.5f + SlicerPadding;

	SlicerHandles.Empty(6);
	for (int32 Axis = 0; Axis <= 2; ++Axis)
	{
		for (int32 Side = 0; Side <= 1; ++Side)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.ObjectFlags = RF_Transient;
			SpawnParams.Owner = this;

			const bool bInvertPosition = Side == 1;

			// location of the slicer handle, updated dynamically later when grid is rotated
			FVector Location = GridHalfSize;
			// flip location along slicer axis depending on which side (front or back)
			Location[Axis] *= bInvertPosition ? -1.f : 1.f;
			// X and Y slicers are centered vertically
			if (Axis < 2)
			{
				Location.Z = 0.f;
			}

			// vector pointing in the opposite direction of this slicer's axis.
			FVector AxisVector(0.f, 0.f, 0.f);
			AxisVector[Axis] = bInvertPosition ? -1.f : 1.f;
			// rotation of the slicer handle
			FRotator Rotation = AxisVector.Rotation();

			APuzzleGridSlicerHandle* NewHandle = GetWorld()->SpawnActor<APuzzleGridSlicerHandle>(
				SlicerHandleClass, Location, Rotation, SpawnParams);
			if (NewHandle)
			{
				NewHandle->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
				NewHandle->bInvertPosition = bInvertPosition;
				NewHandle->SetAxisAndDimensions(Axis, Puzzle.Dimensions[Axis], BlockMeshSet->BlockSize);
				NewHandle->OnPositionChangedEvent.AddUObject(this, &APuzzleGrid::OnSlicerHandlePositionChanged,
				                                             NewHandle);

				SlicerHandles.Add(NewHandle);
			}
		}
	}
}

void APuzzleGrid::OnSlicerHandlePositionChanged(int32 NewPosition, APuzzleGridSlicerHandle* SlicerHandle)
{
	SetSlicerPosition(SlicerHandle->Axis, NewPosition);
}

void APuzzleGrid::OnSlicerChanged()
{
	// update slicer handles based on current slicer axis
	for (APuzzleGridSlicerHandle* SlicerHandle : SlicerHandles)
	{
		const bool bIsInvertedPosition = SlicerPosition < 0;
		if (SlicerHandle->Axis != SlicerAxis || SlicerHandle->bInvertPosition != bIsInvertedPosition)
		{
			SlicerHandle->ResetPosition();
		}
	}

	// update block visibility based on slicing
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
