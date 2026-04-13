#ifndef _UI_LANGUAGE_HPP_
#define _UI_LANGUAGE_HPP_

#include <string>
#include <array>
#include "ui_constants.hpp"

namespace ui {

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
		const wchar_t* optExit;
		const wchar_t* optHelp;

		const wchar_t* labelFileList;
		const wchar_t* labelExprPreview;
		const wchar_t* labelInput;
		const wchar_t* lvColFilePath;
	};

	struct LanguageDef {
		int cmdId;
		const wchar_t* menuName;
		UIStrings strings;
	};

	inline constexpr std::array<LanguageDef, 5> supported_languages = {
		LanguageDef {
			ID_LANG_EN, L"English",
			{
				L"File", L"Expression", L"Options",
				L"Open", L"Clear", L"Submit Rename", L"Auto Match Subtitles",
				L"Constants", L"Push String...", L"Push Number...", L"Push Minimum Num Length...",
				L"Variables", L"Push Index", L"Push OriginFileName",
				L"Operators", L"Add (+)", L"Sub (-)", L"Mul (*)", L"Div (/)",
				L"Brackets", L"Left Bracket (", L"Right Bracket )",
				L"Delete Last", L"Clear Expression",
				L"Language", L"Exit", L"Help",
				L"Selected Files", L"Expression Preview", L"Input", L"File Path"
			}
		},
		LanguageDef {
			ID_LANG_ZH, L"中文(简体)",
			{
				L"文件", L"表达式", L"选项",
				L"打开", L"清空", L"应用重命名", L"自动匹配字幕名",
				L"常量", L"添加字符串...", L"添加数字...", L"添加最小数字格式...",
				L"变量", L"添加序号", L"添加原始文件名",
				L"运算符", L"加 (+)", L"减 (-)", L"乘 (*)", L"除 (/)",
				L"括号", L"左括号 (", L"右括号 )",
				L"删除上一个", L"清空表达式",
				L"语言", L"退出", L"帮助",
				L"已选文件", L"表达式预览", L"输入框", L"文件路径"
			}
		},
		LanguageDef {
			ID_LANG_ZH_TW, L"中文(繁體)",
			{
				L"檔案", L"運算式", L"選項",
				L"開啟", L"清空", L"套用重新命名", L"自動配對字幕名",
				L"常數", L"加入字串...", L"加入數字...", L"加入最小數字格式...",
				L"變數", L"加入序號", L"加入原始檔名",
				L"運算子", L"加 (+)", L"減 (-)", L"乘 (*)", L"除 (/)",
				L"括號", L"左括號 (", L"右括號 )",
				L"刪除上一個", L"清空運算式",
				L"語言", L"退出", L"幫助",
				L"已選檔案", L"運算式預覽", L"輸入框", L"檔案路徑"
			}
		},
		LanguageDef {
			ID_LANG_JA, L"日本語",
			{
				L"ファイル", L"式", L"オプション",
				L"開く", L"クリア", L"名前変更を適用", L"字幕を自動マッチ",
				L"定数", L"文字列を追加...", L"数値を追加...", L"最小数値形式を追加...",
				L"変数", L"連番を追加", L"元のファイル名を追加",
				L"演算子", L"加算 (+)", L"減算 (-)", L"乗算 (*)", L"除算 (/)",
				L"括弧", L"左括弧 (", L"右括弧 )",
				L"最後を削除", L"式をクリア",
				L"言語", L"終了", L"ヘルプ",
				L"選択されたファイル", L"式のプレビュー", L"入力", L"ファイルパス"
			}
		},
		LanguageDef {
			ID_LANG_RU, L"Русский",
			{
				L"Файл", L"Выражение", L"Настройки",
				L"Открыть", L"Очистить", L"Применить", L"Авто-подбор субтитров",
				L"Константы", L"Добавить строку...", L"Добавить число...", L"Добавить мин. длину числа...",
				L"Переменные", L"Добавить индекс", L"Добавить исх. имя файла",
				L"Операторы", L"Сложение (+)", L"Вычитание (-)", L"Умножение (*)", L"Деление (/)",
				L"Скобки", L"Левая скобка (", L"Правая скобка )",
				L"Удалить последнее", L"Очистить выражение",
				L"Язык", L"Выход", L"Помощь",
				L"Выбранные файлы", L"Предпросмотр выражения", L"Ввод", L"Путь к файлу"
			}
		}
	};

	inline size_t current_lang_index = 0;

	inline const UIStrings& GetStrings() {
		return supported_languages[current_lang_index].strings;
	}

	inline std::wstring GetHelpText() {
		switch (current_lang_index) {
			case 0: // English
				return L"How to Use\n"
					L"1. Open Files: Click 'File -> Open' to add files.\n"
					L"2. Build Expression: Use 'Expression' menu or input box to build rules. Mix strings, numbers, and operators.\n"
					L"   - Minimum Num Length: Pads number with zeros (e.g. 1 * Format(3) = '001').\n"
					L"   - Use operators (+, -, *, /) and brackets to link values.\n"
					L"3. Submit Rename: Click 'File -> Submit Rename'.\n\n"
					L"Auto Match Subtitles:\n"
					L"Load an equal number of video and subtitle files, then click 'File -> Auto Match Subtitles'.";
			case 1: // Simplified Chinese
				return L"使用说明\n"
					L"1. 打开文件：点击“文件 -> 打开”添加要重命名的文件。\n"
					L"2. 构建表达式：使用“表达式”菜单或下方输入框来构建规则。\n"
					L"   - 最小数字格式：限制数字的最小长度并自动补零（必须配合乘号使用，例如 1 * 最小数字格式(3) = 001）。\n"
					L"   - 利用加减乘除运算符和括号组合变量及常量。\n"
					L"3. 应用重命名：点击“文件 -> 应用重命名”。\n\n"
					L"自动匹配字幕名：\n"
					L"载入数量严格相等的视频文件与字幕文件，然后点击“文件 -> 自动匹配字幕名”。";
			case 2: // Traditional Chinese
				return L"使用說明\n"
					L"1. 開啟檔案：點擊「檔案 -> 開啟」加入要重新命名的檔案。\n"
					L"2. 建立運算式：使用「運算式」選單或下方輸入框來建立規則。\n"
					L"   - 最小數字格式：限制數字的最小長度並自動補零（必須配合乘號使用，例如 1 * 最小數字格式(3) = 001）。\n"
					L"   - 利用加減乘除運算子和括號組合變數及常數。\n"
					L"3. 套用重新命名：點擊「檔案 -> 套用重新命名」。\n\n"
					L"自動配對字幕名：\n"
					L"載入數量嚴格相等的影片檔案與字幕檔案，然後點擊「檔案 -> 自動配對字幕名」。";
			case 3: // Japanese
				return L"使い方\n"
					L"1. ファイルを開く：「ファイル -> 開く」をクリックしてファイルを追加します。\n"
					L"2. 式を作成：「式」メニューまたは入力ボックスを使用してルールを作成します。\n"
					L"   - 最小数値形式：ゼロ埋めします（例：1 * 形式(3) = '001'。＊乗算記号が必須）。\n"
					L"   - 演算子 (+, -, *, /) と括弧を使用して値をつなぎます。\n"
					L"3. 名前変更を適用：「ファイル -> 名前変更を適用」をクリックします。\n\n"
					L"字幕を自動マッチ：\n"
					L"動画ファイルと同数の字幕ファイルを読み込み、「ファイル -> 字幕を自動マッチ」をクリックします。";
			case 4: // Russian
				return L"Как использовать\n"
					L"1. Открыть файлы: Нажмите «Файл -> Открыть», чтобы добавить файлы.\n"
					L"2. Создать выражение: Используйте меню «Выражение» или поле ввода для создания правил.\n"
					L"   - Мин. длина числа: Добавляет нули (например, 1 * Формат(3) = '001').\n"
					L"   - Используйте операторы (+, -, *, /) и скобки для соединения значений.\n"
					L"3. Применить: Нажмите «Файл -> Применить».\n\n"
					L"Авто-подбор субтитров:\n"
					L"Загрузите равное количество видео и субтитров, затем нажмите «Файл -> Авто-подбор субтитров».";
			default:
				return L"";
		}
	}

}

#endif // !_UI_LANGUAGE_HPP_
