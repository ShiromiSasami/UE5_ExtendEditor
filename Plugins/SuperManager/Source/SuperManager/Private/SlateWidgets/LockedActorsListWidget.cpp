// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/LockedActorsListWidget.h"

#include "SuperManager.h"
#include "DebugHeader.h"

#include "SlateBasics.h"


#define LIST_ALL TEXT("List All Actors in Level")
#define LIST_LOCK TEXT("List Locked Actors")
#define LIST_UNLOCK TEXT("List Unlocked Actors")
void SLockedActorsListTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	//ロック中のアクターリストを取得
	AllActorsData = InArgs._AllActorsInLevel;
	//表示するアセットデータの配列を設定
	DisplayedActorsData = AllActorsData;

	CheckBoxesArray.Empty();
	ActorToToggleLockArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_ALL));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_LOCK));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_UNLOCK));

	//タイトルテキストのフォントの設定
	FSlateFontInfo TitleTextFont = GetEmnossedTextFont();
	TitleTextFont.Size = 30.f;

	//Widgetのスロットの生成
	ChildSlot
		[
			//バーティカルボックスを生成
			SNew(SVerticalBox)
				//タイトルテキストスロットを追加
				+ SVerticalBox::Slot().AutoHeight()
				[
					//テキストブロックを生成
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Locked Actors List")))
						.Font(TitleTextFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FColor::White)
				]

				//サポートスロットの追加
				+ SVerticalBox::Slot().AutoHeight()
				[
					//表示内容の変更ドロップを追加
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							ConstractComboBox()
						]

						//説明テキストスロットを追加
						+ SHorizontalBox::Slot()
						.FillWidth(.6f)
						[
							ConstractComboHelpTexts(
								TEXT("You can manipulate the displayed contents by dropping them."),
								ETextJustify::Center)
						]
				]

				//リストスロットを追加
				//バーによるスクロールを可能にする
				+SVerticalBox::Slot().VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
						//AssetDataリストビューを生成
						+ SScrollBox::Slot()
						[
							ConstructListView()
						]
				]

				//ボタンスロットを追加
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
						//全ロックボタンを追加
						//横幅10.f 間隔5.f に設定
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructLockAllButton()
						]
						//全ロック解除ボタンを追加
						//横幅10.f 間隔5.f に設定
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructUnlockAllButton()
						]
						//全選択ボタンを追加
						//横幅10.f 間隔5.f に設定
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructSelectAllButton()
						]
						//全選択解除ボタンを追加
						//横幅10.f 間隔5.f に設定
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructDeselectAllButton()
						]
				]
		];
}

TSharedRef<SListView<TWeakObjectPtr<AActor>>> SLockedActorsListTab::ConstructListView()
{
	ConstructedActorListView =
		SNew(SListView<TWeakObjectPtr<AActor>>)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedActorsData)
		//紐づけるデリゲートを設定
		.OnGenerateRow(this, &SLockedActorsListTab::OnGenerateRowForList);

	return ConstructedActorListView.ToSharedRef();
}

void SLockedActorsListTab::RefreshActorListView()
{
	CheckBoxesArray.Empty();
	ActorToToggleLockArray.Empty();

	if (ConstructedActorListView.IsValid())
	{
		ConstructedActorListView->RebuildList();
	}
}

#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SLockedActorsListTab::ConstractComboBox()
{
	//選択肢の生成
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstractComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SLockedActorsListTab::OnGenerateComboContent)
		.OnSelectionChanged(this, &SLockedActorsListTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("List Actor Option")))
		];

	return ConstractComboBox;
}

TSharedRef<SWidget> SLockedActorsListTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<SWidget> ConstractComboText =
		SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));

	return ConstractComboText;
}

void SLockedActorsListTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	//モジュールの読み込み
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (*SelectedOption.Get() == LIST_ALL)
	{
		DisplayedActorsData = AllActorsData;
	}
	else if (*SelectedOption.Get() == LIST_LOCK)
	{
		//未使用アセットのリストを取得
		SuperManagerModule.ListLockActorForActorList(AllActorsData, DisplayedActorsData);
	}
	else if (*SelectedOption.Get() == LIST_UNLOCK)
	{
		//任意のワードを持つアセットのリストを取得
		SuperManagerModule.ListUnlockActorForActorList(AllActorsData, DisplayedActorsData);
	}

	RefreshActorListView();
}

TSharedRef<STextBlock> SLockedActorsListTab::ConstractComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock> ConstructedHelpText =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Justification(TextJustify)
		//自動折り返しを設定
		.AutoWrapText(true);

	return ConstructedHelpText;
}

#pragma endregion

#pragma region RowWidgetForAssetListView

TSharedRef<ITableRow> SLockedActorsListTab::OnGenerateRowForList(
	TWeakObjectPtr<AActor> ActorToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!ActorToDisplay.IsValid()) { return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable); }

	const FString DisplayActorName = ActorToDisplay->GetActorLabel();

	FSlateFontInfo ClassNameFont = GetEmnossedTextFont();
	ClassNameFont.Size = 10.f;
	FSlateFontInfo ActorNameFont = GetEmnossedTextFont();
	ActorNameFont.Size = 15.f;

	//リストビューのウィジェットを生成
	//要素間の間隔5.fを設定
	TSharedRef<STableRow<TSharedPtr<AActor>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<AActor>>, OwnerTable)
		.Padding(FMargin(5.f))
		[
			SNew(SHorizontalBox)
				//チェックボックスの追加
				//横左寄せ、縦中央寄せ、幅0.05fを設定
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.FillWidth(0.05f)
				[
					ConstructCheckBox(ActorToDisplay)
				]

				//アセットクラス名の追加
				//横中央寄せ、縦全体、幅0.2fを設定
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				.FillWidth(0.55f)
				[
					ConstructTextForRowWidget(ActorToDisplay->GetClass()->GetName(), ClassNameFont)
				]

				//アセット名の追加
				//横左寄せ、縦全体
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					ConstructTextForRowWidget(DisplayActorName, ActorNameFont)
				]

				//ボタンの追加
				//横右寄せ、縦全体
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					ConstructButtonForRowWidget(ActorToDisplay)
				]
		];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SLockedActorsListTab::ConstructCheckBox(const TWeakObjectPtr<AActor>& ActorToDisplay)
{
	TSharedRef<SCheckBox> ConstructCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		//チェックボックス紐づけのデリゲート
		.OnCheckStateChanged(this, &SLockedActorsListTab::OnCheckBoxStateChanged, ActorToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstructCheckBox);

	return ConstructCheckBox;
}

void SLockedActorsListTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> Actor)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (ActorToToggleLockArray.Contains(Actor))
		{
			ActorToToggleLockArray.Remove(Actor);
		}
		break;

	case ECheckBoxState::Checked:
		ActorToToggleLockArray.AddUnique(Actor);
		break;

	case ECheckBoxState::Undetermined:
		break;

	default:
		break;
	}
}

TSharedRef<STextBlock> SLockedActorsListTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SLockedActorsListTab::ConstructButtonForRowWidget(const TWeakObjectPtr<AActor>& ActorToDisplay)
{
	FString ButtonText =
	ActorToDisplay->ActorHasTag(FName("Locked")) ? FString(TEXT("Unlock")) : FString(TEXT("Lock"));

	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(ButtonText))
		.OnClicked(this, &SLockedActorsListTab::OnButtonClicked, ActorToDisplay);


	return ConstructedButton;
}

FReply SLockedActorsListTab::OnButtonClicked(TWeakObjectPtr<AActor> ClickedActor)
{
	if (ClickedActor->ActorHasTag(FName("Locked")))
	{
		ClickedActor->Tags.Remove(FName("Locked"));
		
	}
	else
	{
		ClickedActor->Tags.AddUnique(FName("Locked"));
	}

	if (ComboDisplayTextBlock->GetText().ToString() != LIST_ALL) 
	{
		if (DisplayedActorsData.Contains(ClickedActor))
		{
			DisplayedActorsData.Remove(ClickedActor);
		}
	}

	RefreshActorListView();

	//イベント処理完了
	return FReply::Handled();
}

#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SLockedActorsListTab::ConstructLockAllButton()
{
	TSharedRef<SButton> LockAllButton =
		//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SLockedActorsListTab::OnLockAllButtonClicked);

	LockAllButton->SetContent(ConstructTextForTabButtons(TEXT("Lock All")));

	return LockAllButton;
}

FReply SLockedActorsListTab::OnLockAllButtonClicked()
{
	if (ActorToToggleLockArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No actor currently selected"));
		return FReply::Handled();
	}

	TArray<TWeakObjectPtr<AActor>> ActorToToggleLock;
	for (const TWeakObjectPtr<AActor>& Actor : ActorToToggleLockArray)
	{
		ActorToToggleLock.Add(Actor);
	}

	//ロックし、DisplayedActorListから削除
	for (TWeakObjectPtr<AActor> Actor : ActorToToggleLock)
	{
		Actor->Tags.AddUnique(FName("Locked"));

		if (ComboDisplayTextBlock->GetText().ToString() != LIST_LOCK) { continue; }
		if (DisplayedActorsData.Contains(Actor))
		{
			DisplayedActorsData.Remove(Actor);
		}
	}

	RefreshActorListView();

	return FReply::Handled();
}

TSharedRef<SButton> SLockedActorsListTab::ConstructUnlockAllButton()
{
	TSharedRef<SButton> LockAllButton =
		//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SLockedActorsListTab::OnUnlockAllButtonClicked);

	LockAllButton->SetContent(ConstructTextForTabButtons(TEXT("Unlock All")));

	return LockAllButton;
}

FReply SLockedActorsListTab::OnUnlockAllButtonClicked()
{
	if (ActorToToggleLockArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No actor currently selected"));
		return FReply::Handled();
	}

	TArray<TWeakObjectPtr<AActor>> ActorToToggleLock;
	for (const TWeakObjectPtr<AActor>& Actor : ActorToToggleLockArray)
	{
		ActorToToggleLock.Add(Actor);
	}

	//ロック解除し、DisplayedActorListから削除
	for (TWeakObjectPtr<AActor> Actor : ActorToToggleLock)
	{
		Actor->Tags.Remove(FName("Locked"));

		if (ComboDisplayTextBlock->GetText().ToString() != LIST_UNLOCK) { continue; }
		if (DisplayedActorsData.Contains(Actor))
		{
			DisplayedActorsData.Remove(Actor);
		}
	}

	RefreshActorListView();

	return FReply::Handled();
}

TSharedRef<SButton> SLockedActorsListTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton =
		//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SLockedActorsListTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

FReply SLockedActorsListTab::OnSelectAllButtonClicked()
{
	if (CheckBoxesArray.IsEmpty()) { return FReply::Handled(); }

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

TSharedRef<SButton> SLockedActorsListTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton =
		//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SLockedActorsListTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SLockedActorsListTab::OnDeselectAllButtonClicked()
{
	if (CheckBoxesArray.IsEmpty()) { return FReply::Handled(); }

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

TSharedRef<STextBlock> SLockedActorsListTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmnossedTextFont();
	ButtonTextFont.Size = 15.f;

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextFont)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion
