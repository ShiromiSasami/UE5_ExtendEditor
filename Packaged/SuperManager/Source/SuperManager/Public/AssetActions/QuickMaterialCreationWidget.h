// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickMaterialCreationWidget.generated.h"

class UMaterialExpressionTextureSample;
class UMaterialInstanceConstant;

UENUM(BlueprintType)
enum class E_ChannelPackingType : uint8 
{
	ECPT_NoChannelPacking UMETA(DisplayName = "No Channel Packing"),
	ECPT_ORM UMETA(DisplayName = "OcclusionRoughnessMetallic"),
	ECPT_MAX UMETA(DisplayName = "DefaultMax")
};

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:

#pragma region QuickMaterialCreation

	/// <summary>
	/// 選択したテクスチャからマテリアルを作成する
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "CreateMaterialFromSelectedTextures")
	void CreateMaterialFromSelectedTextures();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	E_ChannelPackingType ChannelPackingType = E_ChannelPackingType::ECPT_NoChannelPacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCustomMaterialName = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures", meta = (EditCondition = "bCustomMaterialName"))
	FString MaterialName = TEXT("M_");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCreateMaterialInstance = false;

#pragma endregion

#pragma region SupportedTextureNames

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> BaseColorArray = {
		TEXT("_BaseColor"),
		TEXT("_Albedo"),
		TEXT("_Diffuse"),
		TEXT("_diff")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> MetallicArray = {
		TEXT("_Metallic"),
		TEXT("_metal")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> RoughnessArray = {
		TEXT("_Roughness"),
		TEXT("_RoughnessMap"),
		TEXT("_rough")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> NormalArray = {
		TEXT("_Normal"),
		TEXT("_NormalMap"),
		TEXT("_nor")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> AmbientOcclusionArray = {
		TEXT("_AmbientOcclusion"),
		TEXT("_AmbientOcclusionMap"),
		TEXT("_AO")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> ORMArray = {
		TEXT("_arm"),
		TEXT("_OcclusionRoughnessMetallic"),
		TEXT("_ORM")
	};

#pragma endregion

private:

#pragma region QuickMaterialCreationCore

	/// <summary>
	/// 選択されたアセットデータをテクスチャかフィルタリングする
	/// </summary>
	/// <param name="SelectedDataToProccess">選択したAssetData配列</param>
	/// <param name="OutSelectedTexturesArray">フィルタリングしたTexture2Dの配列</param>
	/// <param name="OutSelectedTexturePackagePath">フィルタリングしたテクスチャのパス</param>
	/// <returns>成功状態</returns>
	bool ProcessSelectedData(
		const TArray<FAssetData>& SelectedDataToProccess, 
		TArray<UTexture2D*>& OutSelectedTexturesArray,
		FString& OutSelectedTexturePackagePath);

	/// <summary>
	/// MaterialNameが使用されているかを確認する
	/// </summary>
	/// <param name="FolderPathToCheck">フォルダーパス</param>
	/// <param name="MaterialNameToCheck">対象のマテリアル名</param>
	/// <returns>対象の名前のアセットが存在の有無</returns>
	bool CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck);

	/// <summary>
	/// マテリアルを生成する
	/// </summary>
	/// <param name="NameOfTheMaterial">マテリアルの名前</param>
	/// <param name="PathToPutMaterial">マテリアルのパス</param>
	/// <returns>生成したマテリアル</returns>
	UMaterial* CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial);

	/// <summary>
	/// 通常のマテリアルのノードを設定する
	/// </summary>
	/// <param name="CreateMaterial">設定するマテリアル</param>
	/// <param name="SelectedTexture">対象のテクスチャ</param>
	/// <param name="PinsConnectedCounter">繋がっているピンの数</param>
	void Default_CreateMaterialNodes(
		UMaterial* CreateMaterial,
		UTexture2D* SelectedTexture, 
		uint32& PinsConnectedCounter);

	/// <summary>
	/// ORMテクスチャを利用したマテリアルのノードを設定する
	/// </summary>
	/// <param name="CreateMaterial">設定するマテリアル</param>
	/// <param name="SelectedTexture">対象のテクスチャ</param>
	/// <param name="PinsConnectedCounter">繋がっているピンの数</param>
	void ORM_CreateMaterialNodes(
		UMaterial* CreateMaterial,
		UTexture2D* SelectedTexture,
		uint32& PinsConnectedCounter);

#pragma endregion

#pragma region CreateMaterialNodesConnectPins

	/// <summary>
	/// MterialModeの接続関数セット構造体
	/// </summary>
	struct MaterialNodeConnection {
		bool (UMaterial::* HasConnected)() const;
		bool (UQuickMaterialCreationWidget::* TryConnect)(UMaterialExpressionTextureSample*, UTexture2D*, UMaterial*);
	};

	/// <summary>
	/// ベースカラーノードの生成
	/// </summary>
	/// <param name="TextureSampleNode">対象のテクスチャノード</param>
	/// <param name="SelectedTexture">ベースカラーのテクスチャ</param>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>生成/設定の成功状態</returns>
	bool TryConnectBaseColor(
		UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// メタリックノードの生成
	/// </summary>
	/// <param name="TextureSampleNode">対象のテクスチャノード</param>
	/// <param name="SelectedTexture">メタリックのテクスチャ</param>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>生成/設定の成功状態</returns>
	bool TryConnectMetalic(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// ラフネスノードの生成
	/// </summary>
	/// <param name="TextureSampleNode">対象のテクスチャノード</param>
	/// <param name="SelectedTexture">ラフネスのテクスチャ</param>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>生成/設定の成功状態</returns>
	bool TryConnectRoughness(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// ノーマルノードの生成
	/// </summary>
	/// <param name="TextureSampleNode">対象のテクスチャノード</param>
	/// <param name="SelectedTexture">ノーマルのテクスチャ</param>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>生成/設定の成功状態</returns>
	bool TryConnectNormal(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// アンビエントオクルージョンノードの生成
	/// </summary>
	/// <param name="TextureSampleNode">対象のテクスチャノード</param>
	/// <param name="SelectedTexture">アンビエントオクルージョンのテクスチャ</param>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>生成/設定の成功状態</returns>
	bool TryConnectAmbientOcclusion(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// アンビエントオクルージョン、ラフネス、メタリックを一つにしたノードの生成
	/// </summary>
	/// <param name="TextureSampleNode">対象のテクスチャノード</param>
	/// <param name="SelectedTexture">ORMのテクスチャ</param>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>生成/設定の成功状態</returns>
	bool TryConnectORM(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// ORMのピンが繋がっているかを確認する
	/// </summary>
	/// <param name="CreateMaterial">対象のマテリアル</param>
	/// <returns>どれか一つでも繋がっているかの有無</returns>
	bool HasORMConnected(UMaterial* CreateMaterial) const;

#pragma endregion

	UMaterialInstanceConstant* CreateMaterialInstanceAsset(
		UMaterial* CreatedMaterial,
		FString NameOfMateriaInstance,
		const FString& PathToPut);

};
