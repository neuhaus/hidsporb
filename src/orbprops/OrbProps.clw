; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSensitivityPage
LastTemplate=CPropertyPage
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "OrbProps.h"

ClassCount=10
Class1=COrbPropsApp
Class2=COrbPropsDlg
Class3=CAboutDlg

ResourceCount=10
Resource1=IDD_ORBPROPS_DIALOG
Resource2=IDR_MAINFRAME
Resource3=IDD_Prop_Axes
Resource4=IDD_Prop_Gain
Resource5=IDD_Prop_Buttons
Resource6=IDD_Prop_Sensitivity
Resource7=IDD_Prop_Precision
Resource8=IDD_Prop_Orientation
Class4=CSensitivityPage
Class5=CAxesPage
Class6=CPrecisionPage
Class7=CGainPage
Class8=COrientationPage
Class9=CButtonsPage
Resource9=IDD_ABOUTBOX
Class10=CBatchPage
Resource10=IDD_Prop_Batchfile

[CLS:COrbPropsApp]
Type=0
HeaderFile=OrbProps.h
ImplementationFile=OrbProps.cpp
Filter=N

[CLS:COrbPropsDlg]
Type=0
HeaderFile=OrbPropsDlg.h
ImplementationFile=OrbPropsDlg.cpp
Filter=D
LastObject=COrbPropsDlg

[CLS:CAboutDlg]
Type=0
HeaderFile=OrbPropsDlg.h
ImplementationFile=OrbPropsDlg.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=7
Control1=IDC_STATIC,static,1342308492
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_VERSION,static,1342308352
Control5=IDC_STATIC,static,1342177294
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[DLG:IDD_ORBPROPS_DIALOG]
Type=1
Class=COrbPropsDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=ID_HELP,button,1342242816

[DLG:IDD_Prop_Axes]
Type=1
Class=CAxesPage
ControlCount=14
Control1=IDC_AXES_IMAGE,static,1342177294
Control2=IDC_YAXIS,combobox,1344340226
Control3=IDC_INVERT_YAXIS,button,1342242819
Control4=IDC_STATIC,static,1342308352
Control5=IDC_ZROT,combobox,1344340226
Control6=IDC_INVERT_ZROT,button,1342242819
Control7=IDC_XAXIS,combobox,1344340226
Control8=IDC_INVERT_XAXIS,button,1342242819
Control9=IDC_YROT,combobox,1344340226
Control10=IDC_INVERT_YROT,button,1342242819
Control11=IDC_XROT,combobox,1344340226
Control12=IDC_INVERT_XROT,button,1342242819
Control13=IDC_ZAXIS,combobox,1344340226
Control14=IDC_INVERT_ZAXIS,button,1342242819

[DLG:IDD_Prop_Buttons]
Type=1
Class=CButtonsPage
ControlCount=11
Control1=IDC_BUTTONS_IMAGE,static,1342177294
Control2=IDC_BUTTONA,combobox,1344340226
Control3=IDC_BUTTONB,combobox,1344340226
Control4=IDC_BUTTONC,combobox,1344340226
Control5=IDC_BUTTOND,combobox,1344340226
Control6=IDC_BUTTONE,combobox,1344340226
Control7=IDC_BUTTONF,combobox,1344340226
Control8=IDC_STATIC,button,1342177287
Control9=IDC_ENABLE_CHORDING,button,1342242819
Control10=IDC_SelectedChord,combobox,1344339970
Control11=IDC_STATIC,static,1342308352

[DLG:IDD_Prop_Orientation]
Type=1
Class=COrientationPage
ControlCount=4
Control1=IDC_VERTICAL,button,1342308361
Control2=IDC_HORIZONTAL,button,1342177289
Control3=IDC_STATIC,static,1342177294
Control4=IDC_STATIC,static,1342177294

[DLG:IDD_Prop_Sensitivity]
Type=1
Class=CSensitivityPage
ControlCount=11
Control1=IDC_AXES_IMAGE,static,1342177294
Control2=IDC_SENS_YAXIS,msctls_trackbar32,1342242825
Control3=IDC_USE_MASTER_SENS,button,1342242819
Control4=IDC_SENS_ZROT,msctls_trackbar32,1342242825
Control5=IDC_SENS_XAXIS,msctls_trackbar32,1342242825
Control6=IDC_SENS_YROT,msctls_trackbar32,1342242825
Control7=IDC_SENS_XROT,msctls_trackbar32,1342242825
Control8=IDC_SENS_ZAXIS,msctls_trackbar32,1342242825
Control9=IDC_SENSITIVITY,msctls_trackbar32,1342242825
Control10=IDC_NULLREGION,msctls_trackbar32,1342242825
Control11=IDC_STATIC,static,1342308352

[DLG:IDD_Prop_Gain]
Type=1
Class=CGainPage
ControlCount=9
Control1=IDC_AXES_IMAGE,static,1342177294
Control2=IDC_GAIN_YAXIS,msctls_trackbar32,1342242825
Control3=IDC_USE_MASTER_GAIN,button,1342242819
Control4=IDC_GAIN_ZROT,msctls_trackbar32,1342242825
Control5=IDC_GAIN_XAXIS,msctls_trackbar32,1342242825
Control6=IDC_GAIN_YROT,msctls_trackbar32,1342242825
Control7=IDC_GAIN_XROT,msctls_trackbar32,1342242825
Control8=IDC_GAIN_ZAXIS,msctls_trackbar32,1342242825
Control9=IDC_GAIN,msctls_trackbar32,1342242825

[DLG:IDD_Prop_Precision]
Type=1
Class=CPrecisionPage
ControlCount=15
Control1=IDC_BUTTONS_IMAGE,static,1342177294
Control2=IDC_STATIC,button,1342177287
Control3=IDC_PRECISION_A,button,1342308361
Control4=IDC_PRECISION_B,button,1342177289
Control5=IDC_PRECISION_C,button,1342177289
Control6=IDC_PRECISION_D,button,1342177289
Control7=IDC_PRECISION_E,button,1342177289
Control8=IDC_PRECISION_F,button,1342177289
Control9=IDC_PRECISION_LOGICAL,button,1342177289
Control10=IDC_LOGICAL_BUTTON,combobox,1344340226
Control11=IDC_PRECISION_GAIN,msctls_trackbar32,1342242825
Control12=IDC_STATIC,static,1342308352
Control13=IDC_PRECISION_SENSITIVITY,msctls_trackbar32,1342242825
Control14=IDC_STATIC,static,1342308352
Control15=IDC_ENABLE_PRECISION,button,1342242819

[CLS:CSensitivityPage]
Type=0
HeaderFile=SensitivityPage.h
ImplementationFile=SensitivityPage.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=IDC_USE_MASTER_SENS

[CLS:CAxesPage]
Type=0
HeaderFile=AxesPage.h
ImplementationFile=AxesPage.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=IDC_INVERT_XAXIS

[CLS:CGainPage]
Type=0
HeaderFile=GainPage.h
ImplementationFile=GainPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_USE_MASTER_GAIN
VirtualFilter=idWC

[CLS:COrientationPage]
Type=0
HeaderFile=OrientationPage.h
ImplementationFile=OrientationPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_VERTICAL
VirtualFilter=idWC

[CLS:CPrecisionPage]
Type=0
HeaderFile=PrecisionPage.h
ImplementationFile=PrecisionPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_PRECISION_SENSITIVITY
VirtualFilter=idWC

[CLS:CButtonsPage]
Type=0
HeaderFile=ButtonsPage.h
ImplementationFile=ButtonsPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_ENABLE_CHORDING
VirtualFilter=idWC

[DLG:IDD_Prop_Batchfile]
Type=1
Class=CBatchPage
ControlCount=3
Control1=IDC_BATCH_COMMANDS,listbox,1352730881
Control2=IDC_SAVE_BATCH,button,1342242816
Control3=IDC_LOAD_BATCH,button,1342242816

[CLS:CBatchPage]
Type=0
HeaderFile=BatchPage.h
ImplementationFile=BatchPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_BATCH_COMMANDS
VirtualFilter=idWC

