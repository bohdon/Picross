// Copyright Bohdon Sayre.


#include "PuzzlePlayer.h"


APuzzlePlayer::APuzzlePlayer()
	: bIsStarted(false)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PuzzleGridClass = APuzzleGrid::StaticClass();
}

void APuzzlePlayer::Start()
{
	if (bIsStarted)
	{
		return;
	}

	if (!PuzzleGrid)
	{
		PuzzleGrid = CreatePuzzleGrid();
		if (!PuzzleGrid)
		{
			return;
		}
	}

	PuzzleGrid->SetPuzzle(Puzzle);

	bIsStarted = true;
}

APuzzleGrid* APuzzlePlayer::CreatePuzzleGrid()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.ObjectFlags = RF_Transient;

	APuzzleGrid* Avatar = GetWorld()->SpawnActor<APuzzleGrid>(PuzzleGridClass, GetActorTransform(),
	                                                          SpawnParameters);
	return Avatar;
}
