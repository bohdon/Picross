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
	float TraceMaxDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TraceSphereRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel;

	/**
	 * Action input names for each block type that can be identified.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "Block.Type"))
	TMap<FName, FGameplayTag> IdInputBlockTypes;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	DECLARE_DELEGATE_OneParam(FIdInputDelegate, FGameplayTag /* BlockType */);

	void IdInputPressed(FGameplayTag BlockType);

	void RotatePuzzleRight(float Value);
	void RotatePuzzleUp(float Value);

	/** Return the world position and direction to use for mouse traces */
	bool GetTracePositionAndDirection(FVector& WorldPosition, FVector& WorldDirection) const;

	/** Trace for and return a block avatar if hit */
	APuzzleBlockAvatar* TraceForBlockAvatar(FVector WorldPosition, FVector WorldDirection) const;

	/** Trace for and return a block avatar if hit */
	APuzzleBlockAvatar* TraceForBlockAvatarUnderMouse() const;
};
