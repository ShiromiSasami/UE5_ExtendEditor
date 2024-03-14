// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}
	//要素の設定
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)
	SLATE_END_ARGS()

public:
	//Widget生成時のコンストラクタ
	void Construct(const FArguments& InArgs);

private:
	/// <summary>
	/// アセットリストビューの生成関数
	/// </summary>
	/// <returns>生成したアセットリストビュー</returns>
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	
	/// <summary>
	/// アセットリストビューのリフレッシュ関数
	/// </summary>
	void RefreshAssetListView();

#pragma region ComboBoxForListingCondition

	/// <summary>
	/// ComboBoxの生成関数
	/// </summary>
	/// <returns>ComboBox</returns>
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstractComboBox();

	/// <summary>
	/// フィルターのComboBoxの選択肢を生成する関数
	/// </summary>
	/// <param name="SourceItem">要素の名前</param>
	/// <returns>選択肢ウィジェット</returns>
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	/// <summary>
	/// 選択肢を選択する関数
	/// </summary>
	/// <param name="SelectedOption">選択した要素の名前</param>
	/// <param name="InSelectInfo">選択状態</param>
	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);

	//CombatBoxMemberValiables
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedPtr<STextBlock> ComboDisplayTextBlock;

#pragma endregion

#pragma region RowWidgetForAssetListView

	/// <summary>
	/// リストの要素を生成する関数
	/// </summary>
	/// <param name="AssetDataToDisplay">表示するAssetData</param>
	/// <param name="OwnerTable">テーブルの親</param>
	/// <returns>リストビューテーブル</returns>
	TSharedRef<ITableRow> OnGenerateRowForList(
		TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable
	);

	/// <summary>
	/// AssetDataのチェックボックスを生成する関数
	/// </summary>
	/// <param name="AssetDataToDisplay">表示するAssetData</param>
	/// <returns>チェックボックス</returns>
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/// <summary>
	/// チェックボックスイベントの紐づけ関数
	/// </summary>
	/// <param name="NewState">チェックボックスの状態</param>
	/// <param name="AssetData">関連するAssetData</param>
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	/// <summary>
	/// テキストブロックウィジェットを生成する関数
	/// </summary>
	/// <param name="TextContent">テキストの内容</param>
	/// <param name="FontToUse">フォント</param>
	/// <returns>テキストブロック</returns>
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	/// <summary>
	/// ボタンウィジェットを生成する関数
	/// </summary>
	/// <param name="AssetDataToDisplay">表示するAssetData</param>
	/// <returns>ボタン</returns>
	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/// <summary>
	/// ボタンイベントの紐づけ関数
	/// </summary>
	/// <param name="ClickedAssetData">クリックしたAssetData</param>
	/// <returns>イベントの処理状態</returns>
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

#pragma endregion

#pragma region TabButtons

	/// <summary>
	/// 全削除ボタンの生成関数
	/// </summary>
	/// <returns>全削除ボタン</returns>
	TSharedRef<SButton> ConstructDeleteAllButton();

	/// <summary>
	/// 全削除ボタンのイベント関数
	/// </summary>
	/// <returns>処理状態</returns>
	FReply OnDeleteAllButtonClicked();

	/// <summary>
	/// 全選択ボタンの生成関数
	/// </summary>
	/// <returns>全選択ボタン</returns>
	TSharedRef<SButton> ConstructSelectAllButton();

	/// <summary>
	/// 全選択ボタンのイベント関数
	/// </summary>
	/// <returns>処理状態</returns>
	FReply OnSelectAllButtonClicked();

	/// <summary>
	/// 全選択解除ボタンの生成関数
	/// </summary>
	/// <returns>全選択解除ボタン</returns>
	TSharedRef<SButton> ConstructDeselectAllButton();

	/// <summary>
	/// 全選択解除ボタンのイベント関数
	/// </summary>
	/// <returns>処理状態</returns>
	FReply OnDeselectAllButtonClicked();

	/// <summary>
	/// ボタンタブのテキストブロックを生成する関数
	/// </summary>
	/// <param name="TextContent">テキスト内容</param>
	/// <returns>テキストブロック</returns>
	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion

	FSlateFontInfo GetEmnossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); };

private:
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;

	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
};