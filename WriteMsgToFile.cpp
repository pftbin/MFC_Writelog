// WriteMsgToFile.cpp: implementation of the WriteMsgToFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WriteMsgToFile.h"
#include "WriteAction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWriteMsgToFile::CWriteMsgToFile()
{
	m_pLogWriter = new CWriteAction;
	this->Initialize();
}

CWriteMsgToFile::~CWriteMsgToFile()
{
	if(m_pLogWriter)
		delete m_pLogWriter;
	m_pLogWriter = NULL;
}

//void CWriteMsgToFile::AddMsg(CStringArray *pMsgArr, CDWordArray* pdwArray)
//{
//	if(m_pLogWriter) m_pLogWriter->AddMsg(pMsgArr, pdwArray);
//}

//CString CWriteMsgToFile::GetMsg(DWORD& dwType)
//{
//	if(m_pLogWriter) return m_pLogWriter->GetMsg(dwType);
//	return _T("");
//}

void CWriteMsgToFile::Initialize()
{
	if(m_pLogWriter) m_pLogWriter->Initialize();
}

//void CWriteMsgToFile::WriteToFile(CString strMsg, DWORD dwType)
//{
//	if(m_pLogWriter) m_pLogWriter->WriteToFile(strMsg, dwType);
//}

//void CWriteMsgToFile::SetOpenFileHandle(HANDLE hFile)
//{
//	if(m_pLogWriter) m_pLogWriter->SetOpenFileHandle(hFile);
//}

//HANDLE CWriteMsgToFile::GetOpenFileHandle()
//{
//	if(m_pLogWriter) return m_pLogWriter->GetOpenFileHandle();
//	return NULL;
//}

//void CWriteMsgToFile::AddMsg(CString strMsg, DWORD nType)
//{
//	if(m_pLogWriter) m_pLogWriter->AddMsg(strMsg, nType);
//}

void CWriteMsgToFile::SetLogParam(CString strPath,CString strName)
{		
	if(m_pLogWriter) m_pLogWriter->SetLogParam(strPath, strName);
}
void CWriteMsgToFile::WriteLog(CString strInfo,int nType,BOOL bAsync,BOOL bSnmpLog)
{
	if(m_pLogWriter) m_pLogWriter->WriteLog(strInfo, nType, bAsync, bSnmpLog);
}

BOOL CWriteMsgToFile::WriteSnmp(int nAction,LPVOID pData)
{
	if(m_pLogWriter) return m_pLogWriter->WriteSnmp(nAction, pData);
	return FALSE;
}

void CWriteMsgToFile::WriteLogInfo(LOGINFO loginfo)
{
	if(m_pLogWriter) return m_pLogWriter->WriteLogInfo(loginfo);
}

