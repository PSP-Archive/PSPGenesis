//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
#ifndef __YPSPTYPES_H
#define __YPSPTYPES_H
//------------------------------------------------------------------------------------------
#if defined (_DEBUG)
	#define YDEBUG
#else
	#define YRELEASE
#endif

//------------------------------------------------------------------------------------------
#if !defined (__PLATFORM_PSP__)
	#define __PLATFORM_PSP__
#endif

//------------------------------------------------------------------------------------------
typedef int	YBOOL;
#define	YFALSE	0
#define	YTRUE	1

typedef unsigned char		YUCHAR8;
typedef unsigned short		YUCHAR16;
typedef unsigned char		YUINT8;
typedef unsigned short		YUINT16;
typedef unsigned int		YUINT32;
typedef unsigned long long	YUINT64;

typedef char				YCHAR8;
typedef short				YCHAR16;
typedef char				YINT8;
typedef short				YINT16;
typedef int					YINT32;
typedef long long			YINT64;

typedef float				YFLOAT32;
typedef double				YDOUBLE64;

//------------------------------------------------------------------------------------------
#endif //__YPSPTYPES_H
