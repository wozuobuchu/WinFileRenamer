#ifndef _UPDATE_MAIN_HPP_
#define _UPDATE_MAIN_HPP_

#include "ui.hpp"

inline void update_main_ui_LOOP(ui::RegisterReturn rrt) {

	WNDCLASSEX* wndclass = rrt.wndclass;
	HWND hwnd = rrt.hwnd;
	(void)wndclass;

	calc::warmup_operator_tables();

	if (!shared_data::sts_.stop_requested()) {

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		MSG msg{ 0 };
		while (GetMessage(&msg, NULL, 0, 0) && (!shared_data::sts_.stop_requested())) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(shared_data::pt_.get_and_clear_msg_box()) {
				SetCurrentDirectoryW(pt::ProcessThread::get_old_dir().c_str());
				MessageBox(hwnd, shared_data::pt_.get_res_wstr().c_str(), TEXT("Result"), MB_ICONINFORMATION | MB_OK | MB_TOPMOST);
			}

		}

	}

	shared_data::sts_.request_stop();
}

#endif // !_UPDATE_MAIN_HPP_
