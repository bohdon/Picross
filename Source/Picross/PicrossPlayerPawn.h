// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "PuzzleBlockAvatar.h"
#include "GameFramework/Pawn.h"
#include "PicrossPlayerPawn.generated.h"


UCLASS()
class PICROSS_API APicrossPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	APicrossPlayerPawn();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector TraceMaxDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TraceSphereRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void RemoveBlockPressed();
	void SetTypeAlphaPressed();
	void SetTypeBetaPressed();

	void SetTypePressed(FGameplayTag Type);

	/** Return the world position and direction to use for mouse traces */
	bool GetTracePositionAndDirection(FVector& WorldPosition, FVector& WorldDirection) const;

	/** Trace for and return a block avatar if hit */
	APuzzleBlockAvatar* TraceForBlockAvatar(FVector WorldPosition, FVector WorldDirection) const;

	/** Trace for and return a block avatar if hit */
	APuzzleBlockAvatar* TraceForBlockAvatarUnderMouse() const;
};
