// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"


#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"

#include "PuzzleDesigner.generated.h"

class APuzzleBlockAvatar;
class APuzzleGrid;


/**
 * Designer used to build puzzles
 */
UCLASS()
class PICROSS_API APuzzleDesigner : public AActor
{
	GENERATED_BODY()

public:
	APuzzleDesigner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector DefaultDimensions;

	/** The puzzle grid class to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APuzzleGrid> PuzzleGridClass;

	UFUNCTION(BlueprintCallable)
	void SetDimensions(FIntVector NewDimensions, bool bCommit = false);
	
	UFUNCTION(BlueprintCallable)
    void SetBlockType(FIntVector Position, FGameplayTag NewBlockType);

	/** Cleanup any puzzle blocks outside the current dimensions */
	UFUNCTION(BlueprintCallable)
	void CommitDimensions();

	FORCEINLINE APuzzleGrid* GetPuzzleGrid() const { return PuzzleGrid; }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	APuzzleGrid* PuzzleGrid;

	APuzzleGrid* CreatePuzzleGrid();

	virtual void BeginPlay() override;

	void OnBlockIdentifyAttempt(APuzzleBlockAvatar* BlockAvatar, FGameplayTag BlockType);
};
