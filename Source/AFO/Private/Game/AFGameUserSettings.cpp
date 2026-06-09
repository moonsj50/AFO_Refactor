// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AFGameUserSettings.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"

static void SetSoundClassVolume(USoundClass* SC, float Volume)
{
	if (SC)
	{
		SC->Properties.Volume = FMath::Clamp(Volume, 0.f, 1.f);
	}
}

void UAFGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);
	ApplySoundSettings();
}

void UAFGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);
	ApplySoundSettings();
	SaveSettings();
}

void UAFGameUserSettings::SetMasterVolume(float InValue)
{
	MasterVolume = FMath::Clamp(InValue, 0.f, 1.f);
	ApplySoundSettings();
}

void UAFGameUserSettings::SetBGMVolume(float InValue)
{
	BGMVolume = FMath::Clamp(InValue, 0.f, 1.f);
	ApplySoundSettings();
}

void UAFGameUserSettings::SetSFXVolume(float InValue)
{
	SFXVolume = FMath::Clamp(InValue, 0.f, 1.f);
	ApplySoundSettings();
}

void UAFGameUserSettings::ApplySoundSettings()
{

}