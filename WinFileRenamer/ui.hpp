#ifndef _UI_HPP_
#define _UI_HPP_

#include <Windows.h>
#include <CommCtrl.h>
#include <commdlg.h>

#include <string>
#include <chrono>
#include <fstream>
#include <format>
#include <iostream>
#include <thread>
#include <stop_token>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <memory>
#include <cstring>
#include <atomic>
#include <cmath>
#include <sstream>

#include "calc.hpp"
#include "process_thread.hpp"

// Link the common controls library
#pragma comment(lib, "Comctl32.lib")


namespace ui {

	constexpr int UI_WIDTH = 1280;
	constexpr int UI_HEIGHT = 720;

	// Define IDs for menu items and controls
	constexpr int ID_FILE_OPEN = 1001;
	constexpr int ID_FILE_CLEAR = 1002;
	constexpr int ID_LISTVIEW = 1003;

	constexpr int ID_OPTIONS_SUBMIT = 9001;
	constexpr int ID_OPTIONS_EXIT = 9002;

	constexpr int ID_EDIT_PUSH_STR = 2001;
	constexpr int ID_EDIT_PUSH_NUM = 2002;
	constexpr int ID_EDIT_PUSH_IDX = 2003;
	constexpr int ID_EDIT_PUSH_LB = 2004;
	constexpr int ID_EDIT_PUSH_RB = 2005;
	constexpr int ID_EDIT_PUSH_ADD = 2006;
	constexpr int ID_EDIT_PUSH_SUB = 2007;
	constexpr int ID_EDIT_PUSH_MUL = 2008;
	constexpr int ID_EDIT_PUSH_DIV = 2009;
	constexpr int ID_EDIT_PUSH_DEL = 2010;
	constexpr int ID_EDIT_CLEAR = 2011;


	std::stop_source sts_ui_;

	pt::ProcessThread pt_;

	// Handle for the list view control
	static HWND hListView_ = NULL;

	struct RegisterReturn {
		WNDCLASSEX* wndclass;
		HWND hwnd;
	};

	// Helper function to add a file path to the list view
	inline void AddFileToList(const std::wstring& filePath) {
		if (!hListView_) return;

		LVITEMW lvi = { 0 };
		lvi.mask = LVIF_TEXT;
		lvi.pszText = const_cast<LPWSTR>(filePath.c_str()); // Safe cast for insertion
		lvi.iItem = ListView_GetItemCount(hListView_); // Insert at the end
		lvi.iSubItem = 0;

		ListView_InsertItem(hListView_, &lvi);

		pt_.push_filepath(filePath);
	}

	// Helper function to handle the "Open File" dialog logic
	inline void HandleFileOpen(HWND hwnd) {
		// Heap memory
		std::unique_ptr<wchar_t[]> szFile = std::make_unique<wchar_t[]>(8192);
		std::memset(szFile.get(), 0, 8192 * sizeof(wchar_t));

		OPENFILENAMEW ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile.get();
		ofn.nMaxFile = 8192;
		ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		if (GetOpenFileNameW(&ofn) == TRUE) {
			wchar_t* p = szFile.get();
			std::wstring dir = p; // First string is the directory

			// Move past the directory string
			p += dir.length() + 1;

			if (*p == 0) {
				// Only one file was selected. 'dir' holds the full path.
				AddFileToList(dir);
			} else {
				// Multiple files were selected.
				// Loop through the subsequent null-terminated file names.
				while (*p) {
					std::wstring file = p;
					std::wstring fullPath = dir + L"\\" + file;
					AddFileToList(fullPath);

					// Move to the next file name
					p += file.length() + 1;
				}
			}
		}
	}

	LRESULT CALLBACK windowproc_main(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_CREATE:
		{
			// Create the List View control when the window is created
			hListView_ = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				WC_LISTVIEWW, // Use the wide-character version
				L"",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS,
				0, 0, UI_WIDTH, UI_HEIGHT, // Will be resized by WM_SIZE
				hwnd,
				(HMENU)(UINT_PTR)ID_LISTVIEW,
				GetModuleHandle(NULL),
				NULL
			);

			// Initialize the list view columns
			LVCOLUMNW lvc = { 0 };
			lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lvc.cx = UI_WIDTH - 20; // Initial width
			lvc.pszText = (LPWSTR)L"Selected Files";
			ListView_InsertColumn(hListView_, 0, &lvc);

			return 0;
		}

		case WM_COMMAND:
		{
			// Handle menu commands
			switch (LOWORD(wParam)) {
			case ID_FILE_OPEN:
				HandleFileOpen(hwnd);
				break;
			case ID_FILE_CLEAR:
				// Clear all items from the list view
				ListView_DeleteAllItems(hListView_);
				pt_.reset_selected_file();
				break;
				// Exit the application
			case ID_OPTIONS_EXIT:
				PostQuitMessage(0);
				sts_ui_.request_stop();
				break;
			}
			return 0;
		}

		case WM_SIZE:
		{
			// Resize the list view to fill the half client area
			if (hListView_) {
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				MoveWindow(hListView_, 0, 0, width, height / 2, TRUE);
				// Resize the column to match the new width
				ListView_SetColumnWidth(hListView_, 0, width);
			}
			return 0;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			sts_ui_.request_stop();
			return 0;
		default:
			break;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	RegisterReturn register_main_ui(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

		// Initialize common controls
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_LISTVIEW_CLASSES; // Register the list view class
		InitCommonControlsEx(&icex);


		WNDCLASSEX* wndclass = new WNDCLASSEX();
		std::memset(wndclass, 0, sizeof(WNDCLASSEX));

		RegisterReturn ret;
		std::memset(&ret, 0, sizeof(RegisterReturn));

		wndclass->cbSize = sizeof(WNDCLASSEX);
		wndclass->style = NULL;
		wndclass->lpfnWndProc = windowproc_main;
		wndclass->cbClsExtra = NULL;
		wndclass->cbWndExtra = NULL;
		wndclass->hInstance = hInstance;
		wndclass->hIcon = NULL;
		wndclass->hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Standard window background
		wndclass->lpszMenuName = NULL; // We set the menu on the window, not the class
		wndclass->lpszClassName = TEXT("MainUIWindowClass");
		wndclass->hIconSm = NULL;

		if (!RegisterClassEx(wndclass)) {
			MessageBox(NULL, TEXT("Failed to register window."), TEXT("Error"), MB_ICONWARNING | MB_OK);
			sts_ui_.request_stop();
			return ret;
		}

		// --- Create the menu ---
		HMENU hMenu = CreateMenu();

		HMENU OptionMenu = CreatePopupMenu();
		HMENU hFileMenu = CreatePopupMenu();
		HMENU EditMenu = CreatePopupMenu();

		// Append the "Exit" submenu to the options menu bar
		AppendMenu(OptionMenu, MF_STRING, ID_OPTIONS_SUBMIT, TEXT("Submit"));
		AppendMenu(OptionMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(OptionMenu, MF_STRING, ID_OPTIONS_EXIT, TEXT("Exit"));

		// Append "Open" and "Clear" to the "File" submenu
		AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, TEXT("Open"));
		AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hFileMenu, MF_STRING, ID_FILE_CLEAR, TEXT("Clear"));

		// Append expression element to the "Edit" submenu
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_STR, TEXT("Push Wstring"));
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_NUM, TEXT("Push Number"));
		AppendMenu(EditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_IDX, TEXT("Push Index"));
		AppendMenu(EditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_LB, TEXT("LB ["));
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_RB, TEXT("RB ]"));
		AppendMenu(EditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_ADD, TEXT("Push +"));
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_SUB, TEXT("Push -"));
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_MUL, TEXT("Push *"));
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_DIV, TEXT("Push /"));
		AppendMenu(EditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_DEL, TEXT("Delete"));
		AppendMenu(EditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_CLEAR, TEXT("Clear"));

		// Append the "Options" submenu to the main menu bar
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)OptionMenu, TEXT("Option"));

		// Append the "File" submenu to the main menu bar
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));

		// Append the "Edit" submenu to the main menu bar
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)EditMenu, TEXT("Edit"));

		// --- End menu creation ---

		HWND hwnd = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			wndclass->lpszClassName,
			TEXT("WinFileRename"),
			(WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX),
			CW_USEDEFAULT, CW_USEDEFAULT,
			UI_WIDTH, UI_HEIGHT,
			NULL,
			hMenu, // Assign the menu to the window
			hInstance,
			NULL
		);

		if (hwnd == NULL) {
			MessageBox(NULL, TEXT("Failed to create window."), TEXT("Error"), MB_ICONWARNING | MB_OK);
			sts_ui_.request_stop();
			return ret;
		}

		ret.wndclass = wndclass;
		ret.hwnd = hwnd;

		return ret;
	}

}

#endif // !_UI_HPP_