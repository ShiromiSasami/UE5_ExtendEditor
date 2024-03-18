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
	/// �I�������e�N�X�`������}�e���A�����쐬����
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

	/// <summary>
	/// ORM�e�N�X�`���𗘗p�����}�e���A���̃m�[�h��ݒ肷��
	/// </summary>
	/// <param name="CreateMaterial">�ݒ肷��}�e���A��</param>
	/// <param name="SelectedTexture">�Ώۂ̃e�N�X�`��</param>
	/// <param name="PinsConnectedCounter">�q�����Ă���s���̐�</param>
	void ORM_CreateMaterialNodes(
		UMaterial* CreateMaterial,
		UTexture2D* SelectedTexture,
		uint32& PinsConnectedCounter);

#pragma endregion

#pragma region CreateMaterialNodesConnectPins

	/// <summary>
	/// MterialMode�̐ڑ��֐��Z�b�g�\����
	/// </summary>
	struct MaterialNodeConnection {
		bool (UMaterial::* HasConnected)() const;
		bool (UQuickMaterialCreationWidget::* TryConnect)(UMaterialExpressionTextureSample*, UTexture2D*, UMaterial*);
	};

	/// <summary>
	/// �x�[�X�J���[�m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">�x�[�X�J���[�̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectBaseColor(
		UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// ���^���b�N�m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">���^���b�N�̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectMetalic(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// ���t�l�X�m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">���t�l�X�̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectRoughness(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// �m�[�}���m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">�m�[�}���̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectNormal(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// �A���r�G���g�I�N���[�W�����m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">�A���r�G���g�I�N���[�W�����̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectAmbientOcclusion(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// �A���r�G���g�I�N���[�W�����A���t�l�X�A���^���b�N����ɂ����m�[�h�̐���
	/// </summary>
	/// <param name="TextureSampleNode">�Ώۂ̃e�N�X�`���m�[�h</param>
	/// <param name="SelectedTexture">ORM�̃e�N�X�`��</param>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>����/�ݒ�̐������</returns>
	bool TryConnectORM(
		class UMaterialExpressionTextureSample* TextureSampleNode,
		UTexture2D* SelectedTexture,
		UMaterial* CreateMaterial);

	/// <summary>
	/// ORM�̃s�����q�����Ă��邩���m�F����
	/// </summary>
	/// <param name="CreateMaterial">�Ώۂ̃}�e���A��</param>
	/// <returns>�ǂꂩ��ł��q�����Ă��邩�̗L��</returns>
	bool HasORMConnected(UMaterial* CreateMaterial) const;

#pragma endregion

	UMaterialInstanceConstant* CreateMaterialInstanceAsset(
		UMaterial* CreatedMaterial,
		FString NameOfMateriaInstance,
		const FString& PathToPut);

};
