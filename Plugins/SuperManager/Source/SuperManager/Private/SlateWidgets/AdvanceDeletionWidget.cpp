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

	//�I�������t�H���_�[���̃A�Z�b�g�f�[�^�̔z����擾
	StoredAssetsData = InArgs._AssetsDataToStore;
	//�\������A�Z�b�g�f�[�^�̔z���ݒ�
	DisplayedAssetsData = StoredAssetsData;

	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_ALL));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_UNUSED));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_SAMENAME));

	//�^�C�g���e�L�X�g�̃t�H���g�̐ݒ�
	FSlateFontInfo TitleTextFont = GetEmnossedTextFont();
	TitleTextFont.Size = 30.f;

	//Widget�̃X���b�g�̐���
	ChildSlot
	[
		//�o�[�e�B�J���{�b�N�X�𐶐�
		SNew(SVerticalBox)
			//�^�C�g���e�L�X�g�X���b�g��ǉ�
			+SVerticalBox::Slot().AutoHeight()
			[
				//�e�L�X�g�u���b�N�𐶐�
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Advance Deletion")))
				.Font(TitleTextFont)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FColor::White)
			]

			//�T�|�[�g�X���b�g�̒ǉ�
			+ SVerticalBox::Slot().AutoHeight()
			[
				//�\�����e�̕ύX�h���b�v��ǉ�
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					ConstractComboBox()
				]

				//�����e�L�X�g�X���b�g��ǉ�
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

			//Asest���X�g�X���b�g��ǉ�
			//�o�[�ɂ��X�N���[�����\�ɂ���
			+ SVerticalBox::Slot().VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
				//AssetData���X�g�r���[�𐶐�
				+SScrollBox::Slot()
				[
					ConstructAssetListView()
				]
			]

			//�{�^���X���b�g��ǉ�
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				//�S�폜�{�^����ǉ�
				//����10.f �Ԋu5.f �ɐݒ�
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructDeleteAllButton()
				]
				//�S�I���{�^����ǉ�
				//����10.f �Ԋu5.f �ɐݒ�
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructSelectAllButton()
				]
				//�S�I�������{�^����ǉ�
				//����10.f �Ԋu5.f �ɐݒ�
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
		//�R�Â���f���Q�[�g��ݒ�
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
	//�I�����̐���
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

	//���W���[���̓ǂݍ���
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (*SelectedOption.Get() == LIST_ALL)
	{
		DisplayedAssetsData = StoredAssetsData;
	}
	else if (*SelectedOption.Get() == LIST_UNUSED)
	{
		//���g�p�A�Z�b�g�̃��X�g���擾
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
	}
	else if (*SelectedOption.Get() == LIST_SAMENAME)
	{
		//�C�ӂ̃��[�h�����A�Z�b�g�̃��X�g���擾
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
		//�����܂�Ԃ���ݒ�
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

	//���X�g�r���[�̃E�B�W�F�b�g�𐶐�
	//�v�f�Ԃ̊Ԋu5.f��ݒ�
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = 
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(5.f))
		[
			SNew(SHorizontalBox)
			//�`�F�b�N�{�b�N�X�̒ǉ�
			//�����񂹁A�c�����񂹁A��0.05f��ݒ�
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(0.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)
			]

			//�A�Z�b�g�N���X���̒ǉ�
			//�������񂹁A�c�S�́A��0.2f��ݒ�
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(0.55f)
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
			]

			//�A�Z�b�g���̒ǉ�
			//�����񂹁A�c�S��
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
			]

			//�{�^���̒ǉ�
			//���E�񂹁A�c�S��
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
	//���W���[���̓ǂݍ���
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	//�A�Z�b�g�u���E�U�̓���
	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->GetObjectPathString());
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		//�`�F�b�N�{�b�N�X�R�Â��̃f���Q�[�g
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
	//���W���[���̓ǂݍ���
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	//AssetList����P�̃A�Z�b�g���폜
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
	//�C�x���g��������
	return FReply::Handled();
}

#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton =
	//�R���e���c�̊Ԋu5.f��ݒ�
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

	//���W���[���̓ǂݍ���
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	//AssetList���畡���A�Z�b�g���폜
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
		//�R���e���c�̊Ԋu5.f��ݒ�
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
		//�R���e���c�̊Ԋu5.f��ݒ�
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
