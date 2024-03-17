// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidget.generated.h"

class UEditorActorSubsystem;

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickActorActionsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
private:
	bool GetEditorActorSubSystem();

private:
	UPROPERTY()
	UEditorActorSubsystem* EditorActorSubsystem;

};
