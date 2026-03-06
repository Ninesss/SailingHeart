// Sailing Heart

#include "SHDebugLibrary.h"
#include "Block/SHBlockBase.h"
#include "Character/SHCharacterBase.h"

void USHDebugLibrary::SetGlobalDebugDisplay(bool bShow)
{
	ASHBlockBase::SetGlobalShowDebugAttributes(bShow);
	ASHCharacterBase::SetGlobalShowDebugAttributes(bShow);
}

bool USHDebugLibrary::GetGlobalDebugDisplay()
{
	return ASHBlockBase::GetGlobalShowDebugAttributes();
}

void USHDebugLibrary::ToggleGlobalDebugDisplay()
{
	SetGlobalDebugDisplay(!GetGlobalDebugDisplay());
}
