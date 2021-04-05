// Copyright Bohdon Sayre.


#include "PuzzleGridSlicerHandle.h"

#include "Kismet/GameplayStatics.h"


APuzzleGridSlicerHandle::APuzzleGridSlicerHandle()
	: Axis(0),
	  bInvertPosition(false),
	  Dimension(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

void APuzzleGridSlicerHandle::SetDimension(int32 NewDimension)
{
	Dimension = NewDimension;
}

void APuzzleGridSlicerHandle::SetPrecisePosition(float NewPosition)
{
	PrecisePosition = FMath::Clamp(NewPosition, 0.f, static_cast<float>(GetMaxPosition()));
	SetPositionImpl(FMath::RoundToInt(PrecisePosition), true);
}

void APuzzleGridSlicerHandle::SetPosition(int32 NewPosition, bool bTriggerNotifiers)
{
	SetPositionImpl(bInvertPosition ? -NewPosition : NewPosition, bTriggerNotifiers);
	PrecisePosition = Position;
}

int32 APuzzleGridSlicerHandle::GetPosition() const
{
	return bInvertPosition ? -Position : Position;
}

void APuzzleGridSlicerHandle::SetAxisAndDimensions(int32 InAxis, int32 InDimension, FVector InBlockSize)
{
	Axis = FMath::Clamp(InAxis, 0, 2);
	Dimension = FMath::Max(InDimension, 1);
	BlockSize = InBlockSize;
}

void APuzzleGridSlicerHandle::SetSlicerVisible(bool bNewVisible)
{
	bIsSlicerVisible = bNewVisible;

	SetActorHiddenInGame(!bIsSlicerVisible);
	SetActorEnableCollision(bIsSlicerVisible);

	if (!bIsSlicerVisible)
	{
		if (bIsDragging)
		{
			StopDragging();
		}
	}
}

int32 APuzzleGridSlicerHandle::GetMaxPosition() const
{
	return Dimension - 1;
}

void APuzzleGridSlicerHandle::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	StartDragging();
}

void APuzzleGridSlicerHandle::NotifyActorOnReleased(FKey ButtonReleased)
{
	Super::NotifyActorOnReleased(ButtonReleased);
	StopDragging();
}

void APuzzleGridSlicerHandle::StartDragging()
{
	bIsDragging = true;
}

void APuzzleGridSlicerHandle::StopDragging()
{
	bIsDragging = false;

	PrecisePosition = Position;
}

void APuzzleGridSlicerHandle::ResetPosition(bool bTriggerNotifiers)
{
	PrecisePosition = 0.f;
	SetPositionImpl(0, false);
}

void APuzzleGridSlicerHandle::BeginPlay()
{
	Super::BeginPlay();
}

void APuzzleGridSlicerHandle::SetPositionImpl(int32 NewPosition, bool bTriggerNotifiers)
{
	const int32 OldPosition = Position;
	Position = FMath::Clamp(NewPosition, 0, GetMaxPosition());

	if (Position != OldPosition)
	{
		OnPositionChanged();
		OnPositionChanged_BP();

		if (bTriggerNotifiers)
		{
			OnPositionChangedEvent.Broadcast(GetPosition());
		}
	}
}

void APuzzleGridSlicerHandle::OnPositionChanged()
{
}

void APuzzleGridSlicerHandle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDragging)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			FVector WorldLocation;
			FVector WorldDirection;
			if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				const FVector CameraVector = PC->PlayerCameraManager->GetCameraRotation().Vector();

				const FVector PlaneNormal = FMath::Abs(CameraVector | GetActorUpVector()) > 0.5f
					                            ? GetActorUpVector()
					                            : GetActorRightVector();

				// get closest point on x-axis of this actor
				const FPlane Plane(GetActorLocation(), PlaneNormal);
				const FVector Intersect = FMath::RayPlaneIntersection(WorldLocation, WorldDirection, Plane);
				const FVector LocalLocation = GetActorTransform().InverseTransformPosition(Intersect);
				const float NewPosition = LocalLocation.X / BlockSize[Axis];

				SetPrecisePosition(NewPosition);

				if (!PC->IsInputKeyDown(EKeys::LeftMouseButton))
				{
					StopDragging();
				}
			}
		}
	}
}
