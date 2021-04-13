//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPFONT_H
#define __YPSPFONT_H

#include <YPSPTypes.h>

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------
typedef struct
{
	YCHAR8	szHeader[4];
	YCHAR8	szFontName[32];
	YINT32	iMaxFontSizePixels;
	YINT32	iNumIndexes;
	YINT32	iNumGlyphs;
} YBitmapFont;

YBitmapFont *YPSPFont_Load(const YCHAR8 *a_szFontName);
YUCHAR8 *YPSPFont_GetGlyphStream(YBitmapFont *a_pFont, YUINT16 a_uUnicodeCharID);

#endif //__YPSPFONT_H
