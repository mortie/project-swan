#include "SwanLauncher.h"
#include "MainWindow.h"
#include "src/system.h"
#include "worlds.h"
#include <cstdlib>
#include <ctime>

wxDEFINE_EVENT(EVT_SWAN_CLOSED, wxEvent);

void SwanLauncher::launch(std::string id)
{
	if (isRunning_) {
		return;
	}

	isRunning_ = true;
	mainWindow_->disable();

	updateWorldLastPlayedTime(id);
	std::thread([&, id] {
		std::string cmd = "./bin/swan";
		appendArg(cmd, "--mod");
		appendArg(cmd, "core.mod");
		appendArg(cmd, "--world");
		appendArg(cmd, worldPath(id));
		appendArg(cmd, "--thumbnail");
		appendArg(cmd, thumbnailPath(id));
		std::cerr << "Running command: " << cmd << '\n';
		runCommand(cmd.c_str());

		wxQueueEvent(this, new wxThreadEvent(EVT_SWAN_CLOSED));
	}).detach();
}

void SwanLauncher::OnSwanClosed()
{
	isRunning_ = false;
	mainWindow_->onSwanClosed();
	mainWindow_->reload();
	mainWindow_->enable();
}

bool SwanLauncher::OnInit()
{
	// We need rand() later
	srand(time(nullptr));

	// Same with image loading
	wxInitAllImageHandlers();

	Bind(EVT_SWAN_CLOSED, [&](auto &evt) { OnSwanClosed(); });

	mainWindow_ = new MainWindow(this);
	mainWindow_->Show(true);
	mainWindow_->SetSize(550, 350);
	mainWindow_->SetMinSize({520, 300});
	return true;
}
