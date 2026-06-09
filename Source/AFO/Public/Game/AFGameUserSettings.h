#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "AFGameUserSettings.generated.h"

UCLASS()
class AFO_API UAFGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable) void SetMasterVolume(float InValue);
	UFUNCTION(BlueprintCallable) void SetBGMVolume(float InValue);
	UFUNCTION(BlueprintCallable) void SetSFXVolume(float InValue);

	UFUNCTION(BlueprintCallable) float GetMasterVolume() const { return MasterVolume; }
	UFUNCTION(BlueprintCallable) float GetBGMVolume() const { return BGMVolume; }
	UFUNCTION(BlueprintCallable) float GetSFXVolume() const { return SFXVolume; }

	virtual void LoadSettings(bool bForceReload) override;
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

private:
	UPROPERTY(config) float MasterVolume = 1.0f;
	UPROPERTY(config) float BGMVolume = 1.0f;
	UPROPERTY(config) float SFXVolume = 1.0f;

	void ApplySoundSettings();
};
