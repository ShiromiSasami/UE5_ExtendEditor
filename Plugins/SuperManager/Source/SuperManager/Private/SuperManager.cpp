// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"

#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SlateWidgets/LockedActorsListWidget.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "CustomUICommands/SuperManagerUICommands.h"
#include "DebugHeader.h"

#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	FSuperManagerStyle::InitializeIcons();
	InitCBMenuExtention();
	RegisterTab();
	FSuperManagerUICommands::Register();
	InitCustomUICommands();
	InitLevelEditorExtension();
	InitCustomSelectionEvent();
}

void FSuperManagerModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("LockedActorList"));

	FSuperManagerStyle::Shutdown();

	FSuperManagerUICommands::Unregister();
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
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all folders")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"),
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
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"),
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

void FSuperManagerModule::RegisterTab()
{
	//�^�u�̓o�^
	//������: �^�u�̖��O
	//������: �^�u�̐�������
	//��O����: �^�u�̕\����
	
	//Advance Deletion�^�u�̓o�^
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
		.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"));

	//Locked Actors List�^�u�̓o�^
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("LockedActorsList"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnLockedActorsListTab))
		.SetDisplayName(FText::FromString(TEXT("Locked Actors List")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if (!FolderPathsSelected.IsEmpty())
	{
		//Advance Deletion�^�u�̐���
		return
			SNew(SDockTab).TabRole(ETabRole::NomadTab)
			[
				//SAdvanceDeletionTab�̗v�f�̏����l�ݒ�
				SNew(SAdvanceDeletionTab)
					.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
					.CurrentSelectedFolder(FolderPathsSelected[0])
			];
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No folder selected"));
		return SNew(SDockTab);
	}
	
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;

	if(FolderPathsSelected.IsEmpty()){ return AvaiableAssetsData; }

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

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnLockedActorsListTab(const FSpawnTabArgs& SpawnTabArgs)
{
	//Locked Actors List�^�u�̐���
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			//SLockedActorsListTab�̗v�f�̏����l�ݒ�
			SNew(SLockedActorsListTab)
				.AllActorsInLevel(GetAllLevelActors())
		];
}

TArray<TWeakObjectPtr<AActor>> FSuperManagerModule::GetAllLevelActors()
{
	TArray<TWeakObjectPtr<AActor>> AllLevelActors;

	if (!GetEditorActorSubSystem()) { return AllLevelActors; }

	TArray<AActor*> AllActorsOnLevel = WeakEditorActorSubSystem->GetAllLevelActors();

	for (AActor* ActorInLevel : AllActorsOnLevel)
	{
		if (!ActorInLevel) { continue; }
		AllLevelActors.Add(ActorInLevel);
	}

	return AllLevelActors;
}


#pragma endregion

#pragma region LevelEditorMenuExtension

void FSuperManagerModule::InitLevelEditorExtension()
{
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();
	ExistingLevelCommands->Append(CustomUICommands.ToSharedRef());

	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelViewportMenuExtenders =
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	LevelViewportMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::
		CreateRaw(this, &FSuperManagerModule::CustomLevelEditorMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomLevelEditorMenuExtender(
	const TSharedRef<FUICommandList> UICommandList,
	const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	if (!SelectedActors.IsEmpty())
	{
		MenuExtender->AddMenuExtension(
			FName("ActorOptions"),
			EExtensionHook::Before,
			UICommandList,
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddLevelEditorMenuEntry));
	}

	return MenuExtender;
}

void FSuperManagerModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	//Lock�̍��ڒǉ�
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Lock Actor Selection")),
		FText::FromString(TEXT("Prevent actor from being selected")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), FName("LevelEditor.LockSelection")),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnLockActorSelectionButtonClicked)
	);
	//Unlock�̍��ڒǉ�
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Unlock all actor Selection")),
		FText::FromString(TEXT("Remove the selection constraint on all actor")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), FName("LevelEditor.UnlockSelection")),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockActorSelectionButtonClicked)
	);

	//Lock����Actor�ꗗ�\���̍��ڒǉ�
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Display List of Locked Actors")),
		FText::FromString(TEXT("Locked All Actors On Display")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), FName("LevelEditor.DisplayListOfLockedActors")),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDisplayListOfLockedActorsButtonClicked)
	);
}

void FSuperManagerModule::OnLockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubSystem()) { return; }

	TArray<AActor*> SelectedActors = WeakEditorActorSubSystem->GetSelectedLevelActors();

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}

	FString CurrentLockedActorNames = TEXT("Locked selection for:");
	for (AActor* SelectedActor : SelectedActors)
	{
		if(!SelectedActor){ continue; }

		LockActorSelection(SelectedActor);

		WeakEditorActorSubSystem->SetActorSelectionState(SelectedActor, false);
		CurrentLockedActorNames.Append(TEXT("\n"));
		CurrentLockedActorNames.Append(SelectedActor->GetActorLabel());
	}

	DebugHeader::ShowNotifyInfo(CurrentLockedActorNames);
}

void FSuperManagerModule::OnUnlockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubSystem()) { return; }

	TArray<AActor*> AllActorsInLevel = WeakEditorActorSubSystem->GetAllLevelActors();
	TArray<AActor*> AllLockedActors;

	for (AActor* ActorsInLevel : AllActorsInLevel)
	{
		if (!ActorsInLevel) { continue; }

		if (CheckIsActorSelectionLocked(ActorsInLevel))
		{
			AllLockedActors.Add(ActorsInLevel);
		}
	}

	if (AllLockedActors.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor locked actor currently"));
	}

	FString UnlockedActorNames = TEXT("Lifted selection constraint for:");
	for (AActor* LockedActor : AllLockedActors)
	{
		UnlockActorSelection(LockedActor);

		UnlockedActorNames.Append(TEXT("\n"));
		UnlockedActorNames.Append(LockedActor->GetActorLabel());
	}

	DebugHeader::ShowNotifyInfo(UnlockedActorNames);
}

void FSuperManagerModule::OnDisplayListOfLockedActorsButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("LockedActorsList"));
}

#pragma endregion

#pragma region SelectionLock

void FSuperManagerModule::InitCustomSelectionEvent()
{
	//�I�����ꂽ�A�N�^�[�̎擾
	//��USelection�͑I�����ꂽ�A�N�^�[�̃��X�g��ێ�����N���X
	USelection* UserSelection = GEditor->GetSelectedActors();

	UserSelection->SelectObjectEvent.AddRaw(this, &FSuperManagerModule::OnActorSelected);
}

void FSuperManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if(!GetEditorActorSubSystem()){ return; }

	if (AActor* SelectActor = Cast<AActor>(SelectedObject))
	{
		if (CheckIsActorSelectionLocked(SelectActor))
		{
			WeakEditorActorSubSystem->SetActorSelectionState(SelectActor, false);
		}
	}
}

void FSuperManagerModule::LockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess) { return; }

	if (!ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Add(FName("Locked"));
	}
}

void FSuperManagerModule::UnlockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess) { return; }

	if (ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Remove(FName("Locked"));
	}
}

bool FSuperManagerModule::CheckIsActorSelectionLocked(AActor* ActorToProcess)
{
	if (!ActorToProcess) { return false; }

	return ActorToProcess->ActorHasTag(FName("Locked"));
}

#pragma endregion

#pragma region CustomEditorUICommands

void FSuperManagerModule::InitCustomUICommands()
{
	CustomUICommands = MakeShareable(new FUICommandList());

	CustomUICommands->MapAction(
		FSuperManagerUICommands::Get().LockActorSelection,
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnSelectionLockHotkeyPressed)
	);

	CustomUICommands->MapAction(
		FSuperManagerUICommands::Get().UnlockActorSelection,
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockActorsSelectionHotkeyPressed)
	);
}

void FSuperManagerModule::OnSelectionLockHotkeyPressed()
{
	OnLockActorSelectionButtonClicked();
}

void FSuperManagerModule::OnUnlockActorsSelectionHotkeyPressed()
{
	OnUnlockActorSelectionButtonClicked();
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

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);

	//AssetBrowser��I������Asset�̈ʒu�ɓ���
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);
}

void FSuperManagerModule::ListLockActorForActorList(
	const TArray<TWeakObjectPtr<AActor>>& ActorToFilter,
	TArray<TWeakObjectPtr<AActor>>& OutLockActorData)
{
	OutLockActorData.Empty();

	for (const TWeakObjectPtr<AActor>& ActorSharedPtr : ActorToFilter)
	{
		if (CheckIsActorSelectionLocked(ActorSharedPtr.Get()))
		{
			OutLockActorData.Add(ActorSharedPtr);
		}
	}
}

#pragma endregion

void FSuperManagerModule::ListUnlockActorForActorList(
	const TArray<TWeakObjectPtr<AActor>>& ActorToFilter,
	TArray<TWeakObjectPtr<AActor>>& OutUnlockActorData)
{
	OutUnlockActorData.Empty();

	for (const TWeakObjectPtr<AActor>& ActorSharedPtr : ActorToFilter)
	{
		if (!CheckIsActorSelectionLocked(ActorSharedPtr.Get()))
		{
			OutUnlockActorData.Add(ActorSharedPtr);
		}
	}
}

bool FSuperManagerModule::GetEditorActorSubSystem()
{
	if (!WeakEditorActorSubSystem.IsValid())
	{
		WeakEditorActorSubSystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return WeakEditorActorSubSystem != nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)