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
	/// コンテンツブラウザーのメニュー拡張機能の初期化
	/// </summary>
	void InitCBMenuExtention();

	/// <summary>
	/// パスの選択処理追加
	/// </summary>
	/// <param name="SelectedPaths">選択対象のパス配列</param>
	/// <returns>拡張機能クラス</returns>
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	/// <summary>
	/// メニュー生成
	/// </summary>
	/// <param name="MenuBuilder">メニュー項目生成クラス</param>
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// DeleteUnusedAssetが選択された時の処理
	/// </summary>
	void OnDeleteUnusedAssetButtonClicked();

	/// <summary>
	/// DeleteEmptyFoldersが選択された時の処理
	/// </summary>
	void OnDeleteEmptyFoldersButtonClicked();

	/// <summary>
	/// DeleteUnusedAssetAndFoldersが選択された時の処理
	/// </summary>
	void OnDeleteUnusedAssetAndFoldersButtonClicked();

	/// <summary>
	/// AdvanceDeletionが選択された時の処理
	/// </summary>
	void OnAdvanceDeletionButtonClicked();

	/// <summary>
	/// 参照の修正
	/// </summary>
	void FixUpRedirectors();

	/// <summary>
	/// 制限のあるPathを含むかどうか確認
	/// </summary>
	/// <param name="Path">対象のパス</param>
	/// <returns>true: あり false: なし</returns>
	bool ContainsRestrictedPath(const FString& Path);

private:
	TArray<FString> FolderPathsSelected;
#pragma endregion

#pragma region CustomEditorTab
private:
	/// <summary>
	/// タブの登録初期処理
	/// </summary>
	void RegisterAdvancedDeletionTab();

	/// <summary>
	/// タブの表示時の処理
	/// </summary>
	/// <param name="Args"></param>
	/// <returns>タブの参照ポインタ</returns>
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	/// <summary>
	/// 選択したフォルダー下のアセットデータの配列を取得
	/// </summary>
	/// <returns>アセットデータの配列</returns>
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
#pragma endregion

public:

#pragma region ProccessDataForAdvancedDeletionTab

	/// <summary>
	/// AssetListから単体アセットを削除
	/// </summary>
	/// <param name="AssetDataToDelete">削除するAssetData</param>
	/// <returns>削除成功: true 削除失敗: false</returns>
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);

	/// <summary>
	/// AssetListから複数アセットを削除
	/// </summary>
	/// <param name="AssetsToDelete">削除Asset配列</param>
	/// <returns>削除成功: true 削除失敗: false</returns>
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);

	/// <summary>
	/// 任意のアセットリストを未使用のアセットのみのリストに変換
	/// </summary>
	/// <param name="AssetsDataToFilter">変換したいアセットリスト</param>
	/// <param name="OutUnusedAssetsData">変換後アセットリスト</param>
	void ListUnusedAssetsForAssetList(
		const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, 
		TArray<TSharedPtr<FAssetData>> OutUnusedAssetsData);

#pragma endregion

};
