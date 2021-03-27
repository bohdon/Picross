// Copyright Bohdon Sayre.


#include "PuzzlePlayer.h"

#include "PicrossGameModeBase.h"
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

void APuzzlePlayer::BeginPlay()
{
	Super::BeginPlay();

	APicrossGameModeBase* GameMode = GetWorld()->GetAuthGameMode<APicrossGameModeBase>();
	if (GameMode)
	{
		GameMode->PuzzlePlayer = this;
	}
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
