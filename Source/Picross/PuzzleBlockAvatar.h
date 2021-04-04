// Copyright Bohdon Sayre.

#pragma once

#include "CoreMinimal.h"

#include "PuzzleBlockMeshSet.h"
#include "PuzzleTypes.h"
#include "GameFramework/Actor.h"

#include "PuzzleBlockAvatar.generated.h"

class UStaticMeshComponent;


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
	FPuzzleBlockDef Block;

	UFUNCTION(BlueprintCallable)
	void SetBlock(const FPuzzleBlockDef& InBlock);

	/** Set the annotations to display */
	UFUNCTION(BlueprintCallable)
	void SetAnnotations(const FPuzzleBlockAnnotations& InAnnotations);

	/** Set whether any annotations on this block should be visible */
	UFUNCTION(BlueprintNativeEvent)
    void SetAnnotationsVisible(bool bNewVisible);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UPuzzleBlockMeshSet* BlockMeshSet;

	/** The currently displayed annotations for this block */
	UPROPERTY(Transient, BlueprintReadOnly)
	FPuzzleBlockAnnotations Annotations;

	/** The current display state of the block */
	UPROPERTY(Transient, BlueprintReadOnly)
	EPuzzleBlockState State;

	/** Set the current state of the block */
	UFUNCTION(BlueprintCallable)
	void SetState(EPuzzleBlockState NewState);

	/**
	 * Attempt to identify this block. If the type matches this block type it
	 * will be successfully identified.
	 */
	UFUNCTION(BlueprintCallable)
	void Identify(FGameplayTag GuessType);

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

	/** Called when this block has been incorrectly identified */
	UFUNCTION(BlueprintNativeEvent)
	void OnIncorrectIdentify(FGameplayTag GuessedType);

	/** Called when the annotations for this block have changed */
	void OnAnnotationsChanged();

	/** Called when the annotations for this block have changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnAnnotationsChanged"))
	void OnAnnotationsChanged_BP();

	/** Return the objects that handle displaying row annotations for an axis of this block */
	UFUNCTION(BlueprintNativeEvent)
	TArray<UObject*> GetAnnotationDisplayObjects(int32 Axis) const;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBlockShown"))
	void OnBlockShown_BP();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBlockHidden"))
	void OnBlockHidden_BP();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStateChanged"))
	void OnStateChanged_BP(EPuzzleBlockState NewState, EPuzzleBlockState OldState);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FStateChangedDelegate, EPuzzleBlockState /* NewState */,
	                                     EPuzzleBlockState /* OldState */);

	/** Called when the state of this block has changed */
	FStateChangedDelegate OnStateChangedEvent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateChangedDynDelegate, EPuzzleBlockState, NewState,
	                                             EPuzzleBlockState, OldState);

	/** Called when the state of this block has changed */
	UPROPERTY(BlueprintAssignable)
	FStateChangedDynDelegate OnStateChangedEvent_BP;


	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnMarkedTypeChanged"))
	void OnMarkedTypeChanged_BP(FGameplayTag NewMarkedType, FGameplayTag OldMarkedType);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMarkedTypeChangedDynDelegate, FGameplayTag, NewMarkedType,
	                                             FGameplayTag, OldMarkedType);

	UPROPERTY(BlueprintAssignable)
	FMarkedTypeChangedDynDelegate OnMarkedTypeChangedEvent_BP;

protected:
	void SetDisplayedAnnotation(UObject* DisplayObject, const FPuzzleRowAnnotations& Annotation) const;

public:
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
};
