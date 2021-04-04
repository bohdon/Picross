// Copyright Bohdon Sayre.


#include "PuzzlePlayer.h"

#include "Picross.h"
#include "PicrossGameModeBase.h"
#include "PicrossGameSettings.h"
#include "PuzzleBlockAvatar.h"
#include "Kismet/GameplayStatics.h"


APuzzlePlayer::APuzzlePlayer()
	: SmoothInputSpeed(10.f),
	  RotateSpeed(45.f),
	  MaxPitchAngle(85.f),
	  bIsStarted(false)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PuzzleGridClass = APuzzleGrid::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void APuzzlePlayer::Start()
{
	if (bIsStarted)
	{
		return;
	}

	if (!PuzzleGrid)
	{
		PuzzleGrid = CreatePuzzleGrid();
		if (!PuzzleGrid)
		{
			return;
		}
	}

	PuzzleGrid->SetPuzzle(Puzzle);
	UpdateAllAnnotations();

	bIsStarted = true;
}

void APuzzlePlayer::SetPuzzleRotation(float Pitch, float Yaw)
{
	RotatePitch = FMath::ClampAngle(Pitch, -MaxPitchAngle, MaxPitchAngle);
	RotateYaw = FRotator::NormalizeAxis(Yaw);
}

void APuzzlePlayer::AddRotateRightInput(float Value)
{
	RotateRightInput += Value;
}

void APuzzlePlayer::AddRotateUpInput(float Value)
{
	RotateUpInput += Value;
}

void APuzzlePlayer::OnBlockIdentified(APuzzleBlockAvatar* BlockAvatar)
{
	// TODO(bsayre): Update only changed block annotations

	// update all block annotations
	UpdateAllAnnotations();
}

void APuzzlePlayer::UpdateAllAnnotations()
{
	RegenerateAllAnnotations();
	RefreshAllBlockAnnotations();
}

void APuzzlePlayer::RefreshAllBlockAnnotations()
{
	for (int32 X = 0; X < Puzzle.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < Puzzle.Dimensions.Y; ++Y)
		{
			for (int32 Z = 0; Z < Puzzle.Dimensions.Z; ++Z)
			{
				FIntVector Position(X, Y, Z);
				APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(Position);
				if (!BlockAvatar)
				{
					UE_LOG(LogPicross, Warning, TEXT("Missing block avatar for position: %s"), *Position.ToString());
					continue;
				}

				BlockAvatar->SetAnnotations(GetBlockAnnotations(Position));
			}
		}
	}
}

void APuzzlePlayer::RegenerateAllAnnotations()
{
	XAnnotations.Reset();
	YAnnotations.Reset();
	ZAnnotations.Reset();

	// x-axis
	for (int32 Y = 0; Y < Puzzle.Dimensions.Y; ++Y)
	{
		for (int32 Z = 0; Z < Puzzle.Dimensions.Z; ++Z)
		{
			const FIntVector StartPosition(0, Y, Z);
			FPuzzleRowAnnotation XAnnotation = CalculateRowAnnotation(Puzzle, StartPosition, 0);
			XAnnotations.Add(StartPosition.ToString(), XAnnotation);
		}
	}

	// y-axis
	for (int32 X = 0; X < Puzzle.Dimensions.X; ++X)
	{
		for (int32 Z = 0; Z < Puzzle.Dimensions.Z; ++Z)
		{
			const FIntVector StartPosition(X, 0, Z);
			FPuzzleRowAnnotation YAnnotation = CalculateRowAnnotation(Puzzle, StartPosition, 1);
			YAnnotations.Add(StartPosition.ToString(), YAnnotation);
		}
	}

	// z-axis
	for (int32 X = 0; X < Puzzle.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < Puzzle.Dimensions.Y; ++Y)
		{
			const FIntVector StartPosition(X, Y, 0);
			FPuzzleRowAnnotation ZAnnotation = CalculateRowAnnotation(Puzzle, StartPosition, 2);
			ZAnnotations.Add(StartPosition.ToString(), ZAnnotation);
		}
	}
}

FPuzzleBlockAnnotations APuzzlePlayer::GetBlockAnnotations(FIntVector Position) const
{
	FPuzzleBlockAnnotations Annotations;
	Annotations.XAnnotation = GetRowAnnotation(Position, 0);
	Annotations.YAnnotation = GetRowAnnotation(Position, 1);
	Annotations.ZAnnotation = GetRowAnnotation(Position, 2);
	return Annotations;
}

FPuzzleRowAnnotation APuzzlePlayer::GetRowAnnotation(FIntVector Position, int32 Axis) const
{
	FIntVector RowStartPosition = Position;
	RowStartPosition[Axis] = 0;

	const TMap<FString, FPuzzleRowAnnotation>& Annotations = Axis == 0
		                                                         ? XAnnotations
		                                                         : (Axis == 1 ? YAnnotations : ZAnnotations);

	return Annotations.FindRef(RowStartPosition.ToString());
}

void APuzzlePlayer::BeginPlay()
{
	APicrossGameModeBase* GameMode = GetWorld()->GetAuthGameMode<APicrossGameModeBase>();
	if (GameMode)
	{
		GameMode->PuzzlePlayer = this;
	}

	Super::BeginPlay();
}

void APuzzlePlayer::Tick(float DeltaSeconds)
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

	// apply rotation to puzzle grid
	if (PuzzleGrid)
	{
		// rotations applied relative to camera
		const FRotator PitchRot = FRotator(RotatePitch, 0.f, 0.f);
		const FRotator YawRot = FRotator(0.f, RotateYaw, 0.f);
		const FRotator NewRotation = (FQuat(CameraRotation) * FQuat(PitchRot) * FQuat(YawRot)).Rotator();
		PuzzleGrid->SetActorRotation(NewRotation);
	}
}

FPuzzleRowAnnotation APuzzlePlayer::CalculateRowAnnotation(const FPuzzle& InPuzzle, FIntVector Position,
                                                           int32 Axis) const
{
	const FGameplayTag EmptyTag = GetDefault<UPicrossGameSettings>()->BlockEmptyTag;

	FPuzzleRowAnnotation Annotation;
	TMap<FGameplayTag, FPuzzleRowTypeAnnotation> TypeAnnotations;

	FIntVector CurrentPos = Position;

	// iterate through this row of blocks along the given axis,
	// keep track of the last discovered block type to build group counts
	FGameplayTag LastType;
	const int32 Dimension = InPuzzle.Dimensions[Axis];
	for (int32 Idx = 0; Idx < Dimension; ++Idx)
	{
		CurrentPos[Axis] = Idx;

		// retrieve block avatars since they currently store state, which is relevant for annotations
		// TODO: consider storing state in the puzzle so that avatars are only visuals, not data models
		APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(CurrentPos);
		if (!BlockAvatar)
		{
			continue;
		}

		if (BlockAvatar->Block.Type != EmptyTag)
		{
			// start a new row-type-annotation for this block type
			if (!TypeAnnotations.Contains(BlockAvatar->Block.Type))
			{
				FPuzzleRowTypeAnnotation NewTypeAnnotation;
				NewTypeAnnotation.Type = BlockAvatar->Block.Type;
				// assume all are identified, until proven false
				NewTypeAnnotation.bAreIdentified = true;
				TypeAnnotations.Add(BlockAvatar->Block.Type, NewTypeAnnotation);
			}

			FPuzzleRowTypeAnnotation& TypeAnnotation = TypeAnnotations[BlockAvatar->Block.Type];

			// increase number of found blocks
			++TypeAnnotation.NumBlocks;

			// increment group count if last block type was different
			if (LastType != BlockAvatar->Block.Type)
			{
				++TypeAnnotation.NumGroups;
			}

			// check if identified
			if (!BlockAvatar->IsIdentified())
			{
				TypeAnnotation.bAreIdentified = false;
			}
		}

		LastType = BlockAvatar->Block.Type;
	}

	if (TypeAnnotations.Num() == 0)
	{
		// add a 0 annotation
		FPuzzleRowTypeAnnotation ZeroTypeAnnotation;
		ZeroTypeAnnotation.Type = EmptyTag;
		TypeAnnotations.Add(EmptyTag, ZeroTypeAnnotation);
	}

	// store type annotations on the full annotation
	TypeAnnotations.GenerateValueArray(Annotation.TypeAnnotations);

	return Annotation;
}

APuzzleGrid* APuzzlePlayer::CreatePuzzleGrid()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.ObjectFlags = RF_Transient;

	APuzzleGrid* Grid = GetWorld()->SpawnActor<APuzzleGrid>(PuzzleGridClass, GetActorTransform(),
	                                                        SpawnParameters);
	if (Grid)
	{
		Grid->OnBlockIdentifiedEvent.AddUObject(this, &APuzzlePlayer::OnBlockIdentified);
	}
	return Grid;
}

FRotator APuzzlePlayer::GetPlayerCameraRotation()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->PlayerCameraManager)
	{
		return PC->PlayerCameraManager->GetCameraRotation();
	}
	return FRotator();
}
