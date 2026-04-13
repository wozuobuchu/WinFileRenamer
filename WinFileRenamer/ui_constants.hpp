#ifndef _UI_CONSTANTS_HPP_
#define _UI_CONSTANTS_HPP_

namespace ui {

	constexpr int UI_WIDTH = 1280;
	constexpr int UI_HEIGHT = 720;

	constexpr int ID_FILE_OPEN = 1001;
	constexpr int ID_FILE_CLEAR = 1002;
	constexpr int ID_LISTVIEW = 1003;
	constexpr int ID_OPTIONS_SUBMIT = 1004;
	constexpr int ID_OPTIONS_SUBMIT_AUTO = 1005;

	constexpr int ID_OPTIONS_EXIT = 9002;
	constexpr int ID_OPTIONS_HELP = 9008;

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

	constexpr int LABEL_HEIGHT = 24;

	constexpr int CONTENT_MARGIN_H = 40;
	constexpr int CONTENT_MAX_WIDTH = 1000;

}

#endif // !_UI_CONSTANTS_HPP_