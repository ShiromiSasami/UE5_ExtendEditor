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
	/// <param name="SelectedPaths">選択パス</param>
	/// <returns>拡張機能クラス</returns>
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	/// <summary>
	/// メニュー生成
	/// </summary>
	/// <param name="MenuBuilder">メニュー項目生成クラス</param>
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// 削除ボタン選択時の処理(バインド処理)
	/// </summary>
	void OnDeleteUnusedAssetButtonClicked();
#pragma endregion

};
