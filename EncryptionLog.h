#pragma once

class CEncryptLog
{
public:
	CEncryptLog(void);
	~CEncryptLog(void);
	void ReOpenFile(CString strName);
	void WriteInfo(char chBuf[], int nLen);
	void ReCloseFile(void);

protected:
	BOOL Encrypting(char SrcBuff[], int nSrcLen, char DesBuff[], int& nDesLen);
	void _OpenFile();

protected:
	CString m_strFileName;
	HANDLE   m_hFile;
};
