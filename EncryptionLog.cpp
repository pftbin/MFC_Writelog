#include "stdafx.h"
#include "EncryptionLog.h"


#define  MAX_LENGTH    MAX_LOGLINE+256

CEncryptLog::CEncryptLog(void):m_hFile(INVALID_HANDLE_VALUE),m_strFileName(_T(""))
{
}

CEncryptLog::~CEncryptLog(void)
{
	if(INVALID_HANDLE_VALUE != m_hFile) ReCloseFile();
}

void CEncryptLog::ReOpenFile(CString strName)
{
	int nFind = strName.ReverseFind(_T('.'));
	if( nFind > 0) strName = strName.Left(nFind);
	m_strFileName.Format(_T("%s_Sub.log"),strName);

	if(INVALID_HANDLE_VALUE != m_hFile) ReCloseFile();
	_OpenFile();
}

void CEncryptLog::_OpenFile()
{
	if(m_strFileName.IsEmpty()) return;

	m_hFile = CreateFile(m_strFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile==INVALID_HANDLE_VALUE) return;
	DWORD dwsz = GetFileSize( m_hFile, 0 );
	SetFilePointer(m_hFile, dwsz, 0, FILE_BEGIN);
}

void CEncryptLog::WriteInfo(char chBuf[], int nLen)
{
	if(INVALID_HANDLE_VALUE == m_hFile) _OpenFile();
	if(INVALID_HANDLE_VALUE == m_hFile) return;

	DWORD dwWrite;
	int nWriteLen = 0;
	//char WriteBuff[MAX_LENGTH];
	char* pWriteBuff = new char[nLen];
	if(!Encrypting(chBuf, nLen, pWriteBuff, nWriteLen))
	{
		memset(pWriteBuff,0xff,10);
		nWriteLen = 10;
	}
	WriteFile(m_hFile, pWriteBuff, nWriteLen, &dwWrite, NULL);
	delete []pWriteBuff;
}

void CEncryptLog::ReCloseFile(void)
{
	if(INVALID_HANDLE_VALUE == m_hFile) return;
	CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
}

BOOL CEncryptLog::Encrypting(char SrcBuff[], int nSrcLen, char DesBuff[], int& nDesLen)
{
	nDesLen = nSrcLen;
	for(int i=0; i<nSrcLen; i++) DesBuff[i] = ~SrcBuff[i];
	return TRUE;
}
