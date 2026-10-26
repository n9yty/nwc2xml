// wxshim.cpp : implementation of the minimal wxWidgets stand-ins declared
// in wxshim.h. Standard C library only, plus iconv() for charsets other
// than the built-in CP1252.

#include "wxshim.h"

#include <iconv.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <vector>

//////////////////////////////////////////////////////////////////////
// wxString

wxString::wxString(const wxChar* psz, const wxMBConv& conv)
{
	if ( psz )
		conv.ToUTF8(psz, *this);
}

wxString wxString::Left(size_t nCount) const
{
	if ( nCount > m_str.size() )
		nCount = m_str.size();
	return wxString(m_str.substr(0, nCount));
}

wxString wxString::Right(size_t nCount) const
{
	if ( nCount > m_str.size() )
		nCount = m_str.size();
	return wxString(m_str.substr(m_str.size() - nCount));
}

wxString wxString::Mid(size_t nFirst) const
{
	if ( nFirst > m_str.size() )
		return wxString();
	return wxString(m_str.substr(nFirst));
}

size_t wxString::Replace(const wxChar* szOld, const wxChar* szNew)
{
	size_t nCount = 0;
	size_t nOldLen = szOld ? strlen(szOld) : 0;
	if ( nOldLen == 0 )
		return 0;

	const char* szWith = szNew ? szNew : "";
	size_t nNewLen = strlen(szWith);

	size_t nPos = 0;
	while ( (nPos = m_str.find(szOld, nPos)) != std::string::npos )
	{
		m_str.replace(nPos, nOldLen, szWith);
		nPos += nNewLen;
		nCount++;
	}

	return nCount;
}

static const char s_szTrimChars[] = " \t\r\n\v\f";

wxString& wxString::Trim(bool bFromRight)
{
	if ( bFromRight )
	{
		size_t n = m_str.find_last_not_of(s_szTrimChars);
		m_str.erase(n == std::string::npos ? 0 : n + 1);
	}
	else
	{
		size_t n = m_str.find_first_not_of(s_szTrimChars);
		m_str.erase(0, n == std::string::npos ? m_str.size() : n);
	}

	return *this;
}

void wxString::Printf(const wxChar* szFormat, ...)
{
	va_list ap, ap2;
	va_start(ap, szFormat);
	va_copy(ap2, ap);

	int nLen = vsnprintf(NULL, 0, szFormat, ap);
	va_end(ap);

	if ( nLen < 0 )
	{
		va_end(ap2);
		m_str.clear();
		return;
	}

	std::string result((size_t)nLen + 1, '\0');
	vsnprintf(&result[0], result.size(), szFormat, ap2);
	va_end(ap2);

	result.resize((size_t)nLen);
	m_str.swap(result);
}

//////////////////////////////////////////////////////////////////////
// charset conversion

wxUTF8Conv wxConvLocal;

static void AppendUTF8(std::string& out, unsigned long cp)
{
	if ( cp < 0x80 )
	{
		out += (char)cp;
	}
	else if ( cp < 0x800 )
	{
		out += (char)(0xC0 | (cp >> 6));
		out += (char)(0x80 | (cp & 0x3F));
	}
	else if ( cp < 0x10000 )
	{
		out += (char)(0xE0 | (cp >> 12));
		out += (char)(0x80 | ((cp >> 6) & 0x3F));
		out += (char)(0x80 | (cp & 0x3F));
	}
	else
	{
		out += (char)(0xF0 | (cp >> 18));
		out += (char)(0x80 | ((cp >> 12) & 0x3F));
		out += (char)(0x80 | ((cp >> 6) & 0x3F));
		out += (char)(0x80 | (cp & 0x3F));
	}
}

static bool IsValidUTF8(const std::string& str)
{
	const unsigned char* p = (const unsigned char*)str.c_str();
	size_t nLen = str.size();
	size_t i = 0;
	while ( i < nLen )
	{
		unsigned char c = p[i];
		if ( c < 0x80 )
		{
			i++;
			continue;
		}

		size_t n;
		unsigned long cp;
		if ( (c & 0xE0) == 0xC0 )      { n = 2; cp = c & 0x1F; }
		else if ( (c & 0xF0) == 0xE0 ) { n = 3; cp = c & 0x0F; }
		else if ( (c & 0xF8) == 0xF0 ) { n = 4; cp = c & 0x07; }
		else
			return false;

		if ( i + n > nLen )
			return false;

		for ( size_t j = 1; j < n; j++ )
		{
			if ( (p[i+j] & 0xC0) != 0x80 )
				return false;
			cp = (cp << 6) | (p[i+j] & 0x3F);
		}

		// reject overlong encodings, surrogates and out-of-range values
		if ( n == 2 && cp < 0x80 )
			return false;
		if ( n == 3 && cp < 0x800 )
			return false;
		if ( n == 4 && cp < 0x10000 )
			return false;
		if ( cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF) )
			return false;

		i += n;
	}

	return true;
}

bool wxUTF8Conv::ToUTF8(const std::string& in, wxString& out) const
{
	if ( !IsValidUTF8(in) )
	{
		out.Empty();
		return false;
	}

	out = wxString(in);
	return true;
}

// CP1252 0x80-0x9F mapped to Unicode; bytes Windows leaves undefined map
// to U+FFFD. 0xA0-0xFF match ISO 8859-1 and need no table.
static const unsigned short s_CP1252High[32] =
{
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178
};

wxCSConv::wxCSConv(const wxChar* szCharset)
	: m_strCharset(szCharset ? szCharset : "")
	, m_bOk(false)
	, m_bBuiltinCP1252(false)
{
	if ( strcasecmp(m_strCharset.c_str(), "CP1252") == 0 ||
	     strcasecmp(m_strCharset.c_str(), "WINDOWS-1252") == 0 )
	{
		m_bBuiltinCP1252 = true;
		m_bOk = true;
		return;
	}

	if ( !m_strCharset.empty() )
	{
		iconv_t cd = iconv_open("UTF-8", m_strCharset.c_str());
		if ( cd != (iconv_t)-1 )
		{
			m_bOk = true;
			iconv_close(cd);
		}
	}
}

bool wxCSConv::ToUTF8(const std::string& in, wxString& out) const
{
	out.Empty();
	if ( !m_bOk )
		return false;

	if ( m_bBuiltinCP1252 )
	{
		std::string result;
		result.reserve(in.size());
		for ( size_t i = 0; i < in.size(); i++ )
		{
			unsigned char b = (unsigned char)in[i];
			if ( b < 0x80 )
				result += (char)b;
			else if ( b < 0xA0 )
				AppendUTF8(result, s_CP1252High[b - 0x80]);
			else
				AppendUTF8(result, b);
		}

		out = wxString(result);
		return true;
	}

	iconv_t cd = iconv_open("UTF-8", m_strCharset.c_str());
	if ( cd == (iconv_t)-1 )
		return false;

	std::vector<char> inbuf(in.begin(), in.end());
	char* pIn = inbuf.empty() ? NULL : &inbuf[0];
	size_t nInLeft = inbuf.size();

	std::string result;
	char buf[1024];
	bool bOk = true;
	while ( nInLeft )
	{
		char* pOut = buf;
		size_t nOutLeft = sizeof(buf);
		size_t rc = iconv(cd, &pIn, &nInLeft, &pOut, &nOutLeft);
		result.append(buf, sizeof(buf) - nOutLeft);
		if ( rc == (size_t)-1 )
		{
			if ( errno == E2BIG )
				continue;	// output buffer full, go round again

			bOk = false;	// invalid or incomplete input sequence
			break;
		}
	}

	if ( bOk )
	{
		// flush any shift-state the conversion may hold
		char* pOut = buf;
		size_t nOutLeft = sizeof(buf);
		if ( iconv(cd, NULL, NULL, &pOut, &nOutLeft) != (size_t)-1 )
			result.append(buf, sizeof(buf) - nOutLeft);
	}

	iconv_close(cd);

	if ( !bOk )
		return false;

	out = wxString(result);
	return true;
}

//////////////////////////////////////////////////////////////////////
// wxFile

bool wxFile::Open(const wxChar* szFileName)
{
	Close();
	m_fp = fopen(szFileName, "rb");
	return m_fp != NULL;
}

bool wxFile::Create(const wxChar* szFileName, bool /*bOverwrite*/)
{
	Close();
	m_fp = fopen(szFileName, "wb");
	return m_fp != NULL;
}

void wxFile::Close()
{
	if ( m_fp )
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

size_t wxFile::Read(void* pBuf, size_t nCount)
{
	return m_fp ? fread(pBuf, 1, nCount, m_fp) : 0;
}

size_t wxFile::Write(const void* pBuf, size_t nCount)
{
	return m_fp ? fwrite(pBuf, 1, nCount, m_fp) : 0;
}

long wxFile::Seek(long nOffset, wxSeekMode mode)
{
	if ( m_fp == NULL || fseek(m_fp, nOffset, (int)mode) != 0 )
		return -1;
	return ftell(m_fp);
}

long wxFile::Tell() const
{
	return m_fp ? ftell(m_fp) : -1;
}

long wxFile::Length() const
{
	if ( m_fp == NULL )
		return -1;

	struct stat st;
	if ( fstat(fileno(m_fp), &st) != 0 )
		return -1;

	return (long)st.st_size;
}

//////////////////////////////////////////////////////////////////////
// wxFileName

wxDateTime wxFileName::GetModificationTime() const
{
	wxDateTime dt;

	struct stat st;
	if ( stat(m_strPath.c_str(), &st) == 0 )
	{
		struct tm tmv;
		if ( localtime_r(&st.st_mtime, &tmv) != NULL )
		{
			dt.m_nYear = tmv.tm_year + 1900;
			dt.m_nMonth0 = tmv.tm_mon;
			dt.m_nDay = tmv.tm_mday;
		}
	}

	return dt;
}

wxString wxFileName::CreateTempFileName(const wxChar* szPrefix)
{
	const char* szTmpDir = getenv("TMPDIR");
	if ( szTmpDir == NULL || *szTmpDir == '\0' )
		szTmpDir = "/tmp";

	std::string strTemplate(szTmpDir);
	if ( strTemplate[strTemplate.size()-1] != '/' )
		strTemplate += '/';
	if ( szPrefix )
		strTemplate += szPrefix;
	strTemplate += "XXXXXX";

	std::vector<char> buf(strTemplate.begin(), strTemplate.end());
	buf.push_back('\0');

	int fd = mkstemp(&buf[0]);
	if ( fd == -1 )
		return wxString();

	close(fd);
	return wxString(&buf[0]);
}
