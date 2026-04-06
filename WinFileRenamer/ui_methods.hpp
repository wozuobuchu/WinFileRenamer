#ifndef _UI_METHODS_HPP_
#define _UI_METHODS_HPP_

#include <Windows.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <string>
#include <memory>
#include <stdexcept>
#include "shared_data.hpp"
#include "ui_constants.hpp"
#include "ui_state.hpp"
#include "ui_language.hpp"
#include "ui_inputbox.hpp"

namespace ui {

	// Guard helper: when an operation is rejected (usually because the rename thread is running),
	// caller should NOT update UI state.
	inline bool GuardUiOp(HWND hwnd, bool ok,
		const wchar_t* msg = L"Process is ongoing. Please wait for it to finish.") {
		if (ok) return true;
		MessageBeep(MB_ICONWARNING);
		if (hwnd) {
			MessageBoxW(hwnd, msg, L"Busy", MB_OK | MB_ICONWARNING | MB_TOPMOST);
		}
		return false;
	}


	// Disable File/Edit menus while a rename task is running.
	// This prevents users from triggering actions that ProcessThread would reject anyway.
	inline void UpdateMenuEnabledState(HWND hwnd) {
		if (!hwnd) return;
		HMENU hMenu = GetMenu(hwnd);
		if (!hMenu) return;

		const bool ongoing = (shared_data::pt_.get_state() == pt::ProcessThread::STATE_ONGOING);
		const UINT flags = MF_BYPOSITION | (ongoing ? (MF_GRAYED | MF_DISABLED) : MF_ENABLED);

		// Menu bar order is: File (0), Expression (1), Option (2)
		EnableMenuItem(hMenu, 0, flags);
		EnableMenuItem(hMenu, 1, flags);
		DrawMenuBar(hwnd);
	}

	inline void RebuildMenu(HWND hwnd) {
		HMENU hOldMenu = GetMenu(hwnd);
		const auto& s = GetStrings();
		HMENU hMenu = CreateMenu();

		HMENU hFileMenu = CreatePopupMenu();
		AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, s.fileOpen);
		AppendMenu(hFileMenu, MF_STRING, ID_FILE_CLEAR, s.fileClear);
		AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hFileMenu, MF_STRING, ID_OPTIONS_SUBMIT, s.fileSubmit);
		AppendMenu(hFileMenu, MF_STRING, ID_OPTIONS_SUBMIT_AUTO, s.fileSubmitAuto);

		HMENU hConstMenu = CreatePopupMenu();
		AppendMenu(hConstMenu, MF_STRING, ID_EDIT_PUSH_STR, s.exprPushStr);
		AppendMenu(hConstMenu, MF_STRING, ID_EDIT_PUSH_NUM, s.exprPushNum);
		AppendMenu(hConstMenu, MF_STRING, ID_EDIT_PUSH_NUM_FORMAT, s.exprPushFmt);

		HMENU hVarsMenu = CreatePopupMenu();
		AppendMenu(hVarsMenu, MF_STRING, ID_EDIT_PUSH_IDX, s.exprPushIdx);
		AppendMenu(hVarsMenu, MF_STRING, ID_EDIT_PUSH_OFNAME, s.exprPushOfname);

		HMENU hOpsMenu = CreatePopupMenu();
		AppendMenu(hOpsMenu, MF_STRING, ID_EDIT_PUSH_ADD, s.exprPushAdd);
		AppendMenu(hOpsMenu, MF_STRING, ID_EDIT_PUSH_SUB, s.exprPushSub);
		AppendMenu(hOpsMenu, MF_STRING, ID_EDIT_PUSH_MUL, s.exprPushMul);
		AppendMenu(hOpsMenu, MF_STRING, ID_EDIT_PUSH_DIV, s.exprPushDiv);

		HMENU hBracketsMenu = CreatePopupMenu();
		AppendMenu(hBracketsMenu, MF_STRING, ID_EDIT_PUSH_LB, s.exprPushLb);
		AppendMenu(hBracketsMenu, MF_STRING, ID_EDIT_PUSH_RB, s.exprPushRb);

		HMENU hEditMenu = CreatePopupMenu();
		AppendMenu(hEditMenu, MF_POPUP, (UINT_PTR)hConstMenu, s.exprConstants);
		AppendMenu(hEditMenu, MF_POPUP, (UINT_PTR)hVarsMenu, s.exprVars);
		AppendMenu(hEditMenu, MF_POPUP, (UINT_PTR)hOpsMenu, s.exprOps);
		AppendMenu(hEditMenu, MF_POPUP, (UINT_PTR)hBracketsMenu, s.exprBrackets);
		AppendMenu(hEditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hEditMenu, MF_STRING, ID_EDIT_PUSH_DEL, s.exprDel);
		AppendMenu(hEditMenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hEditMenu, MF_STRING, ID_EDIT_CLEAR, s.exprClear);

		HMENU hLangMenu = CreatePopupMenu();
		for (size_t i = 0; i < supported_languages.size(); ++i) {
			UINT flags = MF_STRING | (current_lang_index == i ? MF_CHECKED : MF_UNCHECKED);
			AppendMenu(hLangMenu, flags, supported_languages[i].cmdId, supported_languages[i].menuName);
		}

		HMENU hOptionMenu = CreatePopupMenu();
		AppendMenu(hOptionMenu, MF_POPUP, (UINT_PTR)hLangMenu, s.optLang);
		AppendMenu(hOptionMenu, MF_STRING, ID_OPTIONS_EXIT, s.optExit);

		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hFileMenu, s.fileMenu);
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hEditMenu, s.exprMenu);
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hOptionMenu, s.optionMenu);

		SetMenu(hwnd, hMenu);
		if (hOldMenu) DestroyMenu(hOldMenu);
		UpdateMenuEnabledState(hwnd);
	}

	inline void UpdateLanguageUI(HWND hwnd) {
		const auto& s = GetStrings();
		if (hLabelFileList_) SetWindowTextW(hLabelFileList_, s.labelFileList);
		if (hLabelExpr_) SetWindowTextW(hLabelExpr_, s.labelExprPreview);

		if (hListView_) {
			LVCOLUMNW lvc;
			lvc.mask = LVCF_TEXT;
			lvc.pszText = (LPWSTR)s.lvColFilePath;
			ListView_SetColumn(hListView_, 0, &lvc);
		}

		RebuildMenu(hwnd);
	}


	// Helper function to update the expression display
	inline void UpdateExpressionDisplay() {
		try {
			if (hExprDisplay_) {
				SendMessage(hExprDisplay_, WM_APP + 1, 0, 0); // WM_UPDATE_EXPR
			}
		} catch (...) {
			MessageBox(NULL, L"UI Update error!", L"Warning", MB_OK | MB_ICONWARNING | MB_TOPMOST);
		}
	}

	// Helper function to add a file path to the list view.
	// Returns true on success. If returns false, UI should not change.
	inline bool AddFileToList(HWND hwnd, const std::wstring& filePath) {
		if (!hListView_) return false;

		LVITEMW lvi = { 0 };
		lvi.mask = LVIF_TEXT;
		lvi.pszText = const_cast<LPWSTR>(filePath.c_str()); // Safe cast for insertion
		lvi.iItem = ListView_GetItemCount(hListView_); // Insert at the end
		lvi.iSubItem = 0;

		int inserted = ListView_InsertItem(hListView_, &lvi);
		if (inserted < 0) {
			MessageBeep(MB_ICONWARNING);
			if (hwnd) MessageBoxW(hwnd, L"Failed to insert item into list view.", L"Warning", MB_OK | MB_ICONWARNING | MB_TOPMOST);
			return false;
		}

		// Keep UI/backend consistent: if backend rejects, rollback the UI insertion.
		if (!shared_data::pt_.push_filepath(filePath)) {
			ListView_DeleteItem(hListView_, inserted);
			return GuardUiOp(hwnd, false);
		}

		return true;
	}

	// Helper function to handle the "Open File" dialog logic
	inline void HandleFileOpen(HWND hwnd) {
		// Reject file selection while background rename is running.
		if (shared_data::pt_.get_state() == pt::ProcessThread::STATE_ONGOING) {
			GuardUiOp(hwnd, false);
			return;
		}

		// Heap memory
		constexpr DWORD OPENFILENAME_BUFFER = 32767;
		std::unique_ptr<wchar_t[]> szFile = std::make_unique<wchar_t[]>(OPENFILENAME_BUFFER);
		std::memset(szFile.get(), 0, OPENFILENAME_BUFFER * sizeof(wchar_t));

		OPENFILENAMEW ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile.get();
		ofn.nMaxFile = OPENFILENAME_BUFFER;

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
				AddFileToList(hwnd, dir);
			} else {
				// Multiple files were selected.
				// Loop through the subsequent null-terminated file names.
				while (*p) {
					std::wstring file = p;
					std::wstring fullPath = dir + L"\\" + file;
					AddFileToList(hwnd, fullPath);

					// Move to the next file name
					p += file.length() + 1;
				}
			}
		}
	}

}

#endif // !_UI_METHODS_HPP_