// Copyright Bohdon Sayre.


#include "PicrossPlayerPawn.h"


#include "PuzzleBlockAvatar.h"
#include "Kismet/GameplayStatics.h"


APicrossPlayerPawn::APicrossPlayerPawn()
	: TraceMaxDistance(1000.f),
	  TraceSphereRadius(1.f),
	  TraceChannel(ECC_Visibility)
{
	PrimaryActorTick.bCanEverTick = true;
}

void APicrossPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APicrossPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("RemoveBlock"), IE_Pressed, this, &APicrossPlayerPawn::RemoveBlockPressed);
	PlayerInputComponent->BindAction(FName("SetTypeAlpha"), IE_Pressed, this, &APicrossPlayerPawn::SetTypeAlphaPressed);
	PlayerInputComponent->BindAction(FName("SetTypeBeta"), IE_Pressed, this, &APicrossPlayerPawn::SetTypeBetaPressed);
}

void APicrossPlayerPawn::RemoveBlockPressed()
{
	APuzzleBlockAvatar* BlockAvatar = TraceForBlockAvatarUnderMouse();
	if (BlockAvatar && !BlockAvatar->IsDiscovered())
	{
		if (BlockAvatar->IsEmptySpace())
		{
			BlockAvatar->SetState(EPuzzleBlockAvatarState::Discovered);
		}
		else
		{
			BlockAvatar->NotifyGuessedWrong();
		}
	}
}

void APicrossPlayerPawn::SetTypeAlphaPressed()
{
	SetTypePressed(FGameplayTag::RequestGameplayTag("Block.Type.Alpha"));
}

void APicrossPlayerPawn::SetTypeBetaPressed()
{
	SetTypePressed(FGameplayTag::RequestGameplayTag("Block.Type.Beta"));
}

void APicrossPlayerPawn::SetTypePressed(FGameplayTag Type)
{
	APuzzleBlockAvatar* BlockAvatar = TraceForBlockAvatarUnderMouse();
	if (BlockAvatar && !BlockAvatar->IsDiscovered())
	{
		if (BlockAvatar->Block.Type == Type)
		{
			BlockAvatar->SetState(EPuzzleBlockAvatarState::Discovered);
		}
		else
		{
			BlockAvatar->NotifyGuessedWrong();
		}
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
