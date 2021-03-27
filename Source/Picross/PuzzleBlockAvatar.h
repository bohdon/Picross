// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "PuzzleBlockMeshSet.h"
#include "PuzzleTypes.h"
#include "GameFramework/Actor.h"

#include "PuzzleBlockAvatar.generated.h"

class UStaticMeshComponent;


UENUM(BlueprintType)
enum class EPuzzleBlockAvatarState : uint8
{
	/** Default, unidentified, generic block */
	Unidentified,
	/** Correctly identified type */
	Identified,
	/** Revealed, true form of the block */
	TrueForm,
};


/**
 * The visual representation of a single puzzle block
 */
UCLASS()
class PICROSS_API APuzzleBlockAvatar : public AActor
{
	GENERATED_BODY()

	/** The mesh representing this block */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** The mesh representing this block */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

public:
	APuzzleBlockAvatar();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPuzzleBlock Block;

	UFUNCTION(BlueprintCallable)
	void SetBlock(const FPuzzleBlock& InBlock);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UPuzzleBlockMeshSet* BlockMeshSet;

	/** The current display state of the block */
	UPROPERTY(Transient, BlueprintReadOnly)
	EPuzzleBlockAvatarState State;

	/** Set the current state of the block */
	UFUNCTION(BlueprintCallable)
	void SetState(EPuzzleBlockAvatarState NewState);

	/** The marked type of the block, if any */
	UPROPERTY(Transient, BlueprintReadOnly)
	FGameplayTag MarkedType;

	/** Set the current marked type of the block */
	UFUNCTION(BlueprintCallable)
	void SetMarkedType(FGameplayTag NewMarkedType);

	/** Is the block currently hidden temporarily? */
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsBlockHidden;

	/** Is the block currently hidden temporarily? */
	UFUNCTION(BlueprintCallable)
	void SetIsBlockHidden(bool bNewHidden);

	/** Return true if the block is Identified or in its TrueForm */
	UFUNCTION(BlueprintPure)
	bool IsIdentified() const;

	/** Return true if the block represents an empty space */
	UFUNCTION(BlueprintPure)
	bool IsEmptySpace() const;

	/**
	 * Is the block currently visible to the player?
	 * False if an empty space block has been destroyed, or the block is hidden due to slicing.
	 */
	UFUNCTION(BlueprintPure)
	bool IsBlockVisible() const;

	/** Update the mesh to display for the current state */
	UFUNCTION(BlueprintCallable)
	void UpdateMesh();

	UFUNCTION(BlueprintNativeEvent)
	void NotifyGuessedWrong();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBlockShown"))
	void OnBlockShown_BP();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBlockHidden"))
	void OnBlockHidden_BP();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStateChanged"))
	void OnStateChanged_BP(EPuzzleBlockAvatarState NewState, EPuzzleBlockAvatarState OldState);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateChangedDynDelegate, EPuzzleBlockAvatarState, NewState,
	                                             EPuzzleBlockAvatarState, OldState);

	UPROPERTY(BlueprintAssignable)
	FStateChangedDynDelegate OnStateChangedEvent_BP;


	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnMarkedTypeChanged"))
	void OnMarkedTypeChanged_BP(FGameplayTag NewMarkedType, FGameplayTag OldMarkedType);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMarkedTypeChangedDynDelegate, FGameplayTag, NewMarkedType,
	                                             FGameplayTag, OldMarkedType);

	UPROPERTY(BlueprintAssignable)
	FMarkedTypeChangedDynDelegate OnMarkedTypeChangedEvent_BP;

public:
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
};
