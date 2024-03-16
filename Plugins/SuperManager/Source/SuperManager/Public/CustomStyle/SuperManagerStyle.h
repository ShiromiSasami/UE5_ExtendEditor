// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Styling/SlateStyle.h"

class FSuperManagerStyle
{
public:
	static void InitializeIcons();
	static void Shutdown();

	static FName GetStyleSetName() { return StyleSetName; }

private:
	/// <summary>
	/// SlateStyleSetÇÃê∂ê¨ä÷êî
	/// </summary>
	/// <returns>SlateStyleSet</returns>
	static TSharedRef< FSlateStyleSet > CreateSlateStyleSet();

private:
	static FName StyleSetName;
	static TSharedPtr< FSlateStyleSet > CreatedSlateStyleSet;

};