#ifndef _UI_HPP_
#define _UI_HPP_

#include "resource.hpp"
#include <Windows.h>
#include <CommCtrl.h>
#include <commdlg.h>

#include <string>
#include <vector>
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

#include "ui_constants.hpp"
#include "ui_state.hpp"
#include "ui_language.hpp"
#include "ui_inputbox.hpp"
#include "ui_methods.hpp"

namespace ui {

	struct TokenBlock {
		std::wstring txt = L"";
		int64_t type = 0;
		RECT rc = {0, 0, 0, 0};
		COLORREF bgCol = RGB(0, 0, 0);
		COLORREF borderCol = RGB(0, 0, 0);
		COLORREF textCol = RGB(0, 0, 0);
	};

	struct PreviewState {
		int scrollY = 0;
		int totalHeight = 0;
		std::vector<TokenBlock> blocks;
		HFONT hFont = NULL;
	};

	#define WM_UPDATE_EXPR (WM_APP + 1)

	inline LRESULT CALLBACK ExprPreviewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_NCCREATE:
			{
				PreviewState* state = new PreviewState();
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
				return TRUE;
			}
			case WM_CREATE:
			{
				PreviewState* state = (PreviewState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
				HDC hdc = GetDC(hwnd);
				// Make font larger
				state->hFont = CreateFontW(-MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
				ReleaseDC(hwnd, hdc);
				return 0;
			}
			case WM_DESTROY:
			{
				PreviewState* state = (PreviewState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
				if (state) {
					if (state->hFont) DeleteObject(state->hFont);
					delete state;
					SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
				}
				return 0;
			}
			case WM_SIZE:
			{
				if (wParam == SIZE_MINIMIZED) return 0;
				KillTimer(hwnd, 8888);
				SetTimer(hwnd, 8888, 30, NULL);
				return 0;
			}
			case WM_TIMER:
			{
				if (wParam == 8888) {
					KillTimer(hwnd, 8888);
					SendMessage(hwnd, WM_UPDATE_EXPR, 0, 0);
				}
				return 0;
			}
			case WM_UPDATE_EXPR:
			{
				PreviewState* state = (PreviewState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
				if (!state) return 0;

				RECT rc;
				GetClientRect(hwnd, &rc);
				int clientW = rc.right - rc.left;

				auto tokens = shared_data::pt_.get_expression_tokens();
				state->blocks.clear();

				HDC hdc = GetDC(hwnd);
				HFONT oldFont = (HFONT)SelectObject(hdc, state->hFont);

				int x = 5;
				int y = 5;
				int lineHeight = 30;

				for (const auto& token : tokens) {
					int64_t type = token.first;
					std::wstring txt = token.second;
					while(!txt.empty() && txt.back() == L' ') txt.pop_back();

					SIZE sz;
					GetTextExtentPoint32W(hdc, txt.c_str(), (int)txt.length(), &sz);

					int paddingX = 10;
					int paddingY = 4;
					int blockW = sz.cx + paddingX * 2;
					int blockH = sz.cy + paddingY * 2;
					lineHeight = max(lineHeight, blockH + 10);

					if (x + blockW + 5 > clientW && x > 5) {
						x = 5;
						y += lineHeight;
					}

					COLORREF bgCol = RGB(230, 230, 230);
					COLORREF borderCol = RGB(180, 180, 180);
					COLORREF textCol = RGB(0, 0, 0);

					if (type == 'S') { bgCol = RGB(212, 237, 218); borderCol = RGB(195, 230, 203); textCol = RGB(21, 87, 36); }
					else if (type == 'Z' || type == 'F') { bgCol = RGB(204, 229, 255); borderCol = RGB(184, 218, 255); textCol = RGB(0, 64, 133); }
					else if (type == 'X') { bgCol = RGB(255, 243, 205); borderCol = RGB(255, 238, 186); textCol = RGB(133, 100, 4); }
					else if (type == '#' || type == '+' || type == '-' || type == '*' || type == '/') { bgCol = RGB(248, 215, 218); borderCol = RGB(245, 198, 203); textCol = RGB(114, 28, 36); }
					else if (type == '(' || type == ')') { bgCol = RGB(226, 227, 229); borderCol = RGB(214, 216, 219); textCol = RGB(56, 61, 65); }

					TokenBlock blk;
					blk.txt = txt;
					blk.type = type;
					blk.rc = { x, y, x + blockW, y + blockH };
					blk.bgCol = bgCol;
					blk.borderCol = borderCol;
					blk.textCol = textCol;

					state->blocks.push_back(blk);
					x += blockW + 5;
				}

				SelectObject(hdc, oldFont);
				ReleaseDC(hwnd, hdc);

				state->totalHeight = y + lineHeight + 5;

				int maxScroll = state->totalHeight - (rc.bottom - rc.top);
				if (maxScroll < 0) maxScroll = 0;
				if (state->scrollY > maxScroll) state->scrollY = maxScroll;

				SCROLLINFO si = { sizeof(SCROLLINFO) };
				si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
				si.nMin = 0;
				si.nMax = max(state->totalHeight, 0);
				si.nPage = rc.bottom - rc.top;
				si.nPos = state->scrollY;
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

				InvalidateRect(hwnd, NULL, TRUE);
				return 0;
			}
			case WM_PAINT:
			{
				PreviewState* state = (PreviewState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
				if (!state) return DefWindowProc(hwnd, uMsg, wParam, lParam);

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);

				RECT rc;
				GetClientRect(hwnd, &rc);

				HDC memDC = CreateCompatibleDC(hdc);
				HBITMAP memBm = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
				HBITMAP oldBm = (HBITMAP)SelectObject(memDC, memBm);

				HBRUSH bgBrush = GetSysColorBrush(COLOR_WINDOW);
				FillRect(memDC, &rc, bgBrush);

				HFONT oldFont = (HFONT)SelectObject(memDC, state->hFont);
				SetBkMode(memDC, TRANSPARENT);

				for (const auto& blk : state->blocks) {
					RECT blockRc = blk.rc;
					blockRc.top -= state->scrollY;
					blockRc.bottom -= state->scrollY;

					if (blockRc.bottom < 0 || blockRc.top > rc.bottom) continue;

					HBRUSH blockBrush = CreateSolidBrush(blk.bgCol);
					HPEN blockPen = CreatePen(PS_SOLID, 1, blk.borderCol);
					HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, blockBrush);
					HPEN oldPen = (HPEN)SelectObject(memDC, blockPen);

					RoundRect(memDC, blockRc.left, blockRc.top, blockRc.right, blockRc.bottom, 8, 8);

					SetTextColor(memDC, blk.textCol);
					RECT textRc = blockRc;
					DrawTextW(memDC, blk.txt.c_str(), -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					SelectObject(memDC, oldBrush);
					SelectObject(memDC, oldPen);
					DeleteObject(blockBrush);
					DeleteObject(blockPen);
				}

				SelectObject(memDC, oldFont);

				BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, memDC, 0, 0, SRCCOPY);

				SelectObject(memDC, oldBm);
				DeleteObject(memBm);
				DeleteDC(memDC);

				EndPaint(hwnd, &ps);
				return 0;
			}
			case WM_VSCROLL:
			{
				PreviewState* state = (PreviewState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
				if (!state) return 0;

				SCROLLINFO si = { sizeof(SCROLLINFO) };
				si.fMask = SIF_ALL;
				GetScrollInfo(hwnd, SB_VERT, &si);

				int yPos = si.nPos;
				switch (LOWORD(wParam)) {
					case SB_TOP: yPos = si.nMin; break;
					case SB_BOTTOM: yPos = si.nMax; break;
					case SB_LINEUP: yPos -= 30; break;
					case SB_LINEDOWN: yPos += 30; break;
					case SB_PAGEUP: yPos -= si.nPage; break;
					case SB_PAGEDOWN: yPos += si.nPage; break;
					case SB_THUMBTRACK: yPos = si.nTrackPos; break;
				}

				int maxPos = si.nMax - max((int)si.nPage - 1, 0);
				if (maxPos < 0) maxPos = 0;
				if (yPos < si.nMin) yPos = si.nMin;
				if (yPos > maxPos) yPos = maxPos;

				if (yPos != si.nPos) {
					SetScrollPos(hwnd, SB_VERT, yPos, TRUE);
					state->scrollY = yPos;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				return 0;
			}
			case WM_MOUSEWHEEL:
			{
				PreviewState* state = (PreviewState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
				if (!state) return 0;

				int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				int scrollLines = 3;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);

				SCROLLINFO si = { sizeof(SCROLLINFO) };
				si.fMask = SIF_ALL;
				GetScrollInfo(hwnd, SB_VERT, &si);

				int yPos = si.nPos;
				yPos -= (zDelta / WHEEL_DELTA) * scrollLines * 10;

				int maxPos = si.nMax - max((int)si.nPage - 1, 0);
				if (maxPos < 0) maxPos = 0;
				if (yPos < si.nMin) yPos = si.nMin;
				if (yPos > maxPos) yPos = maxPos;

				if (yPos != si.nPos) {
					SetScrollPos(hwnd, SB_VERT, yPos, TRUE);
					state->scrollY = yPos;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				return 0;
			}
			case WM_ERASEBKGND:
				return 1;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_CLIPSIBLINGS,
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
					WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | WS_CLIPSIBLINGS,
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
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_CLIPSIBLINGS,
					8, bottomY + 4,
					UI_WIDTH - 16, LABEL_HEIGHT,
					hwnd,
					NULL,
					GetModuleHandle(NULL),
					NULL
				);

				// Create Expression Display (Bottom Half)
				int exprDisplayY = bottomY + 4 + LABEL_HEIGHT;
				int exprDisplayHeight = bottomHeight - LABEL_HEIGHT - 12; // Adjust padding
				if (exprDisplayHeight < 0) exprDisplayHeight = 0;

				hExprDisplay_ = CreateWindowEx(
					WS_EX_CLIENTEDGE,
					L"ModernExprPreview", 
					L"",       
					WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | WS_CLIPSIBLINGS,
					0, exprDisplayY, UI_WIDTH, exprDisplayHeight,
					hwnd,
					(HMENU)(UINT_PTR)ID_EXPR_DISPLAY,
					GetModuleHandle(NULL),
					NULL
				);

				if (hFont) {
					SendMessage(hLabelFileList_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hLabelExpr_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hListView_, WM_SETFONT, (WPARAM)hFont, TRUE);
					SendMessage(hExprDisplay_, WM_SETFONT, (WPARAM)hFont, TRUE);
				}

				UpdateMenuEnabledState(hwnd);

				return 0;
			}


			case WM_COMMAND:
			{
				int cmd = LOWORD(wParam);

				// Check if the command is a language switch
				bool lang_changed = false;
				for (size_t i = 0; i < supported_languages.size(); ++i) {
					if (cmd == supported_languages[i].cmdId) {
						current_lang_index = i;
						UpdateLanguageUI(hwnd);
						lang_changed = true;
						break;
					}
				}
				if (lang_changed) return 0;

				// Handle menu commands
				switch (cmd) {

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

					case ID_OPTIONS_HELP:
					{
						MessageBoxW(hwnd, GetHelpText().c_str(), GetStrings().optHelp, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
						break;
					}

					// Handlers for Edit Menu
					case ID_EDIT_PUSH_STR:
					{
						std::wstring inputStr;
						if (ShowInputBox(hwnd, GetStrings().labelInput, GetStrings().exprPushStr, inputStr)) {
							if (inputStr.empty()) {
								MessageBoxW(hwnd, L"Input cannot be empty.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
								break;
							}
							if (!shared_data::pt_.push_expr<calc::Str>(inputStr)) {
								GuardUiOp(hwnd, false);
								break;
							}
							UpdateExpressionDisplay();
						}
						break;
					}

					case ID_EDIT_PUSH_NUM:
					{
						std::wstring inputStr;
						if (ShowInputBox(hwnd, GetStrings().labelInput, GetStrings().exprPushNum, inputStr)) {
							try {
								int64_t val = std::stoll(inputStr);
								if (!shared_data::pt_.push_expr<calc::Int64>(val)) {
									GuardUiOp(hwnd, false);
									break;
								}
								UpdateExpressionDisplay();
							} catch (...) {
								MessageBoxW(hwnd, L"Invalid number. Please enter a valid 64-bit integer.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
							}
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
						std::wstring inputStr;
						if (ShowInputBox(hwnd, GetStrings().labelInput, GetStrings().exprPushFmt, inputStr)) {
							try {
								int64_t val = std::stoll(inputStr);
								if ((val < 0) || (val > 100)) {
									MessageBoxW(hwnd, L"Number format out of range.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
									break;
								}
								if (!shared_data::pt_.push_expr<calc::Int64_Format>(val)) {
									GuardUiOp(hwnd, false);
									break;
								}
								UpdateExpressionDisplay();
							} catch (...) {
								MessageBoxW(hwnd, L"Invalid number. Please enter a valid 64-bit integer.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
							}
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
					int listHeight = (height - 2 * marginY - 2 * LABEL_HEIGHT - 2 * spacingY) * 6 / 10;
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
					int exprHeight = (height - currentY - marginY);
					// Guarantee some height
					if (exprHeight < 60) {
						exprHeight = 60;
					}

					if (hExprDisplay_) {
						MoveWindow(hExprDisplay_, contentX, currentY, contentWidth, exprHeight, TRUE);
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
		(void)hInstance;
		(void)hPrevInstance;
		(void)lpCmdLine;
		(void)nCmdShow;

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


		WNDCLASSEXW wndclassPreview = { sizeof(WNDCLASSEXW) };
		wndclassPreview.lpfnWndProc = ExprPreviewProc;
		wndclassPreview.hInstance = hInstance;
		wndclassPreview.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclassPreview.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wndclassPreview.lpszClassName = L"ModernExprPreview";
		RegisterClassExW(&wndclassPreview);

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
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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