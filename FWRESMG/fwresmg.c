/*********************************************/
/* Firewire Resource Manager                 */
/* Autor: Robert Henschel                    */
/* created : 17.07.2004                      */
/* modified: 05.08.2004                      */

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#define __MULTI__

/**************/
/* Headerfile */
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include "fwresmg.h"
#include "RHNLS.h"
#include "global.h"
#include "fwcom.h"

/*************/
/* Functions */
MRESULT EXPENTRY ClientWindowProc(HWND,ULONG,MPARAM,MPARAM);
MRESULT EXPENTRY ClientWindowProcSplash(HWND,ULONG,MPARAM,MPARAM);
VOID WinSetWindowFontMy(HWND hwnd, PSZ pszFontName);
VOID UtlSetMleFont( HWND hwndParent, USHORT usMleId, PSZ szFacename, USHORT usPointSize, LONG lMaxHeight, USHORT fsSelection );

/********************/
/* Global variables */
HWND  hwndDriverText, hwndClientWindow, hwndSplashWindow, hwndFrameWindow, hwndFrameWindowSplash;
HAB   hab;
ULONG SplashStyleFlags;
BOOL  isAbout = FALSE;

/********/
/* MAIN */
INT main (VOID)
{
	BOOL  RC;
   HMQ   hmq;
   QMSG  qmsg;
   LONG width, hight;

   ULONG ClientStyleFlags = FCF_TITLEBAR   |   FCF_SYSMENU  |  FCF_SIZEBORDER  | FCF_MINBUTTON  |
                            FCF_TASKLIST   |   FCF_MENU | FCF_ICON;
   SplashStyleFlags = FCF_BORDER  | FCF_SYSMODAL;
   hab = WinInitialize(0);
   hmq = WinCreateMsgQueue(hab,0);
   WinRegisterClass(hab, "MainWindow", (PFNWP)ClientWindowProc, CS_SIZEREDRAW, 0);
   WinRegisterClass(hab, "SplashWindow", (PFNWP)ClientWindowProcSplash, CS_SIZEREDRAW, 0);
   hwndFrameWindow = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE,
                                         &ClientStyleFlags, "MainWindow",
                                         "Firewire Resource Manager", WS_VISIBLE,
                                         (HMODULE)0, ID_FrameWindow,
                                         &hwndClientWindow);
   hwndFrameWindowSplash = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE,
                                         &SplashStyleFlags, "SplashWindow",
                                         "Firewire Resource Manager Splash Screen", WS_VISIBLE,
                                         (HMODULE)0, ID_FrameWindow,
                                         &hwndSplashWindow);

   width = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
   hight = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
   RC=WinSetWindowPos(hwndFrameWindow, HWND_TOP, (width / 2) - (APP_width / 2), (hight / 2) - (APP_hight / 2), APP_width, APP_hight, SWP_ACTIVATE | SWP_SHOW | SWP_SIZE | SWP_MOVE);
   RC=WinSetWindowPos(hwndFrameWindowSplash, HWND_TOP, (width / 2) - (SPLASH_width / 2), (hight / 2) - (SPLASH_hight / 2), SPLASH_width, SPLASH_hight, SWP_ACTIVATE | SWP_SHOW | SWP_SIZE | SWP_MOVE);

   while (WinGetMsg(hab,&qmsg,0L,0,0))
      WinDispatchMsg(hab,&qmsg);

   WinDestroyWindow(hwndFrameWindow);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}

/********************/
/* ClientWindowProc */
MRESULT EXPENTRY ClientWindowProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
	static SHORT MaxXClient, MaxYClient;
	HPS    hps;
	POINTL ptl;
	//ULONG  DialogResult;

	switch (msg)
	{
		case WM_CREATE: {
			pSupportedLanguages sl;
			// get all supported languages
			sl=getSupportedLanguages();
			//set the first language as active
			setLanguage("English");
			WinPostMsg(hwnd, WM_REBUILDMENU, NULL, NULL);
			hwndDriverText = WinCreateWindow(hwnd, WC_MLE, "gathering information...", WS_VISIBLE | MLS_HSCROLL | MLS_VSCROLL | MLS_READONLY, 1, 1, 100, 100, hwnd, HWND_TOP, ID_DRIVERTEXT, NULL, NULL);
			UtlSetMleFont( hwnd, ID_DRIVERTEXT, "Courier", 8, 12, 0);
			drivertext = NULL;
			_beginthread(doFWioctl, NULL, 8192, (void *)hwnd);
		} break;

		case WM_SIZE:
			MaxXClient = SHORT1FROMMP(mp2);
			MaxYClient = SHORT2FROMMP(mp2);
			WinSetWindowPos(hwndDriverText, HWND_TOP, 0, 0, MaxXClient, MaxYClient, SWP_ACTIVATE | SWP_SHOW | SWP_SIZE | SWP_MOVE);
		break;

		case WM_PAINT:
			hps = WinBeginPaint (hwnd,NULLHANDLE,NULL);
			/* draw the background */
			ptl.x = 0; ptl.y = 0;
			GpiMove(hps, &ptl);
			ptl.x = MaxXClient;
			ptl.y = MaxYClient;
			GpiSetColor(hps, CLR_PALEGRAY);
			GpiBox(hps, DRO_OUTLINEFILL, &ptl,0,0);
			WinEndPaint(hps);
		break;

		case WM_UPDATEDRIVERTEXT: {
			IPT ipt;
			ULONG count = 100;
			ipt = 0;
			WinSendMsg(hwndDriverText, MLM_DELETE, (MPARAM)ipt, (MPARAM)count);
			WinSendMsg(hwndDriverText, MLM_SETIMPORTEXPORT, drivertext, (MPARAM)strlen(drivertext));
			WinSendMsg(hwndDriverText, MLM_IMPORT, &ipt, &count);
		} break;

		case WM_REBUILDMENU: {
			PSZ title;
			pSupportedLanguages sl;
			SHORT i = 0;
			HWND hwndLanguage;
			MENUITEM mi;

			sl=getSupportedLanguages();
			WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMTEXT, MPFROMSHORT(IDM_File), getNLSString("MainMenu.Test"));
			WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMTEXT, MPFROMSHORT(IDM_Language), getNLSString("MainMenu.Test.Language"));
			WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMTEXT, MPFROMSHORT(IDM_Report), getNLSString("MainMenu.Test.Report"));
			WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMTEXT, MPFROMSHORT(IDM_Exit), getNLSString("MainMenu.Test.Exit"));
			/* Help */
			WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMTEXT, MPFROMSHORT(IDM_Help), getNLSString("MainMenu.Help"));
			WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMTEXT, MPFROMSHORT(IDM_About), getNLSString("MainMenu.Help.About"));

			WinSendMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_QUERYITEM, MPFROM2SHORT(IDM_Language, TRUE), (MPARAM) &mi);
			hwndLanguage = mi.hwndSubMenu;
			
			// remove all languages from the submenu
			while(sl != NULL) {
				WinSendMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_DELETEITEM, MPFROM2SHORT(IDM_LANG_BASE + i, TRUE), NULL);
				sl = (pSupportedLanguages)sl->next;
				i++;
			}
			i = 0;
			//add all available languages
			sl=getSupportedLanguages();
			while(sl != NULL) {
				mi.iPosition = MIT_END;
				mi.afStyle = MIS_TEXT;
				mi.afAttribute = 0;
				mi.id = IDM_LANG_BASE + i;
				mi.hwndSubMenu = NULL;
				mi.hItem = 0;
				WinSendMsg(hwndLanguage, MM_INSERTITEM, (MPARAM) &mi, (MPARAM) sl->englishName);
				if (sl->id == getLanguage())
					WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMATTR, MPFROM2SHORT(IDM_LANG_BASE + i, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
				else
					WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMATTR, MPFROM2SHORT(IDM_LANG_BASE + i, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
				sl = (pSupportedLanguages)sl->next;
				i++;
			}

			title = calloc(100,1);
			//sprintf(title, "%s - %s", getNLSString("Application.Title"), getCurrentLanguageName());
			sprintf(title, "%s", getNLSString("Application.Title"));
			WinSetWindowText(hwndFrameWindow, title);
			free(title);
		} break;

		case WM_COMMAND: {
			pSupportedLanguages sl;
			int i = 0;
			BOOL didSomething;

			switch (SHORT1FROMMP(mp1)) {
				case IDM_Exit:
					WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
				break;
				case IDM_Report: {
					FILEDLG fild;
					char pszFullFile[CCHMAXPATH] = "*.txt";
					HWND hwndDlg;
					/* Initially set all fields to 0 */
					memset(&fild, 0, sizeof(FILEDLG));
					/* Initialize those fields that are used */
					fild.cbSize = sizeof(FILEDLG);
					fild.fl = FDS_CENTER | FDS_SAVEAS_DIALOG;
					fild.pszTitle = getNLSString("CreateReportDialog.Title");
					fild.pszOKButton = getNLSString("CreateReportDialog.OKButtonText");
					/* Initial path, file name, or file filter */
					strcpy(fild.szFullFile, pszFullFile);
					hwndDlg = WinFileDlg(HWND_DESKTOP, hwnd, &fild);
					if (hwndDlg && (fild.lReturn == DID_OK)) {
						HFILE  hfFileHandle   = 0L;
						ULONG  ulAction       = 0;
						ULONG  ulWrote        = 0;
						APIRET rc;
						rc = DosOpen(fild.szFullFile, &hfFileHandle, &ulAction, 0L,
									FILE_ARCHIVED | FILE_NORMAL,
									OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS,
									OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE  | OPEN_ACCESS_READWRITE,
									0L);
						if (rc != 0) {
							rc = DosClose(hfFileHandle);
							rc = WinMessageBox(HWND_DESKTOP, hwnd, getNLSString("CreateReportDialog.Overwrite"), getNLSString("CreateReportDialog.FileExists"), 0, MB_ICONQUESTION | MB_YESNO);
							if (rc == MBID_YES) {
								rc = DosOpen(fild.szFullFile, &hfFileHandle, &ulAction, 0L,
									FILE_ARCHIVED | FILE_NORMAL,
									OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
									OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE  | OPEN_ACCESS_READWRITE,
									0L);
								if (rc != 0) {
									WinMessageBox(HWND_DESKTOP, hwnd, getNLSString("CreateReportDialog.ErrorOverwrite"), getNLSString("CreateReportDialog.Error"), 0, MB_ERROR | MB_OK);
									break;
								}
							} else {
								WinPostMsg(hwnd, msg, mp1, mp2);
								break;
							}
						}
						rc = DosWrite (hfFileHandle, (PVOID)drivertext, strlen(drivertext), &ulWrote);
						if (rc != 0) {
							WinMessageBox(HWND_DESKTOP, hwnd, getNLSString("CreateReportDialog.ErrorWriting"), getNLSString("CreateReportDialog.Error"), 0, MB_ERROR | MB_OK);
						}
						rc = DosClose(hfFileHandle);
					}
				} break;
				case IDM_About: {
					LONG width, hight;
					isAbout = TRUE;
					hwndFrameWindowSplash = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE,
                                         &SplashStyleFlags, "SplashWindow",
                                         "Firewire Resource Manager Splash Screen", WS_VISIBLE,
                                         (HMODULE)0, ID_FrameWindow,
                                         &hwndSplashWindow);
					width = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
					hight = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
					WinSetWindowPos(hwndFrameWindowSplash, HWND_TOP, (width / 2) - (SPLASH_width / 2), (hight / 2) - (SPLASH_hight / 2), SPLASH_width, SPLASH_hight, SWP_ACTIVATE | SWP_SHOW | SWP_SIZE | SWP_MOVE);
				} break;
			}
			//check for the language menus
			sl=getSupportedLanguages();
			didSomething = FALSE;
			i = 0;
			while(sl != NULL) {
				if (sl->id + IDM_LANG_BASE == (SHORT1FROMMP(mp1))) {
					WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMATTR, MPFROM2SHORT(SHORT1FROMMP(mp1), TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
					setLanguage(sl->englishName);
					didSomething = TRUE;
				} else {
					WinPostMsg(WinWindowFromID(hwndFrameWindow, FID_MENU), MM_SETITEMATTR, MPFROM2SHORT(SHORT1FROMMP(mp1), TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
				}
				sl = (pSupportedLanguages)sl->next;
				i++;
			}
			if (didSomething) WinPostMsg(hwnd, WM_REBUILDMENU, NULL, NULL);
		} break;

		case WM_CLOSE:
			WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
			free(drivertext);
		break;

		default:
			return WinDefWindowProc (hwnd,msg,mp1,mp2);
		break;
	}/* EndMessageSwitch */
	return (MRESULT)FALSE;
}

/**************************/
/* ClientWindowProcSplash */
MRESULT EXPENTRY ClientWindowProcSplash (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
	static SHORT MaxXClient, MaxYClient;
	HBITMAP hbm;
	HPS    hps;
	POINTL ptl;
	//ULONG  DialogResult;

	switch (msg)
	{
		case WM_CREATE: {
			/* OS/2 Logo settings could be retrieved from INI file...
				Application name "PM_ControlPanel"
				Key name "LogoDisplayTime"
				Type integer
				Content/value -1 <= time <= 32767 milliseconds.
					Indefinite display  -1
					No display   0
					Timed display  >0 */
			/* for more information search for "logo" in PMREF */
			WinStartTimer(hab, hwnd, ID_SPLASH_Timer, 2000); //2000
		} break;

		case WM_SIZE:
			MaxXClient = SHORT1FROMMP(mp2);
			MaxYClient = SHORT2FROMMP(mp2);
		break;

		case WM_TIMER:
			WinStopTimer(hab, hwnd, ID_SPLASH_Timer);
			if (!(isAbout)) {
				WinSetWindowPos(hwndFrameWindow, HWND_TOP, (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) / 2) - (APP_width / 2), (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) / 2) - (APP_hight / 2), APP_width, APP_hight, SWP_ACTIVATE | SWP_SHOW );
				WinDestroyWindow(hwndFrameWindowSplash);
			}
		break;

		case WM_PAINT:
			hps = WinBeginPaint (hwnd,NULLHANDLE,NULL);
			/* draw the background */
			ptl.x = 0; ptl.y = 0;
			GpiMove(hps, &ptl);
			ptl.x = MaxXClient;
			ptl.y = MaxYClient;
			GpiSetColor(hps, CLR_PALEGRAY);
			GpiBox(hps, DRO_OUTLINEFILL, &ptl,0,0);
			ptl.x = 0;
			ptl.y = 0;
			hbm = GpiLoadBitmap(hps, NULLHANDLE, ID_SPLASH, 216, 216);
			WinDrawBitmap(hps, hbm, NULL, (PPOINTL)&ptl, 0L, 0L, DBM_NORMAL);
			WinEndPaint(hps);
		break;

		case WM_BUTTON1DOWN:
		case WM_BUTTON2DOWN:
		case WM_BUTTON3DOWN:
			if (isAbout) {
				WinStopTimer(hab, hwnd, ID_SPLASH_Timer);
				WinSetWindowPos(hwndFrameWindow, HWND_TOP, (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) / 2) - (APP_width / 2), (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) / 2) - (APP_hight / 2), APP_width, APP_hight, SWP_ACTIVATE | SWP_SHOW );
				WinDestroyWindow(hwndFrameWindowSplash);
			}
		break;

		case WM_CHAR:
			if (isAbout) {
				if( CHARMSG( &msg)->vkey == VK_ESC) {
					WinStopTimer(hab, hwnd, ID_SPLASH_Timer);
					WinSetWindowPos(hwndFrameWindow, HWND_TOP, (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) / 2) - (APP_width / 2), (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) / 2) - (APP_hight / 2), APP_width, APP_hight, SWP_ACTIVATE | SWP_SHOW );
					WinDestroyWindow(hwndFrameWindowSplash);
				}
			}
		break;

		case WM_COMMAND: {
			switch (SHORT1FROMMP(mp1)) {
				case IDM_Exit:
					WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
				break;
			}
		} break;

		case WM_CLOSE:
			WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
		break;

		default:
			return WinDefWindowProc (hwnd,msg,mp1,mp2);
		break;
	}/* EndMessageSwitch */
	return (MRESULT)FALSE;
}

/* WinSetWindowFontMy */
VOID WinSetWindowFontMy(HWND hwnd, PSZ pszFontName)
{
   BOOL brc = FALSE;
   /* pszFontName format - "10.Helv"  "12.Tms Rmn" "14.Tms Rmn" */

   brc = WinSetPresParam (hwnd, PP_FONTNAMESIZE,
         (ULONG)((ULONG)strlen(pszFontName)+(ULONG)1), (PSZ)pszFontName);
   return;
}

/*OS/2 Programming FAq, part 6*/
VOID UtlSetMleFont( HWND hwndParent, USHORT usMleId, PSZ szFacename, USHORT usPointSize, LONG lMaxHeight, USHORT fsSelection ) {
	PFONTMETRICS pfm;
	HDC          hdc;
	HPS          hps;
	HWND         hwndMle;
	LONG         lHorzRes, lVertRes, lRequestFonts = 0, lNumberFonts;
	FATTRS       fat;
	SHORT        sOutlineIndex = -1;
	INT          i;

	(void) memset( &fat, 0, sizeof( FATTRS ) );
	fat.usRecordLength  = sizeof( FATTRS );
	fat.fsSelection  = fsSelection;
	strcpy( fat.szFacename, szFacename );
	hwndMle = WinWindowFromID( hwndParent, usMleId );
	hps = WinGetPS( hwndMle );
	hdc = GpiQueryDevice( hps );
	DevQueryCaps( hdc, CAPS_HORIZONTAL_FONT_RES, 1L, &lHorzRes );
	DevQueryCaps( hdc, CAPS_VERTICAL_FONT_RES,   1L, &lVertRes );
	lNumberFonts = GpiQueryFonts( hps, QF_PUBLIC, szFacename, &lRequestFonts, 0L, NULL);
	pfm = malloc( (SHORT) lNumberFonts * sizeof( FONTMETRICS ) );
	GpiQueryFonts( hps, QF_PUBLIC, szFacename, &lNumberFonts, (LONG) sizeof( FONTMETRICS ), pfm );
	for( i = 0; i < (USHORT) lNumberFonts; i++ ) {
		if( pfm[ i ].fsDefn & 1 ) {
			sOutlineIndex = (SHORT) i;
			continue;
		}
		if (pfm[ i ].sXDeviceRes == (SHORT) lHorzRes &&
			pfm[ i ].sYDeviceRes == (SHORT) lVertRes &&
			pfm[ i ].sNominalPointSize == (SHORT) (usPointSize * 10) ) {
			fat.lMatch          = pfm[ i ].lMatch;
			fat.lMaxBaselineExt = pfm[ i ].lMaxBaselineExt;
			fat.lAveCharWidth   = pfm[ i ].lAveCharWidth;
			break;
		}
	}

	if( i >= (USHORT) lNumberFonts )
		if( sOutlineIndex >= 0 )
		if( lMaxHeight ) {
			fat.fsFontUse = FATTR_FONTUSE_OUTLINE;
			if( !(fat.usCodePage = pfm[ sOutlineIndex ].usCodePage) )
				fat.usCodePage  = 850;
			fat.lMaxBaselineExt = lMaxHeight;
			WinSendMsg( hwndMle, MLM_SETFONT, MPFROMP( &fat ), 0 );
		}
	WinSendMsg( hwndMle, MLM_SETFONT, MPFROMP( &fat ), 0 );
	WinReleasePS( hps );
	free( pfm );
}
