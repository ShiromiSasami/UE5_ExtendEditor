#pragma once

#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

namespace DebugHeader
{
	/// <summary>
	/// 画面表示のデバッグメッセージ
	/// </summary>
	/// <param name="Message">メッセージ内容</param>
	/// <param name="Color">メッセージの色</param>
	static void Print(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	/// <summary>
	/// ワーニングログ出力
	/// </summary>
	/// <param name="Message">メッセージ内容</param>
	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	/// <summary>
	/// メッセージダイアログの表示
	/// </summary>
	/// <param name="MsgType">ダイアログの種類</param>
	/// <param name="Message">表示メッセージ内容</param>
	/// <param name="bShowMsgAsWarning">Warningの表記の有無</param>
	/// <returns>ダイアログ選択の結果</returns>
	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true)
	{
		if (bShowMsgAsWarning)
		{
			FText MsgTitle = FText::FromString(TEXT("Warning"));

			return FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), MsgTitle);
		}
		else
		{
			return FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		}
	}

	/// <summary>
	/// 時限イベント通知の表示
	/// </summary>
	/// <param name="Message">メッセージ内容</param>
	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}