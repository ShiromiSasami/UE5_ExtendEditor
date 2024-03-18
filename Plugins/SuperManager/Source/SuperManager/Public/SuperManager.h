// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UEditorActorSubsystem;
class ISceneOutlinerColumn;
class ISceneOutliner;
class SLockedActorsListTab;

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/// <summary>
	/// �A�N�^�[�����b�N��Ԃ��m�F
	/// </summary>
	bool CheckIsActorSelectionLocked(AActor* ActorToProcess);

	/// <summary>
	/// �A�N�^�[��C�ӂ̃��b�N��Ԃɂ���
	/// </summary>
	/// <param name="ActorToProcess">�Ώۂ�Actor</param>
	/// <param name="bShouldLocked">�w��̃��b�N���</param>
	void ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLocked);

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
	/// �폜�^�u�̓o�^��������
	/// </summary>
	void RegisterTab();

	/// <summary>
	/// �폜�^�u�̕\�����̏���
	/// </summary>
	/// <param name="Args"></param>
	/// <returns>�^�u�̎Q�ƃ|�C���^</returns>
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	/// <summary>
	/// �I�������t�H���_�[���̃A�Z�b�g�f�[�^�̔z����擾
	/// </summary>
	/// <returns>�A�Z�b�g�f�[�^�̔z��</returns>
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

	/// <summary>
	/// �폜�^�u��������̏���
	/// </summary>
	/// <param name="TabToClose">�����^�u</param>
	void OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose);

	/// <summary>
	/// ���b�N����Actor�ꗗ�^�u�̕\�����̏���
	/// </summary>
	/// <param name="Args"></param>
	/// <returns>�^�u�̎Q�ƃ|�C���^</returns>
	TSharedRef<SDockTab> OnSpawnLockedActorsListTab(const FSpawnTabArgs& SpawnTabArgs);

	/// <summary>
	/// ���x����ɑ��݂���S�ẴA�N�^�[�̔z����擾
	/// </summary>
	/// <returns>�A�N�^�[�̔z��</returns>
	TArray<TWeakObjectPtr<AActor>> GetAllLevelActors();

private:
	TSharedPtr<SDockTab> ConstructedDockTab;

#pragma endregion

#pragma region LevelEditorMenuExtension
private:
	/// <summary>
	/// LevelEditor�̃��j���[�g���@�\�̏�����
	/// </summary>
	void InitLevelEditorExtension();

	/// <summary>
	/// LevelEditor�̃��j���[�g���@�\�̒ǉ�
	/// </summary>
	/// <param name="UICommandList">UI�R�}���h���X�g</param>
	/// <param name="SelectedActors">�I�𒆂̃A�N�^�[</param>
	/// <returns>�g���@�\</returns>
	TSharedRef<FExtender> CustomLevelEditorMenuExtender(
		const TSharedRef<FUICommandList> UICommandList,
		const TArray<AActor*> SelectedActors);

	/// <summary>
	/// LevelEditor�̃��j���[���ڂ̒ǉ�
	/// </summary>
	/// <param name="MenuBuilder">���ڒǉ��N���X</param>
	void AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// LockActorSelection���ڂ�I�����̏���
	/// </summary>
	void OnLockActorSelectionButtonClicked();

	/// <summary>
	/// UnlockActorSelection���ڂ�I�����̏���
	/// </summary>
	void OnUnlockActorSelectionButtonClicked();

	/// <summary>
	/// LockedActorsList���ڂ�I�����̏���
	/// </summary>
	void OnDisplayListOfLockedActorsButtonClicked();

#pragma endregion

#pragma region SelectionLock

	/// <summary>
	/// �I���C�x���g�̏�����
	/// </summary>
	void InitCustomSelectionEvent();

	/// <summary>
	/// �I�����̏������e
	/// </summary>
	void OnActorSelected(UObject* SelectedObject);

	/// <summary>
	/// �A�N�^�[�����b�N��Ԃɂ���
	/// </summary>
	void LockActorSelection(AActor* ActorToProcess);

	/// <summary>
	/// �A�N�^�[�����b�N��������
	/// </summary>
	void UnlockActorSelection(AActor* ActorToProcess);

#pragma endregion

#pragma region CustomEditorUICommands

private:
	/// <summary>
	/// �J�X�^���R�}���h���X�g�̏�����
	/// </summary>
	void InitCustomUICommands();

	/// <summary>
	/// Lock�R�}���h�L�[�������ꂽ���̏���
	/// </summary>
	void OnSelectionLockHotkeyPressed();

	/// <summary>
	/// Lock�R�}���h�L�[�������ꂽ���̏���
	/// </summary>
	void OnUnlockActorsSelectionHotkeyPressed();

private:
	TSharedPtr<class FUICommandList> CustomUICommands;

#pragma endregion

#pragma region SceneOutlinerExtension

	/// <summary>
	/// �V�[���A�E�g���C�i�[�̃J�����g���@�\�̏�����
	/// </summary>
	void InitSceneOutlinerColumnExtension();

	/// <summary>
	/// ���b�N�̃J������I���������̏���
	/// </summary>
	/// <param name="SceneOutliner">�I�������A�E�g���C�i�[</param>
	/// <returns>�I�������J����</returns>
	TSharedRef<ISceneOutlinerColumn> OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner);

	/// <summary>
	/// �A�E�g���C�i�J�����̓o�^����
	/// </summary>
	void UnregisterSceneOutlinerColumnExtension();

#pragma endregion

#pragma region ProccessDataForTab

public:
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
		TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);

	/// <summary>
	/// �C�Ӄ��[�h���܂񂾂̃A�Z�b�g�݂̂̃��X�g�ɕϊ�
	/// </summary>
	/// <param name="AssetsDataToFilter">�ϊ��������A�Z�b�g���X�g</param>
	/// <param name="OutSameNameAssetsData">�ϊ���A�Z�b�g���X�g</param>
	void ListSameNameAssetsForAssetList(
		const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
		TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);

	/// <summary>
	/// �I������Asset�p�X�ʒu��AssetBrowser�𓯊�����
	/// </summary>
	/// <param name="AssetPathToSync">��������Asset��Path</param>
	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);

	/// <summary>
	/// �C�ӂ̃A�N�^�[�z������b�N���Ă���A�Z�b�g�݂̂̃��X�g�ɕϊ�
	/// </summary>
	/// <param name="ActorToFilter">�ϊ��������A�Z�b�g���X�g</param>
	/// <param name="OutLockActorData">�ϊ���A�Z�b�g���X�g</param>
	void ListLockActorForActorList(
		const TArray<TWeakObjectPtr<AActor>>& ActorToFilter,
		TArray<TWeakObjectPtr<AActor>>& OutLockActorData);

	/// <summary>
	/// �C�ӂ̃A�N�^�[�z������b�N����ĂȂ��A�N�^�[�݂̂̃��X�g�ɕϊ�
	/// </summary>
	/// <param name="ActorToFilter">�ϊ��������A�Z�b�g���X�g</param>
	/// <param name="OutUnlockActorData">�ϊ���A�Z�b�g���X�g</param>
	void ListUnlockActorForActorList(
		const TArray<TWeakObjectPtr<AActor>>& ActorToFilter,
		TArray<TWeakObjectPtr<AActor>>& OutUnlockActorData);

#pragma endregion

private:
	bool GetEditorActorSubSystem();

	/// <summary>
	/// Outliner�̃��t���b�V��
	/// </summary>
	void RefreshSceneOutliner();

private:
	TWeakObjectPtr<UEditorActorSubsystem> WeakEditorActorSubSystem;
	TSharedPtr<SLockedActorsListTab> LockedActorsListTab;
};
