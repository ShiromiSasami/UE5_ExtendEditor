#pragma once

#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

namespace DebugHeader
{
	/// <summary>
	/// ��ʕ\���̃f�o�b�O���b�Z�[�W
	/// </summary>
	/// <param name="Message">���b�Z�[�W���e</param>
	/// <param name="Color">���b�Z�[�W�̐F</param>
	static void Print(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	/// <summary>
	/// ���[�j���O���O�o��
	/// </summary>
	/// <param name="Message">���b�Z�[�W���e</param>
	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	/// <summary>
	/// ���b�Z�[�W�_�C�A���O�̕\��
	/// </summary>
	/// <param name="MsgType">�_�C�A���O�̎��</param>
	/// <param name="Message">�\�����b�Z�[�W���e</param>
	/// <param name="bShowMsgAsWarning">Warning�̕\�L�̗L��</param>
	/// <returns>�_�C�A���O�I���̌���</returns>
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
	/// �����C�x���g�ʒm�̕\��
	/// </summary>
	/// <param name="Message">���b�Z�[�W���e</param>
	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}