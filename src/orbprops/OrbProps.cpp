// OrbProps.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OrbProps.h"
#include "OrbPropsDlg.h"
#include "BatchPage.h"
#include "TokenEx.h"

int
parse_args( int arg_count,
	    char** arg_vector,
	    orb_control& control );

char* AxisNames[] = {
  "0 - X Axis",
  "1 - Y Axis",
  "2 - Z Axis",
  "3 - X Rotation",
  "4 - Y Rotation",
  "5 - Z Rotation"
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COrbPropsApp

BEGIN_MESSAGE_MAP(COrbPropsApp, CWinApp)
	//{{AFX_MSG_MAP(COrbPropsApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COrbPropsApp construction

COrbPropsApp::COrbPropsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only COrbPropsApp object

COrbPropsApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COrbPropsApp initialization

BOOL COrbPropsApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

    // check to see if we can even talk to the orb
    orb_control control;
    if (!control.is_initialized()) {
      MessageBox(NULL, "Could not find a SpaceOrb!", "OrbProps Fatal Error", MB_ICONEXCLAMATION|MB_OK);
      return FALSE;
    }

	COrbPropsDlg dlg;

    // add our property pages
    CAxesPage axesopts;
    CButtonsPage buttonsopts;
    CGainPage gainopts;
    COrientationPage orientopts;
    CPrecisionPage precisionopts;
    CSensitivityPage sensopts;
    CBatchPage batchpage;

    // set references
    orientopts.SetPageRefs(&axesopts, &buttonsopts, &gainopts, &precisionopts, &sensopts);
    axesopts.SetNames(AxisNames);
    axesopts.SetPageRefs(&gainopts, &sensopts);
    batchpage.SetPageRefs(&axesopts, &buttonsopts, &gainopts, &orientopts, &precisionopts, &sensopts);
    batchpage.SetOrbRef(this, &control);
    dlg.SetRefs(this, &batchpage, &axesopts, &buttonsopts, &gainopts, &orientopts, &precisionopts, &sensopts, &control);

    // initialize state of property pages
    InitializePropertiesFromOrb(axesopts, buttonsopts, gainopts, orientopts, precisionopts, sensopts, control);

    // add property pages to main dialog
    dlg.AddPage(&orientopts);
    dlg.AddPage(&buttonsopts);
    dlg.AddPage(&axesopts);
    dlg.AddPage(&sensopts);
    dlg.AddPage(&gainopts);
    dlg.AddPage(&precisionopts);
    dlg.AddPage(&batchpage);
    // set Axes as active page (if we don't, the orientation gets screwed up; TODO fix)
    dlg.SetActivePage(2);

	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
      ApplyChanges(batchpage, axesopts, buttonsopts, gainopts, orientopts, precisionopts, sensopts, control);
	}
	else if (nResponse == IDCANCEL)
	{
      // nothing to do here since we're just cancelling out
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


// Function for changing the bitmap on a static control
// returns true if successful
BOOL ChangeStaticBitmap(CStatic *pImage, int iBitmapResourceID)
{
    HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
      MAKEINTRESOURCE(iBitmapResourceID), 
      IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);

    if (hBmp == NULL)
        return FALSE;

    hBmp = pImage->SetBitmap(hBmp);
    if (hBmp != NULL)
        ::DeleteObject(hBmp);
    return TRUE;
}


// changes initial state of properties pages depending on orb state
void COrbPropsApp::InitializePropertiesFromOrb(
    CAxesPage &axesopts,
    CButtonsPage &buttonsopts,
    CGainPage &gainopts,
    COrientationPage &orientopts,
    CPrecisionPage &precisionopts,
    CSensitivityPage &sensopts,
    orb_control &control
)
{
    int i;
    // set initial values from current state
     // handle axes current state
    for (i = 0;  i < 6;  i++) {
      axesopts.SetAxis(i, control.physical_axis_from_logical_axis(i));
    }
    for (i = 0;  i < 6;  i++) {
      axesopts.SetAxisInvert(i, control.polarity(i) == 0);
    }
     // handle button chording current state
    buttonsopts.m_fEnableChording = control.using_chording();
     // handle gain page current state
    for (i = 0;  i < 6;  i++) {
      gainopts.SetAxisGain(i, control.gain(i));
    }
     // handle sensitivity page current state
    sensopts.m_iNullRegion = control.null_region();
    for (i = 0;  i < 6;  i++) {
      if (control.polarity(i) == -1)
        sensopts.SetAxisSensitivity(control.physical_axis_from_logical_axis(i), -1);
      else
        sensopts.SetAxisSensitivity(control.physical_axis_from_logical_axis(i), control.sensitivity(i));
    }
     // handle precision page current state
    precisionopts.m_iSensitivity = control.precision_sensitivity();
    precisionopts.m_iGain = control.precision_gain();
    if (control.precision_button_type() == 0) {
      precisionopts.m_fEnablePrecision = FALSE;
    } else {
      precisionopts.m_fEnablePrecision = TRUE;
      if (control.precision_button_type() == 1) {
        precisionopts.m_iPrecisionButton = control.precision_button_index();
      } else {
        CString csButton;
        csButton.Format("Joy%d", control.precision_button_index()+1);
        precisionopts.m_csLogicalButton = csButton;
      }
    }

     // TODO: we need to read the orientation from the registry
     // instead of doing this because the user may be using horizontal
     // mode but wants to change the axes to something non-standard...
    int iFacing;
    if (control.current_facing() == orb_control::Horizontal_facing)
      iFacing = ORIENT_HORIZONTAL;
    else if (control.current_facing() == orb_control::Vertical_facing)
      iFacing = ORIENT_VERTICAL;
    orientopts.m_iOrientation = iFacing;
    axesopts.m_iOrientation = axesopts.m_iOldOrientation = iFacing;
    buttonsopts.m_iOrientation = iFacing;
    gainopts.m_iOrientation = iFacing;
    precisionopts.m_iOrientation = iFacing;
    sensopts.m_iOrientation = iFacing;
}


void COrbPropsApp::ApplyChanges(
    CBatchPage &batchpage,
    CAxesPage &axesopts,
    CButtonsPage &buttonsopts,
    CGainPage &gainopts,
    COrientationPage &orientopts,
    CPrecisionPage &precisionopts,
    CSensitivityPage &sensopts,
    orb_control &control
)
{
    batchpage.UpdateBatchCommands();
    batchpage.m_csBatchCommands.Replace("\n", " ");
    CTokenEx tok;
    CStringArray csaTokens;
    tok.Split(batchpage.m_csBatchCommands, " ", csaTokens);
    char **ppArgs = new char*[csaTokens.GetSize() + 1];
    int i;
    for (i = 0;  i < csaTokens.GetSize();  i++) {
      // start at index 1 because argv[0] is always program name
      ppArgs[i + 1] = csaTokens[i].GetBuffer(csaTokens[i].GetLength());
    }
    parse_args(csaTokens.GetSize() + 1, ppArgs, control);
    for (i = 0;  i < csaTokens.GetSize();  i++) {
      csaTokens[i].ReleaseBuffer();
    }
    delete []ppArgs;

    // disable modified status on all pages
    batchpage.SetModified(FALSE);
    axesopts.SetModified(FALSE);
    buttonsopts.SetModified(FALSE);
    gainopts.SetModified(FALSE);
    orientopts.SetModified(FALSE);
    precisionopts.SetModified(FALSE);
    sensopts.SetModified(FALSE);
}