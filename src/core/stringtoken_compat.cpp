#include "tier0/platform.h"
#include "tier1/utlstringtoken.h"

PLATFORM_INTERFACE bool g_bUpdateStringTokenDatabase = false;
PLATFORM_INTERFACE bool* g_pUpdateStringTokenDatabase = &g_bUpdateStringTokenDatabase;

PLATFORM_INTERFACE void RegisterStringToken(uint32 nHashCode, const char* pStart, const char* pEnd, bool bExtraAddToDatabase)
{
    (void)nHashCode;
    (void)pStart;
    (void)pEnd;
    (void)bExtraAddToDatabase;
}