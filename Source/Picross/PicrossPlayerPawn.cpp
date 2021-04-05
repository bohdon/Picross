// Copyright Bohdon Sayre.


#include "PicrossPlayerPawn.h"

#include "DrawDebugHelpers.h"
#include "PicrossGameplayStatics.h"
#include "PuzzleBlockAvatar.h"
#include "PuzzleGrid.h"
#include "Kismet/GameplayStatics.h"

TAutoConsoleVariable<bool> CVarDebugInputTraces(
	TEXT("game.DebugInputTraces"), false,
	TEXT("Display debug info for traces for clicking blocks and other input"));


APicrossPlayerPawn::APicrossPlayerPawn()
	: TraceMaxDistance(10000.f),
	  TraceSphereRadius(1.f),
	  TraceChannel(ECC_Visibility)
{
	PrimaryActorTick.bCanEverTick = true;
}

void APicrossPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APuzzleGrid* PuzzleGrid = UPicrossGameplayStatics::GetPuzzleGrid(this))
	{
		APlayerController* PC = GetController<APlayerController>();
		if (PC && PC->PlayerCameraManager)
		{
			// set puzzles to keep their world rotation relative to the player camera
			FMinimalViewInfo ViewInfo;
			CalcCamera(0.f, ViewInfo);
			const FRotator CameraRotation = ViewInfo.Rotation;
			PuzzleGrid->SetPuzzleRotation(-CameraRotation.Pitch, -CameraRotation.Yaw);
		}
	}
}

void APicrossPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APicrossPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	for (auto& Elem : IdInputBlockTypes)
	{
		const FName ActionName = Elem.Key;
		const FGameplayTag BlockType = Elem.Value;
		PlayerInputComponent->BindAction<FIdInputDelegate>(ActionName, IE_Pressed,
		                                                   this, &APicrossPlayerPawn::IdInputPressed, BlockType);
	}
	PlayerInputComponent->BindAxis(FName("RotatePuzzleRight"), this, &APicrossPlayerPawn::RotatePuzzleRight);
	PlayerInputComponent->BindAxis(FName("RotatePuzzleUp"), this, &APicrossPlayerPawn::RotatePuzzleUp);
}

void APicrossPlayerPawn::IdInputPressed(FGameplayTag BlockType)
{
	APuzzleBlockAvatar* BlockAvatar = TraceForBlockAvatarUnderMouse();
	if (BlockAvatar)
	{
		BlockAvatar->Identify(BlockType);
	}
}

void APicrossPlayerPawn::RotatePuzzleRight(float Value)
{
	APuzzleGrid* PuzzleGrid = UPicrossGameplayStatics::GetPuzzleGrid(this);
	if (PuzzleGrid)
	{
		PuzzleGrid->AddRotateRightInput(Value);
	}
}

void APicrossPlayerPawn::RotatePuzzleUp(float Value)
{
	APuzzleGrid* PuzzleGrid = UPicrossGameplayStatics::GetPuzzleGrid(this);
	if (PuzzleGrid)
	{
		PuzzleGrid->AddRotateUpInput(Value);
	}
}

bool APicrossPlayerPawn::GetTracePositionAndDirection(FVector& WorldPosition, FVector& WorldDirection) const
{
	APlayerController* Player = GetController<APlayerController>();
	ULocalPlayer* LocalPlayer = Player ? Player->GetLocalPlayer() : nullptr;

	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		// get mouse position
		FVector2D MousePosition;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
		{
			if (UGameplayStatics::DeprojectScreenToWorld(Player, MousePosition, WorldPosition, WorldDirection))
			{
				return true;
			}
		}
	}

	return false;
}

APuzzleBlockAvatar* APicrossPlayerPawn::TraceForBlockAvatar(FVector WorldPosition, FVector WorldDirection) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Start = WorldPosition;
	const FVector End = WorldPosition + WorldDirection.GetSafeNormal() * TraceMaxDistance;

	FCollisionQueryParams Params;
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(this);
	FCollisionShape Shape = FCollisionShape::MakeSphere(TraceSphereRadius);

	FHitResult Hit;
	World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, TraceChannel, Shape, Params);

	if (Hit.bBlockingHit)
	{
#if ENABLE_DRAW_DEBUG
		if (CVarDebugInputTraces.GetValueOnAnyThread())
		{
			DrawDebugPoint(GetWorld(), Hit.Location, 8.f, FColor::Red, false, 3.f);
			DrawDebugString(GetWorld(), Hit.Location, Hit.Actor.IsValid() ? Hit.Actor->GetName() : FString("(null)"),
			                nullptr, FColor::White, 3.f);
		}
#endif

		APuzzleBlockAvatar* HitBlock = Cast<APuzzleBlockAvatar>(Hit.Actor.Get());
		if (HitBlock)
		{
			return HitBlock;
		}
	}
	return nullptr;
}

APuzzleBlockAvatar* APicrossPlayerPawn::TraceForBlockAvatarUnderMouse() const
{
	FVector WorldPosition;
	FVector WorldDirection;
	GetTracePositionAndDirection(WorldPosition, WorldDirection);
	return TraceForBlockAvatar(WorldPosition, WorldDirection);
}
