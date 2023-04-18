#pragma once

//Shelin, 8/27/2007

/////////////////////////////////
//INTERFACE
/////////////////////////////////
namespace snmp
{
	const int write   = 1;
	const int read    = 2;
	const int trap    = 3;
}

class i_snmp_write
{
public:
	virtual BOOL	 WriteItem(LPCWSTR id, const int row, int value) = 0;
	virtual BOOL	 WriteItem(LPCWSTR id, const int row, long value) = 0;
	virtual BOOL	 WriteItem(LPCWSTR id, const int row, LPCTSTR value) = 0;
	virtual BOOL	 BeginWrite(LPCWSTR id) = 0;
	virtual BOOL	 EndWrite(LPCWSTR id) = 0;
};

class i_snmp_trap
{
public:
	virtual BOOL     TrapInfo(LPCWSTR id, LPCWSTR value) = 0;
	virtual BOOL     TrapError(int level, int code, LPCWSTR descr, int &idx) = 0;
	virtual BOOL     ClearError(int idx) = 0;
};