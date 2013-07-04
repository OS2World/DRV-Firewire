#ifndef fwresmg_def

#define fwresmg_def

/*****************/
/* WindowID 1-1  */
#define ID_FrameWindow           1
#define ID_DRIVERTEXT            2

#define APP_width           400
#define APP_hight           300

/***************/
/* Menu  50-99 */
#define ID_MainMenu             1
#define IDM_File                2
#define IDM_Language            4
#define IDM_LANG_BASE         100
#define IDM_Exit               73
#define IDM_Report             74
#define IDM_Help               10
#define IDM_About              11
#define WM_REBUILDMENU      WM_USER + 1
#define WM_UPDATEDRIVERTEXT WM_USER + 2

/***********/
/* Bitmaps */
#define ID_SPLASH       200
#define SPLASH_width    216
#define SPLASH_hight    216


/*********/
/* Timer */
#define ID_SPLASH_Timer   1



/************************/
/* character code stuff */
typedef struct _CHARMSG    /* charmsg */
   {
      USHORT  fs;           /* mp1     */
      UCHAR   cRepeat;
      UCHAR   scancode;
      USHORT  chr;          /* mp2     */
      USHORT  vkey;
   } CHRMSG;
typedef CHRMSG *PCHRMSG;

#define CHARMSG(pmsg) ((PCHRMSG)((PBYTE)pmsg + sizeof(MPARAM) ))

#endif
