// wxshim.h : minimal stand-in for the wxWidgets APIs used by nwc2xml.
//
// This implements just the surface the converter needs - wxString, wxFile,
// charset conversion, and a few helpers - on top of the C standard library
// (plus iconv for the --charset option and zlib in nwcfile.cpp).
// Strings are stored as UTF-8 in plain char buffers, so wxChar is char and
// the printf-style helpers map straight onto stdio.

#if !defined(__WXSHIM_H__)
#define __WXSHIM_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <string>

typedef char wxChar;

#ifndef _T
#define _T(x) x
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define WXUNUSED(name)

// the original was built with wxDEBUG_LEVEL=0, so asserts were no-ops
#define wxASSERT(cond) ((void)0)

#define wxFprintf fprintf
#define wxStrstr strstr
#define wxFopen fopen

inline bool wxIsDebuggerRunning() { return false; }
inline unsigned long wxSysErrorCode() { return (unsigned long)errno; }

class wxMBConv;

// Buffer returned by wxString::ToUTF8(); usable wherever a const char*
// is expected.
class wxCharBuffer
{
public:
	wxCharBuffer(const std::string& data) : m_data(data) {}
	operator const char*() const { return m_data.c_str(); }

protected:
	std::string m_data;
};

// UTF-8 string in a char buffer, with the subset of the wxString API that
// this program uses. Length/Left/Right/Mid work on bytes, which is fine
// here: they are only applied to ASCII file names and ASCII prefixes.
class wxString
{
public:
	wxString() {}
	wxString(const wxChar* psz) : m_str(psz ? psz : "") {}
	wxString(const wxChar* psz, const wxMBConv& conv);
	wxString(const std::string& str) : m_str(str) {}

	const wxChar* c_str() const { return m_str.c_str(); }
	operator const wxChar*() const { return m_str.c_str(); }

	size_t Length() const { return m_str.size(); }
	size_t size() const { return m_str.size(); }
	bool IsEmpty() const { return m_str.empty(); }
	void Empty() { m_str.clear(); }

	wxChar operator[](size_t nIndex) const { return m_str[nIndex]; }
	wxChar operator[](int nIndex) const { return m_str[(size_t)nIndex]; }

	wxString Left(size_t nCount) const;
	wxString Right(size_t nCount) const;
	wxString Mid(size_t nFirst) const;

	// replace all occurrences, return the number of replacements
	size_t Replace(const wxChar* szOld, const wxChar* szNew);

	// Trim(true) trims the right end, Trim(false) the left, like wx
	wxString& Trim(bool bFromRight = true);

	int CmpNoCase(const wxChar* psz) const
	{ return strcasecmp(m_str.c_str(), psz ? psz : ""); }

	wxString& operator=(const wxChar* psz)
	{ m_str = psz ? psz : ""; return *this; }

	wxString& operator+=(const wxString& str) { m_str += str.m_str; return *this; }
	wxString& operator+=(const wxChar* psz)   { if ( psz ) m_str += psz; return *this; }
	wxString& operator+=(wxChar ch)           { m_str += ch; return *this; }

	bool operator==(const wxString& rhs) const { return m_str == rhs.m_str; }
	bool operator!=(const wxString& rhs) const { return m_str != rhs.m_str; }

	void Printf(const wxChar* szFormat, ...);

	wxCharBuffer ToUTF8() const { return wxCharBuffer(m_str); }

protected:
	std::string m_str;
};

inline wxString operator+(const wxString& lhs, const wxString& rhs)
{ wxString s(lhs); s += rhs; return s; }
inline wxString operator+(const wxString& lhs, const wxChar* rhs)
{ wxString s(lhs); s += rhs; return s; }
inline wxString operator+(const wxChar* lhs, const wxString& rhs)
{ wxString s(lhs); s += rhs; return s; }
inline wxString operator+(const wxString& lhs, wxChar ch)
{ wxString s(lhs); s += ch; return s; }
inline wxString operator+(wxChar ch, const wxString& rhs)
{ wxString s; s += ch; s += rhs; return s; }

// Charset conversion to UTF-8. Replaces the part of wxMBConv that nwc2xml
// uses: converting byte strings read from .nwc files into wxStrings.
class wxMBConv
{
public:
	virtual ~wxMBConv() {}
	virtual bool IsOk() const { return true; }

	// Convert bytes to UTF-8. Returns false (with out cleared) when the
	// input is not valid in this conversion's charset.
	virtual bool ToUTF8(const std::string& in, wxString& out) const = 0;
};

// Strict UTF-8 pass-through. Fails on invalid UTF-8 so that callers can
// fall back to another charset (nwcobj.cpp falls back to CP1252).
class wxUTF8Conv : public wxMBConv
{
public:
	virtual bool ToUTF8(const std::string& in, wxString& out) const;
};

extern wxUTF8Conv wxConvLocal;

// Conversion from a named charset to UTF-8. CP1252 (the charset NWC uses
// for its strings) is built in; any other name is handled through iconv().
class wxCSConv : public wxMBConv
{
public:
	explicit wxCSConv(const wxChar* szCharset);
	virtual bool IsOk() const { return m_bOk; }
	virtual bool ToUTF8(const std::string& in, wxString& out) const;

protected:
	std::string m_strCharset;
	bool m_bOk;
	bool m_bBuiltinCP1252;
};

enum wxSeekMode
{
	wxFromStart   = SEEK_SET,
	wxFromCurrent = SEEK_CUR,
	wxFromEnd     = SEEK_END
};

// Binary file reader/writer on top of stdio.
class wxFile
{
public:
	wxFile() : m_fp(NULL) {}
	~wxFile() { Close(); }

	wxFile(const wxFile&) = delete;
	wxFile& operator=(const wxFile&) = delete;

	bool Open(const wxChar* szFileName);
	bool Create(const wxChar* szFileName, bool bOverwrite);
	void Close();
	bool IsOpened() const { return m_fp != NULL; }

	size_t Read(void* pBuf, size_t nCount);
	size_t Write(const void* pBuf, size_t nCount);

	long Seek(long nOffset, wxSeekMode mode = wxFromStart);
	long Tell() const;
	long Length() const;

protected:
	FILE* m_fp;
};

// Just enough of wxDateTime for the encoding-date written by XMLSaver.
// GetMonth() is 0-based (January == 0), matching wxDateTime.
class wxDateTime
{
public:
	wxDateTime() : m_nYear(1970), m_nMonth0(0), m_nDay(1) {}

	int GetYear() const { return m_nYear; }
	int GetMonth() const { return m_nMonth0; }
	int GetDay() const { return m_nDay; }

	int m_nYear;
	int m_nMonth0;
	int m_nDay;
};

class wxFileName
{
public:
	wxFileName(const wxChar* szPath) : m_strPath(szPath) {}

	wxDateTime GetModificationTime() const;

	// create an empty temporary file and return its full path
	static wxString CreateTempFileName(const wxChar* szPrefix);

protected:
	wxString m_strPath;
};

inline bool wxRemoveFile(const wxString& strFile)
{
	return remove(strFile.c_str()) == 0;
}

#endif // !defined(__WXSHIM_H__)
