#include "Resource.h"
#include "AxesPage.h"
#include "ButtonsPage.h"
#include "GainPage.h"
#include "OrientationPage.h"
#include "PrecisionPage.h"
#include "SensitivityPage.h"
#include "BatchPage.h"
#include "orb_control.h"


// OrbProps.h : main header file for the ORBPROPS application
//

#if !defined(AFX_ORBPROPS_H__F1E09067_4B81_4CF7_94DA_2E9F2F736D8B__INCLUDED_)
#define AFX_ORBPROPS_H__F1E09067_4B81_4CF7_94DA_2E9F2F736D8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// COrbPropsApp:
// See OrbProps.cpp for the implementation of this class
//

class COrbPropsApp : public CWinApp
{
public:
	COrbPropsApp();
    void InitializePropertiesFromOrb(
                                    CAxesPage &axesopts,
                                    CButtonsPage &buttonsopts,
                                    CGainPage &gainopts,
                                    COrientationPage &orientopts,
                                    CPrecisionPage &precisionopts,
                                    CSensitivityPage &sensopts,
                                    orb_control &control);
    void ApplyChanges(CBatchPage &batchpage,
                                    CAxesPage &axesopts,
                                    CButtonsPage &buttonsopts,
                                    CGainPage &gainopts,
                                    COrientationPage &orientopts,
                                    CPrecisionPage &precisionopts,
                                    CSensitivityPage &sensopts,
                                    orb_control &control);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COrbPropsApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(COrbPropsApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


enum e_Orientation {ORIENT_VERTICAL, ORIENT_HORIZONTAL};
BOOL ChangeStaticBitmap(CStatic *pImage, int iBitmapResourceID);


#endif // !defined(AFX_ORBPROPS_H__F1E09067_4B81_4CF7_94DA_2E9F2F736D8B__INCLUDED_)
