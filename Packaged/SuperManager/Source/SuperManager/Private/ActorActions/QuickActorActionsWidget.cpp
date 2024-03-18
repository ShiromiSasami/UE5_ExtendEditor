// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorActions/QuickActorActionsWidget.h"

#include "DebugHeader.h"

#include "Subsystems/EditorActorSubsystem.h"

#pragma region ActorBatchSelection

void UQuickActorActionsWidget::SelectActorsBasedOnCondition()
{
	TargetActorsForSelect.Empty();
	if (bUseSelectingActorsForSelect)
	{
		TargetActorsForSelect = EditorActorSubsystem->GetSelectedLevelActors();
	}
	else
	{
		TargetActorsForSelect = EditorActorSubsystem->GetAllLevelActors();
	}

	if (bUseKeywordForSelect)
	{
		SelectAllActorsWithKeyword();
	}
	else
	{
		SelectAllActorsWithSimilarName();
	}
}

void UQuickActorActionsWidget::SelectAllActorsWithSimilarName()
{
	if (!GetEditorActorSubSystem()) return;

	//レベル上で選択されているアクターを取得
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;

	//何も選択されていない場合エラー表示
	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}
	//選択されているアクターが複数の場合エラー表示
	if (SelectedActors.Num() > 1)
	{
		DebugHeader::ShowNotifyInfo(TEXT("You can only select one actor"));
		return;
	}
	//選択されているアクターのラベル名前を取得し一部を切り抜く
	FString SelectedActorName = SelectedActors[0]->GetActorLabel();
	const FString NameToSearch = SelectedActorName.LeftChop(4);
	//似たラベルのアクターを取得
	for (AActor* ActorInLevel : TargetActorsForSelect)
	{
		if (!ActorInLevel) { continue; }

		if (ActorInLevel->GetActorLabel().Contains(NameToSearch, SearchCaseForSelect))
		{
			//アクターを選択状態にする
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, true);
			SelectionCounter++;
		}
	}

	if (SelectionCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Successfully selected ") +
			FString::FromInt(SelectionCounter)
		);
	}
	else
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor with similar name found"));
	}
}

void UQuickActorActionsWidget::SelectAllActorsWithKeyword()
{
	if (!GetEditorActorSubSystem()) return;

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	uint32 SelectionCounter = 0;

	if (AllLevelActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor in the level"));
		return;
	}

	for (AActor* ActorInLevel : TargetActorsForSelect)
	{
		if (!ActorInLevel) { continue; }

		if (ActorInLevel->GetActorLabel().Contains(SearchKeywordForSelect, SearchCaseForSelect))
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, true);
			SelectionCounter++;
		}
		else
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, false);
		}
	}

	if (SelectionCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Successfully selected ") +
			FString::FromInt(SelectionCounter) + TEXT(" actors")
		);
	}
	else
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor with the keyword found"));
	}
}

#pragma endregion

#pragma region ActorBatchDeletion

void UQuickActorActionsWidget::DeleteAllActorsWithKeyword()
{
	if (!GetEditorActorSubSystem()) return;

	TargetActorsForDelete.Empty();
	if (bUseSelectingActorsForDelete)
	{
		TargetActorsForDelete = EditorActorSubsystem->GetSelectedLevelActors();
	}
	else
	{
		TargetActorsForDelete = EditorActorSubsystem->GetAllLevelActors();
	}

	uint32 Counter = 0;

	for (AActor* ActorInLevel : TargetActorsForDelete)
	{
		if (!ActorInLevel) { continue; }

		if (ActorInLevel->GetActorLabel().Contains(SearchKeywordForDelete, SearchCaseForDelete))
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, true);
			Counter++;
		}
		else
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, false);
		}
	}

	EditorActorSubsystem->DeleteSelectedActors(GetWorld());

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Successfully deleted ") +
			FString::FromInt(Counter) + TEXT(" actors")
		);
	}
	else
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor with the keyword found"));
	}
}

#pragma endregion

#pragma region ActorBatchDuplication

void UQuickActorActionsWidget::DuplicateActors()
{
	if (!GetEditorActorSubSystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}
	//複製数とオフセット値が0だった場合エラー表示
	if (NumberOfDuplicates <= 0 || OffsetDist == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Did not specify a number of duplications or an offset distance"));
		return;
	}

	//選択されているアクターを複製
	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) { continue; }

		for (int32 i = 0; i < NumberOfDuplicates; i++)
		{
			AActor* DuplicatedActor =
				EditorActorSubsystem->DuplicateActor(SelectedActor, SelectedActor->GetWorld());

			if (!DuplicatedActor) { continue; }

			const float DuplicationOffsetDist = (i + 1) * OffsetDist;

			switch (AxisForDuplication)
			{
				using enum E_DuplicationAxis;
			case EDA_XAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(DuplicationOffsetDist, 0.f, 0.f));
				break;

			case EDA_YAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.f, DuplicationOffsetDist, 0.f));
				break;

			case EDA_ZAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.f, 0.f, DuplicationOffsetDist));
				break;

			case EDA_MAX:
				break;

			default:
				break;
			}

			EditorActorSubsystem->SetActorSelectionState(DuplicatedActor, true);
			Counter++;
		}

		if (Counter > 0)
		{
			DebugHeader::ShowNotifyInfo(
				TEXT("Successfully duplicated ") + 
				FString::FromInt(Counter) + TEXT(" actors"));
		}
	}
}

#pragma endregion

#pragma region RandomizeActorTransform

void UQuickActorActionsWidget::RandomizeActorTransform()
{
	const bool ConditionNotSet = 
		RandomActorRotation.IsAllRotAxiesRandomize() &&
		!bRandomizeScale && !bRandomizeOffset;

	if (ConditionNotSet)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No variation condition specified"));
		return;
	}

	if(!GetEditorActorSubSystem()){ return; }

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor){ continue; }

		if (RandomActorRotation.bRandomizeRotYaw)
		{
			const float RandomRotYawValue = FMath::RandRange(RandomActorRotation.RotYawMin, RandomActorRotation.RotYawMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.f, RandomRotYawValue, 0.f));
		}

		if (RandomActorRotation.bRandomizeRotPitch)
		{
			const float RandomRotPitchValue = FMath::RandRange(RandomActorRotation.RotPitchMin, RandomActorRotation.RotPitchMax);
			SelectedActor->AddActorWorldRotation(FRotator(RandomRotPitchValue, 0.f, 0.f));
		}

		if (RandomActorRotation.bRandomizeRotRoll)
		{
			const float RandomRotRollValue = FMath::RandRange(RandomActorRotation.RotRollMin, RandomActorRotation.RotRollMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.f, 0.f, RandomRotRollValue));
		}

		if (bRandomizeScale)
		{
			SelectedActor->SetActorScale3D(FVector(FMath::RandRange(ScaleMin, ScaleMax)));
		}

		if (bRandomizeOffset)
		{
			const float RandomOffsetValue = FMath::RandRange(OffsetMin, OffsetMax);
			SelectedActor->AddActorWorldOffset(FVector(RandomOffsetValue, RandomOffsetValue, 0.f));
		}

		Counter++;
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully set ") + FString::FromInt(Counter) + TEXT(" actors"));
	}
}

#pragma endregion

#pragma region ResetActorTransform

void UQuickActorActionsWidget::ResetActorTransform()
{
	const bool ConditionNotSet = !bResetLocation && !bResetRotation && !bResetScale;

	if (ConditionNotSet)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No variation condition specified"));
		return;
	}

	if (!GetEditorActorSubSystem()) { return; }

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) { continue; }

		if (bResetLocation)
		{
			SelectedActor->SetActorLocation(FVector::ZeroVector);
		}

		if (bResetRotation)
		{
			SelectedActor->SetActorRotation(FRotator::ZeroRotator);
		}

		if (bResetScale)
		{
			SelectedActor->SetActorScale3D(FVector(1.f));
		}

		Counter++;
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully set ") + FString::FromInt(Counter) + TEXT(" actors"));
	}
}

#pragma endregion


bool UQuickActorActionsWidget::GetEditorActorSubSystem()
{
	//Editorのアクター管理システムを取得
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return EditorActorSubsystem != nullptr;
}
