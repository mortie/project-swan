#pragma once

#include <optional>
#include <cstdint>
#include <wx/wx.h>

class MainWindow;

class SwanLauncher: public wxApp {
public:
	void launch(std::string world, std::optional<uint32_t> seed);

private:
	bool OnInit() override;
	void OnSwanClosed();

	bool isRunning_ = false;
	MainWindow *mainWindow_;
};
