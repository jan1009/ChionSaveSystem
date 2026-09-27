#include "ChionSaveArchive.h"


FChionSaveArchive::FChionSaveArchive(FArchive& InnerArchive)
	: FObjectAndNameAsStringProxyArchive(
		InnerArchive,
		true
	)
{
	// Nur Properties mit SaveGame-Flag berÃ¼cksichtigen.
	ArIsSaveGame = true;

	// Werte vollstÃ¤ndig speichern, auch wenn sie Defaults entsprechen.
	ArNoDelta = true;
}
