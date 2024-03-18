// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"

#include "SuperManager.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "ActorTreeItem.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments ConstructedHeaderRow = 
		SHeaderRow::Column(GetColumnID())
			.FixedWidth(24.f)
			.HAlignHeader(HAlign_Center)
			.VAlignHeader(VAlign_Center)
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.DefaultTooltip(FText::FromString(TEXT("Actor Selection Lock - Press icon to lock actor selection")))
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				//LockSelectionのImageBrushを利用
				.Image(FSuperManagerStyle::GetCreateSlateStyleSet()->GetBrush(FName("LevelEditor.LockSelection")))
			];

	return ConstructedHeaderRow;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(
	FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	//ActorTreeItemの中身が空ならなにも追加しない
	FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();
	if (!ActorTreeItem || !ActorTreeItem->IsValid()) { return SNullWidget::NullWidget; }

	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bIsActorSelectionLocked =
		SuperManagerModule.CheckIsActorSelectionLocked(ActorTreeItem->Actor.Get());

	const FCheckBoxStyle& ToggleButtonStyle =
		FSuperManagerStyle::GetCreateSlateStyleSet()->GetWidgetStyle<FCheckBoxStyle>(FName("SceneOutliner.SelectionLock"));

	//カラムにチェックボックスを追加
	TSharedRef<SCheckBox> ConstructedRowWidgetCheckBox =
		SNew(SCheckBox)
		.Visibility(EVisibility::Visible)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Style(&ToggleButtonStyle)
		.HAlign(HAlign_Center)
		.IsChecked(bIsActorSelectionLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.OnCheckStateChanged(this, &FOutlinerSelectionLockColumn::OnRowWidgetCheckStateChanged, ActorTreeItem->Actor);

	return ConstructedRowWidgetCheckBox;
}

void FOutlinerSelectionLockColumn::OnRowWidgetCheckStateChanged(
	ECheckBoxState NewState, 
	TWeakObjectPtr<AActor> CurrespondingActor)
{
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		SuperManagerModule.ProcessLockingForOutliner(CurrespondingActor.Get(), false);
		break;
	case ECheckBoxState::Checked:
		SuperManagerModule.ProcessLockingForOutliner(CurrespondingActor.Get(), true);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}
