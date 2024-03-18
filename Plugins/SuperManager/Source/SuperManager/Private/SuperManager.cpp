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
	// ContentBrowserModule取得
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//コンテンツブラウザーのメニュー拡張機能一覧(デリゲート追加用リスト)を取得
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//拡張機能パスの追加(デリゲートバインド)
	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender)
	);
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	//Deleteに拡張機能の追加(デリゲートバインド)
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
			);

		//選択したパスを保持
		FolderPathsSelected = SelectedPaths;
	}
	
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	//メニュー項目の追加
	//第一引数: メニュー項目の名前
	//第二引数: メニュー項目の説明
	//第三引数: メニューアイコン
	//第四引数: メニュー項目が選択された時の処理
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
	//選択したフォルダーパスの数が1より多い場合、エラーを表示
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	//選択したフォルダーの中にAssetがあるかどうかを確認
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if (AssetsPathNames.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"));
		return;
	}

	//フォルダー内Assetを全て削除するかどうかを確認
	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(
			EAppMsgType::YesNo, 
			TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\nWoudle you like to procceed?"),
			false
		);

	if (ConfirmResult == EAppReturnType::No) { return; }

	//参照状態を修正
	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames)
	{
		//干渉するとクラッシュする可能性のあるフォルダーは対象外
		if (ContainsRestrictedPath(AssetPathName)) { continue; }

		//該当のPathのAssetが存在するかどうかを確認
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) { continue; }
		//該当Assetが参照しているAssetを取得
		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		if (AssetReferencers.IsEmpty())
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	//未参照のAssetが存在しなければエラー、存在した場合は削除
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
	//参照状態を修正
	FixUpRedirectors();

	//選択したフォルダのサブフォルダーを含むフォルダーとAssetのPathを取得
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFolderPathsArray;
	for (const FString& FolderPath : FolderPathsArray)
	{
		//干渉するとクラッシュする可能性のあるフォルダーは対象外
		if (ContainsRestrictedPath(FolderPath)) { continue; }
		//Pathのフォルダーが存在するか確認
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) { continue; }
		//フォルダー内にアセットが存在するか確認
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFolderPathsArray.Add(FolderPath);
		}
	}
	//空のフォルダーが存在しなければエラー
	if (EmptyFolderPathsArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	//空のフォルダーを全て削除するかどうかを確認ダイアログ
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(
		EAppMsgType::Ok,
		TEXT("Empty folders found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"),
		false
	);
	//キャンセルされた場合は処理を終了
	if (ConfirmResult == EAppReturnType::Cancel) { return; }
	//空のフォルダーを全て削除
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
		//選択したフォルダのサブフォルダーを含むフォルダーとAssetのPathを取得
		TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathSeleceted, true, true);

		for (const FString& FolderPath : FolderPathsArray)
		{
			//干渉するとクラッシュする可能性のあるフォルダーは対象外
			if (ContainsRestrictedPath(FolderPath)) { continue; }
			//Pathのフォルダーが存在するか確認
			if (UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
			{
				//フォルダー内にアセットが存在するか確認
				if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
				{
					EmptyFolderPathsNames.Append(FolderPath);
					EmptyFolderPathsNames.Append(TEXT("\n"));

					EmptyFolderPathsArray.Add(FolderPath);
				}
			}
			else if (UEditorAssetLibrary::DoesAssetExist(FolderPath))
			{
				//該当Assetが参照しているAssetを取得
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

	//未使用のアセットと空のフォルダーどちらも存在しなければエラー
	if (EmptyFolderPathsArray.IsEmpty() && UnusedAssetsDataArray.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Unused assets and empty folders were not found."), false);
		return;
	}

	//選択した対象の未使用アセットと空のフォルダーを削除するかの確認ダイアログ
	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(
			EAppMsgType::YesNo,
			TEXT(" Delete unused assets and empty folders for selected targets.\nWoudle you like to procceed?"),
			false
		);

	if (ConfirmResult == EAppReturnType::No) { return; }

	//未参照のAssetが存在しなければエラー、存在した場合は削除
	if (!UnusedAssetsDataArray.IsEmpty())
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	//空のフォルダーを全て削除
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
	//Asset検索/列挙ができるモジュールの取得
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//検索条件の設定(/Game以下のディレクトリでクラスがObjectRedirectorのアセット)
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(TEXT("/Game"));
	Filter.ClassPaths.Emplace(TEXT("/Script/Engine.ObjectRedirector"));

	//検索条件に合致するAssetを取得
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	//UObjectRedirectorのリストを作成
	for (const FAssetData& RedirectorDate : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorDate.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	//RedirectorsToFixArray(参照データ)をもとに参照関係を修正
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
	//タブの登録
	//第一引数: タブの名前
	//第二引数: タブの生成処理
	//第三引数: タブの表示名
	
	//Advance Deletionタブの登録
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
		.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"));

	//Locked Actors Listタブの登録
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("LockedActorsList"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnLockedActorsListTab))
		.SetDisplayName(FText::FromString(TEXT("Locked Actors List")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if (!FolderPathsSelected.IsEmpty())
	{
		//Advance Deletionタブの生成
		return
			SNew(SDockTab).TabRole(ETabRole::NomadTab)
			[
				//SAdvanceDeletionTabの要素の初期値設定
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
		//干渉するとクラッシュする可能性のあるフォルダーは対象外
		if (ContainsRestrictedPath(AssetPathName)) { continue; }

		//該当のPathのAssetが存在するかどうかを確認
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) { continue; }

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvaiableAssetsData.AddUnique(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnLockedActorsListTab(const FSpawnTabArgs& SpawnTabArgs)
{
	//Locked Actors Listタブの生成
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			//SLockedActorsListTabの要素の初期値設定
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
	//Lockの項目追加
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Lock Actor Selection")),
		FText::FromString(TEXT("Prevent actor from being selected")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), FName("LevelEditor.LockSelection")),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnLockActorSelectionButtonClicked)
	);
	//Unlockの項目追加
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Unlock all actor Selection")),
		FText::FromString(TEXT("Remove the selection constraint on all actor")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), FName("LevelEditor.UnlockSelection")),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockActorSelectionButtonClicked)
	);

	//Lock中のActor一覧表示の項目追加
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
	//選択されたアクターの取得
	//※USelectionは選択されたアクターのリストを保持するクラス
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

	//AssetBrowserを選択したAssetの位置に同期
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