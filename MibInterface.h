
#pragma once

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")


typedef void (*IntCallBack)(LPCWSTR, int);
typedef void (*StringCallBack)(LPCWSTR, LPCWSTR);
typedef void (*DoubleCallBack)(LPCWSTR, double);
typedef void (*TimeCallBack)(LPCWSTR, CTime);
typedef void (*TableCallBack)(LPCWSTR, _RecordsetPtr);
class IMib
{
public:
	virtual	BOOL	 OpenAgent(LPCWSTR mibfile, LPCWSTR server = 0) = 0;
	virtual	BOOL	 CloseAgent() = 0;

	virtual BOOL	 WriteItem(LPCWSTR name, int value) = 0;
	virtual BOOL	 WriteItem(LPCWSTR name, LPCWSTR value) = 0;
	virtual BOOL	 WriteItem(LPCWSTR name, double value) = 0;
	virtual BOOL	 WriteItem(LPCWSTR name, CTime value) = 0;
	virtual BOOL	 WriteItem(LPCWSTR name, _RecordsetPtr value) = 0;

	virtual BOOL	 SetItemCallback(IntCallBack function) = 0;
	virtual BOOL	 SetItemCallback(StringCallBack function) = 0;
	virtual BOOL	 SetItemCallback(DoubleCallBack function) = 0;
	virtual BOOL	 SetItemCallback(TimeCallBack function) = 0;
	virtual BOOL	 SetItemCallback(TableCallBack function) = 0;
	
	virtual BOOL	 TrapItem(LPCWSTR name) = 0;
	virtual BOOL	 TrapItems(LPCWSTR *name, int count) = 0;

	virtual BOOL     WriteTable(LPCWSTR name, LPCWSTR value) = 0;
	virtual BOOL     AppendTable(LPCWSTR name, LPCWSTR value) = 0;
};