// Copyright Bohdon Sayre.


#include "PuzzleBlockAvatar.h"


APuzzleBlockAvatar::APuzzleBlockAvatar()
	: Dimensions(FVector(100.f, 100.f, 100.f))
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void APuzzleBlockAvatar::SetBlock(const FPuzzleBlock& InBlock)
{
	Block = InBlock;

	SetActorRelativeLocation(FVector(Block.Position.X * Dimensions.X,
	                                 Block.Position.Y * Dimensions.Y,
	                                 Block.Position.Z * Dimensions.Z));
	UpdateMesh();
}

void APuzzleBlockAvatar::SetState(EPuzzleBlockAvatarState NewState)
{
	const EPuzzleBlockAvatarState OldState = State;
	State = NewState;

	if (OldState != State)
	{
		// clear marked type
		if (State != EPuzzleBlockAvatarState::Undiscovered)
		{
			SetMarkedType(FGameplayTag::EmptyTag);
		}

		UpdateMesh();

		OnStateChanged_BP(NewState, OldState);
		OnStateChangedEvent_BP.Broadcast(NewState, OldState);
	}
}

bool APuzzleBlockAvatar::IsDiscovered() const
{
	return State == EPuzzleBlockAvatarState::Discovered ||
		State == EPuzzleBlockAvatarState::TrueForm;
}

bool APuzzleBlockAvatar::IsEmptySpace() const
{
	return Block.Type == EmptyType;
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
		const FGameplayTag StateType = (IsEmptySpace() || !IsDiscovered()) ? UndiscoveredType : Block.Type;
		UStaticMesh* BlockMesh = BlockMeshSet->Meshes.FindRef(StateType);
		Mesh->SetStaticMesh(BlockMesh);
	}
}

void APuzzleBlockAvatar::NotifyGuessedWrong_Implementation()
{
}
