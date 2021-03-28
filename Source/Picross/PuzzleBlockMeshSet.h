// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "PuzzleBlockMeshSet.generated.h"


/**
 * A set of meshes for each puzzle block type
 */
UCLASS(BlueprintType)
class PICROSS_API UPuzzleBlockMeshSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPuzzleBlockMeshSet();

	/** The dimensions of a single block */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector BlockSize;

	/** The colors associated with each block type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Categories = "Block.Type"))
	TMap<FGameplayTag, FLinearColor> TypeColors;

	/** The block meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Block.Type"))
	TMap<FGameplayTag, UStaticMesh*> Meshes;
};
