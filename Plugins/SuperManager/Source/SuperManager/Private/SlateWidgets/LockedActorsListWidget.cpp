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

	//���b�N���̃A�N�^�[���X�g���擾
	AllActorsData = InArgs._AllActorsInLevel;
	//�\������A�Z�b�g�f�[�^�̔z���ݒ�
	DisplayedActorsData = AllActorsData;

	CheckBoxesArray.Empty();
	ActorToToggleLockArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_ALL));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_LOCK));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_UNLOCK));

	//�^�C�g���e�L�X�g�̃t�H���g�̐ݒ�
	FSlateFontInfo TitleTextFont = GetEmnossedTextFont();
	TitleTextFont.Size = 30.f;

	//Widget�̃X���b�g�̐���
	ChildSlot
		[
			//�o�[�e�B�J���{�b�N�X�𐶐�
			SNew(SVerticalBox)
				//�^�C�g���e�L�X�g�X���b�g��ǉ�
				+ SVerticalBox::Slot().AutoHeight()
				[
					//�e�L�X�g�u���b�N�𐶐�
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Locked Actors List")))
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
								TEXT("You can manipulate the displayed contents by dropping them."),
								ETextJustify::Center)
						]
				]

				//���X�g�X���b�g��ǉ�
				//�o�[�ɂ��X�N���[�����\�ɂ���
				+SVerticalBox::Slot().VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
						//AssetData���X�g�r���[�𐶐�
						+ SScrollBox::Slot()
						[
							ConstructListView()
						]
				]

				//�{�^���X���b�g��ǉ�
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
						//�S���b�N�{�^����ǉ�
						//����10.f �Ԋu5.f �ɐݒ�
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructLockAllButton()
						]
						//�S���b�N�����{�^����ǉ�
						//����10.f �Ԋu5.f �ɐݒ�
						+ SHorizontalBox::Slot()
						.FillWidth(10.f)
						.Padding(5.f)
						[
							ConstructUnlockAllButton()
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

TSharedRef<SListView<TWeakObjectPtr<AActor>>> SLockedActorsListTab::ConstructListView()
{
	ConstructedActorListView =
		SNew(SListView<TWeakObjectPtr<AActor>>)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedActorsData)
		//�R�Â���f���Q�[�g��ݒ�
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
	//�I�����̐���
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

	//���W���[���̓ǂݍ���
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (*SelectedOption.Get() == LIST_ALL)
	{
		DisplayedActorsData = AllActorsData;
	}
	else if (*SelectedOption.Get() == LIST_LOCK)
	{
		//���g�p�A�Z�b�g�̃��X�g���擾
		SuperManagerModule.ListLockActorForActorList(AllActorsData, DisplayedActorsData);
	}
	else if (*SelectedOption.Get() == LIST_UNLOCK)
	{
		//�C�ӂ̃��[�h�����A�Z�b�g�̃��X�g���擾
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
		//�����܂�Ԃ���ݒ�
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

	//���X�g�r���[�̃E�B�W�F�b�g�𐶐�
	//�v�f�Ԃ̊Ԋu5.f��ݒ�
	TSharedRef<STableRow<TSharedPtr<AActor>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<AActor>>, OwnerTable)
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
					ConstructCheckBox(ActorToDisplay)
				]

				//�A�Z�b�g�N���X���̒ǉ�
				//�������񂹁A�c�S�́A��0.2f��ݒ�
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				.FillWidth(0.55f)
				[
					ConstructTextForRowWidget(ActorToDisplay->GetClass()->GetName(), ClassNameFont)
				]

				//�A�Z�b�g���̒ǉ�
				//�����񂹁A�c�S��
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					ConstructTextForRowWidget(DisplayActorName, ActorNameFont)
				]

				//�{�^���̒ǉ�
				//���E�񂹁A�c�S��
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
		//�`�F�b�N�{�b�N�X�R�Â��̃f���Q�[�g
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

	//�C�x���g��������
	return FReply::Handled();
}

#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SLockedActorsListTab::ConstructLockAllButton()
{
	TSharedRef<SButton> LockAllButton =
		//�R���e���c�̊Ԋu5.f��ݒ�
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

	//���b�N���ADisplayedActorList����폜
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
		//�R���e���c�̊Ԋu5.f��ݒ�
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

	//���b�N�������ADisplayedActorList����폜
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
		//�R���e���c�̊Ԋu5.f��ݒ�
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
		//�R���e���c�̊Ԋu5.f��ݒ�
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
