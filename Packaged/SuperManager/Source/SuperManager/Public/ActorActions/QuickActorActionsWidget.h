// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidget.generated.h"

class UEditorActorSubsystem;

UENUM(BlueprintType)
enum class E_DuplicationAxis : uint8
{
	EDA_XAxis UMETA(DisplayName = "X Axis"),
	EDA_YAxis UMETA(DisplayName = "Y Axis"),
	EDA_ZAxis UMETA(DisplayName = "Z Axis"),
	EDA_MAX UMETA(DisplayName = "Default Max"),
};

USTRUCT(BlueprintType)
struct FRandomActorRotation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation)
	bool bRandomizeRotYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation, meta = (EditCondition = "bRandomizeRotYaw"))
	float RotYawMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation, meta = (EditCondition = "bRandomizeRotYaw"))
	float RotYawMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation)
	bool bRandomizeRotPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation, meta = (EditCondition = "bRandomizeRotPitch"))
	float RotPitchMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation, meta = (EditCondition = "bRandomizeRotPitch"))
	float RotPitchMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation)
	bool bRandomizeRotRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation, meta = (EditCondition = "bRandomizeRotRoll"))
	float RotRollMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RandomActorRotation, meta = (EditCondition = "bRandomizeRotRoll"))
	float RotRollMax = 45.f;

	bool IsAllRotAxiesRandomize() const
	{
		return !bRandomizeRotYaw && !bRandomizeRotPitch && !bRandomizeRotRoll;
	}
};

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickActorActionsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
#pragma region ActorBatchSelection

public:
	/// <summary>
	/// 選択方法の分岐管理関数
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsWidget")
	void SelectActorsBasedOnCondition();

private:
	/// <summary>
	/// 似た名前の全てのアクターを選択
	/// </summary>
	void SelectAllActorsWithSimilarName();

	/// <summary>
	/// キーワードを含む全てのアクターを選択
	/// </summary>
	void SelectAllActorsWithKeyword();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection", meta = (DisplayName = "SearchCase"))
	TEnumAsByte<ESearchCase::Type> SearchCaseForSelect = ESearchCase::IgnoreCase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection", meta = (DisplayName = "UseSelectingActors"))
	bool bUseSelectingActorsForSelect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection", meta = (DisplayName = "UseKeyword"))
	bool bUseKeywordForSelect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection", meta = (EditCondition = "bUseKeywordForSelect"), meta = (DisplayName = "SelectKeyword"))
	FString SearchKeywordForSelect = "";

private:
	TArray<AActor*> TargetActorsForSelect;

#pragma endregion

#pragma region ActorBatchDeletion

public:

	/// <summary>
	/// キーワードを含む全てのアクターを削除
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsWidget")
	void DeleteAllActorsWithKeyword();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDeletion", meta = (DisplayName = "SearchCase"))
	TEnumAsByte<ESearchCase::Type> SearchCaseForDelete = ESearchCase::IgnoreCase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDeletion", meta = (DisplayName = "UseSelectingActors"))
	bool bUseSelectingActorsForDelete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDeletion", meta = (DisplayName = "DeleteKeyword"))
	FString SearchKeywordForDelete = "";

private:
	TArray<AActor*> TargetActorsForDelete;

#pragma endregion

#pragma region ActorBatchDuplication

public:
	/// <summary>
	/// アクターの複製
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "ActorBatchDuplication")
	void DuplicateActors();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	E_DuplicationAxis AxisForDuplication = E_DuplicationAxis::EDA_XAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	int32 NumberOfDuplicates = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	float OffsetDist = 300.f;

#pragma endregion

#pragma region RandomizeActorTransform

public:
	/// <summary>
	/// Transformをランダムに変更
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "RandomizeActorTransform")
	void RandomizeActorTransform();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
	FRandomActorRotation RandomActorRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
	bool bRandomizeScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeScale"))
	float ScaleMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeScale"))
	float ScaleMax = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
	bool bRandomizeOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeOffset"))
	float OffsetMin = -50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeOffset"))
	float OffsetMax = 50.f;

#pragma endregion

#pragma region ResetActorTransform

public:
	/// <summary>
	/// Transformを初期値に変更
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsWidget")
	void ResetActorTransform();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResetActorTransform")
	bool bResetLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResetActorTransform")
	bool bResetRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResetActorTransform")
	bool bResetScale = false;

#pragma endregion

private:
	/// <summary>
	/// Editorのアクター管理システムを取得
	/// </summary>
	/// <returns>取得の成功状態</returns>
	bool GetEditorActorSubSystem();

private:
	UPROPERTY()
	UEditorActorSubsystem* EditorActorSubsystem;

};
