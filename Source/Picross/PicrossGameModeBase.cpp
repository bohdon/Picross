// Copyright Epic Games, Inc. All Rights Reserved.


#include "PicrossGameModeBase.h"

#include "PicrossPlayerController.h"
#include "PicrossPlayerPawn.h"


APicrossGameModeBase::APicrossGameModeBase()
{
	PlayerControllerClass = APicrossPlayerController::StaticClass();
	DefaultPawnClass = APicrossPlayerPawn::StaticClass();
}
