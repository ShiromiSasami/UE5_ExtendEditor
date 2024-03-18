// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"

#include "SuperManager.h"
#include "DebugHeader.h"

#include "SlateBasics.h"

#define LIST_ALL TEXT("List All Available Assets")
#define LIST_UNUSED TEXT("List Unused Assets")
#define LIST_SAMENAME TEXT("List Assets With Same Name")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	//選択したフォルダー下のアセットデータの配列を取得
	StoredAssetsData = InArgs._AssetsDataToStore;
	//表示するアセットデータの配列を設定
	DisplayedAssetsData = StoredAssetsData;

	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_ALL));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_UNUSED));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_SAMENAME));

	//タイトルテキストのフォントの設定
	FSlateFontInfo TitleTextFont = GetEmnossedTextFont();
	TitleTextFont.Size = 30.f;

	//Widgetのスロットの生成
	ChildSlot
	[
		//バーティカルボックスを生成
		SNew(SVerticalBox)
			//タイトルテキストスロットを追加
			+SVerticalBox::Slot().AutoHeight()
			[
				//テキストブロックを生成
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Advance Deletion")))
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
						TEXT("You can manipulate the displayed contents by dropping them.\nLeft-click on an asset in the list to move the asset browser to that position."),
						ETextJustify::Center)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(.1f)
				[
					ConstractComboHelpTexts(
						TEXT("Current Folder:\n" + InArgs._CurrentSelectedFolder),
						ETextJustify::Right)
				]
			]

			//Asestリストスロットを追加
			//バーによるスクロールを可能にする
			+ SVerticalBox::Slot().VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
				//AssetDataリストビューを生成
				+SScrollBox::Slot()
				[
					ConstructAssetListView()
				]
			]

			//ボタンスロットを追加
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				//全削除ボタンを追加
				//横幅10.f 間隔5.f に設定
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructDeleteAllButton()
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

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView =
		SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedAssetsData)
		//紐づけるデリゲートを設定
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList)
		.OnMouseButtonClick(this, &SAdvanceDeletionTab::OnRowWidgetMouseClicked);

	return ConstructedAssetListView.ToSharedRef();
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstractComboBox()
{
	//選択肢の生成
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstractComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SAdvanceDeletionTab::OnGenerateComboContent)
		.OnSelectionChanged(this, &SAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];

	return ConstractComboBox;
}

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<SWidget> ConstractComboText =
		SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));

	return ConstractComboText;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	//モジュールの読み込み
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (*SelectedOption.Get() == LIST_ALL)
	{
		DisplayedAssetsData = StoredAssetsData;
	}
	else if (*SelectedOption.Get() == LIST_UNUSED)
	{
		//未使用アセットのリストを取得
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
	}
	else if (*SelectedOption.Get() == LIST_SAMENAME)
	{
		//任意のワードを持つアセットのリストを取得
		SuperManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
	}

	RefreshAssetListView();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstractComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify)
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

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(
	TSharedPtr<FAssetData> AssetDataToDisplay, 
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) { return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable); }

	const FString DisplayAssetClassName = AssetDataToDisplay->GetClass()->GetName();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmnossedTextFont();
	AssetClassNameFont.Size = 10.f;
	FSlateFontInfo AssetNameFont = GetEmnossedTextFont();
	AssetNameFont.Size = 15.f;

	//リストビューのウィジェットを生成
	//要素間の間隔5.fを設定
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = 
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
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
				ConstructCheckBox(AssetDataToDisplay)
			]

			//アセットクラス名の追加
			//横中央寄せ、縦全体、幅0.2fを設定
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(0.55f)
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
			]

			//アセット名の追加
			//横左寄せ、縦全体
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
			]

			//ボタンの追加
			//横右寄せ、縦全体
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				ConstructButtonForRowWidget(AssetDataToDisplay)
			]
		];

	return ListViewRowWidget;
}

void SAdvanceDeletionTab::OnRowWidgetMouseClicked(TSharedPtr<FAssetData> ClickedData)
{
	//モジュールの読み込み
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	//アセットブラウザの同期
	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->GetObjectPathString());
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		//チェックボックス紐づけのデリゲート
		.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstructCheckBox);

	return ConstructCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
		case ECheckBoxState::Unchecked:
			if (AssetsDataToDeleteArray.Contains(AssetData))
			{
				AssetsDataToDeleteArray.Remove(AssetData);
			}
			break;

		case ECheckBoxState::Checked:
			AssetsDataToDeleteArray.AddUnique(AssetData);
			break;

		case ECheckBoxState::Undetermined:
			break;

		default:
			break;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
	SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
	SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	//モジュールの読み込み
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	//AssetListから単体アセットを削除
	if (SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get()))
	{
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}

		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}

		RefreshAssetListView();
	}
	//イベント処理完了
	return FReply::Handled();
}

#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton =
	//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return DeleteAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	if (AssetsDataToDeleteArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetsDataToDelete;
	for (const TSharedPtr<FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetsDataToDelete.Add(*Data.Get());
	}

	//モジュールの読み込み
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	//AssetListから複数アセットを削除
	if (SuperManagerModule.DeleteMultipleAssetsForAssetList(AssetsDataToDelete))
	{
		for (const TSharedPtr<FAssetData>& DeleteData : AssetsDataToDeleteArray)
		{
			if (StoredAssetsData.Contains(DeleteData))
			{
				StoredAssetsData.Remove(DeleteData);
			}

			if (DisplayedAssetsData.Contains(DeleteData))
			{
				DisplayedAssetsData.Remove(DeleteData);
			}
		}
		
		RefreshAssetListView();
	}
	return FReply::Handled();
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton =
		//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
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

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton =
		//コンテンツの間隔5.fを設定
		SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
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

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
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
