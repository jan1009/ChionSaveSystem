#include "ChionSaveSystemEditor.h"

#include "ChionSaveSystemEditorSettings.h"

#include "Dom/JsonObject.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "HttpModule.h"


#define LOCTEXT_NAMESPACE "FChionSaveSystemEditorModule"


DEFINE_LOG_CATEGORY_STATIC(
	LogChionSaveSystemEditor,
	Log,
	All
);


namespace
{
	bool ParseVersion(
		const FString& VersionString,
		int32& OutMajor,
		int32& OutMinor,
		int32& OutPatch)
	{
		FString CleanVersion =
			VersionString.TrimStartAndEnd();


		if (CleanVersion.StartsWith(
			TEXT("v"),
			ESearchCase::IgnoreCase))
		{
			CleanVersion.RightChopInline(1);
		}


		TArray<FString> Parts;

		CleanVersion.ParseIntoArray(
			Parts,
			TEXT("."),
			true
		);


		if (Parts.Num() < 2)
		{
			return false;
		}


		OutMajor =
			FCString::Atoi(*Parts[0]);

		OutMinor =
			FCString::Atoi(*Parts[1]);

		OutPatch =
			Parts.Num() >= 3
			? FCString::Atoi(*Parts[2])
			: 0;


		return true;
	}


	bool IsVersionNewer(
		const FString& CurrentVersion,
		const FString& AvailableVersion)
	{
		int32 CurrentMajor = 0;
		int32 CurrentMinor = 0;
		int32 CurrentPatch = 0;

		int32 AvailableMajor = 0;
		int32 AvailableMinor = 0;
		int32 AvailablePatch = 0;


		if (!ParseVersion(
			CurrentVersion,
			CurrentMajor,
			CurrentMinor,
			CurrentPatch))
		{
			return false;
		}


		if (!ParseVersion(
			AvailableVersion,
			AvailableMajor,
			AvailableMinor,
			AvailablePatch))
		{
			return false;
		}


		if (AvailableMajor != CurrentMajor)
		{
			return AvailableMajor > CurrentMajor;
		}


		if (AvailableMinor != CurrentMinor)
		{
			return AvailableMinor > CurrentMinor;
		}


		return AvailablePatch > CurrentPatch;
	}


	void ShowUpdateNotification(
		const FString& CurrentVersion,
		const FString& AvailableVersion)
	{
		const FText NotificationText =
			FText::Format(
				LOCTEXT(
					"UpdateAvailable",
					"ChionSaveSystem {0} is available.\nInstalled version: {1}"
				),
				FText::FromString(AvailableVersion),
				FText::FromString(CurrentVersion)
			);


		FNotificationInfo NotificationInfo(
			NotificationText
		);

		NotificationInfo.bFireAndForget = true;
		NotificationInfo.FadeInDuration = 0.2f;
		NotificationInfo.FadeOutDuration = 0.5f;
		NotificationInfo.ExpireDuration = 10.0f;


		NotificationInfo.Hyperlink =
			FSimpleDelegate::CreateLambda(
				[]()
				{
					FPlatformProcess::LaunchURL(
						TEXT(
							"https://github.com/jan1009/ChionSaveSystem/releases/latest"
						),
						nullptr,
						nullptr
					);
				}
			);


		NotificationInfo.HyperlinkText =
			LOCTEXT(
				"OpenReleasePage",
				"Open Release Page"
			);


		FSlateNotificationManager::Get()
			.AddNotification(NotificationInfo);
	}


	void CheckForUpdates()
	{
		const UChionSaveSystemEditorSettings* Settings =
			GetDefault<UChionSaveSystemEditorSettings>();


		if (!Settings
			|| !Settings->bCheckForUpdates)
		{
			return;
		}


		const TSharedPtr<IPlugin> Plugin =
			IPluginManager::Get().FindPlugin(
				TEXT("ChionSaveSystem")
			);


		if (!Plugin.IsValid())
		{
			UE_LOG(
				LogChionSaveSystemEditor,
				Warning,
				TEXT(
					"Update check failed: ChionSaveSystem plugin descriptor could not be found."
				)
			);

			return;
		}


		const FString CurrentVersion =
			Plugin->GetDescriptor().VersionName;


		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
			FHttpModule::Get().CreateRequest();


		Request->SetURL(
			TEXT(
				"https://api.github.com/repos/jan1009/ChionSaveSystem/releases/latest"
			)
		);

		Request->SetVerb(
			TEXT("GET")
		);

		Request->SetHeader(
			TEXT("Accept"),
			TEXT("application/vnd.github+json")
		);

		Request->SetHeader(
			TEXT("User-Agent"),
			TEXT("ChionSaveSystem-UnrealEditor")
		);


		Request->OnProcessRequestComplete().BindLambda(
			[CurrentVersion](
				FHttpRequestPtr HttpRequest,
				FHttpResponsePtr HttpResponse,
				bool bWasSuccessful)
			{
				if (!bWasSuccessful
					|| !HttpResponse.IsValid())
				{
					UE_LOG(
						LogChionSaveSystemEditor,
						Verbose,
						TEXT(
							"ChionSaveSystem update check could not reach GitHub."
						)
					);

					return;
				}


				if (HttpResponse->GetResponseCode() != 200)
				{
					UE_LOG(
						LogChionSaveSystemEditor,
						Verbose,
						TEXT(
							"ChionSaveSystem update check returned HTTP %d."
						),
						HttpResponse->GetResponseCode()
					);

					return;
				}


				TSharedPtr<FJsonObject> JsonObject;

				const TSharedRef<TJsonReader<>> JsonReader =
					TJsonReaderFactory<>::Create(
						HttpResponse->GetContentAsString()
					);


				if (!FJsonSerializer::Deserialize(
					JsonReader,
					JsonObject)
					|| !JsonObject.IsValid())
				{
					UE_LOG(
						LogChionSaveSystemEditor,
						Verbose,
						TEXT(
							"ChionSaveSystem update check received invalid JSON."
						)
					);

					return;
				}


				FString AvailableVersion;

				if (!JsonObject->TryGetStringField(
					TEXT("tag_name"),
					AvailableVersion))
				{
					return;
				}


				UE_LOG(
					LogChionSaveSystemEditor,
					Log,
					TEXT(
						"Update check complete. Installed: %s | Latest: %s"
					),
					*CurrentVersion,
					*AvailableVersion
				);


				if (IsVersionNewer(
					CurrentVersion,
					AvailableVersion))
				{
					ShowUpdateNotification(
						CurrentVersion,
						AvailableVersion
					);
				}
			}
		);


		Request->ProcessRequest();
	}
}


void FChionSaveSystemEditorModule::StartupModule()
{
	ISettingsModule* SettingsModule =
		FModuleManager::GetModulePtr<ISettingsModule>(
			TEXT("Settings")
		);


	if (SettingsModule)
	{
		SettingsModule->RegisterSettings(
			TEXT("Project"),
			TEXT("Plugins"),
			TEXT("ChionSaveSystem"),
			LOCTEXT(
				"ChionSaveSystemSettingsName",
				"Chion Save System"
			),
			LOCTEXT(
				"ChionSaveSystemSettingsDescription",
				"Configure editor settings for ChionSaveSystem."
			),
			GetMutableDefault<UChionSaveSystemEditorSettings>()
		);
	}


	CheckForUpdates();
}


void FChionSaveSystemEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule =
		FModuleManager::GetModulePtr<ISettingsModule>(
			TEXT("Settings")
		);


	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings(
			TEXT("Project"),
			TEXT("Plugins"),
			TEXT("ChionSaveSystem")
		);
	}
}


#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(
	FChionSaveSystemEditorModule,
	ChionSaveSystemEditor
)