// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"

#include "DebugHeader.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"

void UQuickAssetAction::DuplicarteAssets(int32 NumOfDuplicates)
{
	//入力値が1以上かの確認
	if (NumOfDuplicates <= 0)
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	//選択したAssetデータを取得
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;
	//Assetデータを元に複製/保存
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (size_t i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourcePath = SelectedAssetData.GetObjectPathString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString()+ TEXT("_") + FString::FromInt(i+1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourcePath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	//複製成功の通知
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully duplicated") + FString::FromInt(Counter) + " files");
	}
}

void UQuickAssetAction::AddPrefixes()
{
	//選択したAssetを取得
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	//選択したAssetのクラスによって、頭につける形式名(Prefix)を設定
	//※具体的な形式と名のMapヘッダーに記載
	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject) { continue; }

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());
		//対応するPrefixが見つからない場合、エラーを表示
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();
		//既にPrefixがついている場合、エラーを表示
		if (OldName.StartsWith(*PrefixFound))
		{
			Print(TEXT("Already has prefix added  "), FColor::Red);
			continue;
		}

		//MaterialInstanceConstantの場合の特殊処理
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		//リネーム
		const FString NewNameWithPrefix = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		++Counter;
	}

	//リネーム成功の通知
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(Counter) + " assets");
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	//選択したAssetデータを取得
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	//参照関係の修正
	FixUpRedirectors();

	//未参照のAssetを探し、未参照リストに追加
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetReferencers = 
			UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString());

		if (AssetReferencers.IsEmpty())
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	//未参照のAssetがない場合、エラーを表示
	if (UnusedAssetsData.IsEmpty())
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
		return;
	}

	//未参照のAssetを削除
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData, true);
	if (NumOfAssetsDeleted == 0) { return; }

	//削除成功の通知
	ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(NumOfAssetsDeleted) + TEXT("unused assets"));
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	//Asset検索/列挙ができるモジュールの取得
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//検索条件の設定(/Game以下のディレクトリでクラスがObjectRedirectorのアセット)
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(TEXT("/Game"));
	Filter.ClassPaths.Emplace(FName("/Script/Engine.ObjectRedirector"));

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
	
