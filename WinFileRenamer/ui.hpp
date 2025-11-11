#ifndef _UI_HPP_
#define _UI_HPP_

#include <string>
#include <chrono>
#include <Windows.h>
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

#include "head_fpslimit.hpp"
#include "calc.hpp"

namespace ui {

constexpr int UI_WIDTH = 1280;
constexpr int UI_HEIGHT = 720;

std::stop_source sts_ui_;

struct RegisterReturn {
	WNDCLASSEX* wndclass;
	HWND hwnd;
};

LRESULT CALLBACK windowproc_main(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
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
	wndclass->hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndclass->lpszMenuName = NULL;
	wndclass->lpszClassName = TEXT("MainUIWindowClass");
	wndclass->hIconSm = NULL;

	if (!RegisterClassEx(wndclass)) {
		MessageBox(NULL, TEXT("Failed to register window."), TEXT("Error"), MB_ICONWARNING | MB_OK);
		sts_ui_.request_stop();
		return ret;
	}

	HWND hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		wndclass->lpszClassName,
		TEXT("WinFileRename"),
		(WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT,
		UI_WIDTH, UI_HEIGHT,
		NULL,
		NULL,
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