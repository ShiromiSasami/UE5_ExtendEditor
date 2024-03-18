// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FSuperManagerStyle::StyleSetName = FName("SuperManagerStyle");
TSharedPtr< FSlateStyleSet > FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

void FSuperManagerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
	}
	//SlateStyleSet�̐���
	FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
}

void FSuperManagerStyle::Shutdown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}

TSharedRef<FSlateStyleSet> FSuperManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet =
		MakeShareable(new FSlateStyleSet(StyleSetName));

	//�v���O�C���t�H���_��Resources�f�B���N�g��Path���擾
	const FString IconDirectory =
	IPluginManager::Get().FindPlugin(TEXT("SuperManager"))->GetBaseDir() / "Resources";
	//Root�ƂȂ�Path��IconDirectory�ɐݒ�
	CustomStyleSet->SetContentRoot(IconDirectory);

	//�A�C�R���̓o�^
	const FVector2D Icon16x16(16.0f, 16.0f);
	CustomStyleSet->Set(
		"ContentBrowser.DeleteUnusedAssets",
		new FSlateImageBrush(IconDirectory / "DeleteUnusedAsset.png", Icon16x16));
	CustomStyleSet->Set(
		"ContentBrowser.DeleteEmptyFolders",
		new FSlateImageBrush(IconDirectory / "DeleteEmptyFolders.png", Icon16x16));
	CustomStyleSet->Set(
		"ContentBrowser.AdvanceDeletion",
		new FSlateImageBrush(IconDirectory / "AdvanceDeletion.png", Icon16x16));
	CustomStyleSet->Set(
		"LevelEditor.LockSelection",
		new FSlateImageBrush(IconDirectory / "SelectionLock.png", Icon16x16));
	CustomStyleSet->Set(
		"LevelEditor.UnlockSelection",
		new FSlateImageBrush(IconDirectory / "SelectionUnlock.png", Icon16x16));


	return CustomStyleSet;
}
