// WriteLog.h : main header file for the WRITELOG DLL
//

#if !defined(AFX_WRITELOG_H__D2C0C42E_4FF2_4E87_9CFD_5E8F441D3F32__INCLUDED_)
#define AFX_WRITELOG_H__D2C0C42E_4FF2_4E87_9CFD_5E8F441D3F32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWriteLogApp
// See WriteLog.cpp for the implementation of this class
//

class CWriteLogApp : public CWinApp
{
public:
	CWriteLogApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWriteLogApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CWriteLogApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WRITELOG_H__D2C0C42E_4FF2_4E87_9CFD_5E8F441D3F32__INCLUDED_)
