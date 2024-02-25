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
	//���͒l��1�ȏォ�̊m�F
	if (NumOfDuplicates <= 0)
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	//�I������Asset�f�[�^���擾
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;
	//Asset�f�[�^�����ɕ���/�ۑ�
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

	//���������̒ʒm
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully duplicated") + FString::FromInt(Counter) + " files");
	}
}

void UQuickAssetAction::AddPrefixes()
{
	//�I������Asset���擾
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	//�I������Asset�̃N���X�ɂ���āA���ɂ���`����(Prefix)��ݒ�
	//����̓I�Ȍ`���Ɩ���Map�w�b�_�[�ɋL��
	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject) { continue; }

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());
		//�Ή�����Prefix��������Ȃ��ꍇ�A�G���[��\��
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();
		//����Prefix�����Ă���ꍇ�A�G���[��\��
		if (OldName.StartsWith(*PrefixFound))
		{
			Print(TEXT("Already has prefix added  "), FColor::Red);
			continue;
		}

		//MaterialInstanceConstant�̏ꍇ�̓��ꏈ��
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		//���l�[��
		const FString NewNameWithPrefix = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		++Counter;
	}

	//���l�[�������̒ʒm
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(Counter) + " assets");
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	//�I������Asset�f�[�^���擾
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	//�Q�Ɗ֌W�̏C��
	FixUpRedirectors();

	//���Q�Ƃ�Asset��T���A���Q�ƃ��X�g�ɒǉ�
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetReferencers = 
			UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString());

		if (AssetReferencers.IsEmpty())
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	//���Q�Ƃ�Asset���Ȃ��ꍇ�A�G���[��\��
	if (UnusedAssetsData.IsEmpty())
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
		return;
	}

	//���Q�Ƃ�Asset���폜
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData, true);
	if (NumOfAssetsDeleted == 0) { return; }

	//�폜�����̒ʒm
	ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(NumOfAssetsDeleted) + TEXT("unused assets"));
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	//Asset����/�񋓂��ł��郂�W���[���̎擾
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//���������̐ݒ�(/Game�ȉ��̃f�B���N�g���ŃN���X��ObjectRedirector�̃A�Z�b�g)
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(TEXT("/Game"));
	Filter.ClassPaths.Emplace(FName("/Script/Engine.ObjectRedirector"));

	//���������ɍ��v����Asset���擾
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	//UObjectRedirector�̃��X�g���쐬
	for (const FAssetData& RedirectorDate : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorDate.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	//RedirectorsToFixArray(�Q�ƃf�[�^)�����ƂɎQ�Ɗ֌W���C��
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}
	
