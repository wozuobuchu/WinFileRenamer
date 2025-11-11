#ifndef _UPDATE_MAIN_HPP_
#define _UPDATE_MAIN_HPP_

#include "ui.hpp"

inline void update_main_ui_LOOP(ui::RegisterReturn rrt) {

	WNDCLASSEX* wndclass = rrt.wndclass;
	HWND hwnd = rrt.hwnd;

	if (!ui::sts_ui_.stop_requested()) {

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		MSG msg{ 0 };
		while (GetMessage(&msg, hwnd, 0, 0) && (!ui::sts_ui_.stop_requested())) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	ui::sts_ui_.request_stop();
}

#endif // !_UPDATE_MAIN_HPP_
