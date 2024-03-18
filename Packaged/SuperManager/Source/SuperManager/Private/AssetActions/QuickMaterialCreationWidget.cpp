// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"

#include "DebugHeader.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialInstanceConstant.h"

#pragma region QuickMaterialCreation

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if (bCustomMaterialName)
	{
		// �}�e���A�������󔒂��AM_�����̏ꍇ�̓G���[��\��
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

	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath) ||
		CheckIsNameUsed(SelectedTextureFolderPath, MaterialName))
	{ 
		MaterialName = TEXT("M_");
		return;
	}

	//�}�e���A���̐����Ɏ��s�����ꍇ�̓G���[��\��
	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);
	if (!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
		return;
	}
	
	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if(!SelectedTexture){ continue; }

		switch (ChannelPackingType)
		{
			using enum E_ChannelPackingType;
		case ECPT_NoChannelPacking:
			Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;

		case ECPT_ORM:
			ORM_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;

		case ECPT_MAX:
			break;

		default:
			break;
		}
	}

	if (PinsConnectedCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Successfully connected ") + FString::FromInt(PinsConnectedCounter) + TEXT(" pins")
		);
	}

	if (bCreateMaterialInstance)
	{
		CreateMaterialInstanceAsset(CreatedMaterial, MaterialName, SelectedTextureFolderPath);
	}

	MaterialName = TEXT("M_");
}

#pragma endregion

#pragma region QuickMaterialCreationCore

bool UQuickMaterialCreationWidget::ProcessSelectedData(
	const TArray<FAssetData>& SelectedDataToProccess,
	TArray<UTexture2D*>& OutSelectedTexturesArray, 
	FString& OutSelectedTexturePackagePath)
{
	// �I�����ꂽ�A�Z�b�g���Ȃ��ꍇ�̓G���[��\��
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
		// �I�����ꂽ�A�Z�b�g���Ƀe�N�X�`���ȊO�̂��̂��܂܂�Ă���ꍇ�̓G���[��\��
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
		// �J�X�^���ł͂Ȃ��ꍇ�̓e�N�X�`�������}�e���A�����ɐݒ�
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
	//�Y���̃t�H���_�[���ɓ������O�̃}�e���A�������݂��邩���m�F
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

	MaterialNodeConnection Connections[] = {
	{ &UMaterial::HasBaseColorConnected, &UQuickMaterialCreationWidget::TryConnectBaseColor },
	{ &UMaterial::HasMetallicConnected, &UQuickMaterialCreationWidget::TryConnectMetalic },
	{ &UMaterial::HasRoughnessConnected, &UQuickMaterialCreationWidget::TryConnectRoughness },
	{ &UMaterial::HasNormalConnected, &UQuickMaterialCreationWidget::TryConnectNormal },
	{ &UMaterial::HasAmbientOcclusionConnected, &UQuickMaterialCreationWidget::TryConnectAmbientOcclusion }
	};

	for (const auto& Connection : Connections) {
		if (!(CreateMaterial->*Connection.HasConnected)() &&
			(this->*Connection.TryConnect)(TextureSampleNode, SelectedTexture, CreateMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	DebugHeader::Print(TEXT("Failed to connect the texture: ") + SelectedTexture->GetName(), FColor::Red);
}

void UQuickMaterialCreationWidget::ORM_CreateMaterialNodes(
	UMaterial* CreateMaterial,
	UTexture2D* SelectedTexture, 
	uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode =
		NewObject<UMaterialExpressionTextureSample>(CreateMaterial);

	if (!TextureSampleNode) { return; }

	MaterialNodeConnection Connections[] = {
	{ &UMaterial::HasBaseColorConnected, &UQuickMaterialCreationWidget::TryConnectBaseColor },
	{ &UMaterial::HasNormalConnected, &UQuickMaterialCreationWidget::TryConnectNormal },
	};

	for (const auto& Connection : Connections) {
		if (!(CreateMaterial->*Connection.HasConnected)() &&
			(this->*Connection.TryConnect)(TextureSampleNode, SelectedTexture, CreateMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
		if (!HasORMConnected(CreateMaterial) &&
			TryConnectORM(TextureSampleNode, SelectedTexture, CreateMaterial))
		{
			PinsConnectedCounter += 3;
			return;
		}
	}

	DebugHeader::Print(TEXT("Failed to connect the texture: ") + SelectedTexture->GetName(), FColor::Red);
}

#pragma endregion

#pragma region CreateMaterialNodesConnectPins

bool UQuickMaterialCreationWidget::TryConnectBaseColor(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture, 
	UMaterial* CreateMaterial)
{
	//BaseColor�̃e�N�X�`�������܂ނ����m�F
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			//Node��BaseColor��ݒ�
			TextureSampleNode->Texture = SelectedTexture;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			FExpressionInput* BaseColor = CreateMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_BaseColor);
			BaseColor->Expression = TextureSampleNode;
			CreateMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectMetalic(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture,
	UMaterial* CreateMaterial)
{
	for (const FString& MetalicName : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetalicName))
		{
			//�e�N�X�`���̐ݒ��ύX
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			//Node��Metalic��ݒ�
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			FExpressionInput* Metallic = CreateMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_Metallic);
			Metallic->Expression = TextureSampleNode;
			CreateMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectRoughness(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture,
	UMaterial* CreateMaterial)
{
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessName))
		{
			//�e�N�X�`���̐ݒ��ύX
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			//Node��Metalic��ݒ�
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			FExpressionInput* Roughnes = CreateMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_Roughness);
			Roughnes->Expression = TextureSampleNode;
			CreateMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 480;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectNormal(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture,
	UMaterial* CreateMaterial)
{
	for (const FString& NormalName : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalName))
		{
			//�e�N�X�`���̐ݒ��ύX
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			//Node��Metalic��ݒ�
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			FExpressionInput* Normal = CreateMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_Normal);
			Normal->Expression = TextureSampleNode;
			CreateMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 720;
			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectAmbientOcclusion(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture, 
	UMaterial* CreateMaterial)
{
	for (const FString& AmbientOcclusionName : AmbientOcclusionArray)
	{
		if (SelectedTexture->GetName().Contains(AmbientOcclusionName))
		{
			//�e�N�X�`���̐ݒ��ύX
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			//Node��Metalic��ݒ�
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			FExpressionInput* AmbientOcclusion = CreateMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_AmbientOcclusion);
			AmbientOcclusion->Expression = TextureSampleNode;
			CreateMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 960;
			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectORM(
	UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture,
	UMaterial* CreateMaterial)
{
	for (const FString& ORMName : ORMArray)
	{
		if (SelectedTexture->GetName().Contains(ORMName))
		{
			//�e�N�X�`���̐ݒ��ύX
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			//Node��Metalic��ݒ�
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Masks;
			CreateMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			//�e�|�[�g�����ꂼ��̃m�[�h�ɐڑ�
			using enum EMaterialProperty;
			FExpressionInput* AmbientOcclusion = CreateMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion);
			AmbientOcclusion->Connect(1, TextureSampleNode);
			FExpressionInput* Roughness = CreateMaterial->GetExpressionInputForProperty(MP_Roughness);
			Roughness->Connect(2, TextureSampleNode);
			FExpressionInput* Metallic = CreateMaterial->GetExpressionInputForProperty(MP_Metallic);
			Metallic->Connect(3, TextureSampleNode);
			CreateMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 960;
			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::HasORMConnected(UMaterial* CreateMaterial) const
{
	return CreateMaterial->HasMetallicConnected() ||
		CreateMaterial->HasRoughnessConnected() ||
		CreateMaterial->HasAmbientOcclusionConnected();
}

UMaterialInstanceConstant* UQuickMaterialCreationWidget::CreateMaterialInstanceAsset(
	UMaterial* CreatedMaterial,
	FString NameOfMateriaInstance,
	const FString& PathToPut)
{
	NameOfMateriaInstance.RemoveFromStart(TEXT("M_"));
	NameOfMateriaInstance.InsertAt(0, TEXT("MI_"));

	UMaterialInstanceConstantFactoryNew* MIFactoryNew =
		NewObject<UMaterialInstanceConstantFactoryNew>(CreatedMaterial);

	//���W���[���̓ǂݍ���
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(
		NameOfMateriaInstance,
		PathToPut, 
		UMaterialInstanceConstant::StaticClass(),
		MIFactoryNew);

	if (UMaterialInstanceConstant* CreatedMI = Cast<UMaterialInstanceConstant>(CreatedObject))
	{
		//�e�}�e���A����ݒ�
		CreatedMI->SetParentEditorOnly(CreatedMaterial);
		CreatedMaterial->PostEditChange();

		return CreatedMI;
	}

	return nullptr;
}

#pragma endregion