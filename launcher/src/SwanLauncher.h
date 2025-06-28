#pragma once

#include <atomic>
#include <wx/wx.h>
#include <thread>

class MainWindow;

class SwanLauncher: public wxApp {
public:
	void launch(std::string world);

private:
	bool OnInit() override;
	void OnSwanClosed();

	bool isRunning_ = false;
	MainWindow *mainWindow_;
};
