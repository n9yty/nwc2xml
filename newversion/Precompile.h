// Precompile.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// This version pulls in the wxWidgets shim (wxshim.h) instead of the real
// wxWidgets headers; the program has no wxWidgets dependency any more.

#if !defined(__Precompile_H__)
#define __Precompile_H__

#pragma once

#include "wxshim.h"
#include "TVector.h"

typedef TVector<unsigned char> TByteArray;

typedef unsigned char BYTE;
typedef unsigned int  UINT;
typedef unsigned int  DWORD;
typedef const wxChar* LPCTSTR;

#define	HIBYTE(W)		(((W) >> 8) & 0xFF)
#define	LOBYTE(W)		((W) & 0xFF)

#define	ANSI_CHARSET		0

#define	ERROR_SUCCESS		0
#define	ERROR_INVALID_DATA	13
#define	ERROR_GEN_FAILURE	31

#define _countof(x)	(sizeof(x)/sizeof(x[0]))

inline long MulDiv(long a, long b, long c) { return a*b/c; }

#define	ERROR_INVALID_VERSION	100
#define	ERROR_INVALID_STAFF		101

#define	ReadBytes(f, x)	( f.Read(&x, sizeof(x)) == sizeof(x) )

#include "nwcendian.h"

// read little endian data
inline bool ReadLEShort(wxFile& f, short& data)
{
	bool bResult = f.Read(&data, sizeof(data)) == sizeof(data);
	if ( bResult )
	{
		data = LE2HostS(data);
	}
	return bResult;
}

#endif // !defined(__Precompile_H__)
