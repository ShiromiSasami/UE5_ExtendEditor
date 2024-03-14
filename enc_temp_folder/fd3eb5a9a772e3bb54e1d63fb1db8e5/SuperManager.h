// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ContentBrowserMenuExtention
private:
	/// <summary>
	/// �R���e���c�u���E�U�[�̃��j���[�g���@�\�̏�����
	/// </summary>
	void InitCBMenuExtention();

	/// <summary>
	/// �p�X�̑I�������ǉ�
	/// </summary>
	/// <param name="SelectedPaths">�I��Ώۂ̃p�X�z��</param>
	/// <returns>�g���@�\�N���X</returns>
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	/// <summary>
	/// ���j���[����
	/// </summary>
	/// <param name="MenuBuilder">���j���[���ڐ����N���X</param>
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// DeleteUnusedAsset���I�����ꂽ���̏���
	/// </summary>
	void OnDeleteUnusedAssetButtonClicked();

	/// <summary>
	/// DeleteEmptyFolders���I�����ꂽ���̏���
	/// </summary>
	void OnDeleteEmptyFoldersButtonClicked();

	/// <summary>
	/// DeleteUnusedAssetAndFolders���I�����ꂽ���̏���
	/// </summary>
	void OnDeleteUnusedAssetAndFoldersButtonClicked();

	/// <summary>
	/// AdvanceDeletion���I�����ꂽ���̏���
	/// </summary>
	void OnAdvanceDeletionButtonClicked();

	/// <summary>
	/// �Q�Ƃ̏C��
	/// </summary>
	void FixUpRedirectors();

	/// <summary>
	/// �����̂���Path���܂ނ��ǂ����m�F
	/// </summary>
	/// <param name="Path">�Ώۂ̃p�X</param>
	/// <returns>true: ���� false: �Ȃ�</returns>
	bool ContainsRestrictedPath(const FString& Path);

private:
	TArray<FString> FolderPathsSelected;
#pragma endregion

#pragma region CustomEditorTab
private:
	/// <summary>
	/// �^�u�̓o�^��������
	/// </summary>
	void RegisterAdvancedDeletionTab();

	/// <summary>
	/// �^�u�̕\�����̏���
	/// </summary>
	/// <param name="Args"></param>
	/// <returns>�^�u�̎Q�ƃ|�C���^</returns>
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	/// <summary>
	/// �I�������t�H���_�[���̃A�Z�b�g�f�[�^�̔z����擾
	/// </summary>
	/// <returns>�A�Z�b�g�f�[�^�̔z��</returns>
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
#pragma endregion

public:

#pragma region ProccessDataForAdvancedDeletionTab

	/// <summary>
	/// AssetList����P�̃A�Z�b�g���폜
	/// </summary>
	/// <param name="AssetDataToDelete">�폜����AssetData</param>
	/// <returns>�폜����: true �폜���s: false</returns>
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);

	/// <summary>
	/// AssetList���畡���A�Z�b�g���폜
	/// </summary>
	/// <param name="AssetsToDelete">�폜Asset�z��</param>
	/// <returns>�폜����: true �폜���s: false</returns>
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);

	/// <summary>
	/// �C�ӂ̃A�Z�b�g���X�g�𖢎g�p�̃A�Z�b�g�݂̂̃��X�g�ɕϊ�
	/// </summary>
	/// <param name="AssetsDataToFilter">�ϊ��������A�Z�b�g���X�g</param>
	/// <param name="OutUnusedAssetsData">�ϊ���A�Z�b�g���X�g</param>
	void ListUnusedAssetsForAssetList(
		const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, 
		TArray<TSharedPtr<FAssetData>> OutUnusedAssetsData);

#pragma endregion

};
