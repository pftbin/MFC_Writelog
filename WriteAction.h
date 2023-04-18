#pragma once
#include "snmp_it.h"//zengkairong2008/05/29
#include "EncryptionLog.h"
#include "Shlwapi.h"

class CWriteAction
{
public:
	CWriteAction(void);
	~CWriteAction(void);

public:
	BOOL WriteSnmp(int nAction,LPVOID pData);
	void SetLogParam(CString strPath,CString strName);
	void WriteLog(CString strInfo,int nType,BOOL bAsync=FALSE,BOOL bSnmpLog = FALSE);
	void WriteLogInfo(LOGINFO loginfo);
	void AddMsg(CString strMsg, DWORD nType);
	HANDLE GetOpenFileHandle();
	void SetOpenFileHandle(HANDLE hFile);
	void ThreadProc();
	void Initialize();
	CString GetMsg(DWORD& dwType);
	void AddMsg(CStringArray *pMsgArr, CDWordArray* pdwArray);
	void WriteToFile(CString strMsg, DWORD dwType);
	
	void DeleteLogFile();

//	HANDLE m_hJMFile;	
protected:
	CStringArray  m_MsgArray;
	CDWordArray   m_MsgType;

	BOOL		  m_bLocal;
	int			  m_bAsync;
	int			  m_nMaxByte;
	int			  m_nDays;
	CString		  m_strLogName;
	HANDLE        m_hThreadProc;
protected:
	BOOL WriteNMLog(LOGINFO loginfo);

	CString GetTargetPath();
	void    GetConfigParams();

	BOOL    LocationCurrentDir(CString strFile);

	HANDLE m_hGetEvent;
	HANDLE m_hFileHandle;
	CRITICAL_SECTION m_CritilSec;
	CRITICAL_SECTION  m_CritilSecFile;
	BOOL m_bThreadExit;
	CString m_strPath;
	CString m_strName;
	DWORD   m_dwThreadID;
	//CString m_strStatusFileName;
	CString m_strDay;
	static HINSTANCE m_hSnmpDll;
	HANDLE m_hDelFileThread;

	CEncryptLog   m_SubLog;
};

class CAutoLock
{
public:
CAutoLock(CRITICAL_SECTION &lock)
:m_lock(lock)
	{
		EnterCriticalSection(&m_lock);
	}
		  ~CAutoLock()
		  {
			LeaveCriticalSection(&m_lock);
		  }
protected:
	CRITICAL_SECTION &m_lock;
};