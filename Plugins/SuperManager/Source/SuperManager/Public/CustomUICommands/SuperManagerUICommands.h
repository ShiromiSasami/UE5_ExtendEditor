// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Framework/Commands/Commands.h"

class FSuperManagerUICommands : public TCommands<FSuperManagerUICommands>
{
public:
	//コンストラクタ兼TCommandsの設定
	FSuperManagerUICommands() :TCommands<FSuperManagerUICommands>(
		TEXT("SuperManager"),
		FText::FromString(TEXT("Super Manager UI Commands")),
		NAME_None,
		TEXT("SuperManager")
	) {}

	//UIコマンドの登録処理
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockActorSelection;
	TSharedPtr<FUICommandInfo> DisplayListOfLockedActors;
};