// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "PuzzleBlockMeshSet.generated.h"


/**
 * A set of meshes for each puzzle block type
 */
UCLASS()
class PICROSS_API UPuzzleBlockMeshSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The block meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, UStaticMesh*> Meshes;
};
