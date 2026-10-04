#include "ChionConfigLibrary.h"

#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"


bool UChionConfigLibrary::BuildConfigFilePath(
	const FString& FileName,
	FString& OutFilePath)
{
	if (FileName.IsEmpty())
	{
		return false;
	}


	/*
	 * Nur den eigentlichen Dateinamen übernehmen.
	 * Dadurch können keine beliebigen Pfade übergeben werden.
	 *
	 * Beispiele:
	 * SoundSettings
	 * SoundSettings.ini
	 *
	 * ergeben beide:
	 * SoundSettings.ini
	 */
	const FString CleanFileName =
		FPaths::GetBaseFilename(
			FPaths::GetCleanFilename(FileName)
		);


	if (CleanFileName.IsEmpty())
	{
		return false;
	}


	const FString ConfigDirectory =
		FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("Config")
		);


	IFileManager::Get().MakeDirectory(
		*ConfigDirectory,
		true
	);


	OutFilePath =
		FPaths::Combine(
			ConfigDirectory,
			CleanFileName + TEXT(".ini")
		);


	return true;
}


bool UChionConfigLibrary::SaveConfigFloat(
	const FString& FileName,
	const FString& Section,
	const FString& Key,
	float Value)
{
	if (Section.IsEmpty() || Key.IsEmpty())
	{
		return false;
	}


	FString FilePath;

	if (!BuildConfigFilePath(
		FileName,
		FilePath))
	{
		return false;
	}


	FConfigFile ConfigFile;


	/*
	 * Vorhandene Datei zuerst einlesen,
	 * damit andere Werte erhalten bleiben.
	 */
	if (IFileManager::Get().FileExists(*FilePath))
	{
		ConfigFile.Read(FilePath);
	}


	ConfigFile.SetFloat(
		*Section,
		*Key,
		Value
	);


	return ConfigFile.Write(FilePath);
}


bool UChionConfigLibrary::LoadConfigFloat(
	const FString& FileName,
	const FString& Section,
	const FString& Key,
	float DefaultValue,
	float& OutValue)
{
	OutValue = DefaultValue;


	if (Section.IsEmpty() || Key.IsEmpty())
	{
		return false;
	}


	FString FilePath;

	if (!BuildConfigFilePath(
		FileName,
		FilePath))
	{
		return false;
	}


	if (!IFileManager::Get().FileExists(*FilePath))
	{
		return false;
	}


	FConfigFile ConfigFile;

	ConfigFile.Read(FilePath);


	float LoadedValue = DefaultValue;

	if (!ConfigFile.GetFloat(
		*Section,
		*Key,
		LoadedValue))
	{
		return false;
	}


	OutValue = LoadedValue;

	return true;
}