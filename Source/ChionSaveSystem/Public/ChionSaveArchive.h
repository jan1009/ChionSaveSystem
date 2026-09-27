#pragma once

#include "CoreMinimal.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


/**
 * Archive fÃ¼r das ChionSaveSystem.
 *
 * ArIsSaveGame = true:
 * Unreal berÃ¼cksichtigt beim Serialisieren die Properties,
 * die mit SaveGame markiert wurden.
 *
 * ArNoDelta = true:
 * Auch Werte, die dem Default entsprechen, werden vollstÃ¤ndig
 * serialisiert. Das verhindert Probleme beim Wiederherstellen.
 */
class CHIONSAVESYSTEM_API FChionSaveArchive
	: public FObjectAndNameAsStringProxyArchive
{
public:
	explicit FChionSaveArchive(FArchive& InnerArchive);
};

