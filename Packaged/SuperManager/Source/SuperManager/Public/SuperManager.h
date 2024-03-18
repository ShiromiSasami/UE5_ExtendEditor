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
	/// アクターがロック状態か確認
	/// </summary>
	bool CheckIsActorSelectionLocked(AActor* ActorToProcess);

	/// <summary>
	/// アクターを任意のロック状態にする
	/// </summary>
	/// <param name="ActorToProcess">対象のActor</param>
	/// <param name="bShouldLocked">指定のロック状態</param>
	void ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLocked);

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
	/// 削除タブの登録初期処理
	/// </summary>
	void RegisterTab();

	/// <summary>
	/// 削除タブの表示時の処理
	/// </summary>
	/// <param name="Args"></param>
	/// <returns>タブの参照ポインタ</returns>
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	/// <summary>
	/// 選択したフォルダー下のアセットデータの配列を取得
	/// </summary>
	/// <returns>アセットデータの配列</returns>
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

	/// <summary>
	/// 削除タブを閉じた時の処理
	/// </summary>
	/// <param name="TabToClose">閉じたタブ</param>
	void OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose);

	/// <summary>
	/// ロック中のActor一覧タブの表示時の処理
	/// </summary>
	/// <param name="Args"></param>
	/// <returns>タブの参照ポインタ</returns>
	TSharedRef<SDockTab> OnSpawnLockedActorsListTab(const FSpawnTabArgs& SpawnTabArgs);

	/// <summary>
	/// レベル上に存在する全てのアクターの配列を取得
	/// </summary>
	/// <returns>アクターの配列</returns>
	TArray<TWeakObjectPtr<AActor>> GetAllLevelActors();

private:
	TSharedPtr<SDockTab> ConstructedDockTab;

#pragma endregion

#pragma region LevelEditorMenuExtension
private:
	/// <summary>
	/// LevelEditorのメニュー拡張機能の初期化
	/// </summary>
	void InitLevelEditorExtension();

	/// <summary>
	/// LevelEditorのメニュー拡張機能の追加
	/// </summary>
	/// <param name="UICommandList">UIコマンドリスト</param>
	/// <param name="SelectedActors">選択中のアクター</param>
	/// <returns>拡張機能</returns>
	TSharedRef<FExtender> CustomLevelEditorMenuExtender(
		const TSharedRef<FUICommandList> UICommandList,
		const TArray<AActor*> SelectedActors);

	/// <summary>
	/// LevelEditorのメニュー項目の追加
	/// </summary>
	/// <param name="MenuBuilder">項目追加クラス</param>
	void AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// LockActorSelection項目を選択時の処理
	/// </summary>
	void OnLockActorSelectionButtonClicked();

	/// <summary>
	/// UnlockActorSelection項目を選択時の処理
	/// </summary>
	void OnUnlockActorSelectionButtonClicked();

	/// <summary>
	/// LockedActorsList項目を選択時の処理
	/// </summary>
	void OnDisplayListOfLockedActorsButtonClicked();

#pragma endregion

#pragma region SelectionLock

	/// <summary>
	/// 選択イベントの初期化
	/// </summary>
	void InitCustomSelectionEvent();

	/// <summary>
	/// 選択時の処理内容
	/// </summary>
	void OnActorSelected(UObject* SelectedObject);

	/// <summary>
	/// アクターをロック状態にする
	/// </summary>
	void LockActorSelection(AActor* ActorToProcess);

	/// <summary>
	/// アクターをロック解除する
	/// </summary>
	void UnlockActorSelection(AActor* ActorToProcess);

#pragma endregion

#pragma region CustomEditorUICommands

private:
	/// <summary>
	/// カスタムコマンドリストの初期化
	/// </summary>
	void InitCustomUICommands();

	/// <summary>
	/// Lockコマンドキーが押された時の処理
	/// </summary>
	void OnSelectionLockHotkeyPressed();

	/// <summary>
	/// Lockコマンドキーが押された時の処理
	/// </summary>
	void OnUnlockActorsSelectionHotkeyPressed();

private:
	TSharedPtr<class FUICommandList> CustomUICommands;

#pragma endregion

#pragma region SceneOutlinerExtension

	/// <summary>
	/// シーンアウトライナーのカラム拡張機能の初期化
	/// </summary>
	void InitSceneOutlinerColumnExtension();

	/// <summary>
	/// ロックのカラムを選択した時の処理
	/// </summary>
	/// <param name="SceneOutliner">選択したアウトライナー</param>
	/// <returns>選択したカラム</returns>
	TSharedRef<ISceneOutlinerColumn> OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner);

	/// <summary>
	/// アウトライナカラムの登録解除
	/// </summary>
	void UnregisterSceneOutlinerColumnExtension();

#pragma endregion

#pragma region ProccessDataForTab

public:
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
		TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);

	/// <summary>
	/// 任意ワードを含んだのアセットのみのリストに変換
	/// </summary>
	/// <param name="AssetsDataToFilter">変換したいアセットリスト</param>
	/// <param name="OutSameNameAssetsData">変換後アセットリスト</param>
	void ListSameNameAssetsForAssetList(
		const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
		TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);

	/// <summary>
	/// 選択したAssetパス位置にAssetBrowserを同期する
	/// </summary>
	/// <param name="AssetPathToSync">同期するAssetのPath</param>
	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);

	/// <summary>
	/// 任意のアクター配列をロックしているアセットのみのリストに変換
	/// </summary>
	/// <param name="ActorToFilter">変換したいアセットリスト</param>
	/// <param name="OutLockActorData">変換後アセットリスト</param>
	void ListLockActorForActorList(
		const TArray<TWeakObjectPtr<AActor>>& ActorToFilter,
		TArray<TWeakObjectPtr<AActor>>& OutLockActorData);

	/// <summary>
	/// 任意のアクター配列をロックされてないアクターのみのリストに変換
	/// </summary>
	/// <param name="ActorToFilter">変換したいアセットリスト</param>
	/// <param name="OutUnlockActorData">変換後アセットリスト</param>
	void ListUnlockActorForActorList(
		const TArray<TWeakObjectPtr<AActor>>& ActorToFilter,
		TArray<TWeakObjectPtr<AActor>>& OutUnlockActorData);

#pragma endregion

private:
	bool GetEditorActorSubSystem();

	/// <summary>
	/// Outlinerのリフレッシュ
	/// </summary>
	void RefreshSceneOutliner();

private:
	TWeakObjectPtr<UEditorActorSubsystem> WeakEditorActorSubSystem;
	TSharedPtr<SLockedActorsListTab> LockedActorsListTab;
};
