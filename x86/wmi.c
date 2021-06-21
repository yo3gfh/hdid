#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include "main.h"

#define _WIN32_DCOM

#include <windows.h>
#include <Wbemidl.h>
#include <strsafe.h>
#include <wchar.h>

BOOL EBAddLine ( HWND hWnd, WCHAR * pwStr );
const WCHAR * wswap ( const WCHAR * src );
BOOL WMIGetStorageInfo ( HWND hWnd );
BOOL WMIGetCPUInfo ( HWND hWnd );

/*-@@+@@--------------------------------------------------------------------*/
//       Function: EBAddLine 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hWnd     : handle to edit control
//    Param.    2: WCHAR * pwStr : ptr. to widestring
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 19.06.2021
//    DESCRIPTION: adds a line to a edit box
/*--------------------------------------------------------------------@@-@@-*/
BOOL EBAddLine ( HWND hWnd, WCHAR * pwStr )
/*--------------------------------------------------------------------------*/
{
    if ( hWnd == NULL || pwStr == NULL )
        return FALSE;

    SendMessage ( hWnd,  EM_REPLACESEL, 0, ( LPARAM )pwStr );

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: wswap 
/*--------------------------------------------------------------------------*/
//           Type: const WCHAR * 
//    Param.    1: const WCHAR * src : source string 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 19.06.2021
//    DESCRIPTION: swap pairs of bytes in the source string and return 
//                 another string with the result. Does not work if src
//                 is an odd # of bytes or if it contains -, _, /
//                 Example: 0520B677C5505E41 -> 50026B775C05E514
/*--------------------------------------------------------------------@@-@@-*/
const WCHAR * wswap ( const WCHAR * src )
/*--------------------------------------------------------------------------*/
{
    static WCHAR    res[1024];
    UINT_PTR        i, len;

    res[0] = L'\0';

    if ( src == NULL )
        return res;

    len = lstrlenW ( src );

    if ( len > ARRAYSIZE(res) )
        len = ARRAYSIZE(res);

    // abort if odd nelem.
    if ( len & 1 )
        return src;

    i = 0;

    while ( i < len-1 )
    {
        // abort if any char is not 
        // alphanumeric, no purpose in
        // swapping garbage :-)
        if ( src[i] == L'-' || src[i+1] == L'-' )
            return src;

        if ( src[i] == L'_' || src[i+1] == L'_' )
            return src;

        if ( src[i] == L'/' || src[i+1] == L'/' )
            return src;

        res[i] = src[i+1];
        res[i+1] = src[i];

        i += 2;
    }

    return res;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: WMIGetStorageInfo 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hWnd : hwnd to edit box to receive results
/*--------------------------------------------------------------------------*/
//         AUTHOR: kind ppl from web / Adrian Petrila, YO3GFH
//           DATE: 19.06.2021
//    DESCRIPTION: well, where do i start :))
//                 converted to C from StackOverflow examples, initializes
//                 WMI and enums disk drives and their properties. It could be
//                 made in a way more flexible, general purpose function. Also
//                 could be broken in smaller parts, it's bloody humongous..
//                 TODO :)
/*--------------------------------------------------------------------@@-@@-*/
BOOL WMIGetStorageInfo ( HWND hWnd )
/*--------------------------------------------------------------------------*/
{
    WCHAR                   buf[2048];
    WCHAR                   wszDevID[2048];
    HRESULT                 hres;
    BSTR                    bstr;
    BSTR                    bstrLanguage;
    BSTR                    bstrQuery;
    VARIANT                 vtProp;
    ULONG                   uReturn         = 0;
    float                   s;
    IWbemLocator            * pLoc          = NULL;
    IWbemServices           * pSvc          = NULL;
    IEnumWbemClassObject    * pEnumerator   = NULL;
    IEnumWbemClassObject    * pEnumerator2  = NULL;
    IEnumWbemClassObject    * pEnumerator3  = NULL;
    IWbemClassObject        * pclsObj       = NULL;
    IWbemClassObject        * pclsObj2      = NULL;
    IWbemClassObject        * pclsObj3      = NULL;

    // init COM
    hres = CoInitializeEx ( 0, COINIT_MULTITHREADED );

    if ( FAILED(hres) )
        return FALSE;

    // Set general COM security levels
    hres = CoInitializeSecurity ( NULL, 
        -1,                          // COM authentication 
        NULL,                        // Authentication services 
        NULL,                        // Reserved 
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication  
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation   
        NULL,                        // Authentication info 
        EOAC_NONE,                   // Additional capabilities  
        NULL                         // Reserved 
        );

    if ( FAILED(hres) )
    {
        CoUninitialize();
        return FALSE;
    }

    // Obtain the initial locator to WMI
    hres = CoCreateInstance ( &CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER, 
        &IID_IWbemLocator, 
        (LPVOID *) &pLoc );

    if ( FAILED(hres) )
    {
        CoUninitialize();
        return FALSE;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    bstr = SysAllocString ( L"ROOT\\CIMV2" );
    hres = pLoc->lpVtbl->ConnectServer ( pLoc, 
        bstr,       // Object path of WMI namespace 
        NULL,       // User name. NULL = current user 
        NULL,       // User password. NULL = current 
        0,          // Locale. NULL indicates current 
        0,          // Security flags. 
        0,          // Authority (for example, Kerberos) 
        0,          // Context object  
        &pSvc       // pointer to IWbemServices proxy 
    );

    SysFreeString ( bstr );

    if ( FAILED (hres) )
    {
        pLoc->lpVtbl->Release ( pLoc );  
        CoUninitialize();
        return FALSE;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket( 
        (LPUNKNOWN)pSvc,             // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
        NULL,                        // Server principal name  
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx  
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx 
        NULL,                        // client identity 
        EOAC_NONE                    // proxy capabilities 
    );

    if ( FAILED(hres) )
    {
        pSvc->lpVtbl->Release ( pSvc );
        pLoc->lpVtbl->Release ( pLoc );  
        CoUninitialize();
        return FALSE;
    }

    // Execute the query
    bstrLanguage = SysAllocString ( L"WQL" );
    bstrQuery    = SysAllocString ( L"SELECT Model, MediaType, "
        "DeviceID, SerialNumber, Size, InterfaceType FROM Win32_DiskDrive" );

    hres = pSvc->lpVtbl->ExecQuery ( pSvc, 
        bstrLanguage,
        bstrQuery, 
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, 
        &pEnumerator 
    );

    SysFreeString ( bstrLanguage );
    SysFreeString ( bstrQuery );

    // Check for errors.
    if ( FAILED(hres) )
    {
        pSvc->lpVtbl->Release(pSvc);
        pLoc->lpVtbl->Release(pLoc);
        CoUninitialize();    

        return FALSE;
    }

    // Enumerate returned objects and get data
    while ( 1 )
    {
        pEnumerator->lpVtbl->Next ( pEnumerator, WBEM_INFINITE,
            1, &pclsObj, &uReturn );

        if ( 0 == uReturn )
            break;

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"Model", 0, &vtProp,
            0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\r\n[ %ls ]\r\n", vtProp.bstrVal );

                EBAddLine ( hWnd, buf );
            }

            VariantClear ( &vtProp );
        }

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"MediaType", 
            0, &vtProp, 0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\tType\t\t\t\t %ls\r\n", vtProp.bstrVal );

                EBAddLine ( hWnd, buf );
            }   

            VariantClear ( &vtProp );
        }

        wszDevID[0] = L'\0'; // play safe

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"DeviceID", 
            0, &vtProp, 0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\tDevice ID\t\t\t\t %ls\r\n", vtProp.bstrVal );

                EBAddLine ( hWnd, buf );
                // save dev. id for later
                StringCchCopyW ( wszDevID, ARRAYSIZE(wszDevID), 
                    vtProp.bstrVal );
            }

            VariantClear ( &vtProp );
        }

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"SerialNumber", 
            0, &vtProp, 0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\tSerial Number (reported by OS)\t %ls\r\n", 
                    vtProp.bstrVal );

                EBAddLine ( hWnd, buf );

                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\tSerial Number (with swapped bytes)\t %ls\r\n", 
                    wswap ( vtProp.bstrVal ) );

                EBAddLine ( hWnd, buf );
            }

            VariantClear ( &vtProp );
        }

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"Size", 0, &vtProp, 0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                s = wcstoull ( vtProp.bstrVal, NULL, 10 );

                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\tSize\t\t\t\t %.2f GB (%ls bytes)\r\n", 
                    s / 1073741824, vtProp.bstrVal );

                EBAddLine ( hWnd, buf );
            }

            VariantClear ( &vtProp );
        }

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"InterfaceType", 0, 
            &vtProp, 0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"\tInterface\t\t\t\t %ls\r\n", vtProp.bstrVal );

                EBAddLine ( hWnd, buf );
            }

            VariantClear ( &vtProp );
        }

        // now, for every disk, enumerate volumes
        bstrLanguage = SysAllocString ( L"WQL" );
        StringCchPrintfW ( buf, ARRAYSIZE(buf), 
            L"ASSOCIATORS OF {Win32_DiskDrive.DeviceID=\'%ls\'} "
            "WHERE AssocClass = Win32_DiskDriveToDiskPartition", wszDevID );

        bstrQuery    = SysAllocString ( buf );

        hres = pSvc->lpVtbl->ExecQuery ( pSvc, 
            bstrLanguage,
            bstrQuery, 
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL, 
            &pEnumerator2 );

        SysFreeString ( bstrLanguage );
        SysFreeString ( bstrQuery );

        if ( SUCCEEDED(hres) )
        {
            while ( 1 )
            {
                hres = pEnumerator2->lpVtbl->Next ( pEnumerator2, 
                    WBEM_INFINITE, 1, &pclsObj2, &uReturn );

                if ( 0 == uReturn )
                    break;                

                if ( SUCCEEDED(hres) )
                {
                    hres = pclsObj2->lpVtbl->Get ( pclsObj2, 
                        L"DeviceID", 0, &vtProp, 0, 0 );

                    wszDevID[0] = L'\0';

                    if ( S_OK == hres )
                    {
                        if ( vtProp.vt == VT_BSTR )
                        {
                            StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                                L"\tVolume\t\t\t\t %ls", vtProp.bstrVal );

                            EBAddLine ( hWnd, buf );

                            StringCchCopyW ( wszDevID, ARRAYSIZE(wszDevID), 
                                vtProp.bstrVal );
                        }

                        VariantClear ( &vtProp );
                    }

                    hres = pclsObj2->lpVtbl->Get ( pclsObj2, 
                        L"Size", 0, &vtProp, 0, 0 );

                    if ( S_OK == hres )
                    {
                        if ( vtProp.vt == VT_BSTR )
                        {
                            s = wcstoull ( vtProp.bstrVal, NULL, 10 );

                            StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                                L" (%.2f GB)\r\n", s / 1073741824 );

                            EBAddLine ( hWnd, buf );

                        }

                        VariantClear ( &vtProp );

                        // ..and for every volume, see if it has any
                        // partitions :-))
                        bstrLanguage = SysAllocString ( L"WQL" );

                        StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                            L"ASSOCIATORS OF {Win32_DiskPartition"
                            ".DeviceID=\'%ls\'} WHERE AssocClass "
                            "= Win32_LogicalDiskToPartition", wszDevID );

                        bstrQuery    = SysAllocString ( buf );

                        hres = pSvc->lpVtbl->ExecQuery ( pSvc, 
                            bstrLanguage,
                            bstrQuery, 
                            WBEM_FLAG_FORWARD_ONLY | 
                            WBEM_FLAG_RETURN_IMMEDIATELY, 
                            NULL, &pEnumerator3 );

                        SysFreeString ( bstrLanguage );
                        SysFreeString ( bstrQuery );
                    
                        if ( SUCCEEDED(hres) )
                        {
                            while ( 1 )
                            {
                                hres = pEnumerator3->lpVtbl->Next ( 
                                    pEnumerator3, WBEM_INFINITE, 1, 
                                    &pclsObj3, &uReturn );

                                if ( 0 == uReturn )
                                    break;

                                if ( SUCCEEDED(hres) )
                                {
                                    hres = pclsObj3->lpVtbl->Get ( pclsObj3, 
                                        L"DeviceID", 0, &vtProp, 0, 0 );
                                    
                                    if ( S_OK == hres )
                                    {
                                        if ( vtProp.vt == VT_BSTR )
                                        {
                                            StringCchPrintfW ( buf, 
                                                ARRAYSIZE(buf), L"\t\t--> "
                                                "Drive\t\t\t %ls\r\n", 
                                                vtProp.bstrVal );

                                            EBAddLine ( hWnd, buf );
                                        }

                                        VariantClear ( &vtProp );
                                    }
                                }

                                pclsObj3->lpVtbl->Release ( pclsObj3 );
                            }

                            pEnumerator3->lpVtbl->Release ( pEnumerator3 );
                        }
                    }
                }

                pclsObj2->lpVtbl->Release ( pclsObj2 );
            }

            pEnumerator2->lpVtbl->Release ( pEnumerator2 );
        }

        pclsObj->lpVtbl->Release ( pclsObj );
    }

    pEnumerator->lpVtbl->Release ( pEnumerator );
    pSvc->lpVtbl->Release ( pSvc );
    pLoc->lpVtbl->Release ( pLoc );

    CoUninitialize();

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: WMIGetCPUInfo 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hWnd : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 19.06.2021
//    DESCRIPTION: See WMIGetStorageInfo above
/*--------------------------------------------------------------------@@-@@-*/
BOOL WMIGetCPUInfo ( HWND hWnd )
/*--------------------------------------------------------------------------*/
{
    WCHAR                   buf[2048];
    HRESULT                 hres;
    BSTR                    bstr;
    BSTR                    bstrLanguage;
    BSTR                    bstrQuery;
    VARIANT                 vtProp;
    ULONG                   uReturn         = 0;
    IWbemLocator            * pLoc          = NULL;
    IWbemServices           * pSvc          = NULL;
    IEnumWbemClassObject    * pEnumerator   = NULL;
    IWbemClassObject        * pclsObj       = NULL;

    // init COM
    hres = CoInitializeEx ( 0, COINIT_MULTITHREADED );

    if ( FAILED(hres) )
        return FALSE;

    // Set general COM security levels
    hres = CoInitializeSecurity ( NULL, 
        -1,                          // COM authentication 
        NULL,                        // Authentication services 
        NULL,                        // Reserved 
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication  
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation   
        NULL,                        // Authentication info 
        EOAC_NONE,                   // Additional capabilities  
        NULL                         // Reserved 
        );

    if ( FAILED(hres) )
    {
        CoUninitialize();
        return FALSE;
    }

    // Obtain the initial locator to WMI
    hres = CoCreateInstance ( &CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER, 
        &IID_IWbemLocator, 
        (LPVOID *) &pLoc );

    if ( FAILED(hres) )
    {
        CoUninitialize();
        return FALSE;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    bstr = SysAllocString ( L"ROOT\\CIMV2" );
    hres = pLoc->lpVtbl->ConnectServer ( pLoc, 
        bstr,       // Object path of WMI namespace 
        NULL,       // User name. NULL = current user 
        NULL,       // User password. NULL = current 
        0,          // Locale. NULL indicates current 
        0,          // Security flags. 
        0,          // Authority (for example, Kerberos) 
        0,          // Context object  
        &pSvc       // pointer to IWbemServices proxy 
    );

    SysFreeString ( bstr );

    if ( FAILED (hres) )
    {
        pLoc->lpVtbl->Release ( pLoc );  
        CoUninitialize();
        return FALSE;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket( 
        (LPUNKNOWN)pSvc,             // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
        NULL,                        // Server principal name  
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx  
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx 
        NULL,                        // client identity 
        EOAC_NONE                    // proxy capabilities 
    );

    if ( FAILED(hres) )
    {
        pSvc->lpVtbl->Release ( pSvc );
        pLoc->lpVtbl->Release ( pLoc );  
        CoUninitialize();
        return FALSE;
    }

    // Execute the query
    bstrLanguage = SysAllocString ( L"WQL" );
    bstrQuery    = SysAllocString ( L"SELECT Description, Name FROM Win32_Processor" );

    hres = pSvc->lpVtbl->ExecQuery ( pSvc, 
        bstrLanguage,
        bstrQuery, 
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, 
        &pEnumerator 
    );

    SysFreeString ( bstrLanguage );
    SysFreeString ( bstrQuery );

    // Check for errors.
    if ( FAILED(hres) )
    {
        pSvc->lpVtbl->Release(pSvc);
        pLoc->lpVtbl->Release(pLoc);
        CoUninitialize();    

        return FALSE;
    }

    // Enumerate returned objects and get data
    while ( pEnumerator )
    {
        hres = pEnumerator->lpVtbl->Next ( pEnumerator, WBEM_INFINITE,
            1, &pclsObj, &uReturn );

        if ( 0 == uReturn )
            break;

        StringCchPrintfW ( buf, ARRAYSIZE(buf), 
            L"Unable to get any CPU info" );

        hres = pclsObj->lpVtbl->Get ( pclsObj, L"Description", 0,
            &vtProp, 0, 0 );

        if ( S_OK == hres )
        {
            if ( vtProp.vt == VT_BSTR )
            {
                StringCchPrintfW ( buf, ARRAYSIZE(buf), 
                    L"%ls\n", vtProp.bstrVal );
            }

            VariantClear ( &vtProp );

            hres = pclsObj->lpVtbl->Get ( pclsObj, L"Name", 
                0, &vtProp, 0, 0 );

            if ( S_OK == hres )
            {
                if ( vtProp.vt == VT_BSTR )
                {
                    StringCchCatW ( buf, ARRAYSIZE(buf), 
                        vtProp.bstrVal );

                    StringCchCatW ( buf, ARRAYSIZE(buf), 
                        L"\n" );
                }   

                VariantClear ( &vtProp );
            }
        }

        SendMessage ( hWnd, WM_SETTEXT, (WPARAM)0, (LPARAM)buf );
    }

    pSvc->lpVtbl->Release ( pSvc );
    pLoc->lpVtbl->Release ( pLoc );
    pEnumerator->lpVtbl->Release ( pEnumerator );
    pclsObj->lpVtbl->Release ( pclsObj );

    CoUninitialize();

    return TRUE;
}
