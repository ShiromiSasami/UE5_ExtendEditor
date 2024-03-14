// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}
	//�v�f�̐ݒ�
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)
	SLATE_END_ARGS()

public:
	//Widget�������̃R���X�g���N�^
	void Construct(const FArguments& InArgs);

private:
	/// <summary>
	/// �A�Z�b�g���X�g�r���[�̐����֐�
	/// </summary>
	/// <returns>���������A�Z�b�g���X�g�r���[</returns>
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	
	/// <summary>
	/// �A�Z�b�g���X�g�r���[�̃��t���b�V���֐�
	/// </summary>
	void RefreshAssetListView();

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

	//CombatBoxMemberValiables
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedPtr<STextBlock> ComboDisplayTextBlock;

#pragma endregion

#pragma region RowWidgetForAssetListView

	/// <summary>
	/// ���X�g�̗v�f�𐶐�����֐�
	/// </summary>
	/// <param name="AssetDataToDisplay">�\������AssetData</param>
	/// <param name="OwnerTable">�e�[�u���̐e</param>
	/// <returns>���X�g�r���[�e�[�u��</returns>
	TSharedRef<ITableRow> OnGenerateRowForList(
		TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable
	);

	/// <summary>
	/// AssetData�̃`�F�b�N�{�b�N�X�𐶐�����֐�
	/// </summary>
	/// <param name="AssetDataToDisplay">�\������AssetData</param>
	/// <returns>�`�F�b�N�{�b�N�X</returns>
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/// <summary>
	/// �`�F�b�N�{�b�N�X�C�x���g�̕R�Â��֐�
	/// </summary>
	/// <param name="NewState">�`�F�b�N�{�b�N�X�̏��</param>
	/// <param name="AssetData">�֘A����AssetData</param>
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

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
	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/// <summary>
	/// �{�^���C�x���g�̕R�Â��֐�
	/// </summary>
	/// <param name="ClickedAssetData">�N���b�N����AssetData</param>
	/// <returns>�C�x���g�̏������</returns>
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

#pragma endregion

#pragma region TabButtons

	/// <summary>
	/// �S�폜�{�^���̐����֐�
	/// </summary>
	/// <returns>�S�폜�{�^��</returns>
	TSharedRef<SButton> ConstructDeleteAllButton();

	/// <summary>
	/// �S�폜�{�^���̃C�x���g�֐�
	/// </summary>
	/// <returns>�������</returns>
	FReply OnDeleteAllButtonClicked();

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
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;

	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
};