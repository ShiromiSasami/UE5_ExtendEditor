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
	// ContentBrowserModule取得
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//コンテンツブラウザーのメニュー拡張機能一覧(デリゲート追加用リスト)を取得
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//拡張機能パスの追加(デリゲートバインド)
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

void FSuperManagerModule::RegisterAdvancedDeletionTab()
{
	//Advance Deletionタブの登録
	
	//第一引数: タブの名前
	//第二引数: タブの生成処理
	//第三引数: タブの表示名
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	//Advance Deletionタブの生成
	return 
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			//SAdvanceDeletionTabの要素の初期値設定
			SNew(SAdvanceDeletionTab).AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;

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