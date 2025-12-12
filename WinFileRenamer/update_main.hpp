#ifndef _UPDATE_MAIN_HPP_
#define _UPDATE_MAIN_HPP_

#include "ui.hpp"

inline void update_main_ui_LOOP(ui::RegisterReturn rrt) {

	WNDCLASSEX* wndclass = rrt.wndclass;
	HWND hwnd = rrt.hwnd;

	calc::warmup_operator_tables();
	ui::pt_.prewarm_expr_table();

	if (!ui::sts_ui_.stop_requested()) {

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		MSG msg{ 0 };
		while (GetMessage(&msg, NULL, 0, 0) && (!ui::sts_ui_.stop_requested())) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(ui::pt_.msg_box_.load()) {
				ui::pt_.msg_box_.store(false);
				MessageBox(hwnd, ui::pt_.get_res_wstr().c_str(), TEXT("Result"), MB_ICONINFORMATION | MB_OK | MB_TOPMOST);
			}

		}

	}

	ui::sts_ui_.request_stop();
}

#endif // !_UPDATE_MAIN_HPP_
