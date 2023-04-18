#ifndef _TOOLKIT_H_
#define _TOOLKIT_H_
#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cassert>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

typedef std::basic_ostream<TCHAR,std::char_traits<TCHAR> >		OStream;

inline SYSTEMTIME operator+(const SYSTEMTIME& tm, long nMSec)
{
	assert(nMSec >= 0);
	SYSTEMTIME rTime;
	memcpy(&rTime, &tm, sizeof SYSTEMTIME);
	nMSec += rTime.wMilliseconds;
	rTime.wMilliseconds = 0;
	COleDateTime time(rTime);
	time += COleDateTimeSpan(0, 0, 0, nMSec/1000);
	time.GetAsSystemTime(rTime);
	rTime.wMilliseconds = WORD(nMSec%1000);
	return rTime;
}

inline SYSTEMTIME operator+(long nMSec, const SYSTEMTIME& tm)
{
	return (tm+nMSec);
}

inline SYSTEMTIME operator-(const SYSTEMTIME& tm, long nMSec)
{
	assert(nMSec >= 0);
	SYSTEMTIME rTime;
	memcpy(&rTime, &tm, sizeof SYSTEMTIME);

	if(nMSec <= rTime.wMilliseconds)
	{
		rTime.wMilliseconds -= WORD(nMSec);
	}
	else
	{
		nMSec -= rTime.wMilliseconds;
		rTime.wMilliseconds = 0;
		COleDateTime time(rTime);
		long nSec = nMSec/1000;
		WORD nTemp = WORD(nMSec%1000);
		if(nTemp > 0) nSec++;
		time -= COleDateTimeSpan(0, 0, 0, nSec);
		time.GetAsSystemTime(rTime);
		if(nTemp > 0) rTime.wMilliseconds = 1000 - nTemp;
	}

	return rTime;
}

inline bool operator<(const SYSTEMTIME& tm1, const SYSTEMTIME& tm2)
{
	if(tm1.wYear > tm2.wYear) return false;
	if(tm1.wMonth > tm2.wMonth) return false;
	if(tm1.wDay > tm2.wDay) return false;
	if(tm1.wHour > tm2.wHour) return false;
	if(tm1.wMinute > tm2.wMinute) return false;
	if(tm1.wSecond > tm2.wSecond) return false;
	return (tm1.wMilliseconds < tm2.wMilliseconds);
}

inline bool operator==(const SYSTEMTIME& tm1, const SYSTEMTIME& tm2)
{
	return (tm1.wYear == tm2.wYear && tm1.wMonth == tm2.wMonth
		&& tm1.wDay == tm2.wDay && tm1.wHour == tm2.wHour
		&& tm1.wMinute == tm2.wMinute && tm1.wSecond == tm2.wSecond
		&& tm1.wMilliseconds == tm2.wMilliseconds);
}

inline bool operator<=(const SYSTEMTIME& tm1, const SYSTEMTIME& tm2)
{
	if(tm1.wYear > tm2.wYear) return false;
	if(tm1.wMonth > tm2.wMonth) return false;
	if(tm1.wDay > tm2.wDay) return false;
	if(tm1.wHour > tm2.wHour) return false;
	if(tm1.wMinute > tm2.wMinute) return false;
	if(tm1.wSecond > tm2.wSecond) return false;
	return (tm1.wMilliseconds <= tm2.wMilliseconds);
}

inline bool operator>(const SYSTEMTIME& tm1, const SYSTEMTIME& tm2)
{
	return (tm2 <= tm1);
}

inline bool operator>=(const SYSTEMTIME& tm1, const SYSTEMTIME& tm2)
{
	return (tm2 < tm1);
}

inline BOOL GetAppFolder(LPTSTR szFolder, int nBufSize)
{
	TCHAR szPath[MAX_PATH];
	::GetModuleFileName(AfxGetInstanceHandle(), szPath, MAX_PATH);
	size_t nLen = lstrlen(szPath);
	for(size_t i=nLen-1; i>=0; i--)
	{
		if(szPath[i] == _T('\\'))
		{
			szPath[i] = 0;
			break;
		}
	}
	if(lstrlen(szPath) >= nBufSize) return FALSE;
	lstrcpy(szFolder, szPath);
	return TRUE;
}

inline CString GetAppFolder(void)
{
	TCHAR exeFullName[MAX_PATH];
	::GetModuleFileName(AfxGetInstanceHandle(),exeFullName,MAX_PATH);
	CString mPath=exeFullName;

	int mPos=mPath.ReverseFind('\\'); 

	if(mPos==-1)return CString(_T(""));	//没有发现

	mPath=mPath.Left(mPos);

	if(mPath.Right(1)==_T("\\"))
	{
		mPath=mPath.Left(mPos-1);
	}

	return mPath;
}

inline CString GetStrFromDateTime(const COleDateTime& tm)
{
	CString strFormat; 
	strFormat.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), 
		tm.GetYear(), tm.GetMonth(), tm.GetDay(),
		tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	return strFormat;
}

inline COleDateTime GetDateTimeFromStr(CString& strTime)
{
	strTime.Replace(_T("T"), _T(" "));
	int nYear = _ttoi(strTime.Left(4).GetBuffer());
	int nMonth = _ttoi(strTime.Mid(5, 2).GetBuffer());
	int nDay = _ttoi(strTime.Mid(8, 2).GetBuffer());
	int nHour = _ttoi(strTime.Mid(11, 2).GetBuffer());
	int nMin = _ttoi(strTime.Mid(14, 2).GetBuffer());
	int nSec = _ttoi(strTime.Right(2).GetBuffer());
	return COleDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
}

inline COleDateTime GetDateFromStr(CString& strTime)
{
	int nHour = 0, nMin = 0, nSec = 0;
	int nYear = _ttoi(strTime.Left(4).GetBuffer());
	int nMonth = _ttoi(strTime.Mid(5, 2).GetBuffer());
	int nDay = _ttoi(strTime.Mid(8, 2).GetBuffer());
	return COleDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
}

inline CString Length2HMSF(int nLength,double dfps)
{
	ASSERT( nLength >= 0 );
	int HH,MM,SS,FF;
	HH = 0;
	while(nLength >= 60*60*dfps)
	{
		nLength -= 60*60*dfps;
		HH++;
	}
	MM = 0;
	while(nLength >= 60*dfps)
	{
		nLength -= 60*dfps;
		MM++;
	}
	SS = 0;
	while(nLength >= dfps)
	{
		nLength -= dfps;
		SS++;
	}
	FF = nLength; 
	CString str;
	str.Format(_T("%02d:%02d:%02d:%02d"),HH,MM,SS,FF);
	
	return str;
}

inline BOOL GetPCNameAndIP(CString& strName, CString& strIP)
{
	WORD   wVersionRequested;   
	WSADATA   wsaData;   
	wVersionRequested = MAKEWORD(2,0);
	WSAStartup(wVersionRequested,&wsaData);
	char buf[256];
	int ret = gethostname(buf,256);
	if(ret == SOCKET_ERROR) return FALSE;
	USES_CONVERSION;
	PHOSTENT phe=gethostbyname(buf);
	if(phe)
	{
		IN_ADDR adr;
		adr.S_un.S_addr = *((DWORD *)phe->h_addr);
		char* pBuf=inet_ntoa(adr);
		strIP=A2W(pBuf);
	}
	WSACleanup();   
	strName = buf;
	return TRUE;
}

//inline ILoger* CreateLoger(void)
//{
//	ILoger* pLoger = NULL;
//	TCHAR szPath[MAX_PATH];
//	GetAppFolder(szPath, MAX_PATH);
//	lstrcat(szPath, _T("\\WriteLog.dll"));
//	HMODULE hModule = LoadLibrary(szPath);
//	if(hModule != NULL)
//	{
//		ILoger* (*CreateLoger)(void);
//		(FARPROC&)CreateLoger = GetProcAddress(hModule, "CreateLoger");
//		if(CreateLoger != NULL)
//		{
//			pLoger = CreateLoger();
//		}
//	}
//	return pLoger;
//}

inline void AbortApp(LPCTSTR szText, LPCTSTR szTitle)
{
	TCHAR szAppName[MAX_PATH];
	::GetModuleFileName(NULL, szAppName, MAX_PATH);

	CString strTitle;
	size_t nLen = lstrlen(szAppName);
	for(size_t i=nLen-1; i>=0; i--)
	{
		if(szAppName[i] == _T('\\'))
		{
			strTitle = &szAppName[i+1];
			break;
		}
	}

	strTitle += _T(" - ");
	strTitle += szTitle;

	::MessageBox(NULL, szText, strTitle, MB_ICONHAND|MB_ICONSTOP|MB_ICONERROR);
	exit(EXIT_FAILURE);
}

inline BOOL WriteRegString(HKEY hRoot, LPCTSTR szPath, LPCTSTR lpValueName, LPCTSTR lpString)
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	LONG ret = RegCreateKeyEx(hRoot, szPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if(ret == ERROR_SUCCESS)
	{
		ret = RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (LPBYTE)lpString, (lstrlen(lpString)+1)*sizeof(TCHAR));
		bRet = (ret == ERROR_SUCCESS);
		RegCloseKey(hKey);
	}
	return bRet;
}

inline BOOL WriteRegBinary(HKEY hRoot, LPCTSTR szPath, LPCTSTR lpValueName, LPBYTE lpBinary, int nSize)
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	LONG ret = RegCreateKeyEx(hRoot, szPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if(ret == ERROR_SUCCESS)
	{
		ret = RegSetValueEx(hKey, lpValueName, 0, REG_BINARY, (LPBYTE)lpBinary, nSize);
		bRet = (ret == ERROR_SUCCESS);
		RegCloseKey(hKey);
	}
	return bRet;
}

inline CString GetRegString(HKEY hRoot, LPCTSTR szPath, LPCTSTR lpValueName, LPCTSTR lpszDefault = NULL)
{
	CString strRet;
	if(lpszDefault != NULL) strRet = lpszDefault;
	HKEY hKey = NULL;
	LONG ret = RegOpenKeyEx(hRoot, szPath, 0, KEY_READ, &hKey);
	if(ret == ERROR_SUCCESS)
	{
		DWORD dwType, cbData=0;
		BYTE *pBuf, szBuf[2];
		ret = RegQueryValueEx(hKey, lpValueName, 0, &dwType, (LPBYTE)szBuf, &cbData);
		if(ret == ERROR_MORE_DATA && dwType == REG_SZ)
		{
			pBuf = new BYTE[cbData];
			ret = RegQueryValueEx(hKey, lpValueName, 0, &dwType, pBuf, &cbData);
			if(ret == ERROR_SUCCESS)
			{
				strRet = (LPCTSTR)pBuf;
			}
			delete [] pBuf;
		}
		RegCloseKey(hKey);
	}
	return strRet;
}

inline BOOL GetRegBinary(HKEY hRoot, LPCTSTR szPath, LPCTSTR lpValueName, LPBYTE lpData, LPDWORD lpcbData)
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
	LONG ret = RegOpenKeyEx(hRoot, szPath, 0, KEY_READ, &hKey);
	if(ret == ERROR_SUCCESS)
	{
		DWORD dwType;
		ret = RegQueryValueEx(hKey, lpValueName, 0, &dwType, lpData, lpcbData);
		if(ret == ERROR_SUCCESS && dwType == REG_BINARY)
		{
			bRet = TRUE;
		}
		RegCloseKey(hKey);
	}
	return bRet;
}

inline FARPROC LoadFunction(LPCTSTR szDll, LPCSTR szFunc)
{
	TCHAR szPath[MAX_PATH];
	GetAppFolder(szPath, sizeof szPath);
	lstrcat(szPath, _T("\\"));
	lstrcat(szPath, szDll);

	HMODULE hModule = ::LoadLibrary(szPath);
	if(hModule == NULL) return NULL;

	return GetProcAddress(hModule, szFunc);
}

inline COleDateTime Second2DateTime(long nTSec)
{
	int nSec = nTSec%60;
	nTSec /= 60;
	int nMin = nTSec%60;
	nTSec /= 60;
	int nHour = nTSec;
	return COleDateTime(0, 0, 0, nHour, nMin, nSec);
}

template <class T>
CArchive& operator<<(CArchive& ar, const std::vector<T>& rtn)
{
	int nSize;
	nSize = rtn.size();
	ar << nSize;
	for(int i=0;i<nSize;i++)
		ar << rtn[i];
	return ar;
}
template <class T>
CArchive& operator>>(CArchive& ar, std::vector<T>& rtn)
{
	int nSize; ar >> nSize;
	for(int i=0; i<nSize; i++)
	{
		rtn.push_back(T());
		ar >> rtn.back();
	}
	return ar;
}

////////////////////////////////////////////////奥运




#endif