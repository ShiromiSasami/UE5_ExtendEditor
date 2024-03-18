// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomUICommands/SuperManagerUICommands.h"

#define LOCTEXT_NAMESPACE "SuperManager"

void FSuperManagerUICommands::RegisterCommands()
{
	//UIƒRƒ}ƒ“ƒh‚Ì“o˜^
	UI_COMMAND(
		LockActorSelection,
		"Lock Actor Selection",
		"Lock actor selection in leven, one triggered, actor can no longer be selected",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt)
	);

	UI_COMMAND(
		UnlockActorSelection,
		"Unlock Actor Selection",
		"Remove selection lock on all actors",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt | EModifierKey::Shift)
	);

	UI_COMMAND(
		DisplayListOfLockedActors,
		"Display List of Locked Actors",
		"Locked All Actors On Display",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Q, EModifierKey::Alt | EModifierKey::Shift)
	);
}

#undef LOCTEXT_NAMESPACE