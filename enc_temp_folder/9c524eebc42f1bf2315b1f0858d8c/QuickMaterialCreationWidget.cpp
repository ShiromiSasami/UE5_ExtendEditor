// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"

#include "DebugHeader.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"

#pragma region QuickMaterialCreationCore

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if (bCustomMaterialName)
	{
		// マテリアル名が空白か、M_だけの場合はエラーを表示
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));
			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;

	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath)) { return; }
	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName)) { return; }

	//マテリアルの生成に失敗した場合はエラーを表示
	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);
	if (!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
		return;
	}
	
	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if(!SelectedTexture){ continue; }

		Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
	}
}

#pragma endregion

#pragma region QuickMaterialCreation

bool UQuickMaterialCreationWidget::ProcessSelectedData(
	const TArray<FAssetData>& SelectedDataToProccess,
	TArray<UTexture2D*>& OutSelectedTexturesArray, 
	FString& OutSelectedTexturePackagePath)
{
	// 選択されたアセットがない場合はエラーを表示
	if (SelectedDataToProccess.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No texture selected"));
		return false;
	}
	
	for (const FAssetData& SelectedData : SelectedDataToProccess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();
		
		if (!SelectedAsset) continue;

		bool bMaterialNameSet = false;
		// 選択されたアセット内にテクスチャ以外のものが含まれている場合はエラーを表示
		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);
		if (!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(
				EAppMsgType::Ok,
				TEXT("No texture selected ") + SelectedAsset->GetName() + TEXT(" is not a texture")
			);

			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture);
		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}
		// カスタムではない場合はテクスチャ名をマテリアル名に設定
		if (!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedTexture->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0, TEXT("M_"));

			bMaterialNameSet = true;
		}
	}

	return true;
}

bool UQuickMaterialCreationWidget::CheckIsNameUsed(
	const FString& FolderPathToCheck,
	const FString& MaterialNameToCheck)
{
	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);
	//該当のフォルダー内に同じ名前のマテリアルが存在するかを確認
	for (const FString& ExistingAssetPath : ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);
		if (MaterialNameToCheck.Equals(ExistingAssetName))
		{
			DebugHeader::ShowMsgDialog(
				EAppMsgType::Ok,
				ExistingAssetName + TEXT("is already used by asset")
			);
			return true;
		}
	}

	return false;
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset(
	const FString& NameOfTheMaterial,
	const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	UObject* CreatedObject =
		AssetToolsModule.Get().CreateAsset(
			NameOfTheMaterial, 
			PathToPutMaterial, 
			UMaterial::StaticClass(),
			MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

void UQuickMaterialCreationWidget::Default_CreateMaterialNodes(
	UMaterial* CreateMaterial, 
	UTexture2D* SelectedTexture,
	uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode =
		NewObject<UMaterialExpressionTextureSample>(CreateMaterial);

	if (!TextureSampleNode){ return; }

	if (!CreateMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreateMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
}

#pragma endregion

#pragma region CreateMaterialNodes

bool UQuickMaterialCreationWidget::TryConnectBaseColor(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture, 
	UMaterial* CreateMaterial)
{
	//BaseColorのテクスチャ名を含むかを確認
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			//NodeでBaseColorを設定
			TextureSampleNode->Texture = SelectedTexture;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			FExpressionInput* BaseColor = CreateMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_BaseColor);
			BaseColor->Expression = TextureSampleNode;

			CreateMaterial->PostEditChange();
			return true;
		}
	}

	return false;
}

#pragma endregion