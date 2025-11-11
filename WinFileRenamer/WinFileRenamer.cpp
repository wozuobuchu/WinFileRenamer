#include "update_main.hpp"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

	update_main_ui_LOOP(ui::register_main_ui(hInstance, hPrevInstance, lpCmdLine, nCmdShow));


	return 0;
}