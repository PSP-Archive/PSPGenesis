//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifndef __YPSPFILE_H
#define __YPSPFILE_H

#include <YPSPTypes.h>

//------------------------------------------------------------------------------------------
// YPSPFile operations
//------------------------------------------------------------------------------------------
#define YPSPFILE_MAX_PATH		1024

YBOOL YPSPFile_Init(const YCHAR8 *a_pszFullAppPathname);
const YCHAR8 *YPSPFile_GetWorkingDir();

YINT32	YPSPFile_GetFileSize(const YCHAR8 *a_szFileName);
YBOOL	YPSPFile_GetFilePath(const YCHAR8 *a_szFileName, YCHAR8 *a_szFilePath, YINT32 a_iDestBufSize);
YBOOL	YPSPFile_GetFileBaseName(const YCHAR8 *a_szFileName, YCHAR8 *a_szFileBaseName, YINT32 a_iDestBufSize);

YUCHAR8 *YPSPFile_AllocReadFile(const YCHAR8 *a_szFileName, YINT32 *a_piBytesAllocRead);
YINT32 YPSPFile_ReadFileToBuffer(const YCHAR8 *a_szFileName, YUCHAR8 *a_pBuffer, YINT32 a_iMaxBufSize);

//------------------------------------------------------------------------------------------
// File operations
//------------------------------------------------------------------------------------------

// open modes
#define YPSPFILE_O_RDONLY    0x0001
#define YPSPFILE_O_WRONLY    0x0002
#define YPSPFILE_O_RDWR      0x0003
#define YPSPFILE_O_NBLOCK    0x0010
#define YPSPFILE_O_APPEND    0x0100
#define YPSPFILE_O_CREAT     0x0200
#define YPSPFILE_O_TRUNC     0x0400
#define YPSPFILE_O_NOWAIT    0x8000

// seek modes
#define YPSPFILE_SEEK_SET        (0)
#define YPSPFILE_SEEK_CUR        (1)
#define YPSPFILE_SEEK_END        (2)

YINT32	YPSP_fopen(const YCHAR8 *a_pFilename, YINT32 a_iMode);
void	YPSP_fclose(YINT32 a_iFileHandle);
YINT32	YPSP_fread(YINT32 a_iFileHandle, void *a_pBuf, YINT32 a_iSize);
YINT32	YPSP_fwrite(YINT32 a_iFileHandle, const void *a_pBuf, YINT32 a_iSize);
YINT64	YPSP_fseek(YINT32 a_iFileHandle, YINT64 a_iOffset, YINT32 a_iWhence);
YINT32	YPSP_fremove(const YCHAR8 *a_pFilename);
YINT32	YPSP_fmkdir(const YCHAR8 *a_pDirname, YINT32 a_iMode);
YINT32	YPSP_frmdir(const YCHAR8 *a_pDirname);
YINT32	YPSP_frename(const YCHAR8 *a_pOldName, const YCHAR8 *a_pNewName);
YINT32	YPSP_fdevctl(const YCHAR8 *a_pName, YINT32 a_iCmd, void *a_pArg, YINT32 a_iArgLen, void *a_pBuf, YINT32 a_iBuflen);

//------------------------------------------------------------------------------------------
// Directory operations
//------------------------------------------------------------------------------------------

#define YPSPFILE_TYPE_DIR		0x10
#define YPSPFILE_TYPE_FILE		0x20

typedef struct
{ 
    YUINT32 unk0; 
    YUINT32 type; 
    YUINT32 size; 
    YUINT32 unk[19]; 
    YCHAR8	name[0x108]; 
	YUCHAR8	dmy[128];
} YPSP_DirEntry;

YINT32	YPSP_DirOpen(const YCHAR8 *a_pDirName);
YINT32	YPSP_DirRead(YINT32 a_iDirHandle, YPSP_DirEntry *a_de);
void	YPSP_DirClose(YINT32 a_iDirHandle);

#endif //__YPSPFILE_H
