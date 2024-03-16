// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickMaterialCreationWidget.generated.h"

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:

#pragma region QuickMaterialCreationCore

	/// <summary>
	/// �I�������e�N�X�`������}�e���A�����쐬����
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void CreateMaterialFromSelectedTextures();


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCustomMaterialName = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures", meta = (EditCondition = "bCustomMaterialName"))
	FString MaterialName = TEXT("M_");

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

#pragma endregion


private:

#pragma region QuickMaterialCreation

	/// <summary>
	/// �I�����ꂽ�A�Z�b�g�f�[�^���e�N�X�`�����t�B���^�����O����
	/// </summary>
	/// <param name="SelectedDataToProccess">�I������AssetData�z��</param>
	/// <param name="OutSelectedTexturesArray">�t�B���^�����O����Texture2D�̔z��</param>
	/// <param name="OutSelectedTexturePackagePath">�t�B���^�����O�����e�N�X�`���̃p�X</param>
	/// <returns>�������</returns>
	bool ProcessSelectedData(
		const TArray<FAssetData>& SelectedDataToProccess, 
		TArray<UTexture2D*>& OutSelectedTexturesArray,
		FString& OutSelectedTexturePackagePath);

	/// <summary>
	/// MaterialName���g�p����Ă��邩���m�F����
	/// </summary>
	/// <param name="FolderPathToCheck">�t�H���_�[�p�X</param>
	/// <param name="MaterialNameToCheck">�Ώۂ̃}�e���A����</param>
	/// <returns>�Ώۂ̖��O�̃A�Z�b�g�����݂̗L��</returns>
	bool CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck);

	/// <summary>
	/// �}�e���A���𐶐�����
	/// </summary>
	/// <param name="NameOfTheMaterial">�}�e���A���̖��O</param>
	/// <param name="PathToPutMaterial">�}�e���A���̃p�X</param>
	/// <returns>���������}�e���A��</returns>
	UMaterial* CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial);

	/// <summary>
	/// �ʏ�̃}�e���A���̃m�[�h��ݒ肷��
	/// </summary>
	/// <param name="CreateMaterial">�ݒ肷��}�e���A��</param>
	/// <param name="SelectedTexture">�Ώۂ̃e�N�X�`��</param>
	/// <param name="PinsConnectedCounter">�q�����Ă���s���̐�</param>
	void Default_CreateMaterialNodes(
		UMaterial* CreateMaterial,
		UTexture2D* SelectedTexture, 
		uint32& PinsConnectedCounter);

#pragma endregion

#pragma region CreateMaterialNodes

	/// <summary>
	/// �x�[�X�J���[�m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">�x�[�X�J���[�̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectBaseColor(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);


#pragma endregion

};
