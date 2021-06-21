
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include "main.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <wchar.h>

#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))

/** Prototypes **************************************************************/

static INT_PTR CALLBACK MainDlgProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL WMIGetStorageInfo ( HWND hWnd );
extern BOOL WMIGetCPUInfo ( HWND hWnd );
static void BeginDraw ( HWND hWnd );
static void EndDraw ( HWND hWnd );

/** Global variables ********************************************************/

static HANDLE ghInstance;
static HWND ghEd;


/*-@@+@@--------------------------------------------------------------------*/
//       Function: wWinMain 
/*--------------------------------------------------------------------------*/
//           Type: int APIENTRY 
//    Param.    1: HINSTANCE hInstance    : 
//    Param.    2: HINSTANCE hPrevInstance: 
//    Param.    3: WCHAR *pszCmdLine      : 
//    Param.    4: int nCmdShow           : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 19.06.2021
//    DESCRIPTION: <lol>
//
/*--------------------------------------------------------------------@@-@@-*/
int APIENTRY wWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    WCHAR *pszCmdLine, int nCmdShow )
/*--------------------------------------------------------------------------*/
{
    INITCOMMONCONTROLSEX icc;
    WNDCLASSEX wcx;

    ghInstance = hInstance;

    icc.dwSize = sizeof ( icc );
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx ( &icc );

    /* Get system dialog information */
    wcx.cbSize = sizeof ( wcx );
    if ( !GetClassInfoExW ( NULL, MAKEINTRESOURCE(32770), &wcx) )
        return 0;

    /* Add our own stuff */
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIconW ( hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN) );
    wcx.lpszClassName = L"hdidClass";

    if ( !RegisterClassExW (&wcx) )
        return 0;

    /* The user interface is a modal dialog box */
    return DialogBoxW ( hInstance, MAKEINTRESOURCE(DLG_MAIN), 
        NULL, (DLGPROC)MainDlgProc );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MainDlgProc 
/*--------------------------------------------------------------------------*/
//           Type: static INT_PTR CALLBACK 
//    Param.    1: HWND hwndDlg : 
//    Param.    2: UINT uMsg    : 
//    Param.    3: WPARAM wParam: 
//    Param.    4: LPARAM lParam: 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 19.06.2021
//    DESCRIPTION: <lol>
//
/*--------------------------------------------------------------------@@-@@-*/
static INT_PTR CALLBACK MainDlgProc ( HWND hwndDlg, UINT uMsg, 
    WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            ghEd = GetDlgItem ( hwndDlg, IDC_ED );
            SendMessage ( ghEd, WM_SETTEXT, (WPARAM)0, (LPARAM)L"" );
            WMIGetCPUInfo ( GetDlgItem ( hwndDlg, IDC_CPU ) );
            WMIGetStorageInfo ( ghEd );
            SendMessage ( ghEd, EM_SETSEL, (WPARAM)0, (LPARAM)0 );
            SendMessage ( ghEd, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0 );
            SetFocus ( ghEd );
            return FALSE;
            
        case WM_COMMAND:
            switch ( GET_WM_COMMAND_ID(wParam, lParam) )
            {
                case IDOK:
                    EndDialog ( hwndDlg, TRUE );
                    return TRUE;

                case IDCANCEL:
                    EndDialog ( hwndDlg, FALSE );
                    return TRUE;

                case IDC_REFRESH:
                    // disable redraw for the update duration
                    BeginDraw ( ghEd );
                    SendMessage ( ghEd, WM_SETTEXT, 0, (LPARAM)L"" );
                    WMIGetStorageInfo ( ghEd );
                    SendMessage ( ghEd, EM_SETSEL, (WPARAM)0, (LPARAM)0 );
                    SendMessage ( ghEd, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0 );
                    // re-enable draw and force a repaint
                    EndDraw ( ghEd );
                    SetFocus ( ghEd );
                    return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog ( hwndDlg, 0 );
            return TRUE;
    }

    return FALSE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: BeginDraw 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hwnd : handle to window
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 28.09.2020
//    DESCRIPTION: Mark the beginning of a lengthy operation. Use before some
//                 long updates in a list, edit control etc. to avoid flicker
/*--------------------------------------------------------------------@@-@@-*/
static void BeginDraw ( HWND hWnd )
/*--------------------------------------------------------------------------*/
{
    SendMessage ( hWnd, WM_SETREDRAW, FALSE, 0 );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: EndDraw 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hwnd : HWND to window
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 28.09.2020
//    DESCRIPTION: Mark the end of a lengthy operation. Generates a repaint
/*--------------------------------------------------------------------@@-@@-*/
static void EndDraw ( HWND hWnd )
/*--------------------------------------------------------------------------*/
{
    SendMessage ( hWnd, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect ( hWnd, NULL, TRUE );
}
