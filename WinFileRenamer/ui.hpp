#ifndef _UI_HPP_
#define _UI_HPP_

#include "resource1.h"
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

#include "shared_data.hpp"

// Link the common controls library
#pragma comment(lib, "Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace ui {

	constexpr int UI_WIDTH = 1280;
	constexpr int UI_HEIGHT = 720;

	constexpr int ID_FILE_OPEN = 1001;
	constexpr int ID_FILE_CLEAR = 1002;
	constexpr int ID_LISTVIEW = 1003;
	constexpr int ID_OPTIONS_SUBMIT = 1004;
	constexpr int ID_OPTIONS_SUBMIT_AUTO = 1005;

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
	constexpr int ID_EDIT_PUSH_NUM_FORMAT = 2012;
	constexpr int ID_EDIT_CLEAR = 2013;

	constexpr int ID_LANG_EN = 9003;
	constexpr int ID_LANG_ZH = 9004;
	constexpr int ID_LANG_ZH_TW = 9005;
	constexpr int ID_LANG_JA = 9006;
	constexpr int ID_LANG_RU = 9007;

	constexpr int ID_EXPR_DISPLAY = 8001;
	constexpr int ID_INPUT_EDIT = 8002;


	// Handle for the list view control
	static HWND hListView_ = NULL;
	// Handles for new controls
	static HWND hExprDisplay_ = NULL;
	static HWND hInputEdit_ = NULL;

	static HWND hLabelFileList_ = NULL;
	static HWND hLabelExpr_ = NULL;
	static HWND hLabelInput_ = NULL;

	constexpr int LABEL_HEIGHT = 24;
	constexpr int INPUT_HEIGHT = 40;

	constexpr int CONTENT_MARGIN_H = 40;
	constexpr int CONTENT_MAX_WIDTH = 1000;

	inline std::wstring oldDir;

	enum class Lang { EN, ZH, ZH_TW, JA, RU };
	inline Lang default_lang = Lang::EN;

	struct UIStrings {
		const wchar_t* fileMenu;
		const wchar_t* exprMenu;
		const wchar_t* optionMenu;

		const wchar_t* fileOpen;
		const wchar_t* fileClear;
		const wchar_t* fileSubmit;
		const wchar_t* fileSubmitAuto;

		const wchar_t* exprConstants;
		const wchar_t* exprPushStr;
		const wchar_t* exprPushNum;
		const wchar_t* exprPushFmt;

		const wchar_t* exprVars;
		const wchar_t* exprPushIdx;
		const wchar_t* exprPushOfname;

		const wchar_t* exprOps;
		const wchar_t* exprPushAdd;
		const wchar_t* exprPushSub;
		const wchar_t* exprPushMul;
		const wchar_t* exprPushDiv;

		const wchar_t* exprBrackets;
		const wchar_t* exprPushLb;
		const wchar_t* exprPushRb;

		const wchar_t* exprDel;
		const wchar_t* exprClear;

		const wchar_t* optLang;
		const wchar_t* optLangEn;
		const wchar_t* optLangZh;
		const wchar_t* optLangZhTw;
		const wchar_t* optLangJa;
		const wchar_t* optLangRu;
		const wchar_t* optExit;

		const wchar_t* labelFileList;
		const wchar_t* labelExprPreview;
		const wchar_t* labelInput;
		const wchar_t* lvColFilePath;
	};

	inline UIStrings strings_en = {
		L"File", L"Expression", L"Options",
		L"Open", L"Clear", L"Submit Rename", L"Auto Match Subtitles",
		L"Constants", L"Push String [INPUT]", L"Push Number [INPUT]", L"Push Minimum Num Length [INPUT]",
		L"Variables", L"Push Index", L"Push OriginFileName",
		L"Operators", L"Add (+)", L"Sub (-)", L"Mul (*)", L"Div (/)",
		L"Brackets", L"Left Bracket (", L"Right Bracket )",
		L"Delete Last", L"Clear Expression",
		L"Language", L"English", L"Chinese (Simplified)", L"Chinese (Traditional)", L"Japanese", L"Russian", L"Exit",
		L"Selected Files", L"Expression Preview", L"Input", L"File Path"
	};

	inline UIStrings strings_zh = {
		L"文件", L"表达式", L"选项",
		L"打开", L"清空", L"应用重命名", L"自动匹配字幕名",
		L"常量", L"添加字符串 [输入框]", L"添加数字 [输入框]", L"添加最小数字格式 [输入框]",
		L"变量", L"添加序号", L"添加原始文件名",
		L"运算符", L"加 (+)", L"减 (-)", L"乘 (*)", L"除 (/)",
		L"括号", L"左括号 (", L"右括号 )",
		L"删除上一个", L"清空表达式",
		L"语言", L"English", L"中文(简体)", L"中文(繁体)", L"日本語", L"Русский", L"退出",
		L"已选文件", L"表达式预览", L"输入框", L"文件路径"
	};

	inline UIStrings strings_zh_tw = {
		L"檔案", L"運算式", L"選項",
		L"開啟", L"清空", L"套用重新命名", L"自動配對字幕名",
		L"常數", L"加入字串 [輸入框]", L"加入數字 [輸入框]", L"加入最小數字格式 [輸入框]",
		L"變數", L"加入序號", L"加入原始檔名",
		L"運算子", L"加 (+)", L"減 (-)", L"乘 (*)", L"除 (/)",
		L"括號", L"左括號 (", L"右括號 )",
		L"刪除上一個", L"清空運算式",
		L"語言", L"English", L"中文(簡體)", L"中文(繁體)", L"日本語", L"Русский", L"退出",
		L"已選檔案", L"運算式預覽", L"輸入框", L"檔案路徑"
	};

	inline UIStrings strings_ja = {
		L"ファイル", L"式", L"オプション",
		L"開く", L"クリア", L"名前変更を適用", L"字幕を自動マッチ",
		L"定数", L"文字列を追加 [入力]", L"数値を追加 [入力]", L"最小数値形式を追加 [入力]",
		L"変数", L"連番を追加", L"元のファイル名を追加",
		L"演算子", L"加算 (+)", L"減算 (-)", L"乗算 (*)", L"除算 (/)",
		L"括弧", L"左括弧 (", L"右括弧 )",
		L"最後を削除", L"式をクリア",
		L"言語", L"English", L"中文(簡体)", L"中文(繁体)", L"日本語", L"Русский", L"終了",
		L"選択されたファイル", L"式のプレビュー", L"入力", L"ファイルパス"
	};

	inline UIStrings strings_ru = {
		L"Файл", L"Выражение", L"Настройки",
		L"Открыть", L"Очистить", L"Применить", L"Авто-подбор субтитров",
		L"Константы", L"Добавить строку [ВВОД]", L"Добавить число [ВВОД]", L"Добавить мин. длину числа [ВВОД]",
		L"Переменные", L"Добавить индекс", L"Добавить исх. имя файла",
		L"Операторы", L"Сложение (+)", L"Вычитание (-)", L"Умножение (*)", L"Деление (/)",
		L"Скобки", L"Левая скобка (", L"Правая скобка )",
		L"Удалить последнее", L"Очистить выражение",
		L"Язык", L"English", L"Chinese (Simplified)", L"Chinese (Traditional)", L"Japanese", L"Русский", L"Выход",
		L"Выбранные файлы", L"Предпросмотр выражения", L"Ввод", L"Путь к файлу"
	};

	inline const UIStrings& GetStrings() {
		switch (default_lang) {
			case Lang::ZH: return strings_zh;
			case Lang::ZH_TW: return strings_zh_tw;
			case Lang::JA: return strings_ja;
			case Lang::RU: return strings_ru;
			case Lang::EN:
			default: return strings_en;
		}
	}

	struct RegisterReturn {
		WNDCLASSEX* wndclass;
		HWND hwnd;
	};


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
		AppendMenu(hLangMenu, MF_STRING | (default_lang == Lang::EN ? MF_CHECKED : MF_UNCHECKED), ID_LANG_EN, s.optLangEn);
		AppendMenu(hLangMenu, MF_STRING | (default_lang == Lang::ZH ? MF_CHECKED : MF_UNCHECKED), ID_LANG_ZH, s.optLangZh);
		AppendMenu(hLangMenu, MF_STRING | (default_lang == Lang::ZH_TW ? MF_CHECKED : MF_UNCHECKED), ID_LANG_ZH_TW, s.optLangZhTw);
		AppendMenu(hLangMenu, MF_STRING | (default_lang == Lang::JA ? MF_CHECKED : MF_UNCHECKED), ID_LANG_JA, s.optLangJa);
		AppendMenu(hLangMenu, MF_STRING | (default_lang == Lang::RU ? MF_CHECKED : MF_UNCHECKED), ID_LANG_RU, s.optLangRu);

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
		if (hLabelInput_) SetWindowTextW(hLabelInput_, s.labelInput);

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
				std::wstring exprStr = shared_data::pt_.get_expression_str();
				SetWindowTextW(hExprDisplay_, exprStr.c_str());
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

	LRESULT CALLBACK windowproc_main(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {

			case WM_SYSCOMMAND:
			{
				if (wParam == SC_CLOSE) {
					shared_data::pt_.join();
					PostQuitMessage(0);
					shared_data::sts_.request_stop();
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
				HDC hdc = GetDC(hwnd);
				HFONT hFont = CreateFontW(-MulDiv(11, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
				ReleaseDC(hwnd, hdc);

				hLabelFileList_ = CreateWindowEx(
					0,
					L"STATIC",
					L"Selected Files",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 4, UI_WIDTH - 16, LABEL_HEIGHT,
					hwnd,
					NULL,
					GetModuleHandle(NULL),
					NULL
				);

				// Create the List View control (Top Half)
				int topHeight = UI_HEIGHT / 2;

				hListView_ = CreateWindowEx(
					WS_EX_CLIENTEDGE,
					WC_LISTVIEWW, // Use the wide-character version
					L"",
					WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS,
					0, LABEL_HEIGHT + 4,
					UI_WIDTH,
					topHeight - (LABEL_HEIGHT + 4),
					hwnd,
					(HMENU)(UINT_PTR)ID_LISTVIEW,
					GetModuleHandle(NULL),
					NULL
				);

				DWORD exStyle = ListView_GetExtendedListViewStyle(hListView_);
				exStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
				ListView_SetExtendedListViewStyle(hListView_, exStyle);

				// Initialize the list view columns
				LVCOLUMNW lvc = { 0 };
				lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
				lvc.cx = UI_WIDTH - 20; // Initial width
				lvc.pszText = (LPWSTR)L"File Path";
				ListView_InsertColumn(hListView_, 0, &lvc);

				HWND hHeader = ListView_GetHeader(hListView_);
				if (hHeader)
				{
					LONG style = GetWindowLong(hHeader, GWL_STYLE);

					style &= ~HDS_HOTTRACK;
					style &= ~HDS_BUTTONS;

					SetWindowLong(hHeader, GWL_STYLE, style);

					InvalidateRect(hHeader, NULL, TRUE);
				}

				int bottomY = topHeight;
				int bottomHeight = UI_HEIGHT - topHeight;

				hLabelExpr_ = CreateWindowEx(
					0,
					L"STATIC",
					L"Expression Preview",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, bottomY + 4,
					UI_WIDTH - 16, LABEL_HEIGHT,
					hwnd,
					NULL,
					GetModuleHandle(NULL),
					NULL
				);

				// Create Expression Display (Bottom Half - Top)
				int exprDisplayY = bottomY + 4 + LABEL_HEIGHT;
				int exprDisplayHeight = bottomHeight - INPUT_HEIGHT - LABEL_HEIGHT * 2 - 8;
				if (exprDisplayHeight < 0) exprDisplayHeight = 0;

				hExprDisplay_ = CreateWindowEx(
					WS_EX_CLIENTEDGE,
					L"EDIT", // EDIT control for better scrolling and word wrapping
					L"",       // Initial text
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
					0, exprDisplayY, UI_WIDTH, exprDisplayHeight,
					hwnd,
					(HMENU)(UINT_PTR)ID_EXPR_DISPLAY,
					GetModuleHandle(NULL),
					NULL
				);

				int inputLabelY = bottomY + bottomHeight - INPUT_HEIGHT - LABEL_HEIGHT - 4;
				if (inputLabelY < exprDisplayY) inputLabelY = exprDisplayY;

				hLabelInput_ = CreateWindowEx(
					0,
					L"STATIC",
					L"Input",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, inputLabelY,
					UI_WIDTH - 16, LABEL_HEIGHT,
					hwnd,
					NULL,
					GetModuleHandle(NULL),
					NULL
				);

				int inputEditY = inputLabelY + LABEL_HEIGHT;

				hInputEdit_ = CreateWindowEx(
					WS_EX_CLIENTEDGE,
					L"EDIT",
					L"",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_MULTILINE,
					0, inputEditY, UI_WIDTH, INPUT_HEIGHT, // Initial size
					hwnd,
					(HMENU)(UINT_PTR)ID_INPUT_EDIT,
					GetModuleHandle(NULL),
					NULL
				);

				if (hFont) {
					SendMessage(hLabelFileList_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hLabelExpr_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hLabelInput_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hListView_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hExprDisplay_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hInputEdit_, WM_SETFONT, (WPARAM)hFont, TRUE);
				}

				UpdateMenuEnabledState(hwnd);

				return 0;
			}


			case WM_COMMAND:
			{
				// Handle menu commands
				switch (LOWORD(wParam)) {

					case ID_FILE_OPEN:
					{
						HandleFileOpen(hwnd);
						break;
					}

					case ID_FILE_CLEAR:
					{
						if (!shared_data::pt_.reset_selected_file()) {
							GuardUiOp(hwnd, false);
							break;
						}
						ListView_DeleteAllItems(hListView_);
						break;
					}

					case ID_OPTIONS_SUBMIT:
					{
						// Call the process_lunch function
						if (!shared_data::pt_.process_launch(0)) {
							MessageBox(hwnd, L"Process is already ongoing!", L"Warning", MB_OK | MB_ICONWARNING | MB_TOPMOST);
						} else {
							UpdateMenuEnabledState(hwnd);
						}
						break;
					}

					case ID_OPTIONS_SUBMIT_AUTO:
					{
						if (!shared_data::pt_.process_launch(1)) {
							MessageBox(hwnd, L"Process is already ongoing!", L"Warning", MB_OK | MB_ICONWARNING | MB_TOPMOST);
						} else {
							UpdateMenuEnabledState(hwnd);
						}
						break;
					}

					case ID_OPTIONS_EXIT:
					{
						PostQuitMessage(0);
						shared_data::sts_.request_stop();
						break;
					}

					case ID_LANG_EN:
					{
						default_lang = Lang::EN;
						UpdateLanguageUI(hwnd);
						break;
					}

					case ID_LANG_ZH:
					{
						default_lang = Lang::ZH;
						UpdateLanguageUI(hwnd);
						break;
					}

					case ID_LANG_ZH_TW:
					{
						default_lang = Lang::ZH_TW;
						UpdateLanguageUI(hwnd);
						break;
					}

					case ID_LANG_JA:
					{
						default_lang = Lang::JA;
						UpdateLanguageUI(hwnd);
						break;
					}

					case ID_LANG_RU:
					{
						default_lang = Lang::RU;
						UpdateLanguageUI(hwnd);
						break;
					}


					// Handlers for Edit Menu
					case ID_EDIT_PUSH_STR:
					{
						wchar_t buffer[256] = { 0 };
						GetWindowTextW(hInputEdit_, buffer, 256);
						if (!shared_data::pt_.push_expr<calc::Str>(std::wstring(buffer))) {
							GuardUiOp(hwnd, false);
							break;
						}
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
							if (!shared_data::pt_.push_expr<calc::Int64>(val)) {
								GuardUiOp(hwnd, false);
								break;
							}
							UpdateExpressionDisplay();
							SetWindowTextW(hInputEdit_, L""); // Clear input box
						} catch (...) {
							MessageBoxW(hwnd, L"Invalid number. Please enter a valid 64-bit integer.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
						}
						break;
					}
					case ID_EDIT_PUSH_IDX:
					{
						if (!shared_data::pt_.push_expr<calc::Index_Var>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_OFNAME:
					{
						if (!shared_data::pt_.push_expr<calc::OriginFileName_Var>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_NUM_FORMAT:
					{
						wchar_t buffer[256] = { 0 };
						GetWindowTextW(hInputEdit_, buffer, 256);
						try {
							int64_t val = std::stoll(std::wstring(buffer));
							if ((val < 0) || (val > 100)) {
								MessageBoxW(hwnd, L"Number format out of range.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
								break;
							}
							if (!shared_data::pt_.push_expr<calc::Int64_Format>(val)) {
								GuardUiOp(hwnd, false);
								break;
							}
							UpdateExpressionDisplay();
							SetWindowTextW(hInputEdit_, L""); // Clear input box
						} catch (...) {
							MessageBoxW(hwnd, L"Invalid number. Please enter a valid 64-bit integer.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
						}
						break;
					}
					case ID_EDIT_PUSH_LB:
					{
						if (!shared_data::pt_.push_expr<calc::Lbracket>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_RB:
					{
						if (!shared_data::pt_.push_expr<calc::Rbracket>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_ADD:
					{
						if (!shared_data::pt_.push_expr<calc::Add_Int64Opt>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_SUB:
					{
						if (!shared_data::pt_.push_expr<calc::Sub_Int64Opt>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_MUL:
					{
						if (!shared_data::pt_.push_expr<calc::Mul_Int64Opt>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_DIV:
					{
						if (!shared_data::pt_.push_expr<calc::Div_Int64Opt>()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_PUSH_DEL:
					{
						if (!shared_data::pt_.pop_expr_ptr()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}
					case ID_EDIT_CLEAR:
					{
						if (!shared_data::pt_.reset_input_expr_ptr()) {
							GuardUiOp(hwnd, false);
							break;
						}
						UpdateExpressionDisplay();
						break;
					}

				}
				return 0;
			}

			case WM_SIZE:
			{
				if (wParam == SIZE_MINIMIZED) return 0;
				// Debounce window resize (lazy evaluation) to prevent UI freezing with large lists
				KillTimer(hwnd, 9999);
				SetTimer(hwnd, 9999, 30, NULL);
				return 0;
			}

			case WM_TIMER:
			{
				if (wParam == 9999) {
					KillTimer(hwnd, 9999);

					RECT rc;
					GetClientRect(hwnd, &rc);

					int width = rc.right - rc.left;
					int height = rc.bottom - rc.top;

					int marginX = 20;
					int marginY = 10;
					int spacingY = 10;

					int contentX = marginX;
					int contentWidth = width - 2 * marginX;
					if (contentWidth < 100) contentWidth = 100;

					// Top section: List View (takes about 50%)
					int listHeight = (height - 2 * marginY - 2 * LABEL_HEIGHT - INPUT_HEIGHT - 3 * spacingY) * 6 / 10;
					if (listHeight < 100) listHeight = 100;

					int currentY = marginY;

					// BeginDeferWindowPos could also be used here, but simple move is fine
					if (hLabelFileList_) {
						MoveWindow(hLabelFileList_, contentX, currentY, contentWidth, LABEL_HEIGHT, TRUE);
					}
					currentY += LABEL_HEIGHT;

					if (hListView_) {
						MoveWindow(hListView_, contentX, currentY, contentWidth, listHeight, TRUE);
						ListView_SetColumnWidth(hListView_, 0, contentWidth - 25); // Account for scrollbar
					}
					currentY += listHeight + spacingY;

					if (hLabelExpr_) {
						MoveWindow(hLabelExpr_, contentX, currentY, contentWidth, LABEL_HEIGHT, TRUE);
					}
					currentY += LABEL_HEIGHT;

					// Middle section: Expression Display
					int exprHeight = (height - currentY - marginY - LABEL_HEIGHT - INPUT_HEIGHT - spacingY);
					// Guarantee some height
					if (exprHeight < 60) {
						exprHeight = 60;
					}

					if (hExprDisplay_) {
						MoveWindow(hExprDisplay_, contentX, currentY, contentWidth, exprHeight, TRUE);
					}
					currentY += exprHeight + spacingY;

					if (hLabelInput_) {
						MoveWindow(hLabelInput_, contentX, currentY, contentWidth, LABEL_HEIGHT, TRUE);
					}
					currentY += LABEL_HEIGHT;

					if (hInputEdit_) {
						MoveWindow(hInputEdit_, contentX, currentY, contentWidth, INPUT_HEIGHT, TRUE);
					}
				}
				return 0;
			}

			case WM_INITMENU:
			case WM_INITMENUPOPUP:
			{
				UpdateMenuEnabledState(hwnd);
				break;
			}


			case WM_CTLCOLORSTATIC:
			{
				HDC hdc = (HDC)wParam;
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(0, 0, 0));
				return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
			}

			case WM_DESTROY:
			{
				shared_data::pt_.join();
				PostQuitMessage(0);
				shared_data::sts_.request_stop();
				return 0;
			}

			default:
			{
				break;
			}

		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	RegisterReturn register_main_ui(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

		// Store the current directory
		DWORD len = GetCurrentDirectoryW(0, nullptr);
		if (len != 0) {
			std::wstring buffer(len, L'\0');
			DWORD realLen = GetCurrentDirectoryW(len, buffer.data());
			if (realLen != 0) {
				buffer.resize(realLen);
				ui::oldDir = std::move(buffer);
			}
		}

		shared_data::pt_.set_old_dir(ui::oldDir);

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
		wndclass->hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wndclass->hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Standard window background
		wndclass->lpszMenuName = NULL; // We set the menu on the window, not the class
		wndclass->lpszClassName = TEXT("MainUIWindowClass");
		wndclass->hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

		if (!RegisterClassEx(wndclass)) {
			MessageBox(NULL, TEXT("Failed to register window."), TEXT("Error"), MB_ICONWARNING | MB_OK);
			shared_data::sts_.request_stop();
			return ret;
		}

		HWND hwnd = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			wndclass->lpszClassName,
			TEXT("WinFileRenamer"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			UI_WIDTH, UI_HEIGHT,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (hwnd == NULL) {
			MessageBox(NULL, TEXT("Failed to create window."), TEXT("Error"), MB_ICONWARNING | MB_OK);
			shared_data::sts_.request_stop();
			return ret;
		}

		UpdateLanguageUI(hwnd);

		ret.wndclass = wndclass;
		ret.hwnd = hwnd;

		return ret;
	}

}

#endif // !_UI_HPP_