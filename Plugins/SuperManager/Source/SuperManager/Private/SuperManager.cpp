// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"

#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "DebugHeader.h"

#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	InitCBMenuExtention();
	RegisterAdvancedDeletionTab();

}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ContentBrowserMenuExtention

void FSuperManagerModule::InitCBMenuExtention()
{
	// ContentBrowserModule�擾
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//�R���e���c�u���E�U�[�̃��j���[�g���@�\�ꗗ(�f���Q�[�g�ǉ��p���X�g)���擾
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//�g���@�\�p�X�̒ǉ�(�f���Q�[�g�o�C���h)
	/*FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);*/
	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender)
	);
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	//Delete�Ɋg���@�\�̒ǉ�(�f���Q�[�g�o�C���h)
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
			);

		//�I�������p�X��ێ�
		FolderPathsSelected = SelectedPaths;
	}
	
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	//���j���[���ڂ̒ǉ�
	//������: ���j���[���ڂ̖��O
	//������: ���j���[���ڂ̐���
	//��O����: ���j���[�A�C�R��
	//��l����: ���j���[���ڂ��I�����ꂽ���̏���
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all folders")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets And Empty Folders")),
		FText::FromString(TEXT("Safely delete all unused assets and empty folders")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetAndFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advence Deletion")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	//�I�������t�H���_�[�p�X�̐���1��葽���ꍇ�A�G���[��\��
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	//�I�������t�H���_�[�̒���Asset�����邩�ǂ������m�F
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if (AssetsPathNames.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"));
		return;
	}

	//�t�H���_�[��Asset��S�č폜���邩�ǂ������m�F
	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(
			EAppMsgType::YesNo, 
			TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\nWoudle you like to procceed?"),
			false
		);

	if (ConfirmResult == EAppReturnType::No) { return; }

	//�Q�Ə�Ԃ��C��
	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames)
	{
		//������ƃN���b�V������\���̂���t�H���_�[�͑ΏۊO
		if (ContainsRestrictedPath(AssetPathName)) { continue; }

		//�Y����Path��Asset�����݂��邩�ǂ������m�F
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) { continue; }
		//�Y��Asset���Q�Ƃ��Ă���Asset���擾
		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		if (AssetReferencers.IsEmpty())
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	//���Q�Ƃ�Asset�����݂��Ȃ���΃G���[�A���݂����ꍇ�͍폜
	if (!UnusedAssetsDataArray.IsEmpty())
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false);
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	//�Q�Ə�Ԃ��C��
	FixUpRedirectors();

	//�I�������t�H���_�̃T�u�t�H���_�[���܂ރt�H���_�[��Asset��Path���擾
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFolderPathsArray;
	for (const FString& FolderPath : FolderPathsArray)
	{
		//������ƃN���b�V������\���̂���t�H���_�[�͑ΏۊO
		if (ContainsRestrictedPath(FolderPath)) { continue; }
		//Path�̃t�H���_�[�����݂��邩�m�F
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) { continue; }
		//�t�H���_�[���ɃA�Z�b�g�����݂��邩�m�F
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFolderPathsArray.Add(FolderPath);
		}
	}
	//��̃t�H���_�[�����݂��Ȃ���΃G���[
	if (EmptyFolderPathsArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	//��̃t�H���_�[��S�č폜���邩�ǂ������m�F�_�C�A���O
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(
		EAppMsgType::Ok,
		TEXT("Empty folders found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"),
		false
	);
	//�L�����Z�����ꂽ�ꍇ�͏������I��
	if (ConfirmResult == EAppReturnType::Cancel) { return; }
	//��̃t�H���_�[��S�č폜
	for (const FString& EmptyFolderPath : EmptyFolderPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT("folders"));
	}

}

void FSuperManagerModule::OnDeleteUnusedAssetAndFoldersButtonClicked()
{
	FixUpRedirectors();

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFolderPathsArray;
	TArray<FAssetData> UnusedAssetsDataArray;

	for (FString FolderPathSeleceted : FolderPathsSelected)
	{
		//�I�������t�H���_�̃T�u�t�H���_�[���܂ރt�H���_�[��Asset��Path���擾
		TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathSeleceted, true, true);

		for (const FString& FolderPath : FolderPathsArray)
		{
			//������ƃN���b�V������\���̂���t�H���_�[�͑ΏۊO
			if (ContainsRestrictedPath(FolderPath)) { continue; }
			//Path�̃t�H���_�[�����݂��邩�m�F
			if (UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
			{
				//�t�H���_�[���ɃA�Z�b�g�����݂��邩�m�F
				if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
				{
					EmptyFolderPathsNames.Append(FolderPath);
					EmptyFolderPathsNames.Append(TEXT("\n"));

					EmptyFolderPathsArray.Add(FolderPath);
				}
			}
			else if (UEditorAssetLibrary::DoesAssetExist(FolderPath))
			{
				//�Y��Asset���Q�Ƃ��Ă���Asset���擾
				TArray<FString> AssetReferencers =
					UEditorAssetLibrary::FindPackageReferencersForAsset(FolderPath);
				if (AssetReferencers.IsEmpty())
				{
					const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(FolderPath);
					UnusedAssetsDataArray.Add(UnusedAssetData);
				}
			}
		}
				
	}

	//���g�p�̃A�Z�b�g�Ƌ�̃t�H���_�[�ǂ�������݂��Ȃ���΃G���[
	if (EmptyFolderPathsArray.IsEmpty() && UnusedAssetsDataArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Unused assets and empty folders were not found."), false);
		return;
	}

	//�I�������Ώۂ̖��g�p�A�Z�b�g�Ƌ�̃t�H���_�[���폜���邩�̊m�F�_�C�A���O
	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(
			EAppMsgType::YesNo,
			TEXT(" Delete unused assets and empty folders for selected targets.\nWoudle you like to procceed?"),
			false
		);

	if (ConfirmResult == EAppReturnType::No) { return; }

	//���Q�Ƃ�Asset�����݂��Ȃ���΃G���[�A���݂����ꍇ�͍폜
	if (!UnusedAssetsDataArray.IsEmpty())
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	//��̃t�H���_�[��S�č폜
	for (const FString& EmptyFolderPath : EmptyFolderPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
			DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
	}

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted unused asset and empty folders"));
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FixUpRedirectors();

	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	//Asset����/�񋓂��ł��郂�W���[���̎擾
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//���������̐ݒ�(/Game�ȉ��̃f�B���N�g���ŃN���X��ObjectRedirector�̃A�Z�b�g)
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(TEXT("/Game"));
	Filter.ClassPaths.Emplace(TEXT("/Script/Engine.ObjectRedirector"));

	//���������ɍ��v����Asset���擾
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	//UObjectRedirector�̃��X�g���쐬
	for (const FAssetData& RedirectorDate : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorDate.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	//RedirectorsToFixArray(�Q�ƃf�[�^)�����ƂɎQ�Ɗ֌W���C��
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

bool FSuperManagerModule::ContainsRestrictedPath(const FString& Path)
{
	return (Path.Contains(TEXT("Developers")) ||
			Path.Contains(TEXT("Collections")) ||
			Path.Contains(TEXT("__ExternalActors__")) ||
			Path.Contains(TEXT("__ExternalObjects__")));
}

#pragma endregion

#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvancedDeletionTab()
{
	//Advance Deletion�^�u�̓o�^
	
	//������: �^�u�̖��O
	//������: �^�u�̐�������
	//��O����: �^�u�̕\����
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	//Advance Deletion�^�u�̐���
	return 
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			//SAdvanceDeletionTab�̗v�f�̏����l�ݒ�
			SNew(SAdvanceDeletionTab).AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	for (const FString& AssetPathName : AssetsPathNames)
	{
		//������ƃN���b�V������\���̂���t�H���_�[�͑ΏۊO
		if (ContainsRestrictedPath(AssetPathName)) { continue; }

		//�Y����Path��Asset�����݂��邩�ǂ������m�F
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) { continue; }

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvaiableAssetsData.AddUnique(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}

#pragma endregion

#pragma region ProccessDataForAdvancedDeletionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}

	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}
	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(
	const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->GetObjectPathString());
		if (AssetReferencers.IsEmpty())
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

void FSuperManagerModule::ListSameNameAssetsForAssetList(
	const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();

	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMap;
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		AssetsInfoMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1) { continue; }

		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)