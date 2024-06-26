#include <Windows.h>
#include <stdio.h>
#include "resource.h"

HWND hListbox;
WCHAR sDir[MAX_PATH];
WCHAR sMsg[MAX_PATH];
WCHAR sErrorMsg[MAX_PATH];
DWORD nEnumCount;

#define WM_USER_ARG WM_USER + 1

VOID AddItemtoListbox(WCHAR* sMsg)
{
	SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)sMsg);
	SendMessage(hListbox, LB_SETCURSEL, SendMessage(hListbox, LB_GETCOUNT, 0, 0) - 1, 0);
}
VOID ClearListboxItems()
{
	INT n = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	while (n > 0)
	{
		SendMessage(hListbox, LB_DELETESTRING, n - 1, 0);
		--n;
	}
}
VOID PrintErrorMessage(WCHAR* sError)
{
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), sErrorMsg, MAX_PATH, NULL);
	swprintf_s(sMsg, MAX_PATH,  sError, sErrorMsg);
	AddItemtoListbox(sMsg);
}
/*
VOID EnumerateFilesinDrectory(WCHAR* sDirectory)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	WCHAR sPath[MAX_PATH];
	WCHAR sWildcard[MAX_PATH];
	WCHAR sPathNew[MAX_PATH];

	wcscpy_s(sPath, MAX_PATH, sDirectory);
	wcscat_s(sPath, MAX_PATH, L"\\");
	swprintf_s(sMsg, MAX_PATH,  L"Searching inside %s", sPath);
	AddItemtoListbox(sMsg);

	wcscpy_s(sWildcard, MAX_PATH, sPath);
	wcscat_s(sWildcard, MAX_PATH, L"*");

	hFind = FindFirstFile(sWildcard, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		PrintErrorMessage(L"FindFirstFile Failed: %s");
		return;
	}
	else
	{
		do
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				swprintf_s(sMsg, MAX_PATH,  L"%s%s", sPath, FindFileData.cFileName);
				AddItemtoListbox(sMsg);
			}
			else if (wcscmp(FindFileData.cFileName, L".") && wcscmp(FindFileData.cFileName, L".."))
			{
				WCHAR sPathNew[MAX_PATH];
				wcscpy_s(sPathNew, MAX_PATH, sPath);
				wcscat_s(sPathNew, MAX_PATH, FindFileData.cFileName);
				EnumerateFilesinDrectory(sPathNew);
			}
		} while (FindNextFile(hFind, &FindFileData));
	}
}
*/
VOID EnumerateFilesinDrectory(WCHAR* sDirectory)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	WCHAR sPath[MAX_PATH];
	WCHAR* sSlash;
	wcscpy_s(sPath, MAX_PATH, sDirectory);
	wcscat_s(sPath, MAX_PATH, L"\\*");
	// sSlash = wcsrchr(sPath, '\\') + 1;
	sSlash = wcsrchr(sPath, '*');

	hFind = FindFirstFile(sPath, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		PrintErrorMessage(L"FindFirstFile Failed: %s");
		return;
	}
	else
	{
		do
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				wcscpy_s(sSlash, MAX_PATH, FindFileData.cFileName);
				++nEnumCount;
				AddItemtoListbox(sPath);
			}
			else if (wcscmp(FindFileData.cFileName, L".") && wcscmp(FindFileData.cFileName, L".."))
			{
				wcscpy_s(sSlash, MAX_PATH, FindFileData.cFileName);
				EnumerateFilesinDrectory(sPath);
			}
		} while (FindNextFile(hFind, &FindFileData));
	}
}
VOID ListFiles(HWND hDlg, PWSTR sDir)
{
	swprintf_s(sMsg, MAX_PATH,  L"Files of DIR [%s]", sDir);
	SetWindowText(hDlg, sMsg);

	ClearListboxItems();
	nEnumCount = 0;
	EnumerateFilesinDrectory(sDir);
	AddItemtoListbox(L"");
	swprintf_s(sMsg, MAX_PATH,  L"%d Files Found", nEnumCount);
	AddItemtoListbox(sMsg);
}
LRESULT CALLBACK DlgFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		DragAcceptFiles(hDlg, TRUE);
		hListbox = GetDlgItem(hDlg, IDC_LIST_LOGS);
		if (lParam)
			SendMessage(hDlg, WM_USER_ARG, (WPARAM)lParam, 0);
		else
			AddItemtoListbox(L"[Drag a file to list all files within the same folder]");
		break;
		
	case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, 0, sDir, MAX_PATH);
		DragFinish((HDROP)wParam);
		// MessageBox(hDlg, sDir, L"", MB_OK);
		*wcsrchr(sDir, '\\') = '\0';
		ListFiles(hDlg, sDir);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;

	case WM_USER_ARG:
		ListFiles(hDlg, (PWSTR)wParam);
		break;
		
	default:
		break;
	}
	return 0;
}
/*
BOOL CALLBACK EnumWindowbyModuleHandle(HWND hWnd, LPARAM lParam)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	WCHAR sTitle[MAX_PATH];
	GetWindowText(hWnd, sTitle, MAX_PATH);
	if (!wcscmp(sTitle, (PWSTR)((ThreadParam*)lParam)->sTitle))
	{
		WCHAR sMsg[200];
		swprintf_s(sMsg, MAX_PATH,  L"hInstance %08X %08X\nsTitle %s %s", hInstance, ((ThreadParam*)lParam)->hModule, sTitle, ((ThreadParam*)lParam)->sTitle);
		MessageBox(NULL, sMsg, L"", MB_OK);
	}
	if (hInstance == (HINSTANCE)((ThreadParam*)lParam)->hModule && !wcscmp(sTitle, (PWSTR)((ThreadParam*)lParam)->sTitle))
	{
		SendMessage(hWnd, WM_USER_ARG, (WPARAM)((ThreadParam*)lParam)->sArg1, 0);
		return FALSE;
	}
	else
		return TRUE;
}
DWORD WINAPI ThreadGUIProc(LPVOID pM)
{
	DWORD nTimeout = 0;

	for (; nTimeout < 3000; nTimeout += 100)
	{
		if (EnumWindows(EnumWindowbyModuleHandle, (LPARAM)pM))
		{

			break;
		}
		Sleep(100);
	}
	return 0;
}
HWND GetWindowHandlebyModule(HMODULE hModule)
{
}
*/

VOID EntryPoint()
{
	PWSTR* sArgv;
	INT nArgs;
	LPARAM sDirectoryPath;

	sArgv = CommandLineToArgvW(GetCommandLine(), &nArgs);
	if (sArgv)
	{
		if (nArgs > 1)
			sDirectoryPath = (LPARAM)sArgv[1];
		else
			sDirectoryPath = 0;
		DialogBoxParamW(NULL, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgFunc, sDirectoryPath);
		LocalFree(sArgv);
	}

	ExitProcess(0);
}
