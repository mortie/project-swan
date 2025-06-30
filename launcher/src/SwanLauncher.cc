#include "SwanLauncher.h"
#include "MainWindow.h"
#include "src/system.h"
#include "worlds.h"
#include <cstdlib>
#include <ctime>

wxDEFINE_EVENT(EVT_SWAN_CLOSED, wxEvent);

void SwanLauncher::launch(std::string world)
{
	if (isRunning_) {
		return;
	}

	isRunning_ = true;
	mainWindow_->disable();

	std::thread([&, world] {
		std::string cmd = "./bin/swan";
		appendArg(cmd, "--mod");
		appendArg(cmd, "core.mod");
		appendArg(cmd, "--world");
		appendArg(cmd, worldPath(world));
		std::cerr << "Running command: " << cmd << '\n';
		runCommand(cmd.c_str());

		wxQueueEvent(this, new wxThreadEvent(EVT_SWAN_CLOSED));
	}).detach();
}

void SwanLauncher::OnSwanClosed()
{
	isRunning_ = false;
	mainWindow_->reload();
	mainWindow_->enable();
}

bool SwanLauncher::OnInit()
{
	// We need rand() later
	srand(time(nullptr));

	Bind(EVT_SWAN_CLOSED, [&](auto &evt) { OnSwanClosed(); });

	mainWindow_ = new MainWindow(this);
	mainWindow_->Show(true);
	return true;
}
