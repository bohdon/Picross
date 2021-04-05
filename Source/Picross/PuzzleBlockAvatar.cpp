// Copyright Bohdon Sayre.


#include "PuzzleBlockAvatar.h"


#include "Picross.h"
#include "PicrossGameSettings.h"
#include "UI/PuzzleRowAnnotationInterface.h"


APuzzleBlockAvatar::APuzzleBlockAvatar()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void APuzzleBlockAvatar::SetBlock(const FPuzzleBlockDef& InBlock)
{
	Block = InBlock;
	UpdateMesh();
}

void APuzzleBlockAvatar::SetAnnotations(const FPuzzleBlockAnnotations& InAnnotations)
{
	Annotations = InAnnotations;

	OnAnnotationsChanged();
}

void APuzzleBlockAvatar::SetAnnotationsVisible_Implementation(bool bNewVisible)
{
	TArray<UObject*> XAnnotationObjects = GetAnnotationDisplayObjects(0);
	for (UObject* Object : XAnnotationObjects)
	{
		if (Object->Implements<UPuzzleRowAnnotationInterface>())
		{
			IPuzzleRowAnnotationInterface::Execute_SetAnnotationsVisible(Object, bNewVisible);
		}
	}
	TArray<UObject*> YAnnotationObjects = GetAnnotationDisplayObjects(1);
	for (UObject* Object : YAnnotationObjects)
	{
		if (Object->Implements<UPuzzleRowAnnotationInterface>())
		{
			IPuzzleRowAnnotationInterface::Execute_SetAnnotationsVisible(Object, bNewVisible);
		}
	}
	TArray<UObject*> ZAnnotationObjects = GetAnnotationDisplayObjects(2);
	for (UObject* Object : ZAnnotationObjects)
	{
		if (Object->Implements<UPuzzleRowAnnotationInterface>())
		{
			IPuzzleRowAnnotationInterface::Execute_SetAnnotationsVisible(Object, bNewVisible);
		}
	}
}

void APuzzleBlockAvatar::SetState(EPuzzleBlockState NewState)
{
	const EPuzzleBlockState OldState = State;
	State = NewState;

	if (OldState != State)
	{
		// clear marked type
		if (State != EPuzzleBlockState::Unidentified)
		{
			SetMarkedType(FGameplayTag::EmptyTag);
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (State == EPuzzleBlockState::Identified)
		{
			UE_LOG(LogPicross, Verbose, TEXT("Identified block: %s"), *Block.ToString());
		}
#endif

		UpdateMesh();

		OnStateChanged_BP(NewState, OldState);
		OnStateChangedEvent.Broadcast(NewState, OldState);
		OnStateChangedEvent_BP.Broadcast(NewState, OldState);
	}
}

void APuzzleBlockAvatar::Identify(FGameplayTag BlockType)
{
	// broadcast, usually to a puzzle player, which will either
	// update this blocks state or notify it of incorrect identify
	OnIdentifyAttemptEvent.Broadcast(BlockType);
}

void APuzzleBlockAvatar::SetIsBlockHidden(bool bNewHidden, bool bAnimate)
{
	if (bIsBlockHidden != bNewHidden)
	{
		bIsBlockHidden = bNewHidden;

		if (bIsBlockHidden)
		{
			OnBlockHidden_BP(bAnimate);
		}
		else
		{
			OnBlockShown_BP(bAnimate);
		}
	}
}

bool APuzzleBlockAvatar::IsIdentified() const
{
	return State == EPuzzleBlockState::Identified ||
		State == EPuzzleBlockState::TrueForm;
}

bool APuzzleBlockAvatar::IsEmptySpace() const
{
	return Block.Type == GetDefault<UPicrossGameSettings>()->BlockEmptyTag;
}

bool APuzzleBlockAvatar::IsBlockVisible() const
{
	return !bIsBlockHidden && !(IsEmptySpace() && IsIdentified());
}

void APuzzleBlockAvatar::SetMarkedType(FGameplayTag NewMarkedType)
{
	const FGameplayTag OldMarkedType = MarkedType;
	MarkedType = NewMarkedType;

	if (OldMarkedType != MarkedType)
	{
		OnMarkedTypeChanged_BP(MarkedType, OldMarkedType);
		OnMarkedTypeChangedEvent_BP.Broadcast(MarkedType, OldMarkedType);
	}
}

void APuzzleBlockAvatar::UpdateMesh()
{
	if (BlockMeshSet)
	{
		const FGameplayTag StateType = (IsEmptySpace() || !IsIdentified())
			                               ? GetDefault<UPicrossGameSettings>()->BlockUnidentifiedTag
			                               : Block.Type;
		UStaticMesh* BlockMesh = BlockMeshSet->Meshes.FindRef(StateType);
		Mesh->SetStaticMesh(BlockMesh);

		OnMeshUpdated();
	}
}

void APuzzleBlockAvatar::OnMeshUpdated_Implementation()
{
}

void APuzzleBlockAvatar::OnAnnotationsChanged()
{
	TArray<UObject*> XAnnotationObjects = GetAnnotationDisplayObjects(0);
	for (UObject* Object : XAnnotationObjects)
	{
		SetDisplayedAnnotation(Object, Annotations.XAnnotations);
	}
	TArray<UObject*> YAnnotationObjects = GetAnnotationDisplayObjects(1);
	for (UObject* Object : YAnnotationObjects)
	{
		SetDisplayedAnnotation(Object, Annotations.YAnnotations);
	}
	TArray<UObject*> ZAnnotationObjects = GetAnnotationDisplayObjects(2);
	for (UObject* Object : ZAnnotationObjects)
	{
		SetDisplayedAnnotation(Object, Annotations.ZAnnotations);
	}

	OnAnnotationsChanged_BP();
}

void APuzzleBlockAvatar::SetDisplayedAnnotation(UObject* DisplayObject, const FPuzzleRowAnnotations& Annotation) const
{
	if (DisplayObject->Implements<UPuzzleRowAnnotationInterface>())
	{
		IPuzzleRowAnnotationInterface::Execute_SetPuzzleRowAnnotations(DisplayObject, Annotation);
	}
}

TArray<UObject*> APuzzleBlockAvatar::GetAnnotationDisplayObjects_Implementation(int32 Axis) const
{
	return TArray<UObject*>();
}

void APuzzleBlockAvatar::OnIncorrectIdentify_Implementation(FGameplayTag GuessedType)
{
}
