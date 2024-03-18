// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Framework/Commands/Commands.h"

class FSuperManagerUICommands : public TCommands<FSuperManagerUICommands>
{
public:
	//�R���X�g���N�^��TCommands�̐ݒ�
	FSuperManagerUICommands() :TCommands<FSuperManagerUICommands>(
		TEXT("SuperManager"),
		FText::FromString(TEXT("Super Manager UI Commands")),
		NAME_None,
		TEXT("SuperManager")
	) {}

	//UI�R�}���h�̓o�^����
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockActorSelection;
	TSharedPtr<FUICommandInfo> DisplayListOfLockedActors;
};