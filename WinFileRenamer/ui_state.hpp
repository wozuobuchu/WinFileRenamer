#ifndef _UI_STATE_HPP_
#define _UI_STATE_HPP_

#include <Windows.h>
#include <string>

namespace ui {

	// Handle for the list view control
	static HWND hListView_ = NULL;
	// Handles for new controls
	static HWND hExprDisplay_ = NULL;

	static HWND hLabelFileList_ = NULL;
	static HWND hLabelExpr_ = NULL;

	inline std::wstring oldDir;

	struct RegisterReturn {
		WNDCLASSEX* wndclass;
		HWND hwnd;
	};

}

#endif // !_UI_STATE_HPP_