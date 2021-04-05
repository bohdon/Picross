// Copyright Bohdon Sayre


#include "PuzzleGrid.h"


#include "PicrossGameModeBase.h"
#include "PuzzleBlockAvatar.h"
#include "PuzzleGridSlicerHandle.h"
#include "Kismet/GameplayStatics.h"


APuzzleGrid::APuzzleGrid()
	: bGenerateOnBeginPlay(false),
	  bGenerateEmptyBlocks(true),
	  DefaultBlockState(EPuzzleBlockState::Unidentified),
	  SlicerPadding(50.f),
	  SmoothInputSpeed(10.f),
	  RotateSpeed(45.f),
	  MaxPitchAngle(85.f)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockAvatarClass = APuzzleBlockAvatar::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void APuzzleGrid::SetPuzzle(const FPuzzleDef& InPuzzle, bool bRegenerateBlocks)
{
	PuzzleDef = InPuzzle;

	for (APuzzleGridSlicerHandle* SlicerHandle : SlicerHandles)
	{
		SlicerHandle->SetDimension(PuzzleDef.Dimensions[SlicerHandle->Axis]);
	}

	// update slicer position, re-applying clamping
	SetSlicerPosition(SlicerAxis, SlicerPosition);

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

	APicrossGameModeBase* GameMode = GetWorld()->GetAuthGameMode<APicrossGameModeBase>();
	if (GameMode)
	{
		GameMode->PuzzleGrid = this;
	}

	CreateSlicerHandles();

	if (bGenerateOnBeginPlay)
	{
		RegenerateBlockAvatars();
	}
}

void APuzzleGrid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DestroyBlockAvatars();
}

void APuzzleGrid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// update smooth input
	SmoothRotateRightInput = FMath::Lerp(SmoothRotateRightInput, RotateRightInput, SmoothInputSpeed * DeltaSeconds);
	SmoothRotateUpInput = FMath::Lerp(SmoothRotateUpInput, RotateUpInput, SmoothInputSpeed * DeltaSeconds);

	// consume inputs
	RotateRightInput = 0.f;
	RotateUpInput = 0.f;

	// get camera rotation to apply camera-relative clamping and pitch
	const FRotator CameraRotation = GetPlayerCameraRotation();

	// update rotation
	if (FMath::Abs(SmoothRotateRightInput) > SMALL_NUMBER ||
		FMath::Abs(SmoothRotateUpInput) > SMALL_NUMBER)
	{
		const float NewYaw = RotateYaw + SmoothRotateRightInput * RotateSpeed * DeltaSeconds;
		const float NewPitch = RotatePitch + SmoothRotateUpInput * RotateSpeed * DeltaSeconds;
		SetPuzzleRotation(NewPitch, NewYaw);
	}

	// rotations applied relative to camera
	const FRotator PitchRot = FRotator(RotatePitch, 0.f, 0.f);
	const FRotator YawRot = FRotator(0.f, RotateYaw, 0.f);
	const FRotator NewRotation = (FQuat(CameraRotation) * FQuat(PitchRot) * FQuat(YawRot)).Rotator();
	SetActorRotation(NewRotation);

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

		const FVector GridHalfSize = FVector(PuzzleDef.Dimensions) * GetBlockSize() * 0.5f + SlicerPadding;

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

	if (PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleGrid, PuzzleDef))
	{
		RegenerateBlockAvatars();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FRotator APuzzleGrid::GetPlayerCameraRotation() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->PlayerCameraManager)
	{
		return PC->PlayerCameraManager->GetCameraRotation();
	}
	return FRotator();
}

void APuzzleGrid::GenerateBlockAvatars()
{
	if (BlockAvatars.Num() > 0)
	{
		return;
	}

	BlocksByPosition.Reset();

	// generate all real blocks from the puzzle
	for (const FPuzzleBlockDef& Block : PuzzleDef.Blocks)
	{
		CreateBlockAvatar(Block);
	}

	// generate any empty blocks
	if (bGenerateEmptyBlocks)
	{
		for (int32 X = 0; X < PuzzleDef.Dimensions.X; ++X)
		{
			for (int32 Y = 0; Y < PuzzleDef.Dimensions.Y; ++Y)
			{
				for (int32 Z = 0; Z < PuzzleDef.Dimensions.Z; ++Z)
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
	const int32 SlicerMaxValue = PuzzleDef.Dimensions[Axis] - 1;
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

	for (APuzzleGridSlicerHandle* SlicerHandle : SlicerHandles)
	{
		SlicerHandle->ResetPosition();
	}

	OnSlicerChanged();
}

void APuzzleGrid::IncreaseFrontSlicerPosition()
{
	int32 Axis;
	int32 Sign;
	GetCameraAlignedAxis(Axis, Sign);

	if (SlicerAxis == Axis)
	{
		SetSlicerPosition(Axis, SlicerPosition + Sign);
	}
	else
	{
		SetSlicerPosition(Axis, Sign);
	}
}

void APuzzleGrid::DecreaseFrontSlicerPosition()
{
	int32 Axis;
	int32 Sign;
	GetCameraAlignedAxis(Axis, Sign);

	if (SlicerAxis == Axis)
	{
		int32 NewPosition = SlicerPosition - Sign;
		if (Sign > 0)
		{
			NewPosition = FMath::Max(NewPosition, 0);
		}
		else
		{
			NewPosition = FMath::Min(NewPosition, 0);
		}
		SetSlicerPosition(Axis, NewPosition);
	}
	else
	{
		SetSlicerPosition(Axis, 0);
	}
}

void APuzzleGrid::SetPuzzleRotation(float Pitch, float Yaw)
{
	RotatePitch = FMath::ClampAngle(Pitch, -MaxPitchAngle, MaxPitchAngle);
	RotateYaw = FRotator::NormalizeAxis(Yaw);
}

void APuzzleGrid::AddRotateRightInput(float Value)
{
	RotateRightInput += Value;
}

void APuzzleGrid::AddRotateUpInput(float Value)
{
	RotateUpInput += Value;
}

FVector APuzzleGrid::GetBlockSize() const
{
	return BlockMeshSet ? BlockMeshSet->BlockSize : FVector(100.f, 100.f, 100.f);
}

APuzzleBlockAvatar* APuzzleGrid::GetBlockAtPosition(const FIntVector& Position) const
{
	if (BlocksByPosition.Contains(Position.ToString()))
	{
		return BlocksByPosition[Position.ToString()];
	}
	return nullptr;
}

void APuzzleGrid::GetCameraAlignedAxis(int32& OutAxis, int32& OutSign) const
{
	const FVector CameraVector = GetPlayerCameraRotation().Vector();

	OutAxis = -1;
	OutSign = 1;
	float BestDot = -1.f;
	for (int32 Axis = 0; Axis <= 2; ++Axis)
	{
		FVector LocalVector(0.f, 0.f, 0.f);
		LocalVector[Axis] = 1.f;
		const float Dot = CameraVector | GetActorTransform().TransformVector(LocalVector);
		if (FMath::Abs(Dot) > BestDot)
		{
			OutAxis = Axis;
			BestDot = FMath::Abs(Dot);
			OutSign = Dot < 0.f ? -1 : 1;
		}
	}
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
		BlockAvatar->SetState(DefaultBlockState);
		BlockAvatar->OnIdentifyAttemptEvent.AddUObject(this, &APuzzleGrid::OnBlockIdentifyAttempt, BlockAvatar);

		// update initial visibility based on current slicing
		const bool bVisible = IsBlockVisibleWithSlicing(BlockAvatar->Block.Position);
		BlockAvatar->SetIsBlockHidden(!bVisible, false);

		BlockAvatars.Add(BlockAvatar);
		BlocksByPosition.Add(Block.Position.ToString(), BlockAvatar);
	}

	return BlockAvatar;
}

FVector APuzzleGrid::CalculateBlockLocation(FIntVector Position) const
{
	const FVector BlockSize = GetBlockSize();
	const FVector CenterOffset = FVector(PuzzleDef.Dimensions) * BlockSize * 0.5f - BlockSize * 0.5f;
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

	const FVector BlockSize = GetBlockSize();
	const FVector GridHalfSize = FVector(PuzzleDef.Dimensions) * BlockSize * 0.5f + SlicerPadding;

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
				NewHandle->SetAxisAndDimensions(Axis, PuzzleDef.Dimensions[Axis], GetBlockSize());
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
	// update slicer handle positions
	for (APuzzleGridSlicerHandle* SlicerHandle : SlicerHandles)
	{
		if (!SlicerHandle->IsDragging())
		{
			const bool bIsInvertedPosition = SlicerPosition < 0;
			if (SlicerHandle->Axis == SlicerAxis && SlicerHandle->bInvertPosition == bIsInvertedPosition)
			{
				SlicerHandle->SetPosition(SlicerPosition, false);
			}
			else
			{
				SlicerHandle->ResetPosition();
			}
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
		return Position[SlicerAxis] < PuzzleDef.Dimensions[SlicerAxis] + SlicerPosition;
	}
}

void APuzzleGrid::OnBlockIdentifyAttempt(FGameplayTag BlockType, APuzzleBlockAvatar* BlockAvatar)
{
	OnBlockIdentifyAttemptEvent.Broadcast(BlockAvatar, BlockType);
}
