// Copyright Bohdon Sayre.


#include "PicrossGameplayStatics.h"

#include "PicrossGameModeBase.h"


APuzzlePlayer* UPicrossGameplayStatics::GetPuzzlePlayer(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		APicrossGameModeBase* GameMode = World->GetAuthGameMode<APicrossGameModeBase>();
		if (GameMode)
		{
			return GameMode->GetPuzzlePlayer();
		}
	}
	return nullptr;
}

APuzzleGrid* UPicrossGameplayStatics::GetPuzzleGrid(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		APicrossGameModeBase* GameMode = World->GetAuthGameMode<APicrossGameModeBase>();
		if (GameMode)
		{
			return GameMode->GetPuzzleGrid();
		}
	}
	return nullptr;
}
