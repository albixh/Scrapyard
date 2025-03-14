#include "Uncooker.h"
#include "Animation/AnimCompress.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceDecompressionContext.h"
#include "AnimationUtils.h"
#include "FileHelpers.h"
#include "AssetRegistryModule.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Misc/MessageDialog.h"
#include "Animation/AnimCompressionTypes.h"
#include "Animation/AnimCurveCompressionSettings.h"
#include "Animation/AnimBoneCompressionSettings.h"

UPackage* FUncooker::UncookAnimSequence(UAnimSequence* srcAsset)
{
	// Create a new package for the uncooked asset
	FString srcPackagePath = FPackageName::ObjectPathToPackageName(srcAsset->GetPathName());
	FString srcPackageDir = FPackageName::GetLongPackagePath(srcPackagePath);
	FString srcPackageName = FPackageName::GetShortName(srcPackagePath);
	FString srcObjectName = FPackageName::ObjectPathToObjectName(srcAsset->GetPathName());
	FString dstPackagePath = srcPackageDir + "/tmp/" + srcPackageName;
	FString dstObjectName = srcObjectName;

	UPackage* package = CreatePackage(NULL, *dstPackagePath);
	if (package == nullptr)
		return nullptr;

	UAnimSequence* dstAnimationSequence = DuplicateObject<UAnimSequence>(srcAsset, package, *srcAsset->GetName());

	//UAnimSequence* dstAnimationSequence = NewObject<UAnimSequence>(package, *srcAsset->GetName(), RF_Standalone | RF_Public);
	if (!dstAnimationSequence)
		return nullptr;

	dstAnimationSequence->CompressedData.CompressedByteStream = srcAsset->CompressedData.CompressedByteStream;

	dstAnimationSequence->RemoveAllTracks();
	dstAnimationSequence->RawCurveData.FloatCurves.Empty();
	dstAnimationSequence->Notifies.Empty();

	// Copy raw animation tracks
	const TArray<FName>& trackNames = srcAsset->GetAnimationTrackNames();
	for (int i = 0; i < trackNames.Num(); ++i)
	{
		FRawAnimSequenceTrack rawAnimTrack;

		int numFrames = srcAsset->GetNumberOfFrames();
		for (int j = 0; j < numFrames; ++j)
		{
			float time = srcAsset->GetTimeAtFrame(j);

			FTransform xform;
			srcAsset->GetBoneTransform(xform, i, time, false);

			// Adjust root bone because of flipped
			// @TODO: Actually fix flipped axis
			if (i == 0)
			{
				FVector Translation = xform.GetTranslation();
				FQuat Rotation = xform.GetRotation();
				FVector Scale = xform.GetScale3D();

				// Convert from Y-up to Z-up
				//Translation = FVector(-Translation.X, Translation.Z, Translation.Y);

				// Rotate 90 degrees around Z, then 90 degrees around Y
				FQuat FixRotation = FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(90)) * 
					FQuat(FVector(0, 1, 0), FMath::DegreesToRadians(90));  
				FQuat FixRotation2 = FQuat(FVector(1, 0, 0), FMath::DegreesToRadians(180));

				Rotation = FixRotation * Rotation * FixRotation2;

				xform.SetTranslation(Translation);
				xform.SetRotation(Rotation);
				xform.SetScale3D(Scale);
			}

			rawAnimTrack.PosKeys.Add(xform.GetTranslation());
			rawAnimTrack.RotKeys.Add(xform.GetRotation());
			rawAnimTrack.ScaleKeys.Add(xform.GetScale3D());
		}

		dstAnimationSequence->AddNewRawTrack(trackNames[i], &rawAnimTrack);
	}


	// Copy animation curves
	const TArray<FSmartName>& curveNames = srcAsset->GetCompressedCurveNames();
	for (int i = 0; i < curveNames.Num(); ++i)
	{
		FFloatCurve floatCurve(curveNames[i], 0);

		int numFrames = srcAsset->GetNumberOfFrames();
		for (int j = 0; j < numFrames; ++j)
		{
			float time = srcAsset->GetTimeAtFrame(j);
			float value = srcAsset->EvaluateCurveData(curveNames[i].UID, time, false);
			floatCurve.UpdateOrAddKey(value, time);
		}

		dstAnimationSequence->RawCurveData.FloatCurves.Add(floatCurve);
	}

	dstAnimationSequence->CompressRawAnimData();

	// Copy animation notify tracks and handle external references
	const TArray<FAnimNotifyTrack>& srcNotifyTracks = srcAsset->AnimNotifyTracks;
	TArray<FAnimNotifyTrack>& dstNotifyTracks = dstAnimationSequence->AnimNotifyTracks;

	dstNotifyTracks.Reset();

	// If the destination has fewer tracks, add the missing tracks
	for (int32 i = dstNotifyTracks.Num(); i < srcNotifyTracks.Num(); ++i)
	{
		// Create a new notify track for the destination asset
		FAnimNotifyTrack NewTrack;
		NewTrack.TrackName = srcNotifyTracks[i].TrackName;
		NewTrack.TrackColor = srcNotifyTracks[i].TrackColor;
		//NewTrack.Notifies = srcNotifyTracks[i].Notifies;
		dstNotifyTracks.Add(NewTrack);
	}

	const TArray<FAnimNotifyEvent>& srcNotifies = srcAsset->Notifies;
	TArray<FAnimNotifyEvent>& dstNotifies = dstAnimationSequence->Notifies;

	for (const FAnimNotifyEvent& srcNotify : srcNotifies)
	{
		// Create a new instance of FAnimNotifyEvent
		FAnimNotifyEvent dstNotify;

		// Manually copy values 
		dstNotify.NotifyName = srcNotify.NotifyName;
		dstNotify.SetTime(srcNotify.GetTime());
		dstNotify.Duration = srcNotify.Duration;
		dstNotify.Guid = srcNotify.Guid;
		dstNotify.NotifyColor = srcNotify.NotifyColor;
		dstNotify.TriggerTimeOffset = srcNotify.TriggerTimeOffset;
		dstNotify.EndTriggerTimeOffset = srcNotify.EndTriggerTimeOffset;
		dstNotify.MontageTickType = srcNotify.MontageTickType;
		dstNotify.NotifyTriggerChance = srcNotify.NotifyTriggerChance;
		dstNotify.NotifyFilterType = srcNotify.NotifyFilterType;
		dstNotify.NotifyFilterLOD = srcNotify.NotifyFilterLOD;
		dstNotify.TrackIndex = srcNotify.TrackIndex;
		dstNotify.bConvertedFromBranchingPoint = srcNotify.bConvertedFromBranchingPoint;

		dstNotify.EndLink.SetTime(srcNotify.EndLink.GetTime());
		dstNotify.EndLink.LinkSequence(dstAnimationSequence, dstAnimationSequence->SequenceLength);
		dstNotify.EndLink.ChangeSlotIndex(srcNotify.EndLink.GetSlotIndex());

		dstNotify.EndLink.SetSegmentIndex(srcNotify.EndLink.GetSegmentIndex());
		dstNotify.EndLink.ChangeLinkMethod(srcNotify.EndLink.GetLinkMethod());

		// Handle NotifyStateClass
		if (srcNotify.NotifyStateClass)
		{
			if (UAnimNotifyState* NotifyState = Cast<UAnimNotifyState>(srcNotify.NotifyStateClass))
			{
				UAnimNotifyState* NewNotifyState = NewObject<UAnimNotifyState>(package, NotifyState->GetClass());

				// Manually copy all properties from old NotifyState to new one
				for (TFieldIterator<FProperty> PropIt(NotifyState->GetClass()); PropIt; ++PropIt)
				{
					FProperty* Property = *PropIt;
					void* SrcValue = Property->ContainerPtrToValuePtr<void>(NotifyState);
					void* DstValue = Property->ContainerPtrToValuePtr<void>(NewNotifyState);
					Property->CopyCompleteValue(DstValue, SrcValue);
				}
				dstNotify.NotifyStateClass = NewNotifyState;
			}
		}

		// Handle Notify
		if (srcNotify.Notify)
		{
			if (UAnimNotify* Notify = Cast<UAnimNotify>(srcNotify.Notify))
			{
				UAnimNotify* NewNotify = NewObject<UAnimNotify>(package, Notify->GetClass());

				// Manually copy all properties from old Notify to new one
				for (TFieldIterator<FProperty> PropIt(Notify->GetClass()); PropIt; ++PropIt)
				{
					FProperty* Property = *PropIt;
					void* SrcValue = Property->ContainerPtrToValuePtr<void>(Notify);
					void* DstValue = Property->ContainerPtrToValuePtr<void>(NewNotify);
					Property->CopyCompleteValue(DstValue, SrcValue);
				}
				dstNotify.Notify = NewNotify;
			}
		}
		dstNotifies.Add(dstNotify);
	}

	dstAnimationSequence->PostProcessSequence();

	return package;
}	

void FUncooker::UncookAssets(TArray<UObject*> Objects)
{
	for (UObject* asset : Objects)
	{
		UPackage* srcPackage = asset->GetOutermost();
		if (srcPackage == nullptr || !srcPackage->bIsCookedForEditor)
			continue;

		UPackage* dstPackage = nullptr;
		if (UAnimSequence* animAsset = Cast<UAnimSequence>(asset))
			dstPackage = UncookAnimSequence(animAsset);

		if (dstPackage != nullptr)
		{
			FAssetRegistryModule::AssetCreated(dstPackage);
			dstPackage->MarkPackageDirty();

			FString UncookedPackageFileName = FPackageName::LongPackageNameToFilename(dstPackage->GetName(), FPackageName::GetAssetPackageExtension());
			bool bSaved = UPackage::SavePackage(dstPackage, nullptr, RF_Standalone, *UncookedPackageFileName, GError, nullptr, true, true, SAVE_NoError);

			if (bSaved)
			{
				// Successfully saved the uncooked asset. Now delete the original cooked asset.
				FString OriginalPackageFileName = FPackageName::LongPackageNameToFilename(asset->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
				asset->MarkPendingKill();
				GEngine->ForceGarbageCollection(true);
				FAssetRegistryModule::AssetDeleted(asset);

				// Move the uncooked asset to the original location of the cooked asset
				if (IFileManager::Get().Move(*OriginalPackageFileName, *UncookedPackageFileName))
				{
					UE_LOG(LogTemp, Log, TEXT("Moved uncooked asset to original location: %s"), *OriginalPackageFileName);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to move uncooked asset to original location: %s"), *OriginalPackageFileName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save uncooked package: %s"), *UncookedPackageFileName);
			}
		}
	}   
	FText Message = FText::FromString(TEXT("Uncooking finished."));
	FMessageDialog::Open(EAppMsgType::Ok, Message);
}