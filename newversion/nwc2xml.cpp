// nwc2xml.cpp : Defines the entry point for the console application.
//

#include "Precompile.h"
#include "nwcfile.h"
#include "XMLSaver.h"
#include "nwc2xml-version.h"

DWORD ConvertNWC2MusicXML(const wxString& strNWC, bool bGenerateLog)
{
	wxFile in;
	if ( in.Open(strNWC) == FALSE )
		return (DWORD)wxSysErrorCode();

	wxString strFile;
	if ( bGenerateLog )
	{
		strFile = strNWC.Left(strNWC.Length()-3);
		strFile += _T("txt");
	}
	else
	{
#ifdef	_WIN32
		strFile = _T("NUL");
#else
		strFile = _T("/dev/null");
#endif
	}

	FILE* out = fopen(strFile.c_str(), "w");
	if ( out == NULL )
	{
		fprintf(stderr, "%s : conversion failed.\n", strNWC.c_str());
		return ERROR_GEN_FAILURE;
	}

	CNWCFile nwc;
	DWORD dwResult = nwc.Load(in, out);

	fclose(out);

	if ( dwResult != ERROR_SUCCESS )
	{
		fprintf(stderr, "%s : conversion failed.\n", strNWC.c_str());
		if ( bGenerateLog )
			wxRemoveFile(strFile);
		return dwResult;
	}

	strFile = strNWC.Left(strNWC.Length()-3);
	strFile += _T("xml");

	XMLSaver saver;
	saver.Save(strNWC, strFile, &nwc);

	fprintf(stderr, "%s : conversion saved to %s.\n", strNWC.c_str(), strFile.c_str());

	return dwResult;
}

static const wxChar* s_ProgramLogo = _T(PROGRAM_ABOUT);

static void PrintUsage()
{
	fprintf(stderr, "%s\n", s_ProgramLogo);
	fprintf(stderr,
		"Usage: nwc2xml [-h] [-l] [--charset <charset>] <file.nwc>...\n"
		"  -h, --help            show this help message\n"
		"  -l, --log             generate log file\n"
		"  --charset <charset>   conversion charset for lyric; default is UTF-8\n"
		"                        with a per-string CP1252 fallback\n");
}

int main(int argc, char* argv[])
{
	bool bGenerateLog = false;
	wxString strCharset;
	TVector<wxString> strFiles;

	for ( int i=1 ; i<argc ; i++ )
	{
		const char* szArg = argv[i];
		if ( strcmp(szArg, "-h") == 0 || strcmp(szArg, "--help") == 0 )
		{
			PrintUsage();
			return 0;
		}
		else if ( strcmp(szArg, "-l") == 0 || strcmp(szArg, "--log") == 0 )
		{
			bGenerateLog = true;
		}
		else if ( strcmp(szArg, "--charset") == 0 )
		{
			if ( ++i >= argc )
			{
				fprintf(stderr, "option --charset requires an argument.\n");
				return -1;
			}
			strCharset = argv[i];
		}
		else if ( strncmp(szArg, "--charset=", 10) == 0 )
		{
			strCharset = szArg + 10;
		}
		else if ( szArg[0] == '-' && szArg[1] != '\0' )
		{
			fprintf(stderr, "unknown option '%s'.\n", szArg);
			PrintUsage();
			return -1;
		}
		else
		{
			strFiles.Add(wxString(szArg));
		}
	}

	if ( strFiles.GetCount() == 0 )
	{
		PrintUsage();
		return -1;
	}

	// Strings are converted with g_pMBConv first; if that fails for a
	// string, nwcobj.cpp falls back to CP1252 (the charset NWC uses).
	g_pMBConv = &wxConvLocal;
	wxCSConv* pCSConv = NULL;
	if ( strCharset.Length() )
	{
		pCSConv = new wxCSConv(strCharset);
		if ( !pCSConv->IsOk() )
		{
			fprintf(stderr, "unknown charset=%s.\n", strCharset.c_str());
			delete pCSConv;
			pCSConv = NULL;
		}
		else
		{
			g_pMBConv = pCSConv;
		}
	}

	for ( size_t i=0 ; i<strFiles.GetCount() ; i++ )
	{
		const wxString& strFile = strFiles[i];
		wxString strExt = strFile.Right(4);
		if ( strExt.CmpNoCase(_T(".nwc")) == 0 )
			ConvertNWC2MusicXML(strFile, bGenerateLog);
	}

	delete pCSConv;

	return 0;
}
