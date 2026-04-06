#ifndef _UI_INPUTBOX_HPP_
#define _UI_INPUTBOX_HPP_

#include <Windows.h>
#include <string>
#include "shared_data.hpp"
#include "ui_constants.hpp"

namespace ui {

	inline std::wstring g_inputResult;
	inline bool g_inputOk = false;

	inline LRESULT CALLBACK InputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch(msg) {
			case WM_COMMAND:
				if (LOWORD(wParam) == IDOK) {
					wchar_t buf[256];
					GetDlgItemTextW(hwnd, 101, buf, 256);
					g_inputResult = buf;
					g_inputOk = true;
					DestroyWindow(hwnd);
				}
				else if (LOWORD(wParam) == IDCANCEL) {
					g_inputOk = false;
					DestroyWindow(hwnd);
				}
				break;
			case WM_CLOSE:
				g_inputOk = false;
				DestroyWindow(hwnd);
				break;
			default:
				return DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		return 0;
	}

	inline bool ShowInputBox(HWND hParent, const std::wstring& title, const std::wstring& prompt, std::wstring& outResult) {
		static bool registered = false;
		if (!registered) {
			WNDCLASSW wc = {0};
			wc.lpfnWndProc = InputWndProc;
			wc.hInstance = GetModuleHandle(NULL);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
			wc.lpszClassName = L"CustomInputBoxClass";
			RegisterClassW(&wc);
			registered = true;
		}

		g_inputOk = false;
		g_inputResult.clear();

		HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"CustomInputBoxClass", title.c_str(), 
			WS_VISIBLE | WS_SYSMENU | WS_CAPTION, 
			0, 0, 350, 150, hParent, NULL, GetModuleHandle(NULL), NULL);

		HWND hPrompt = CreateWindowExW(0, L"STATIC", prompt.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT, 
			10, 10, 310, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

		HWND hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 
			10, 40, 310, 25, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);

		HWND hOk = CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 
			150, 75, 80, 25, hwnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL);

		HWND hCancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 
			240, 75, 80, 25, hwnd, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

		HFONT hFont = (HFONT)SendMessage(hParent, WM_GETFONT, 0, 0);
		if (!hFont) hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(hPrompt, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessage(hOk, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessage(hCancel, WM_SETFONT, (WPARAM)hFont, FALSE);

		RECT pr, cr;
		GetWindowRect(hParent, &pr);
		GetWindowRect(hwnd, &cr);
		int x = pr.left + (pr.right - pr.left) / 2 - (cr.right - cr.left) / 2;
		int y = pr.top + (pr.bottom - pr.top) / 2 - (cr.bottom - cr.top) / 2;
		SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		EnableWindow(hParent, FALSE);
		SetFocus(hEdit);

		MSG msg;
		while (IsWindow(hwnd) && GetMessage(&msg, NULL, 0, 0) && (!shared_data::sts_.stop_requested())) {
			if (!IsDialogMessage(hwnd, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		EnableWindow(hParent, TRUE);
		SetActiveWindow(hParent);

		if (g_inputOk) {
			outResult = g_inputResult;
		}
		return g_inputOk;
	}

}

#endif // !_UI_INPUTBOX_HPP_