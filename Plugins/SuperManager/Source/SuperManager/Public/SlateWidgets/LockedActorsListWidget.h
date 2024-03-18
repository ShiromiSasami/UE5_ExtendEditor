// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SLockedActorsListTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SLockedActorsListTab) {}
		//�v�f�̐ݒ�
		SLATE_ARGUMENT(TArray<TWeakObjectPtr<AActor>>, AllActorsInLevel)
	SLATE_END_ARGS()

public:
	//Widget�������̃R���X�g���N�^
	void Construct(const FArguments& InArgs);

	/// <summary>
	/// ���X�g�r���[�̃��t���b�V���֐�
	/// </summary>
	void RefreshActorListView();

private:
	/// <summary>
	/// ���b�N���̃A�N�^�[���X�g�r���[�̐����֐�
	/// </summary>
	/// <returns>�����������X�g�r���[</returns>
	TSharedRef<SListView<TWeakObjectPtr<AActor>>> ConstructListView();

#pragma region ComboBoxForListingCondition

	/// <summary>
	/// ComboBox�̐����֐�
	/// </summary>
	/// <returns>ComboBox</returns>
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstractComboBox();

	/// <summary>
	/// �t�B���^�[��ComboBox�̑I�����𐶐�����֐�
	/// </summary>
	/// <param name="SourceItem">�v�f�̖��O</param>
	/// <returns>�I�����E�B�W�F�b�g</returns>
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	/// <summary>
	/// �I������I������֐�
	/// </summary>
	/// <param name="SelectedOption">�I�������v�f�̖��O</param>
	/// <param name="InSelectInfo">�I�����</param>
	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);

	/// <summary>
	/// Combo��HelpText�𐶐�����֐�
	/// </summary>
	/// <param name="TextContent">Text���e</param>
	/// <param name="TextJustify">Text�̏��</param>
	/// <returns>HelpTextBlock</returns>
	TSharedRef<STextBlock> ConstractComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify);

	//CombatBoxMemberValiables
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedPtr<STextBlock> ComboDisplayTextBlock;

#pragma endregion

#pragma region RowWidgetForListView

	/// <summary>
	/// ���X�g�̗v�f�𐶐�����֐�
	/// </summary>
	/// <param name="AssetDataToDisplay">�\������AssetData</param>
	/// <param name="OwnerTable">�e�[�u���̐e</param>
	/// <returns>���X�g�r���[�e�[�u��</returns>
	TSharedRef<ITableRow> OnGenerateRowForList(
		TWeakObjectPtr<AActor> ActorToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable
	);

	/// <summary>
	/// AssetData�̃`�F�b�N�{�b�N�X�𐶐�����֐�
	/// </summary>
	/// <param name="AssetDataToDisplay">�\������AssetData</param>
	/// <returns>�`�F�b�N�{�b�N�X</returns>
	TSharedRef<SCheckBox> ConstructCheckBox(const TWeakObjectPtr<AActor>& AssetDataToDisplay);

	/// <summary>
	/// �`�F�b�N�{�b�N�X�C�x���g�̕R�Â��֐�
	/// </summary>
	/// <param name="NewState">�`�F�b�N�{�b�N�X�̏��</param>
	/// <param name="AssetData">�֘A����AssetData</param>
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> AssetData);

	/// <summary>
	/// �e�L�X�g�u���b�N�E�B�W�F�b�g�𐶐�����֐�
	/// </summary>
	/// <param name="TextContent">�e�L�X�g�̓��e</param>
	/// <param name="FontToUse">�t�H���g</param>
	/// <returns>�e�L�X�g�u���b�N</returns>
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	/// <summary>
	/// �{�^���E�B�W�F�b�g�𐶐�����֐�
	/// </summary>
	/// <param name="AssetDataToDisplay">�\������AssetData</param>
	/// <returns>�{�^��</returns>
	TSharedRef<SButton> ConstructButtonForRowWidget(const TWeakObjectPtr<AActor>& ActorToDisplay);

	/// <summary>
	/// �{�^���C�x���g�̕R�Â��֐�
	/// </summary>
	/// <param name="ClickedAssetData">�N���b�N����Actor</param>
	/// <returns>�C�x���g�̏������</returns>
	FReply OnButtonClicked(TWeakObjectPtr<AActor> ClickedActor);

#pragma endregion

#pragma region TabButtons

	/// <summary>
	/// �S���b�N�{�^���̐����֐�
	/// </summary>
	/// <returns>�S���b�N�{�^��</returns>
	TSharedRef<SButton> ConstructLockAllButton();

	/// <summary>
	/// �S���b�N�{�^���̃C�x���g�֐�
	/// </summary>
	/// <returns>�������</returns>
	FReply OnLockAllButtonClicked();

	/// <summary>
	/// �S���b�N�����{�^���̐����֐�
	/// </summary>
	/// <returns>�S���b�N�{�^��</returns>
	TSharedRef<SButton> ConstructUnlockAllButton();

	/// <summary>
	/// �S���b�N�����{�^���̃C�x���g�֐�
	/// </summary>
	/// <returns>�������</returns>
	FReply OnUnlockAllButtonClicked();

	/// <summary>
	/// �S�I���{�^���̐����֐�
	/// </summary>
	/// <returns>�S�I���{�^��</returns>
	TSharedRef<SButton> ConstructSelectAllButton();

	/// <summary>
	/// �S�I���{�^���̃C�x���g�֐�
	/// </summary>
	/// <returns>�������</returns>
	FReply OnSelectAllButtonClicked();

	/// <summary>
	/// �S�I�������{�^���̐����֐�
	/// </summary>
	/// <returns>�S�I�������{�^��</returns>
	TSharedRef<SButton> ConstructDeselectAllButton();

	/// <summary>
	/// �S�I�������{�^���̃C�x���g�֐�
	/// </summary>
	/// <returns>�������</returns>
	FReply OnDeselectAllButtonClicked();

	/// <summary>
	/// �{�^���^�u�̃e�L�X�g�u���b�N�𐶐�����֐�
	/// </summary>
	/// <param name="TextContent">�e�L�X�g���e</param>
	/// <returns>�e�L�X�g�u���b�N</returns>
	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion

	FSlateFontInfo GetEmnossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); };

private:
	TArray<TWeakObjectPtr<AActor>> AllActorsData;
	TArray<TWeakObjectPtr<AActor>> DisplayedActorsData;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TWeakObjectPtr<AActor>> ActorToToggleLockArray;

	TSharedPtr<SListView<TWeakObjectPtr<AActor>>> ConstructedActorListView;
};
