// WriteMsgToFile.h: interface for the WriteMsgToFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WRITEMSGTOFILE_H__169710FA_C9F5_41FB_973D_6A84CDD2AB6D__INCLUDED_)
#define AFX_WRITEMSGTOFILE_H__169710FA_C9F5_41FB_973D_6A84CDD2AB6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000 

#ifdef WRITELOG_EXPORTS
#define WRITELOG_CLASS __declspec(dllexport)
#else
#define WRITELOG_CLASS __declspec(dllimport)
#endif

#define LogLevel_FatefulError 1
#define LogLevel_Error        2
#define LogLevel_Warring      3
#define LogLevel_Info         4
#define LogLevel_Debug        5
#define LogLevel_Memory       6
#define LogLevel_Operation    7

#define Snmp_PlayoutInfo	1
#define Snmp_PlayoutDevice	2	 
#define Snmp_PlayoutUser	3
#define Snmp_PlayoutService 4

#define PlayoutInfoServerStatus_Unknown		0
#define PlayoutInfoServerStatus_Offline		1
#define PlayoutInfoServerStatus_Online		2
#define PlayoutInfoServerStatus_Error		3

#define PlayoutInfoServiceStatus_Unknown	0
#define PlayoutInfoServiceStatus_Offline	1
#define PlayoutInfoServiceStatus_Online		2
#define PlayoutInfoServiceStatus_Error		3

#define PlayoutInfoSystemStatus_Stop		1
#define PlayoutInfoSystemStatus_Play		2

#define PlayoutDeviceType_Vtr				1
#define PlayoutDeviceType_Videoserver		2
#define PlayoutDeviceType_Msv				3
#define PlayoutDeviceType_Storage			4
#define PlayoutDeviceType_Matrix			5
#define PlayoutDeviceType_Gpi				6
#define PlayoutDeviceType_Tally				7
#define PlayoutDeviceType_Xdcam				8
#define PlayoutDeviceType_XdcamCart 		9
#define PlayoutDeviceType_TapeLibrary		10
#define PlayoutDeviceType_Others			11

#define PlayoutDeviceStatus_Unknown			0
#define PlayoutDeviceStatus_Offline			1
#define PlayoutDeviceStatus_Online			2
#define PlayoutDeviceStatus_Error			3

inline void GetReplaceString(BOOL b,CString &s1,CString &s2)
{
	if(b)
	{
		s1 = _T("'");
		s2 = _T("''");
	}
	else
	{
		s1 = _T("''");
		s2 = _T("'");
	}
}


typedef struct tagPlayoutInfo
{
	int nProdId;
	int nStudioId;
	CString strStudioName;
	int nServerStatus;
	int nSystemStatus;
	tagPlayoutInfo()
	{
		nProdId = 0;
		nStudioId = 0;
		strStudioName = _T("");
		nServerStatus = PlayoutInfoServerStatus_Online;
		nSystemStatus = PlayoutInfoSystemStatus_Stop;
	}
	void UpdateString(BOOL b = TRUE)
	{
		CString s1,s2;
		GetReplaceString(b,s1,s2);
		strStudioName.Replace(s1, s2);
	}
}PlayoutInfo;

typedef struct tagPlayoutDevice
{
	int nProdId;
	int nDeviceId;
	CString strDeviceName;
	int nDeviceType;
	int nDeviceStatus;
	CString strDeviceInfo;
	tagPlayoutDevice()
	{
		nProdId = 0;
		nDeviceId = 0;
		strDeviceName = L"";
		nDeviceType = PlayoutDeviceType_Others;
		nDeviceStatus = PlayoutDeviceStatus_Online; 
		strDeviceInfo = L"playout";
	}
	void UpdateString(BOOL b = TRUE)
	{
		CString s1,s2;
		GetReplaceString(b,s1,s2);
		strDeviceName.Replace(s1, s2);
		strDeviceInfo.Replace(s1, s2);
	}
}PlayoutDevice;

typedef struct tagPlayoutUser
{
	int nProdId;
	int nUserId;
	CString strUserName;
	COleDateTime dtLoginTime;
	COleDateTime dtLogoutTime;
	tagPlayoutUser()
	{
		nProdId = 0;
		nUserId = 0;
		strUserName = L"";
	}
	void UpdateString(BOOL b = TRUE)
	{
		CString s1,s2;
		GetReplaceString(b,s1,s2);
		strUserName.Replace(s1, s2);
	}
}PlayoutUser;

typedef struct tagPlayoutServiceInfo
{
	int nProdId;
	int nServiceId;
	CString strServiceName;
	int nServiceStatus;
	CString strServiceInfo;
	tagPlayoutServiceInfo()
	{
		nProdId = 0;
		nServiceId = 0;
		strServiceName = _T("");
		nServiceStatus = PlayoutInfoServiceStatus_Online;
		strServiceInfo = _T("");
	}
	void UpdateString(BOOL b = TRUE)
	{
		CString s1,s2;
		GetReplaceString(b,s1,s2);
		strServiceName.Replace(s1, s2);
		strServiceInfo.Replace(s1,s2);
	}
}PlayoutServiceInfo;

typedef struct _LOGINFO //»’÷æ
{
	CString strInfo;
	int nLogLevel;
	int nErrorCode;
	BOOL bAsync;
	BOOL bSnmpLog;
	_LOGINFO()
	{
		nLogLevel=0;
		nErrorCode=0;
		bAsync=FALSE;
		bSnmpLog=TRUE;
	}
}LOGINFO,*PLOGINFO;

class WRITELOG_CLASS CWriteMsgToFile  
{
public:
	BOOL WriteSnmp(int nAction,LPVOID pData);
	void SetLogParam(CString strPath,CString strName);
	void WriteLog(CString strInfo,int nType,BOOL bAsync=FALSE,BOOL bSnmpLog = FALSE);
	void WriteLogInfo(LOGINFO loginfo);
	void Initialize();
	CWriteMsgToFile();
	virtual ~CWriteMsgToFile();

public:
//	void WriteToFile(CString strMsg, DWORD dwType);
//	CString GetMsg(DWORD& dwType);
//	void AddMsg(CStringArray *pMsgArr, CDWordArray* pdwArray);
//	void AddMsg(CString strMsg, DWORD nType);
//	HANDLE GetOpenFileHandle();
//	void SetOpenFileHandle(HANDLE hFile);
//	void ThreadProc();

//	HANDLE m_hJMFile;	
protected:
	class CWriteAction * m_pLogWriter;
};

inline BOOL WriteLog_in(CWriteMsgToFile& loger, DWORD dwType, BOOL bAsync, LPCTSTR lpszFormat, ...)
{
	CString strLog;
	va_list argList;
	va_start(argList, lpszFormat);
	strLog.FormatV(lpszFormat, argList);
	va_end(argList);
	loger.WriteLog(strLog, dwType, bAsync);
	return FALSE;
}

inline void WriteLogInfo_in(CWriteMsgToFile& loger,int nLevel,int nErrorCode,LPCTSTR lpszFormat, ...)
{
	CString strLog;
	va_list argList;
	va_start(argList, lpszFormat);
	strLog.FormatV(lpszFormat, argList);
	va_end(argList);
	LOGINFO loginfo;
	loginfo.nErrorCode=nErrorCode;
	loginfo.nLogLevel=nLevel;
	loginfo.strInfo=strLog;
	loger.WriteLogInfo(loginfo);
}

#define DECLARE_LOGER(loger) \
	extern CWriteMsgToFile loger;

#define IMPLEMENT_LOGER(loger) \
	CWriteMsgToFile loger;

#define INIT_LOGER(loger, folder, name) \
	loger.SetLogParam(folder, name);

#define WRITE_LOG	WriteLog_in

#define WRITE_LOGINFO WriteLogInfo_in

#endif // !defined(AFX_WRITEMSGTOFILE_H__169710FA_C9F5_41FB_973D_6A84CDD2AB6D__INCLUDED_)
