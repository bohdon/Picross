// Copyright Bohdon Sayre.


#include "PuzzleBlockAvatar.h"

#include "PicrossGameSettings.h"


APuzzleBlockAvatar::APuzzleBlockAvatar()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void APuzzleBlockAvatar::SetBlock(const FPuzzleBlock& InBlock)
{
	Block = InBlock;
	UpdateMesh();
}

void APuzzleBlockAvatar::SetState(EPuzzleBlockAvatarState NewState)
{
	const EPuzzleBlockAvatarState OldState = State;
	State = NewState;

	if (OldState != State)
	{
		// clear marked type
		if (State != EPuzzleBlockAvatarState::Unidentified)
		{
			SetMarkedType(FGameplayTag::EmptyTag);
		}

		UpdateMesh();

		OnStateChanged_BP(NewState, OldState);
		OnStateChangedEvent_BP.Broadcast(NewState, OldState);
	}
}

void APuzzleBlockAvatar::SetIsBlockHidden(bool bNewHidden)
{
	if (bIsBlockHidden != bNewHidden)
	{
		bIsBlockHidden = bNewHidden;

		if (bIsBlockHidden)
		{
			OnBlockHidden_BP();
		}
		else
		{
			OnBlockShown_BP();
		}
	}
}

bool APuzzleBlockAvatar::IsIdentified() const
{
	return State == EPuzzleBlockAvatarState::Identified ||
		State == EPuzzleBlockAvatarState::TrueForm;
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
	}
}

void APuzzleBlockAvatar::NotifyGuessedWrong_Implementation()
{
}
