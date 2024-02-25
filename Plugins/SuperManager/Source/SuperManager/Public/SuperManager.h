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
	/// <param name="SelectedPaths">�I���p�X</param>
	/// <returns>�g���@�\�N���X</returns>
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	/// <summary>
	/// ���j���[����
	/// </summary>
	/// <param name="MenuBuilder">���j���[���ڐ����N���X</param>
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// �폜�{�^���I�����̏���(�o�C���h����)
	/// </summary>
	void OnDeleteUnusedAssetButtonClicked();
#pragma endregion

};
