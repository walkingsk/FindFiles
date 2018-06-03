#include <Windows.h>
#include "resource.h"

HWND hListbox;
WCHAR sDir[MAX_PATH];
WCHAR sMsg[MAX_PATH];
WCHAR sErrorMsg[MAX_PATH];
DWORD nEnumCount;

#define WM_USER_ARG		WM_USER+1

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
	wsprintf(sMsg, sError, sErrorMsg);
	AddItemtoListbox(sMsg);
}
/*
VOID EnumerateFileinDrectory(WCHAR* sDirectory)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFile;
	WCHAR sPath[MAX_PATH];
	WCHAR sWildcard[MAX_PATH];
	WCHAR sPathNew[MAX_PATH];

	wcscpy_s(sPath, MAX_PATH, sDirectory);
	wcscat_s(sPath, MAX_PATH, L"\\");
	wsprintf(sMsg, L"Searching inside %s", sPath);
	AddItemtoListbox(sMsg);

	wcscpy_s(sWildcard, MAX_PATH, sPath);
	wcscat_s(sWildcard, MAX_PATH, L"*");

	hFile = FindFirstFile(sWildcard, &FindFileData);

	if (hFile == INVALID_HANDLE_VALUE)
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
				wsprintf(sMsg, L"%s%s", sPath, FindFileData.cFileName);
				AddItemtoListbox(sMsg);
			}
			else if (wcscmp(FindFileData.cFileName, L".") && wcscmp(FindFileData.cFileName, L".."))
			{
				WCHAR sPathNew[MAX_PATH];
				wcscpy_s(sPathNew, MAX_PATH, sPath);
				wcscat_s(sPathNew, MAX_PATH, FindFileData.cFileName);
				EnumerateFileinDrectory(sPathNew);
			}
		} while (FindNextFile(hFile, &FindFileData));
	}
}
*/
VOID EnumerateFileinDrectory(WCHAR* sDirectory)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFile;
	WCHAR sPath[MAX_PATH];
	WCHAR* sSlash;
	wcscpy_s(sPath, MAX_PATH, sDirectory);
	wcscat_s(sPath, MAX_PATH, L"\\*");
	// sSlash = wcsrchr(sPath, '\\') + 1;
	sSlash = wcsrchr(sPath, '*');

	hFile = FindFirstFile(sPath, &FindFileData);

	if (hFile == INVALID_HANDLE_VALUE)
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
				EnumerateFileinDrectory(sPath);
			}
		} while (FindNextFile(hFile, &FindFileData));
	}
}
VOID ListFiles(HWND hDlg, PWSTR sDir)
{
	wsprintf(sMsg, L"Files of DIR [%s]", sDir);
	SetWindowText(hDlg, sMsg);

	ClearListboxItems();
	nEnumCount = 0;
	EnumerateFileinDrectory(sDir);
	AddItemtoListbox(L"");
	wsprintf(sMsg, L"%d Files Found", nEnumCount);
	AddItemtoListbox(sMsg);
}
VOID RemoveLastPossibleBSlash(PWSTR s)
{
	for (; *s; ++s) {}
	--s;
	if (*s == '\\')
		*s = '\0';
}
LRESULT CALLBACK DlgFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		DragAcceptFiles(hDlg, TRUE);
		hListbox = GetDlgItem(hDlg, IDC_LIST_LOGS);
		if (lParam)
		{
			SendMessage(hDlg, WM_USER_ARG, (WPARAM)lParam, 0);
		}
		break;
		
	case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, 0, sDir, MAX_PATH);
		DragFinish((HDROP)wParam);
		*wcsrchr(sDir, '\\') = '\0';
		ListFiles(hDlg, sDir);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;

	case WM_USER_ARG:
		RemoveLastPossibleBSlash((PWSTR)wParam);
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
		wsprintf(sMsg, L"hInstance %08X %08X\nsTitle %s %s", hInstance, ((ThreadParam*)lParam)->hModule, sTitle, ((ThreadParam*)lParam)->sTitle);
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
PWSTR GetArg1W(PWSTR s)
{
	PWSTR sC;

	for (; *s; ++s)
		if (*s == ' ')
			break;

	if (*s)
	{
		sC = s;
		for (; *s; ++s)
			if (*s == '\"')
				*s = ' ';

		for (;;)
		{
			--s;
			if (*s != ' ')
				break;
			else
				*s = '\0';
		}

		while (*++sC == ' ');

		if (!*sC)
			return NULL;
		else
			return sC;
	}
	else
		return NULL;
}

VOID EntryPoint()
{
	PWSTR sArg1 = GetArg1W(GetCommandLine());
	DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgFunc, (LPARAM)sArg1);

	ExitProcess(0);
}
