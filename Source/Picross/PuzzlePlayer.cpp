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

	PuzzleGrid->SetPuzzle(PuzzleDef);
	RegenerateAllAnnotations();
	RefreshAllBlockAnnotations();

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

void APuzzlePlayer::BreakZeroRows()
{
	for (int32 X = 0; X < PuzzleDef.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < PuzzleDef.Dimensions.Y; ++Y)
		{
			for (int32 Z = 0; Z < PuzzleDef.Dimensions.Z; ++Z)
			{
				const FIntVector Position(X, Y, Z);
				FPuzzleRowAnnotations RowAnnotations;

				Annotations.GetRowAnnotations(FPuzzleRow(Position, 0), RowAnnotations);
				if (RowAnnotations.bIsVisible && RowAnnotations.IsZeroAnnotation())
				{
					AutoIdentifyBlocksInRow(FPuzzleRow(Position, 0));
				}

				Annotations.GetRowAnnotations(FPuzzleRow(Position, 1), RowAnnotations);
				if (RowAnnotations.bIsVisible && RowAnnotations.IsZeroAnnotation())
				{
					AutoIdentifyBlocksInRow(FPuzzleRow(Position, 1));
				}

				Annotations.GetRowAnnotations(FPuzzleRow(Position, 2), RowAnnotations);
				if (RowAnnotations.bIsVisible && RowAnnotations.IsZeroAnnotation())
				{
					AutoIdentifyBlocksInRow(FPuzzleRow(Position, 2));
				}
			}
		}
	}
}

void APuzzlePlayer::AutoIdentifyBlocksInRow(FPuzzleRow Row)
{
	FIntVector CurrentPos = Row.Position;
	if (PuzzleGrid && Row.IsValid())
	{
		for (int32 Idx = 0; Idx < PuzzleDef.Dimensions[Row.Axis]; ++Idx)
		{
			CurrentPos[Row.Axis] = Idx;

			// TODO: flip this relationship, identify the puzzle via the player, then notify the block avatar
			APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(CurrentPos);
			if (BlockAvatar)
			{
				BlockAvatar->Identify(BlockAvatar->Block.Type);
			}
		}
	}
}

void APuzzlePlayer::RefreshAllBlockAnnotations()
{
	for (int32 X = 0; X < PuzzleDef.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < PuzzleDef.Dimensions.Y; ++Y)
		{
			for (int32 Z = 0; Z < PuzzleDef.Dimensions.Z; ++Z)
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

bool APuzzlePlayer::IsRowIdentified(FPuzzleRow Row) const
{
	Row.Normalize();
	if (Row.IsValid() && PuzzleGrid)
	{
		FIntVector Pos = Row.Position;
		for (int32 Idx = 0; Idx < PuzzleDef.Dimensions[Row.Axis]; ++Idx)
		{
			Pos[Row.Axis] = Idx;

			// TODO(bsayre): Use FPuzzleBlock instead of block avatars
			APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(Pos);
			if (BlockAvatar && !BlockAvatar->IsIdentified())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool APuzzlePlayer::IsRowTypeIdentified(FPuzzleRow Row, FGameplayTag BlockType) const
{
	Row.Normalize();
	if (Row.IsValid() && PuzzleGrid)
	{
		FIntVector Pos = Row.Position;
		for (int32 Idx = 0; Idx < PuzzleDef.Dimensions[Row.Axis]; ++Idx)
		{
			Pos[Row.Axis] = Idx;

			// TODO(bsayre): Use FPuzzleBlock instead of block avatars
			APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(Pos);
			if (BlockAvatar && BlockAvatar->Block.Type == BlockType && !BlockAvatar->IsIdentified())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void APuzzlePlayer::RegenerateAllAnnotations()
{
	FPuzzleAnnotations::GenerateAnnotations(PuzzleDef, Annotations);
}

FPuzzleBlockAnnotations APuzzlePlayer::GetBlockAnnotations(FIntVector Position) const
{
	FPuzzleBlockAnnotations Result;
	Annotations.GetBlockAnnotations(Position, Result);

	// get identified state for each type row
	for (FPuzzleRowTypeAnnotation& RowTypeAnnotations : Result.XAnnotations.TypeAnnotations)
	{
		RowTypeAnnotations.bAreIdentified = IsRowTypeIdentified(FPuzzleRow(Position, 0), RowTypeAnnotations.Type);
	}
	for (FPuzzleRowTypeAnnotation& RowTypeAnnotations : Result.YAnnotations.TypeAnnotations)
	{
		RowTypeAnnotations.bAreIdentified = IsRowTypeIdentified(FPuzzleRow(Position, 1), RowTypeAnnotations.Type);
	}
	for (FPuzzleRowTypeAnnotation& RowTypeAnnotations : Result.ZAnnotations.TypeAnnotations)
	{
		RowTypeAnnotations.bAreIdentified = IsRowTypeIdentified(FPuzzleRow(Position, 2), RowTypeAnnotations.Type);
	}

	return Result;
}

FPuzzleRowAnnotations APuzzlePlayer::GetRowAnnotation(FPuzzleRow Row) const
{
	FPuzzleRowAnnotations Result;
	Annotations.GetRowAnnotations(Row, Result);
	return Result;
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

void APuzzlePlayer::SetAllBlockAnnotationsVisible(bool bNewVisible)
{
	if (PuzzleGrid)
	{
		for (APuzzleBlockAvatar* BlockAvatar : PuzzleGrid->GetBlockAvatars())
		{
			if (BlockAvatar)
			{
				BlockAvatar->SetAnnotationsVisible(bNewVisible);
			}
		}
	}
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

void APuzzlePlayer::CheckPuzzleSolved()
{
	if (!PuzzleGrid)
	{
		return;
	}

	for (int32 X = 0; X < PuzzleDef.Dimensions.X; ++X)
	{
		for (int32 Y = 0; Y < PuzzleDef.Dimensions.Y; ++Y)
		{
			for (int32 Z = 0; Z < PuzzleDef.Dimensions.Z; ++Z)
			{
				APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(FIntVector(X, Y, Z));
				if (BlockAvatar && !BlockAvatar->IsIdentified())
				{
					return;
				}
			}
		}
	}

	bIsSolved = true;

	SetAllBlockAnnotationsVisible(false);

	// all blocks identified
	// TODO(bsayre): add other events, setup game mode to change state, etc
	OnPuzzleSolved_BP();
}

void APuzzlePlayer::CheckRowSolved(FPuzzleRow Row)
{
	Row.Normalize();
	const bool bIsRowSolved = IsRowIdentified(Row);

	if (bIsRowSolved)
	{
		if (PuzzleGrid)
		{
			FIntVector Pos = Row.Position;
			for (int32 Idx = 0; Idx < PuzzleDef.Dimensions[Row.Axis]; ++Idx)
			{
				Pos[Row.Axis] = Idx;

				APuzzleBlockAvatar* BlockAvatar = PuzzleGrid->GetBlockAtPosition(Pos);
				if (BlockAvatar)
				{
					BlockAvatar->SetState(EPuzzleBlockState::TrueForm);
				}
			}
		}

		OnRowRevealed_BP(Row);
	}
}

void APuzzlePlayer::OnBlockIdentified(APuzzleBlockAvatar* BlockAvatar)
{
	CheckPuzzleSolved();

	CheckRowSolved(FPuzzleRow(BlockAvatar->Block.Position, 0));
	CheckRowSolved(FPuzzleRow(BlockAvatar->Block.Position, 1));
	CheckRowSolved(FPuzzleRow(BlockAvatar->Block.Position, 2));

	if (!bIsSolved)
	{
		// TODO(bsayre): Update only changed block annotations
		RefreshAllBlockAnnotations();
	}
}

void APuzzlePlayer::OnRowIdentified(FPuzzleRow Row)
{
}
