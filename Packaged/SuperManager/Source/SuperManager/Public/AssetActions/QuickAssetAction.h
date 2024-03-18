// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"	
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"

#include "QuickAssetAction.generated.h"


/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	/// <summary>
	/// Assetの複製
	/// </summary>
	/// <param name="NumOfDuplicates">複製数</param>
	UFUNCTION(CallInEditor)
	void DuplicarteAssets(int32 NumOfDuplicates);
	
	/// <summary>
	/// 形式に合わせてAsset名の頭に付ける
	/// </summary>
	UFUNCTION(CallInEditor)
	void AddPrefixes();
	
	/// <summary>
	/// 未参照のAssetを削除
	/// </summary>
	UFUNCTION(CallInEditor)
	void RemoveUnusedAssets();

private:
	/// <summary>
	/// 参照状態の修正
	/// </summary>
	void FixUpRedirectors();
	
private:
	TMap<UClass*, FString> PrefixMap =
	{
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")}
	};
};
