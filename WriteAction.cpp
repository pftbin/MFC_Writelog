
// WriteAction.cpp: implementation of the WriteMsgToFile class.

#include "stdafx.h"
#include ".\writeaction.h"
#include <atlconv.h> 
#include "MibInterface.h"
#include "toolkit.h"

#define  AOTULOCKFILE   CAutoLock autolock(m_CritilSecFile)


DWORD WINAPI DeleteLogFileThread(LPVOID lpVoid)
{
	static BOOL bRun = FALSE;
	if(bRun) return 0;    //保证只有一个线程在运行
	bRun = TRUE;
//下面对过期日志进行删除

	try
	{
	CWriteAction * pAction = (CWriteAction *)lpVoid;
	if(pAction) pAction->DeleteLogFile();
	}
	catch(...){};

//删除完毕
	bRun = FALSE;	
	return 0;
}

CString GGetAppPath()
{
	TCHAR exeFullName[MAX_PATH];
	::GetModuleFileName(NULL,exeFullName,MAX_PATH);
	CString mPath=exeFullName;

	int mPos=mPath.ReverseFind('\\'); 

	if(mPos==-1)return "";	//没有发现

	mPath=mPath.Left(mPos);

	if(mPath.Right(1)==_T("\\"))
	{
		mPath=mPath.Left(mPos-1);
	}

	return mPath;
}

HINSTANCE		m_hInstance;
HINSTANCE      CWriteAction::m_hSnmpDll=0;
CWriteAction::CWriteAction(void)
{
	m_hGetEvent=CreateEvent(NULL,0,0,NULL);
	InitializeCriticalSection(&m_CritilSec);
	InitializeCriticalSection(&m_CritilSecFile);
	m_bThreadExit=FALSE;
	m_hFileHandle = NULL;
	m_hInstance = NULL;
	m_hDelFileThread = NULL;
	m_strLogName	 = _T("");
	m_dwThreadID = 0;

	m_bLocal = TRUE;
	m_nMaxByte = 20;

	//m_strPath = GetTargetPath();//Studio Log Path
	m_strPath.Empty();
	GetConfigParams();
	m_hThreadProc = NULL;

	CString strPath;
	strPath.Format(_T("%s//NewInterfaceSvc.ini"), GGetAppPath());
	m_bAsync = ::GetPrivateProfileInt(_T("LOG"), _T("Lag"), 1, strPath);
}

CWriteAction::~CWriteAction(void)
{
	if(m_bThreadExit)
	{
		m_bThreadExit = FALSE;
		SetEvent(m_hGetEvent);
		Sleep(100);
	}
	m_bThreadExit = TRUE;
	if(m_hDelFileThread)
	{
		if(WaitForSingleObject(m_hDelFileThread,600) != WAIT_OBJECT_0)
			TerminateThread(m_hDelFileThread,0);
		m_hDelFileThread = NULL;
	}
	CloseHandle(m_hGetEvent);
	
	if(m_hThreadProc)
	{
		if(WaitForSingleObject(m_hThreadProc,600) != WAIT_OBJECT_0)
			TerminateThread(m_hThreadProc,0);
	}

	{
		AOTULOCKFILE;
		if(m_hFileHandle) CloseHandle(m_hFileHandle);
		m_SubLog.ReCloseFile();
	}
	DeleteCriticalSection(&m_CritilSec);
	DeleteCriticalSection(&m_CritilSecFile);
	if (m_hInstance != NULL)
		FreeLibrary(m_hInstance);

	if (m_hSnmpDll!=NULL)
	{
		FreeLibrary(m_hSnmpDll);
	}
}

enum NMLogLevel{logLevelFatefulError = 1, logLevelError,logLevelWarring,
logLevelInfo,logLevelDebug,logLevelMemory};


DWORD WINAPI WriteThread(LPVOID lParam)
{
	CWriteAction *pThreadProc = (CWriteAction*)lParam;
	pThreadProc->ThreadProc();
	return 0;
}

void CWriteAction::AddMsg(CStringArray *pMsgArr, CDWordArray* pdwArray)
{
	EnterCriticalSection(&m_CritilSec);
	for(int i=0; i<pMsgArr->GetSize(); i++)
	{
		m_MsgArray.Add(pMsgArr->GetAt(i));
		m_MsgType.Add(pdwArray->GetAt(i));
	}
	LeaveCriticalSection(&m_CritilSec);
	SetEvent(m_hGetEvent);
}

CString CWriteAction::GetMsg(DWORD& dwType)
{
	EnterCriticalSection(&m_CritilSec);
	CString strMsg=_T("");
	if(m_MsgArray.GetSize()>0)
	{
		strMsg = m_MsgArray.GetAt(0);
		m_MsgArray.RemoveAt(0);
		dwType = m_MsgType.GetAt(0);
		m_MsgType.RemoveAt(0);
	}
	LeaveCriticalSection(&m_CritilSec);
	return strMsg;
}

void CWriteAction::Initialize()
{
	if(m_hThreadProc == NULL)
	{
		m_bThreadExit = FALSE;
		m_hThreadProc = CreateThread(NULL,0,WriteThread,this,CREATE_SUSPENDED,&m_dwThreadID);
	}
	SetThreadPriority(m_hThreadProc, THREAD_PRIORITY_BELOW_NORMAL);
	ResumeThread(m_hThreadProc);
	CString strPath=GGetAppPath();
	strPath+=_T("\\");
	strPath+=_T("MsgHintWrapper.dll");
//	m_hInstance = LoadLibrary(strPath);
}

void CWriteAction::ThreadProc()
{
	int nDelay=INFINITE;

	DWORD dwType;
	while(!m_bThreadExit)
	{
//		WaitForSingleObject(m_hGetEvent,nDelay);
		CString strMsg = GetMsg(dwType);
		if( strMsg.IsEmpty() )
		{
			Sleep(10);
			continue;
		}
		Sleep(10);
		WriteToFile(strMsg, dwType);
	}
}

char* ConvertLPWSTRToLPSTR (LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen (lpwszStrIn);

		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		pszOut = new char [nOutputStrLen];

		if (pszOut)
		{
			memset (pszOut, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}
	return pszOut;
}

void CWriteAction::WriteToFile(CString strMsg, DWORD dwType)
{
	if (m_hInstance != NULL)
	{
		typedef void (WINAPI *SENDMESSAGEHINT)(LPCTSTR srcName, LPCTSTR time, UINT nType, int level, LPCTSTR szDescribe);
		SENDMESSAGEHINT SendMessageHint;
		SendMessageHint = (SENDMESSAGEHINT)GetProcAddress(m_hInstance, "SendMessageHint");
		if (SendMessageHint)
		{
			CTime t = CTime::GetCurrentTime();
			CString strTime;
			strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
			SendMessageHint(strMsg.GetBuffer(0), strTime.GetBuffer(0), dwType, 0, _T(""));
		}
	}

//{{ 解决日志量大的问题
	//int nMaxSize = MAX_LOGLINE;
	//CString strInfo;
	//CString strTempFile;      // = m_strStatusFileName;
	//int lMsgLength = strMsg.GetLength();
	//if(lMsgLength>nMaxSize)
	//{
	//	SYSTEMTIME st; 
	//	GetLocalTime(&st);
	//	
	//	strTempFile.Format(_T("%s——%02d%02d%02d%03d.log"),
	//		/*m_strStatusFileName*/m_strLogName.Left (m_strLogName.GetLength () - 4),st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);

	//	CString str_tmpFileName = strTempFile;
	//	int nFind = strTempFile.ReverseFind(_T('\\'));
	//	if(nFind>0) str_tmpFileName = strTempFile.Right(strTempFile.GetLength()-nFind);

	//	strInfo = strMsg;
	//	strMsg = strMsg.Left(nMaxSize / 2);
	//	strMsg += _T("......(参考文件 ") + str_tmpFileName + _T(")");
	//}
//}}
	
	strMsg = _T("  BeginLog: ") + strMsg + _T(" ---EndLog\r\n");
	DWORD dwWrite;

	char *pBuf = ConvertLPWSTRToLPSTR(strMsg.GetBuffer(0));

	try
	{
		AOTULOCKFILE;
		if(m_hFileHandle)
		{
			WriteFile(m_hFileHandle, pBuf, strlen(pBuf), &dwWrite, NULL);
			m_SubLog.WriteInfo(pBuf, strlen(pBuf));
		}
		if(pBuf)
			delete pBuf;
	}
	catch(...)
	{
	}
//{{ 解决日志量大的问题
	//if(lMsgLength>nMaxSize)
	//{
	//	strMsg = strInfo;
	//	strMsg += _T("\r\n");
	//	DWORD dwWrite;

	//	char *pBuf = ConvertLPWSTRToLPSTR(strMsg.GetBuffer(0));

	//	try
	//	{
	//		//{{ 解决同一毫秒出现2个消息的情况
	//		//HANDLE hFile = CreateFile(strTempFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//		HANDLE hFile = CreateFile(strTempFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	//		int nNumber = 1;
	//		while(hFile == INVALID_HANDLE_VALUE)
	//		{
	//			if(nNumber>20) break;     //防止死循环
	//			CString strTmp = strTempFile.Left(strTempFile.GetLength()-4);
	//			CString strTmpF;
	//			strTmpF.Format(_T("%s_%02d.log"), strTmp, nNumber++);
	//			hFile = CreateFile(strTmpF, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	//		}
	//		//}}
	//		
	//		if(hFile!=INVALID_HANDLE_VALUE)
	//		{
	//			WriteFile(hFile, pBuf, strlen(pBuf), &dwWrite, NULL);
	//			CloseHandle(hFile);
	//		}
	//		delete pBuf;

	//	}
	//	catch(...)
	//	{
	//	}
	//}
//}}
}

void CWriteAction::SetOpenFileHandle(HANDLE hFile)
{
	AOTULOCKFILE;
	if(m_hFileHandle) CloseHandle(m_hFileHandle);
	m_hFileHandle = hFile;
}

HANDLE CWriteAction::GetOpenFileHandle()
{
	AOTULOCKFILE;
	return m_hFileHandle;
}

void CWriteAction::AddMsg(CString strMsg, DWORD nType)
{
	CTime t = CTime::GetCurrentTime();
	CString str;
	str.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	EnterCriticalSection(&m_CritilSec);
	m_MsgArray.Add(strMsg);
	m_MsgType.Add(nType);
	LeaveCriticalSection(&m_CritilSec);
	SetEvent(m_hGetEvent);
}

void CWriteAction::SetLogParam(CString strPath,CString strName)
{		
	if(m_strPath.IsEmpty())
	{
		m_strPath.Format(_T("%s\\%s"),GGetAppPath(),strPath);
	}
	CFileFind finder;
	if(!finder.FindFile(m_strPath))
	{
		::CreateDirectory(m_strPath,NULL);
	}
	finder.Close();
	m_strName = strName;
}
void CWriteAction::WriteLog(CString strInfo,int nType,BOOL bAsync,BOOL bSnmpLog)
{
	if(m_strName.IsEmpty())return;

	try{
		bAsync = TRUE;
		if(m_bLocal)
		{
			goto writelocal;
		}

		bSnmpLog = 1;
		if(bSnmpLog)
		{
			static HMODULE   hin = NULL;
			static VOID (*pfn)(WCHAR* szModuleName,LONG  lLogLevel ,LONG lErrorCode ,WCHAR *szLog);
			if(hin == NULL)
			{
				pfn = NULL;
				CString sAppPath = L"sonaps.logger.client.dll";
				hin = LoadLibrary(sAppPath);
				if(hin == NULL)
				{
					strInfo.Format(_T("%s"),strInfo);
					goto writelocal;
				}

				const char szFn[] = "NMTrace0Ex1";
				*(FARPROC*)&pfn = GetProcAddress(hin, szFn);
				if (pfn == NULL)
				{
					strInfo.Format(_T("%s -- NOT find function in NMLogWriter!"),strInfo);
					goto writelocal;
				}
			}

			if(pfn == NULL)
			{
				strInfo.Format(_T("%s -- NOT find function in NMLogWriter!"),strInfo);
				goto writelocal;
			}
			nType = logLevelInfo;
			//USES_CONVERSION;
			CString sName = L"playout_"+m_strName;
			pfn(sName.GetBuffer(),nType,1,strInfo.GetBuffer());
			return;
		}

writelocal:

		SYSTEMTIME st; 
		GetLocalTime(&st);
		CString strDay;
		strDay.Format(_T("%04d%02d%02d"),st.wYear,st.wMonth,st.wDay);

		{

			AOTULOCKFILE;
			if(NULL != m_hFileHandle)
			{
				DWORD dwFileSize = GetFileSize(m_hFileHandle, NULL);
				if(dwFileSize > m_nMaxByte * 1024 * 1024)
				{
					m_strLogName.Empty ();
					CloseHandle (m_hFileHandle);
					m_hFileHandle = NULL;
				}
			}



			if(m_hFileHandle==NULL || m_strDay != strDay)
			{
				if(NULL != m_hFileHandle)
				{
					CloseHandle(m_hFileHandle);
					m_hFileHandle = NULL;
				}

				CString strPath = _T("");
				strPath.Format(_T("%s\\%s"), m_strPath, strDay);
				if(!PathFileExists(strPath))
				{
					::CreateDirectory(strPath, NULL);
				}
				m_strLogName.Format (_T("%s\\%s %02d-%02d-%02d.log"), strPath, m_strName, st.wHour, st.wMinute, st.wSecond);

				m_strDay = strDay;
				m_hFileHandle = CreateFile(m_strLogName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if(m_hFileHandle==INVALID_HANDLE_VALUE)
				{
					m_hFileHandle = NULL;
					return;
				}
				DWORD dwsz = GetFileSize( m_hFileHandle, 0 );
				SetFilePointer(m_hFileHandle, dwsz, 0, FILE_BEGIN);
				/*if(m_hDelFileThread == NULL)
				m_hDelFileThread = CreateThread(NULL,0,DeleteLogFileThread,this,0,0);*/

				m_SubLog.ReOpenFile(/*m_strStatusFileName*/m_strLogName);
			}
		}

		CString strMsg;
		CString strTime;
		strTime.Format(_T("%02d:%02d:%02d.%03d"),st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		strMsg.Format(_T("%s  %s"),strTime,strInfo);

		if(m_bAsync)
			AddMsg(strMsg, nType);
		else
			WriteToFile(strMsg,nType);
	}
	catch(...)
	{
		DWORD dw = GetLastError();
	}
}

BOOL CWriteAction::WriteSnmp(int nAction,LPVOID pData)
{
	///////zengkairong2008/05/29
	static BOOL (*pfn)(const int type, void** ppv);
	const char szFn[] = "CreateClientInstance";
	if (NULL==m_hSnmpDll || !GetProcAddress(m_hSnmpDll, szFn))  //botao 20090618 播控退出还未完
	{
		return FALSE;
		m_hSnmpDll = LoadLibrary(L"snmp_client.dll");
		if (NULL==m_hSnmpDll)
			return FALSE;
		*(FARPROC*)&pfn = GetProcAddress(m_hSnmpDll, szFn);
	}
		
	if (NULL==pfn)
	{			
		return FALSE;
	}

	//获取写入SNMP接口
	i_snmp_write* p_snmp_write = NULL;
	pfn(snmp::write, (void **)&p_snmp_write);
	if (NULL==p_snmp_write)
	   return FALSE;
	
	/*	//写入SNMP信息
		

		//WriteItem(LPCWSTR id, const int row, LPCTSTR value)
		//id: 需要写入的标识名称及其具体该写何值, 请各个项目参照DOC目录下的XSL文件, 或者是参考snmp_define.hpp文件
		//row: 所处的行号, 整个定义在olympic_define.txt文件中的SNMP信息可以看做一张表, 行从1开始
		//value: 需要写入的值
		
	*/
	//////////
		CString strWriteID;
		CString strWriteValue;
		int     nWriteValeu=0;
		int     nWriteRow=0;
	try{
		switch(nAction)
		{
		case Snmp_PlayoutInfo:
			{
				PlayoutInfo * p = (PlayoutInfo*)pData;
				
				strWriteID=L"proV1SonapsV4PlayoutInfoProdId";
				nWriteValeu=p->nProdId;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);
				
				strWriteID=L"proV1SonapsV4PlayoutInfoStudioID";
				nWriteValeu=p->nStudioId;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutInfoStudioName";
				strWriteValue=p->strStudioName;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);

				strWriteID=L"proV1SonapsV4PlayoutInfoServerStatus";
				nWriteValeu=p->nServerStatus;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutInfoSystemStatus";
				nWriteValeu=p->nSystemStatus;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);
				
				
			}
			break;
		case Snmp_PlayoutDevice:
			{
				std::vector<PlayoutDevice> p = *(std::vector<PlayoutDevice> *)pData;
				CString sw;
				int n = p.size();
				for(int i=0;i<n;i++)
				{
					strWriteID=L"proV1SonapsV4PlayoutDeviceProdId";
					nWriteValeu=p[i].nProdId;
					nWriteRow++;
					p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);
										
					strWriteID=L"proV1SonapsV4PlayoutDeviceID";
					nWriteValeu=p[i].nDeviceId;
					nWriteRow++;
					p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

					strWriteID=L"proV1SonapsV4PlayoutDeviceName";
					strWriteValue=p[i].strDeviceName;
					nWriteRow++;
					p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);

					strWriteID=L"proV1SonapsV4PlayoutDeviceType";
					nWriteValeu=p[i].nDeviceType;
					nWriteRow++;
					p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

					strWriteID=L"proV1SonapsV4PlayoutDeviceStatus";
					nWriteValeu=p[i].nDeviceStatus;
					nWriteRow++;
					p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

					strWriteID=L"proV1SonapsV4PlayoutDeviceErrorString";
					strWriteValue=p[i].strDeviceInfo;
					nWriteRow++;
					p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);

				}
				
			}
			break;
		case Snmp_PlayoutService:
			{
				PlayoutServiceInfo *p = (PlayoutServiceInfo *)pData;
				
				strWriteID=L"proV1SonapsV4PlayoutServiceProdId";
				nWriteValeu=p->nProdId;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutServiceID";
				nWriteValeu=p->nServiceId;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutServiceName";
				strWriteValue=p->strServiceName;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);

				strWriteID=L"proV1SonapsV4PlayoutServiceStatus";
				nWriteValeu=p->nServiceStatus;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutServiceInfo";
				strWriteValue=p->strServiceInfo;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);


			}
			break;
		case Snmp_PlayoutUser:
			{
				PlayoutUser * p = (PlayoutUser *)pData;

				strWriteID=L"proV1SonapsV4PlayoutUserProdId";
				nWriteValeu=p->nProdId;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutUserID";
				nWriteValeu=p->nUserId;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, nWriteValeu);

				strWriteID=L"proV1SonapsV4PlayoutUserName";
				strWriteValue=p->strUserName;
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);

				strWriteID=L"proV1SonapsV4PlayoutUserLoginTime";
				strWriteValue=p->dtLoginTime.Format(L"%Y-%m-%d %H:%M:%S");
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);

				strWriteID=L"proV1SonapsV4PlayoutUserLogoutTime";
				strWriteValue=p->dtLogoutTime.Format(L"%Y-%m-%d %H:%M:%S");
				nWriteRow++;
				p_snmp_write->WriteItem(strWriteID, nWriteRow, strWriteValue);
				
			}
			break;
		
		}
	}
	catch (...)
	{
		
		return FALSE;
	}
	
	
	
	return TRUE;
	/////////////////////////old snmp_client.dll
	/*
	try
	{
		static BOOL bFirst = FALSE;
		static IMib* m_pMib;
		static HMODULE   hin = NULL;
		static BOOL (*pfn)(void**);
		if(!hin)
		{
			hin = LoadLibrary(L"client.dll"); 


			DWORD tt = GetLastError();

			const char szFn[] = "CreateClientInstance";
			*(FARPROC*)&pfn = GetProcAddress(hin, szFn);
			if (pfn == NULL)
			{
			//	WriteLog(L"Load snmp client.dll Error!",0);
				return FALSE;
			}

			pfn((void **)&m_pMib);
		}
		USES_CONVERSION;
		BOOL b = FALSE;
		if (m_pMib && !bFirst)
		{
			b = m_pMib->OpenAgent(A2W("PROV1-SONAPSV4-PLAYOUT-MIB.mib"));
			if(b)
				bFirst = TRUE;
			else
			{
			//	WriteLog(L"OpenAgent Error!",0);
				return FALSE;
			}
		}



		CString sName,sValue;
		CString sTemp = L"<xml><data><insert>%s</insert></data></xml>";
		//根据动作来确定处理流程
		switch(nAction)
		{
		case Snmp_PlayoutInfo:
			{
				sName = L"sony.professionalV1.proV1Product.proV1SonapsV4Playout.proV1SonapsV4PlayoutInformation.proV1SonapsV4PlayoutInfoTable.proV1SonapsV4PlayoutInfoEntry";
				PlayoutInfo * p = (PlayoutInfo*)pData;
				CString s = L"<row proV1SonapsV4PlayoutInfoProdId=\"%d\" proV1SonapsV4PlayoutInfoStudioID=\"%d\" \
							 proV1SonapsV4PlayoutInfoStudioName=\"%s\" proV1SonapsV4PlayoutInfoServerStatus=\"%d\" \
							 proV1SonapsV4PlayoutInfoSystemStatus=\"%d\" />";
				CString ss;
				ss.Format(s,p->nProdId,p->nStudioId,p->strStudioName,p->nServerStatus,p->nSystemStatus);
				sValue.Format(sTemp,ss);
			}
			break;
		case Snmp_PlayoutDevice:
			{
				sName = L"sony.professionalV1.proV1Product.proV1SonapsV4Playout.proV1SonapsV4PlayoutEnvironment.proV1SonapsV4PlayoutDeviceTable.proV1SonapsV4PlayoutDeviceEntry";
				std::vector<PlayoutDevice> p = *(std::vector<PlayoutDevice> *)pData;
				CString sw;
				int n = p.size();
				for(int i=0;i<n;i++)
				{
					CString s = L"<row proV1SonapsV4PlayoutDeviceProdId=\"%d\" proV1SonapsV4PlayoutDeviceId=\"%d\" \
								 proV1SonapsV4PlayoutDeviceName=\"%s\" proV1SonapsV4PlayoutDeviceType=\"%d\" \
								 proV1SonapsV4PlayoutDeviceStatus=\"%d\" proV1SonapsV4PlayoutDeviceInfo=\"%s\" />";
					CString ss;
					ss.Format(s,p[i].nProdId,p[i].nDeviceId,p[i].strDeviceName,p[i].nDeviceType,p[i].nDeviceStatus,p[i].strDeviceInfo);
					sw += ss;
				}
				sValue.Format(sTemp,sw);
			}
			break;
		case Snmp_PlayoutUser:
			{
				sName = L"sony.professionalV1.proV1Product.proV1SonapsV4Playout.proV1SonapsV4PlayoutManagement.proV1SonapsV4PlayoutUserTable.proV1SonapsV4PlayoutUserEntry";
				PlayoutUser * p = (PlayoutUser *)pData;
				CString s = L"<row proV1SonapsV4PlayoutUserProdId=\"%d\" proV1SonapsV4PlayoutUserId=\"%d\" \
							 proV1SonapsV4PlayoutUserName=\"%s\" proV1SonapsV4PlayoutUserLoginTime=\"%s\" \
							 proV1SonapsV4PlayoutUserLogoutTime=\"%s\" />";
				CString ss;
				ss.Format(s,p->nProdId,p->nUserId,p->strUserName,p->dtLoginTime.Format(L"%Y-%m-%d %H:%M:%S"),p->dtLogoutTime.Format(L"%Y-%m-%d %H:%M:%S"));
				sValue.Format(sTemp,ss);
			}
			break;
		case Snmp_PlayoutService:
			{
				sName = L"sony.professionalV1.proV1Product.proV1SonapsV4Playout.proV1SonapsV4PlayoutEnvironment.proV1SonapsV4PlayoutServiceTable.proV1SonapsV4PlayoutServiceEntry";
				PlayoutServiceInfo *p = (PlayoutServiceInfo *)pData;
				CString s = L"<row proV1SonapsV4PlayoutServiceProdId=\"%d\" proV1SonapsV4PlayoutServiceId=\"%d\" \
							 proV1SonapsV4PlayoutServiceName=\"%s\" proV1SonapsV4PlayoutServiceStatus=\"%d\" \
							 proV1SonapsV4PlayoutServiceInfo=\"%s\" />";
				CString ss;
				ss.Format(s,p->nProdId,p->nServiceId,p->strServiceName,p->nServiceStatus,p->strServiceInfo);
				sValue.Format(sTemp,ss);
			}
			break;
		}
		b = m_pMib->WriteTable(sName,sValue);
		Sleep(10);
	}
	catch (...)
	{
		//WriteLog(L"write snmp error!!",0);
		return FALSE;
	}
	//m_pMib->CloseAgent();
	return TRUE;
	*/
}

void CWriteAction::WriteLogInfo(LOGINFO loginfo)
{
	if(m_strName.IsEmpty())return;
	try{
		loginfo.bSnmpLog = 1;
		if(loginfo.bSnmpLog)
		{
			if(WriteNMLog(loginfo))
				return;
		}
		SYSTEMTIME st; 
		GetLocalTime(&st);
		CString strDay;
		strDay.Format(_T("%04d%02d%02d"),st.wYear,st.wMonth,st.wDay);

		{
			AOTULOCKFILE;
			if(NULL != m_hFileHandle)
			{
				DWORD dwFileSize = GetFileSize(m_hFileHandle, NULL);
				if(dwFileSize > m_nMaxByte * 1024 * 1024)
				{
					m_strLogName.Empty ();
					CloseHandle (m_hFileHandle);
					m_hFileHandle = NULL;
					
				}
			}


			if(m_hFileHandle==NULL || m_strDay!=strDay)
			{
				if(NULL != m_hFileHandle)
				{
					CloseHandle(m_hFileHandle);
					m_hFileHandle = NULL;
				}

				CString strPath = _T("");
				strPath.Format(_T("%s\\%s"), m_strPath, strDay);
				if(!PathFileExists(strPath))
				{
					::CreateDirectory(strPath, NULL);
				}
				m_strLogName.Format (_T("%s\\%s %02d-%02d-%02d.log"), strPath, m_strName, st.wHour, st.wMinute, st.wSecond);

				m_strDay = strDay;

				m_hFileHandle = CreateFile(m_strLogName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if(m_hFileHandle==INVALID_HANDLE_VALUE)
				{
					m_hFileHandle = NULL;
					return;
				}
				DWORD dwsz = GetFileSize( m_hFileHandle, 0 );
				SetFilePointer(m_hFileHandle, dwsz, 0, FILE_BEGIN);
				/*if(m_hDelFileThread == NULL)
				m_hDelFileThread = CreateThread(NULL,0,DeleteLogFileThread,this,0,0);*/

				m_SubLog.ReOpenFile(m_strLogName);
			}
		}
		loginfo.bAsync = TRUE;
		CString strMsg;
		CString strTime;
		strTime.Format(_T("%02d:%02d:%02d.%03d"),st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		strMsg.Format(_T("%s  %s"),strTime,loginfo.strInfo);
		if(loginfo.nErrorCode>0)
		{
			CString strTmp=strMsg;
			strMsg.Format(_T("%s [ErrorCode=%d]"),strTmp,loginfo.nErrorCode);
		}
		if(m_bAsync)
			AddMsg(strMsg, loginfo.nLogLevel);
		else
			WriteToFile(strMsg,loginfo.nLogLevel);
	}
	catch(...)
	{
	}
}

BOOL CWriteAction::WriteNMLog(LOGINFO loginfo)
{
	static BOOL (*pfn)(const int type, void** ppv);
	const char szFn[] = "CreateClientInstance"; 
	if(NULL==m_hSnmpDll)
	{
		m_hSnmpDll = LoadLibrary(L"snmp_client.dll");
		if (NULL==m_hSnmpDll)
		return FALSE;
		
	}
	*(FARPROC*)&pfn = GetProcAddress(m_hSnmpDll, szFn);
	if( NULL== pfn)
		return FALSE;

	//获取接口
	i_snmp_trap* p_snmp_trap = NULL;
	pfn(snmp::trap, (void **)&p_snmp_trap);
	if (NULL==p_snmp_trap)
		return FALSE;

	if(loginfo.nLogLevel==0)
		loginfo.nLogLevel = LogLevel_Info;	
	switch (loginfo.nLogLevel)
	{
	case LogLevel_Info:
		//p_snmp_trap->TrapInfo();
		
		break;
	case LogLevel_Error:
		int idx;
		p_snmp_trap->TrapError(loginfo.nLogLevel,loginfo.nErrorCode,loginfo.strInfo,idx);
		//p_snmp_trap->ClearError(idx);
		break;
	}
	
	return TRUE;
}


void CWriteAction::DeleteLogFile()
{
	CString strFileName;
	strFileName.Format(_T("%s\\*.log"), m_strPath);
	CTime CurTm = CTime::GetCurrentTime();
	CTimeSpan span(15,0,0,0);     //默认15天
	CTime OldTm = CurTm - span;

	CStringArray strFileNameArr;
	WIN32_FIND_DATA ws;
	HANDLE hHandle = FindFirstFile(strFileName,&ws);
	if(hHandle == INVALID_HANDLE_VALUE) return;
	do{
		SYSTEMTIME sysTm;
		FileTimeToSystemTime(&ws.ftLastWriteTime, &sysTm);
		CTime filetime(sysTm.wYear,sysTm.wMonth,sysTm.wDay,sysTm.wHour,sysTm.wMinute,sysTm.wSecond);
		if(filetime < OldTm) strFileNameArr.Add(ws.cFileName);
	}while(FindNextFile(hHandle, &ws));
	FindClose(hHandle);

	int i;
	for(i=0; i<strFileNameArr.GetSize(); i++)
	{
		CString strName;
		strName.Format(_T("%s\\%s"),m_strPath, strFileNameArr.GetAt(i));
		DeleteFile(strName);
	}
	strFileNameArr.RemoveAll();
}

void CWriteAction::GetConfigParams()
{
	HKEY hKeyParent = HKEY_LOCAL_MACHINE;
	LPCTSTR lpszSubKey = _T("SOFTWARE\\Sobey\\Public\\StudioLog");
	HKEY  hKey = NULL;
	DWORD dwBufLen = 256;
	DWORD dwRegType = REG_SZ;
	CString strValue = _T("");
	// 打开该注册表键
	long lRet = ::RegOpenKeyEx(hKeyParent, lpszSubKey, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lRet)
	{
		// 创建成功
		LPCTSTR lpszValueName1	= _T("Local");
		LPBYTE  lpValue		    = new BYTE[256];

		memset(lpValue, 0, 256);
		lRet = ::RegQueryValueEx(hKey, lpszValueName1, NULL, &dwRegType, lpValue, &dwBufLen);
		if(ERROR_SUCCESS == lRet)
		{
			strValue = (LPTSTR)(lpValue);

			if(!strValue.IsEmpty() && _ttoi(strValue) == 1)
			{
				m_bLocal = TRUE;
			}
			else
			{
				m_bLocal = FALSE;
			}
		}

		LPCTSTR lpszValueName2 = _T("MaxByte");
		dwBufLen = 256;
		memset(lpValue, 0, 256);
		lRet = ::RegQueryValueEx(hKey, lpszValueName2, NULL, &dwRegType, lpValue, &dwBufLen);
		if(ERROR_SUCCESS == lRet)
		{
			strValue = (LPTSTR)(lpValue);

			if(!strValue.IsEmpty())
			{
				m_nMaxByte = _ttoi(strValue);
			}
			else
			{
				m_nMaxByte = 20;
			}
		}

		LPCTSTR lpszValueName3 = _T("Days");
		dwBufLen = 256;
		memset(lpValue, 0, 256);
		lRet = ::RegQueryValueEx(hKey, lpszValueName3, NULL, &dwRegType, lpValue, &dwBufLen);
		if(ERROR_SUCCESS == lRet)
		{
			strValue = (LPTSTR)(lpValue);

			if(!strValue.IsEmpty())
			{
				m_nDays = _ttoi(strValue);
			}
			else
			{
				m_nDays = 15;
			}
		}

		//LPCTSTR lpszValueName4 = _T("Lag");
		//dwBufLen = 256;
		//memset(lpValue, 0, 256);
		//lRet = ::RegQueryValueEx(hKey, lpszValueName4, NULL, &dwRegType, lpValue, &dwBufLen);
		//if(ERROR_SUCCESS == lRet)
		//{
		//	strValue = (LPTSTR)(lpValue);

		//	if(!strValue.IsEmpty())
		//	{
		//		m_bAsync = _ttoi(strValue);
		//	}
		//	else
		//	{
		//		m_bAsync = 1;
		//	}
		//}
		// 关闭该注册表键
		::RegCloseKey(hKey);
		delete []lpValue;
	}
}

CString CWriteAction::GetTargetPath()
{
	CString strPath = _T("");
	BOOL bLocation = FALSE;

	HKEY hKeyParent = HKEY_LOCAL_MACHINE;
	LPCTSTR lpszSubKey = _T("SOFTWARE\\Sobey\\Public\\StudioLog");
	HKEY  hKey = NULL;
	DWORD dwBufLen = 256;
	DWORD dwRegType = REG_SZ;
	// 打开该注册表键
	long lRet = ::RegOpenKeyEx(hKeyParent, lpszSubKey, 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		// 创建成功
		LPCTSTR lpszValueName	= _T("Path");
		LPBYTE  lpValue		    = new BYTE[256];
		lRet = ::RegQueryValueEx(hKey, lpszValueName, NULL, &dwRegType, lpValue, &dwBufLen);
		// 关闭该注册表键
		::RegCloseKey(hKey);
		if(ERROR_SUCCESS == lRet)
		{
			strPath = (LPTSTR)(lpValue);
			if(!strPath.IsEmpty())
			{
				if(LocationCurrentDir(_T("PlstDoc.dll")))
				{
					strPath = strPath + _T("\\PlayoutTerminal\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("PlayoutCtrl.dll")))
				{
					strPath = strPath + _T("\\PlayoutS\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("PipeServer.dll")))
				{
					strPath = strPath + _T("\\DataPipeServer\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("ITDataHandle.dll")))
				{
					strPath = strPath + _T("\\NewInterfaceSvc\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("ITDeleteService.dll")))
				{
					strPath = strPath + _T("\\DeleteServer\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("StudioDataManager.exe")))
				{
					strPath = strPath + _T("\\StudioDataManage\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("IsynSvc.exe")))
				{
					strPath = strPath + _T("\\IsynSvc\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("PsynSvc.exe")))
				{
					strPath = strPath + _T("\\PsynSvc\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("SBStudioManage.dll")))
				{
					strPath = strPath + _T("\\StudioManage\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("GetFrameRPC_Server.dll")))
				{
					strPath = strPath + _T("\\GetFrameRPC\\");
					bLocation = TRUE;
				}
				else if(LocationCurrentDir(_T("ETSkin.dll")))
				{
					strPath = strPath + _T("\\StudioCheck\\");
					bLocation = TRUE;
				}
			}
		}
		delete []lpValue;
	}

	if(bLocation)
	{
		if(!PathFileExists(strPath))
		{
			for(int i = 4; i < strPath.GetLength(); i++)
			{
				CString strTemp = strPath.Mid(i, 1);
				if(strTemp.Find('\\') >= 0)
				{
					if(!PathFileExists(strPath.Left(i)))
					{
						if(!CreateDirectory(strPath.Left(i), NULL))
						{
							strPath.Empty();
							break;
						}
					}
				}
			}
		}
	}
	return strPath;
}

BOOL CWriteAction::LocationCurrentDir(CString strFile)
{
	CString strLocalPath = GetAppFolder();                          //不能获取主程序名称,现场可能会更改执行文件名,而动态库名不能更改
	CString strFlag = _T("");

	strFlag.Format(_T("%s\\%s"), strLocalPath, strFile);            //播控客户端唯一标识动态库
	if(::PathFileExists(strFlag))
	{
		return TRUE;
	}
	return FALSE;
}