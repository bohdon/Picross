﻿// Copyright Bohdon Sayre

#pragma once

#include "CoreMinimal.h"

#include "PuzzleTypes.h"
#include "GameFramework/Actor.h"

#include "PuzzleGrid.generated.h"

class APuzzleBlockAvatar;
class UPuzzleBlockMeshSet;


/**
 * The visual representation of a grid of blocks making up a puzzle
 */
UCLASS()
class PICROSS_API APuzzleGrid : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

public:
	APuzzleGrid();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzle Puzzle;

	UFUNCTION(BlueprintCallable)
	void SetPuzzle(const FPuzzle& InPuzzle, bool bRegenerateBlocks = true);

	/** The type to use when creating empty blocks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EmptyBlockType;

	/** If true, generate block avatars for empty blocks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGenerateEmptyBlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPuzzleBlockMeshSet* BlockMeshSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APuzzleBlockAvatar> BlockAvatarClass;

	UFUNCTION(BlueprintCallable)
	void GenerateBlockAvatars();

	/** Regenerate all block avatars for this puzzle */
	UFUNCTION(BlueprintCallable)
	void RegenerateBlockAvatars();

	UFUNCTION(BlueprintCallable)
	void DestroyBlockAvatars();

	UFUNCTION(BlueprintPure)
	APuzzleBlockAvatar* GetBlockAtPosition(const FIntVector& Position) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FBlockIdentifiedDelegate, APuzzleBlockAvatar* /* BlockAvatar */);

	/** Called when a block in this grid has ben identified */
	FBlockIdentifiedDelegate OnBlockIdentifiedEvent;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	APuzzleBlockAvatar* CreateBlockAvatar(const FPuzzleBlock& Block);

	/** Calculate the relative location to use for a block in the grid */
	FVector CalculateBlockLocation(FIntVector Position) const;

	void OnBlockStateChanged(EPuzzleBlockState NewState, EPuzzleBlockState OldState,
	                         APuzzleBlockAvatar* BlockAvatar);

protected:
	/** All block avatars in this grid. */
	UPROPERTY(Transient)
	TArray<APuzzleBlockAvatar*> BlockAvatars;

	/** Map of all blocks in the grid by their position */
	UPROPERTY(Transient)
	TMap<FString, APuzzleBlockAvatar*> BlocksByPosition;
};
