#include "overlay.h"

int main() {
	if (!overlay::initialize(L"Notepad", L"Untitled - Notepad"))
		return -1;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		if (!overlay::scale())
			break;

		auto shown = GetKeyState(VK_INSERT);
		overlay::click_through(!shown);

		window::new_frame();

		ImGui::Begin("whats up");

		ImGui::End();
		
		window::draw();
	}

	window::cleanup();

	return 0;
}