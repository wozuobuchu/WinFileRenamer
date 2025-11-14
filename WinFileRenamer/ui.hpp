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
#include <exception>

#include "calc.hpp"
#include "process_thread.hpp"

// Link the common controls library
#pragma comment(lib, "Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
	constexpr int ID_EDIT_PUSH_OFNAME = 2004;
	constexpr int ID_EDIT_PUSH_LB = 2005;
	constexpr int ID_EDIT_PUSH_RB = 2006;
	constexpr int ID_EDIT_PUSH_ADD = 2007;
	constexpr int ID_EDIT_PUSH_SUB = 2008;
	constexpr int ID_EDIT_PUSH_MUL = 2009;
	constexpr int ID_EDIT_PUSH_DIV = 2010;
	constexpr int ID_EDIT_PUSH_DEL = 2011;
	constexpr int ID_EDIT_CLEAR = 2012;

	// IDs for new controls
	constexpr int ID_EXPR_DISPLAY = 3001;
	constexpr int ID_INPUT_EDIT = 3002;


	std::stop_source sts_ui_;

	pt::ProcessThread pt_;

	// Handle for the list view control
	static HWND hListView_ = NULL;
	// Handles for new controls
	static HWND hExprDisplay_ = NULL;
	static HWND hInputEdit_ = NULL;

	struct RegisterReturn {
		WNDCLASSEX* wndclass;
		HWND hwnd;
	};

	// Helper function to update the expression display
	inline void UpdateExpressionDisplay() {
		if (hExprDisplay_) {
			std::wstring exprStr = pt_.get_expression_str();
			SetWindowTextW(hExprDisplay_, exprStr.c_str());
		}
	}

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

		case WM_SYSCOMMAND:
		{
			if (wParam == SC_CLOSE) {
				pt_.join();
				PostQuitMessage(0);
				sts_ui_.request_stop();
				return 0;
			}
			break;
		}
			
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMinMaxInfo = (LPMINMAXINFO)lParam;
			// Set a reasonable minimum tracking size
			
			lpMinMaxInfo->ptMinTrackSize.x = 600;
			
			lpMinMaxInfo->ptMinTrackSize.y = 400;
			// We don't set ptMaxTrackSize, so the default (full screen) is the max.
			return 0;
		}

		case WM_CREATE:
		{
			// Create the List View control (Top Half)
			hListView_ = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				WC_LISTVIEWW, // Use the wide-character version
				L"",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS,
				0, 0, UI_WIDTH, UI_HEIGHT / 2, // Initial size (top half)
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

			// Create Expression Display (Bottom Half - Top)
			hExprDisplay_ = CreateWindowEx(
				0,
				L"STATIC", // Static text control
				L"",       // Initial text
				WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT | WS_VSCROLL,
				0, UI_HEIGHT / 2, UI_WIDTH, (UI_HEIGHT / 2) - 40, // Initial size
				hwnd,
				(HMENU)(UINT_PTR)ID_EXPR_DISPLAY,
				GetModuleHandle(NULL),
				NULL
			);

			// Create Input Edit Box (Bottom-most)
			hInputEdit_ = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				L"EDIT",
				L"",
				WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
				0, UI_HEIGHT - 40, UI_WIDTH, 40, // Initial size
				hwnd,
				(HMENU)(UINT_PTR)ID_INPUT_EDIT,
				GetModuleHandle(NULL),
				NULL
			);

			// Add cue banner text to the input box
			Edit_SetCueBannerText(hInputEdit_, L"Type string or number here, then click 'Edit' menu...");

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
			case ID_OPTIONS_SUBMIT:
				// Call the process_lunch function
				if (!pt_.process_lunch()) {
					MessageBox(hwnd, L"Process is already ongoing!", L"Warning", MB_OK | MB_ICONWARNING | MB_TOPMOST);
				}
				break;
			case ID_OPTIONS_EXIT:
				PostQuitMessage(0);
				sts_ui_.request_stop();
				break;

				// Handlers for Edit Menu
			case ID_EDIT_PUSH_STR:
			{
				wchar_t buffer[256] = { 0 };
				GetWindowTextW(hInputEdit_, buffer, 256);
				pt_.push_expr<calc::Str>(std::wstring(buffer));
				UpdateExpressionDisplay();
				SetWindowTextW(hInputEdit_, L""); // Clear input box
				break;
			}
			case ID_EDIT_PUSH_NUM:
			{
				wchar_t buffer[256] = { 0 };
				GetWindowTextW(hInputEdit_, buffer, 256);
				try {
					int64_t val = std::stoll(std::wstring(buffer));
					pt_.push_expr<calc::Int64>(val);
					UpdateExpressionDisplay();
					SetWindowTextW(hInputEdit_, L""); // Clear input box
				} catch (...) {
					MessageBoxW(hwnd, L"Invalid number. Please enter a valid 64-bit integer.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
				}
				break;
			}
			case ID_EDIT_PUSH_IDX:
				pt_.push_expr<calc::Index_Var>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_OFNAME:
				pt_.push_expr<calc::OriginFileName_Var>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_LB:
				pt_.push_expr<calc::Lbracket>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_RB:
				pt_.push_expr<calc::Rbracket>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_ADD:
				pt_.push_expr<calc::Add_Int64Opt>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_SUB:
				pt_.push_expr<calc::Sub_Int64Opt>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_MUL:
				pt_.push_expr<calc::Mul_Int64Opt>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_DIV:
				pt_.push_expr<calc::Div_Int64Opt>();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_PUSH_DEL:
				pt_.pop_expr_ptr();
				UpdateExpressionDisplay();
				break;
			case ID_EDIT_CLEAR:
				pt_.reset_input_expr_ptr();
				UpdateExpressionDisplay();
				break;
			}
			return 0;
		}

		case WM_SIZE:
		{
			// Resize all three controls
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			int editHeight = 40;
			int displayY = height / 2;
			// Make displayHeight beyond zero
			int displayHeight = (height / 2) - editHeight;
			if (displayHeight < 0) displayHeight = 0;

			// Resize list view (top half)
			if (hListView_) {
				MoveWindow(hListView_, 0, 0, width, height / 2, TRUE);
				
				ListView_SetColumnWidth(hListView_, 0, width - 20);
			}

			// Resize expression display (bottom half, top part)
			if (hExprDisplay_) {
				MoveWindow(hExprDisplay_, 0, displayY, width, displayHeight, TRUE);
			}

			// Resize input edit box (bottom-most part)
			if (hInputEdit_) {
				MoveWindow(hInputEdit_, 0, displayY + displayHeight, width, editHeight, TRUE);
			}

			if (hInputEdit_) {
				Edit_SetCueBannerText(hInputEdit_, L"Type string or number here, click 'Edit' menu to push expression element...");
			}

			return 0;
		}

		case WM_DESTROY:
			pt_.join();
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
		// Also load standard classes (EDIT, STATIC, BUTTON, etc.)
		icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
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
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_OFNAME, TEXT("Push OriginFileName"));
		AppendMenu(EditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_LB, TEXT("LB ("));
		AppendMenu(EditMenu, MF_STRING, ID_EDIT_PUSH_RB, TEXT("RB )"));
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
			WS_OVERLAPPEDWINDOW,
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