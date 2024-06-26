#include <Windows.h>
#include <CommCtrl.h>
#include <stdio.h>
#include "resource.h"

HWND hListview;
LVCOLUMN LvCol;
LVITEM LvItem;
WCHAR sSize[20];
WCHAR sAttributes[20];
WCHAR sCreationTime[MAX_PATH];
WCHAR sLastAccessTime[MAX_PATH];
WCHAR sLastWriteTime[MAX_PATH];
SYSTEMTIME st;
INT iIndex;

WCHAR sDir[MAX_PATH];
WCHAR sMsg[MAX_PATH];
DWORD nEnumCount;

#define WM_USER_ARG		WM_USER+1
/*
反转一个字串
VOID wcsrevs(WCHAR* s)
{
	WCHAR* p;
	WCHAR t;
	for (p = s; *p; ++p) {}
	--p;

	while (p > s)
	{
		t = *s;
		*s = *p;
		*p = t;
		++s;
		--p;
	}
}
itoa 的 i64 宽字节输出版本

VOID i64toaW(INT64 i64, WCHAR* sDigit, DWORD n)
{
	WCHAR* sDst = sDigit;
	DWORD i;
	for (i = 0; i64 && i < n; ++i)
	{
		*sDst++ = '0' + i64 % 10;
		i64 /= 10;
	}
	*sDst = '\0';
	wcsrevs(sDigit);
}
*/
VOID AddItemtoListview(WCHAR* sFilePath, WIN32_FIND_DATA* pFindFileData)
{
	WCHAR* sA = sAttributes;
	if (pFindFileData->nFileSizeHigh)
	{
		INT64 iSize64 = pFindFileData->nFileSizeHigh;
		iSize64 <<= 32;
		iSize64 += pFindFileData->nFileSizeLow;
		_i64tow_s(iSize64, sSize, 20, 10);
		// i64toaW(iSize64, sSize, 20);
	}
	else
		swprintf_s(sSize, 20, L"%d", pFindFileData->nFileSizeLow);

	if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
		*sA++ = 'S';
	if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		*sA++ = 'H';
	if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		*sA++ = 'R';
	if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		*sA++ = 'A';
	if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
		*sA++ = 'F';
	if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
		*sA++ = 'C';
	*sA = '\0';

	FileTimeToSystemTime(&pFindFileData->ftCreationTime, &st);
	swprintf_s(sCreationTime, MAX_PATH, L"%04d-%02d-%02d  %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	FileTimeToSystemTime(&pFindFileData->ftLastAccessTime, &st);
	swprintf_s(sLastAccessTime, MAX_PATH, L"%04d-%02d-%02d  %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	FileTimeToSystemTime(&pFindFileData->ftLastWriteTime, &st);
	swprintf_s(sLastWriteTime, MAX_PATH, L"%04d-%02d-%02d  %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	LvItem.iItem = iIndex++;
	LvItem.iSubItem = 0; LvItem.pszText = sFilePath; SendMessage(hListview, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 1; LvItem.pszText = sAttributes; SendMessage(hListview, LVM_SETITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 2; LvItem.pszText = sSize; SendMessage(hListview, LVM_SETITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 3; LvItem.pszText = sCreationTime; SendMessage(hListview, LVM_SETITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 4; LvItem.pszText = sLastAccessTime; SendMessage(hListview, LVM_SETITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 5; LvItem.pszText = sLastWriteTime; SendMessage(hListview, LVM_SETITEM, 0, (LPARAM)&LvItem);
}
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
		// PrintErrorMessage(L"FindFirstFile Failed: %s");
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
				AddItemtoListview(sPath, &FindFileData);
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
	// swprintf_s(sMsg, MAX_PATH, L"Files of DIR [%s]", sDir);
	// SetWindowText(hDlg, sMsg);

	// ClearListboxItems();
	// SendMessage(hListview, LVM_DELETEALLITEMS, 0, 0);
	nEnumCount = 0;
	EnumerateFilesinDrectory(sDir);
	swprintf_s(sMsg, MAX_PATH, L"%d Files in [%s]", nEnumCount, sDir);
	SetWindowText(hDlg, sMsg);
}
VOID InitListview()
{
	SendMessage(hListview, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_SINGLESEL);
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 720;
	LvCol.pszText = (PWSTR)L"FilePath";
	SendMessage(hListview, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	LvCol.cx = 40;
	LvCol.pszText = (PWSTR)L"Attributes";
	SendMessage(hListview, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);
	LvCol.cx = 80;
	LvCol.pszText = (PWSTR)L"Size";
	SendMessage(hListview, LVM_INSERTCOLUMN, 2, (LPARAM)&LvCol);
	LvCol.cx = 160;
	LvCol.pszText = (PWSTR)L"CreationTime (UTC)";
	SendMessage(hListview, LVM_INSERTCOLUMN, 3, (LPARAM)&LvCol);
	LvCol.cx = 160;
	LvCol.pszText = (PWSTR)L"LastAccessTime (UTC)";
	SendMessage(hListview, LVM_INSERTCOLUMN, 4, (LPARAM)&LvCol);
	LvCol.cx = 160;
	LvCol.pszText = (PWSTR)L"LastWriteTime (UTC)";
	SendMessage(hListview, LVM_INSERTCOLUMN, 5, (LPARAM)&LvCol);


	iIndex = 0;
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask = LVIF_TEXT;
	LvItem.cchTextMax = MAX_PATH;
}
LRESULT CALLBACK DlgFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hListview = GetDlgItem(hDlg, IDC_LISTVIEW);
		InitListview();

		if (lParam)
		{
			SendMessage(hDlg, WM_USER_ARG, (WPARAM)lParam, 0);
		}
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
		swprintf_s(sMsg, MAX_PATH, L"hInstance %08X %08X\nsTitle %s %s", hInstance, ((ThreadParam*)lParam)->hModule, sTitle, ((ThreadParam*)lParam)->sTitle);
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
*/
VOID EntryPoint()
{
	PWSTR* sArgv;
	INT nArgs;

	sArgv = CommandLineToArgvW(GetCommandLine(), &nArgs);
	if (sArgv)
	{
		if (nArgs > 1)
			DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgFunc, (LPARAM)sArgv[1]);
		LocalFree(sArgv);
	}
	ExitProcess(0);
}
