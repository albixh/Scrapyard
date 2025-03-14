#pragma once
#include "CoreMinimal.h"

class UAnimSequence;

class FUncooker
{
public:
	static void UncookAssets(TArray<UObject*> Objects);
	static UPackage* UncookAnimSequence(UAnimSequence* srcAsset);
};
